//
// Created by eli on 2/9/2022.
//

#pragma once
#include <ecs/core/Component.h>

namespace comps
{
	class MeshRendererComponent : public NativeComponent<MeshRendererComponent>
	{
		REGISTER_MEMBERS_1(mesh);
	public:
		uint32_t mesh;
	};

}

