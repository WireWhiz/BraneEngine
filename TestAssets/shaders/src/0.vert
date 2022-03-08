#version 450

layout( push_constant ) uniform meshData {
    mat4 render_matrix;
} md;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec3 normal;

void main() {
    gl_Position = md.render_matrix * vec4(inPosition, 1.0);
    normal = inNormal;
}