//
// Created by eli on 2/9/2022.
//

#pragma once
#include <ecs/core/Component.h>

namespace comps
{
	class MeshRendererComponent : public NativeComponent<MeshRendererComponent>
	{
		REGISTER_MEMBERS_2("Mesh Renderer", mesh, renderer);
	public:
		uint32_t mesh;
		uint32_t renderer;
	};

}

