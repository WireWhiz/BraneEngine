#ifndef VERT_IO
#define VERT_IO

layout(push_constant) uniform PushConstant{
    uint instanceOffset;
} pc;

layout(binding = 0) uniform RenderInfo {
    mat4 cameraMatrix;
} ri;

layout(binding = 1) buffer InstanceInfo {
    mat4 objectMatrix[];
} instances;

layout(location = 0) in vec3 vertPosition;
layout(location = 1) in vec3 vertNormal;

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec3 fragNormal;

#define INSTANCE pc.instanceOffset + gl_InstanceIndex
#define OBJECT_MATRIX instances.objectMatrix[INSTANCE]
#define CAMERA_MATRIX ri.cameraMatrix

#endif