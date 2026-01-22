#version 410 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

#define MAX_POINT_LIGHTS 16

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    float near;
    float far;
    float constant;
    float linear;
    float quadratic;
};

struct DirectionalLight {
    vec3 direction;
    vec3 color;
    float intensity;
};

// Texture uniforms
uniform sampler2D diffuse1;
uniform sampler2DArray depthMap;
uniform samplerCube depthCubemap[MAX_POINT_LIGHTS];
uniform vec2 textureScale;

// Lighting uniforms
uniform int numPointLights;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform DirectionalLight directionalLight;
uniform vec3 viewPos;
uniform float cameraFar;
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
uniform bool drawDirectionalShadows;
uniform bool drawPointShadows;

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

float directionalCalculation(vec3 fragPos, DirectionalLight light) {
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
    float cosTheta = max(dot(normal, -light.direction), 0.0);
    float slopeBiasFactor = 0.275;
    float constantBias = 0.0005;
    float slopeScaledBias = slopeBiasFactor * (1.0 - cosTheta);
    float bias = max(slopeScaledBias, constantBias);

    // Scale by cascade distance
    const float biasModifier = 0.7;
    float scale = (layer == cascadeCount) ? cameraFar : cascadePlaneDistances[layer];
    scale = max(scale, 50); // Cap minimum distance to avoid huge bias in near cascades
    bias *= 1.0 / (scale * biasModifier);

    // Perform PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / vec2(textureSize(depthMap, 0));
    for(int x = -2; x <= 2; x++) {
        for(int y = -2; y <= 2; y++) {
            float pcfDepth = texture(depthMap, vec3(projCoords.xy + vec2(x, y) * texelSize, layer)).r;
            shadow += (currentDepth - bias) > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 25.0;
        
    return shadow;
}

float pointCalculation(vec3 fragPos, PointLight light, int index) {
    vec3 fragToLight = fragPos - light.position;
    float currentDepth = length(fragToLight);
    float shadow = 0.0;
    float bias = 0.25;
    int samples = 20;
    float viewDistance = length(viewPos - fragPos);
    float diskRadius = (1.0 + (viewDistance / light.far)) / 100.0;

    for (int i = 0; i < samples; i++) {
        float closestDepth = texture(depthCubemap[index], fragToLight + gridSamplingDisk[i] * diskRadius).r;
        closestDepth *= light.far;
        if (currentDepth - bias > closestDepth) {
            shadow += 1.0;
        }
    }

    shadow /= float(samples);
    return shadow;
}

void main() {         
    // Sample diffuse  
    vec3 color = texture(diffuse1, fs_in.TexCoords * textureScale).rgb;

    // Get normal
    vec3 normal = normalize(fs_in.Normal);

    // Directional light (CSM)
    vec3 viewDir  = normalize(viewPos - fs_in.FragPos);
    vec3 lightDir = normalize(-directionalLight.direction);
    vec3 lightColor = directionalLight.color * directionalLight.intensity;
    float diffDir = max(dot(lightDir, normal), 0.0);
    vec3 diffuseDir = diffDir * lightColor;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float specDir = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    vec3 specularDir = specular * specDir * lightColor;
    float CSMShadow = 0.0;
    if (drawDirectionalShadows) {
        CSMShadow = directionalCalculation(fs_in.FragPos, directionalLight);
    }
    vec3 directionalLighting = (ambient + (1.0 - CSMShadow) * (diffuseDir + specularDir)) * color;


    // Point light (Omni)
    vec3 pointLighting = vec3(0.0);
    for (int i = 0; i < numPointLights; i++) {
        PointLight light = pointLights[i];
        vec3 fragToLight = fs_in.FragPos - light.position;
        float distance = length(fragToLight);
        vec3 lightDirOmni = normalize(fragToLight);

        float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

        float diff = max(dot(-lightDirOmni, normal), 0.0);
        vec3 diffuse = diff * light.color * light.intensity;

        vec3 halfway = normalize(-lightDirOmni + viewDir);
        float spec = pow(max(dot(normal, halfway), 0.0), shininess);
        vec3 specularComponent = specular * spec * light.color * light.intensity;

        float shadow = 0.0;
        if (drawPointShadows) {
            shadow = pointCalculation(fs_in.FragPos, light, i);
        }

        vec3 lightContribution = (ambient + (1.0 - shadow) * (diffuse + specularComponent)) * color;
        pointLighting += attenuation * lightContribution;
    }

    vec3 combinedLighting = directionalLighting + pointLighting;

    // Fog
    float distance = length(viewPos - fs_in.FragPos);
    float fogFactor = clamp((fogEnd - distance) / (fogEnd - fogStart), 0.0, 1.0);
    vec3 lighting = mix(fogColor, combinedLighting, fogFactor); 

    // Selection highlight
    if (isSelected) {
        lighting = mix(lighting, vec3(1.0, 1.0, 0.0), 0.15);
    }

    // Tonemapping
    vec3 tonemapped = lighting / (lighting + vec3(1.0));

    // Gamma correction
    float gamma = 2.2;
    vec3 gammaCorrected = pow(tonemapped, vec3(1.0 / gamma));
    FragColor = vec4(gammaCorrected, 1.0);
}