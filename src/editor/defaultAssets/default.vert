#version 450

layout( push_constant ) uniform meshData {
    mat4 render_matrix;
    mat4 objectToWorld;
    vec4 light;
} md;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec3 outPoisiton;
layout(location = 1) out vec3 fragNormal;

void main() {
    gl_Position = md.render_matrix * vec4(inPosition, 1.0);
    outPoisiton = (md.objectToWorld * vec4(inPosition, 1.0)).xyz;
    fragNormal = normalize((md.objectToWorld * vec4(inNormal, 1.0)).xyz);
}