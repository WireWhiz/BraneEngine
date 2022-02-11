#pragma once
#include <ecs/core/Component.h>
#include <glm/glm.hpp>
#include "../core/entity.h"

namespace comps
{
	class TransformComponent : public NativeComponent<TransformComponent>
	{
		REGISTER_MEMBERS_1(value);
	public:
		glm::mat4 value;
	};

	class LocalTransformComponent : public NativeComponent<LocalTransformComponent>
	{
		REGISTER_MEMBERS_2(value, parent);
	public:
		glm::mat4 value;
		EntityID parent;
	};

	class ChildrenComponent : public NativeComponent<ChildrenComponent>
	{
		REGISTER_MEMBERS_1(children);
	public:
		std::vector<EntityID> children;
	};

}

