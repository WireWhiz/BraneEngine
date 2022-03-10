#version 450

layout( push_constant ) uniform meshData {
    mat4 render_matrix;
    mat4 objectToWorld;
    vec4 light;
} md;
//layout(binding = 1) uniform sampler2D texSampler;

//layout(location = 0) in vec3 fragColor;
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

layout(location = 0) out vec4 outColor;

void main() {
    float brightness = dot(normalize(md.light.xyz - position), normal);
    outColor = vec4(brightness, brightness, brightness, 1);
}