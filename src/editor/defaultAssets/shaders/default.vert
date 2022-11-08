#version 450
#include "vertIO.glsl"

void main() {
    fragPosition = (OBJECT_MATRIX * vec4(vertPosition, 1)).xyz;
    mat4 rotation_matrix = OBJECT_MATRIX;
    rotation_matrix[3][0] = 0;
    rotation_matrix[3][1] = 0;
    rotation_matrix[3][2] = 0;
    fragNormal = normalize((rotation_matrix * vec4(vertNormal, 1)).xyz);
    gl_Position = CAMERA_MATRIX * OBJECT_MATRIX * vec4(vertPosition, 1.0);
}
