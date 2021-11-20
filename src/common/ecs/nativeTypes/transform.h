#pragma once
#include <ecs/core/Component.h>
#include <glm/mat4x4.hpp>

class Transform : public NativeComponent<Transform>
{
	REGISTER_MEMBERS_1(value);
public:
	glm::mat4x4 value;
};