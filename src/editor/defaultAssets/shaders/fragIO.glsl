#ifndef FRAG_IO
#define FRAG_IO

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNormal;

layout(location = 0) out vec4 fragColor;

#define MATERIAL_PROPERTIES layout(binding = 3) uniform MaterialProperties

#endif