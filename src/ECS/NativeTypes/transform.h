#pragma once
#include <core/Component.h>
#include <glm/mat4x4.hpp>

class Transform : public NativeComponent<Transform, 0>
{
public:
	virtual void getVariableIndicies(std::vector<NativeVarDef>& variables) override
	{
		variables.push_back({ offsetof(Transform, value), virtualFloat4x4 });
	}
	glm::mat4x4 value;
};