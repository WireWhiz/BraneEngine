#version 450
#include "vertIO.glsl"

void main() {
    fragPosition = (OBJECT_MATRIX * vec4(POSITION, 1)).xyz;
    fragNormal = normalize((OBJECT_MATRIX * vec4(NORMAL, 0)).xyz);
    gl_Position = CAMERA_MATRIX * OBJECT_MATRIX * vec4(POSITION, 1.0);
}
