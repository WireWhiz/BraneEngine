#ifndef VERT_IO
#define VERT_IO

layout(push_constant) uniform PushConstant { uint instanceOffset; }
pc;

layout(binding = 0) uniform RenderInfo
{
  mat4 cameraMatrix;
  vec3 cameraPos;
}
ri;

layout(binding = 1) buffer InstanceInfo { mat4 objectMatrix[]; }
instances;

layout(location = 0) in vec3 POSITION;
layout(location = 1) in vec3 NORMAL;

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec3 fragNormal;

#define INSTANCE pc.instanceOffset + gl_InstanceIndex
#define OBJECT_MATRIX instances.objectMatrix[INSTANCE]
#define CAMERA_MATRIX ri.cameraMatrix
#define CAMERA_POSITION ri.cameraPos

#endif