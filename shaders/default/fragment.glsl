#version 410 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

// Texture uniforms
uniform sampler2D texture1;
uniform sampler2DArray CSMDepthMap;
uniform samplerCube omniDepthCubeMap;
uniform vec2 textureScale;

// Lighting uniforms
uniform vec3 lightDir;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform float cameraFar;
uniform float pointFar;
uniform float ambient;

// Fog uniforms
uniform vec3 fogColor;
uniform float fogStart;
uniform float fogEnd;

// Material uniforms
uniform float specular;
uniform float shininess;

// Editor uniforms
uniform bool isSelected;

uniform mat4 view;

layout (std140) uniform LightSpaceMatrices {
    mat4 lightSpaceMatrices[16];
};
uniform float cascadePlaneDistances[16];
uniform int cascadeCount;

vec3 gridSamplingDisk[20] = vec3[] (
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

float CSMCalculation(vec3 fragPos) {
    // Get cascade layer
    vec4 fragPosViewSpace = view * vec4(fragPos, 1.0);
    float depthValue = abs(fragPosViewSpace.z);

    int layer = -1;
    for (int i = 0; i < cascadeCount; i++) {
        if (depthValue < cascadePlaneDistances[i]) {
            layer = i;
            break;
        }
    }
    if (layer == -1) {
        layer = cascadeCount - 1;
    }

    vec4 fragPosLightSpace = lightSpaceMatrices[layer] * vec4(fragPos, 1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    float currentDepth = projCoords.z;

    if (currentDepth > 1.0) {
        return 0.0;
    }

    // Calculate bias
    vec3 normal = normalize(fs_in.Normal);
    float cosTheta = max(dot(normal, -lightDir), 0.0);
    float slopeScaledBias = 0.05 * (1.0 - cosTheta);
    float constantBias = 0.0005;
    float bias = max(slopeScaledBias, constantBias);

    // Scale by cascade distance
    const float biasModifier = 0.5;
    float scale = (layer == cascadeCount) ? cameraFar : cascadePlaneDistances[layer];
    scale = max(scale, 50); // Cap minimum distance to avoid huge bias in near cascades
    bias *= 1.0 / (scale * biasModifier);

    // Perform PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / vec2(textureSize(CSMDepthMap, 0));
    for(int x = -2; x <= 2; x++) {
        for(int y = -2; y <= 2; y++) {
            float pcfDepth = texture(CSMDepthMap, vec3(projCoords.xy + vec2(x, y) * texelSize, layer)).r;
            shadow += (currentDepth - bias) > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 25.0;
        
    return shadow;
}

float omniCalculation(vec3 fragPos) {
    vec3 fragToLight = fragPos - lightPos;
    float currentDepth = length(fragToLight);
    float shadow = 0.0;
    float bias = 0.05;
    int samples = 20;
    float viewDistance = length(viewPos - fragPos);
    float diskRadius = (1.0 + (viewDistance / pointFar)) / 100.0;
    for (int i = 0; i < samples; i++) {
        float closestDepth = texture(omniDepthCubeMap, fragToLight + gridSamplingDisk[i] * diskRadius).r;
        closestDepth *= pointFar;
        if (currentDepth - bias > closestDepth) {
            shadow += 1.0;
        }
    }
    shadow /= float(samples);
    return shadow;
}

void main() {           
    vec3 color = texture(texture1, fs_in.TexCoords * textureScale).rgb;
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightColor = vec3(0.5);
    vec3 ambientColor = ambient * lightColor;
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);

    // Directional light (CSM)
    float diffDir = max(dot(lightDir, normal), 0.0);
    vec3 diffuseDir = diffDir * lightColor;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float specDir = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    vec3 specularDir = specular * specDir * lightColor;
    float CSMShadow = CSMCalculation(fs_in.FragPos);                      
    vec3 directionalLighting = (ambientColor + (1.0 - CSMShadow) * (diffuseDir + specularDir)) * color;

    // Point light (Omni)
    vec3 lightToFrag = fs_in.FragPos - lightPos;
    vec3 lightDirOmni = normalize(lightToFrag);
    float diffOmni = max(dot(-lightDirOmni, normal), 0.0);
    vec3 diffuseOmni = diffOmni * lightColor;
    vec3 halfwayOmni = normalize(-lightDirOmni + viewDir);
    float specOmni = pow(max(dot(normal, halfwayOmni), 0.0), shininess);
    vec3 specularOmni = specular * specOmni * lightColor;
    float omniShadow = omniCalculation(fs_in.FragPos);
    vec3 pointLighting = (ambientColor + (1.0 - omniShadow) * (diffuseOmni + specularOmni)) * color;

    vec3 combinedLighting = directionalLighting + pointLighting;

    // Fog
    float distance = length(viewPos - fs_in.FragPos);
    float fogFactor = clamp((fogEnd - distance) / (fogEnd - fogStart), 0.0, 1.0);
    vec3 lighting = mix(fogColor, combinedLighting, fogFactor); 

    // Selection highlight
    if (isSelected) {
        lighting = mix(lighting, vec3(1.0, 1.0, 0.0), 0.25);
    }

    // Gamma correction
    float gamma = 2.2;
    vec3 gammaCorrected = pow(lighting, vec3(1.0 / gamma));
    FragColor = vec4(gammaCorrected, 1.0);
}