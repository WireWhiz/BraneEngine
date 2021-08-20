#version 450

layout( push_constant ) uniform meshData {
    mat4 render_matrix;
} md;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragUV;

void main() {
    gl_Position = md.render_matrix * vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
    fragUV = inUV;
}