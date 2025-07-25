#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec3 Color;
in vec2 TexCoords;

out vec4 FragColor;

uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform sampler2D texture1;

// Fog uniforms
uniform vec3 fogColor;
uniform float fogStart;
uniform float fogEnd;

void main() {
    vec3 norm = normalize(Normal);
    vec3 lightDirNorm = normalize(-lightDir);
    vec3 viewDir = normalize(viewPos - FragPos);

    // Ambient
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse
    float diff = max(dot(norm, lightDirNorm), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular (Blinn-Phong)
    float specularStrength = 0.5;
    vec3 halfwayDir = normalize(lightDirNorm + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0); // shininess = 32
    vec3 specular = specularStrength * spec * lightColor;

    vec3 lighting = ambient + diffuse + specular;

    // Sample the texture color
    vec3 texColor = texture(texture1, TexCoords).rgb;

    // Combine lighting with texture (ignore Color from vertex)
    vec3 result = lighting * texColor;

    // Apply lighting to vertex color
    //vec3 result = lighting * Color;

    // Gamma correction (assuming gamma = 2.2)
    result = pow(result, vec3(1.0 / 2.2));

    // Fog calculation
    float distance = length(viewPos - FragPos);
    float fogFactor = clamp((fogEnd - distance) / (fogEnd - fogStart), 0.0, 1.0);
    vec3 finalColor = mix(fogColor, result, fogFactor);

    FragColor = vec4(finalColor, 1.0);
}
