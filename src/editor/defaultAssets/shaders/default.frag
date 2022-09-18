#version 450

#include "fragIO.glsl"
#include "lighting.glsl"

void main() {
    vec4 color = vec4(1,1,1,1);
    color = shadeDiffuse(color, fragPosition, fragNormal);
    fragColor = color;
}