#version 450

#include "fragIO.glsl"
#include "lighting.glsl"

MATERIAL_PROPERTIES {
    vec3 color;
} mp;

void main() {
    vec3 color = shadeDiffuse(mp.color, fragPosition, fragNormal);
    fragColor = vec4(color, 1);
}