#version 450

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 normal;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(normal, 1);
}