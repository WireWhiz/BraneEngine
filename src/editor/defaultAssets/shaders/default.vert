#version 450
#include "vertIO.glsl"

void main() {
    fragPosition = (OBJECT_MATRIX *  vec4(vertPosition, 1)).xyz;
    fragNormal = normalize((OBJECT_MATRIX * vec4(vertNormal, 1)).xyz);
    gl_Position = CAMERA_MATRIX * OBJECT_MATRIX * vec4(vertPosition, 1.0);
}