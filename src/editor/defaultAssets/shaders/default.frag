#version 450

#include "fragIO.glsl"
#include "lighting.glsl"

void main() {
    vec3 color = vec3(1);
    color = shadeDiffuse(color, fragPosition, fragNormal);
    fragColor = vec4(color, 1);
}