#version 410 core
in vec4 FragPos;

uniform vec3 lightPos;
uniform float far;

void main() {
    float lightDistance = length(FragPos.xyz - lightPos);
    lightDistance = lightDistance / far;
    gl_FragDepth = lightDistance;
}