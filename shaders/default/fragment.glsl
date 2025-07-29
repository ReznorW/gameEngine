#version 410 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

// Texture uniforms
uniform sampler2D texture1;
uniform sampler2DArray shadowMap;
uniform vec2 textureScale;

// Lighting uniforms
uniform vec3 lightDir;
uniform vec3 viewPos;
uniform float far;
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

float ShadowCalculation(vec3 fragPosWorldSpace) {
    // Get cascade layer
    vec4 fragPosViewSpace = view * vec4(fragPosWorldSpace, 1.0);
    float depthValue = abs(fragPosViewSpace.z);

    int layer = -1;
    for (int i = 0; i < cascadeCount; ++i) {
        if (depthValue < cascadePlaneDistances[i]) {
            layer = i;
            break;
        }
    }
    if (layer == -1) {
        layer = cascadeCount;
    }

    vec4 fragPosLightSpace = lightSpaceMatrices[layer] * vec4(fragPosWorldSpace, 1.0);
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
    float scale = (layer == cascadeCount) ? far : cascadePlaneDistances[layer];
    scale = max(scale, 50); // Cap minimum distance to avoid huge bias in near cascades
    bias *= 1.0 / (scale * biasModifier);

    // Perform PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));
    for(int x = -1; x <= 1; x++) {
        for(int y = -1; y <= 1; y++) {
            float pcfDepth = texture(shadowMap, vec3(projCoords.xy + vec2(x, y) * texelSize, layer)).r;
            shadow += (currentDepth - bias) > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
        
    return shadow;
}

void main()
{           
    // Sample texture
    vec3 color = texture(texture1, fs_in.TexCoords * textureScale).rgb;
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightColor = vec3(0.5);

    // Ambient calculation
    vec3 ambientColor = ambient * lightColor;

    // Diffuse calculation
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular calculation
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    vec3 specularColor = specular * spec * lightColor;  

    // Shadow calculation
    float shadow = ShadowCalculation(fs_in.FragPos);                      
    vec3 shadowColor = (ambientColor + (1.0 - shadow) * (diffuse + specularColor)) * color;

    // Fog calculation
    float distance = length(viewPos - fs_in.FragPos);
    float fogFactor = clamp((fogEnd - distance) / (fogEnd - fogStart), 0.0, 1.0);
    vec3 lighting = mix(fogColor, shadowColor, fogFactor); 

    // Selection highlight
    if (isSelected) {
        lighting = mix(lighting, vec3(1.0, 1.0, 0.0), 0.25);
    }
    
    // Apply gamma
    float gamma = 2.2;
    vec3 gammaCorrected = pow(lighting, vec3(1.0 / gamma));

    FragColor = vec4(gammaCorrected, 1.0);
}