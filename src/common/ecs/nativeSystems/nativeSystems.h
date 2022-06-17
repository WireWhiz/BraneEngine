//
// Created by eli on 3/8/2022.
//

#ifndef BRANEENGINE_NATIVESYSTEMS_H
#define BRANEENGINE_NATIVESYSTEMS_H

#include "../core/entity.h"
#include "../../assets/assetID.h"
#include <unordered_set>
#include "../nativeTypes/transform.h"

namespace systems
{
	void updateTransform(EntityID id, std::unordered_set<EntityID>& updated, EntityManager& em)
	{
		if(em.entityHasComponent(id, LocalTransformComponent::def()->id))
		{
			VirtualComponent localTransform = em.getEntityComponent(id, LocalTransformComponent::def()->id);
			EntityID parent = localTransform.readVar<EntityID>(1);
			if(!updated.count(parent))
				updateTransform(parent, updated, em);

			VirtualComponent parentTransform = em.getEntityComponent(parent, TransformComponent::def()->id);

			TransformComponent transform{};
			transform.value =  parentTransform.readVar<glm::mat4>(0) * localTransform.readVar<glm::mat4>(0);
			em.setEntityComponent(id, transform.toVirtual());
		}

		updated.insert(id);
	}

	void addTransformSystem(EntityManager& em, Timeline& tl)
	{
		tl.addTask("transform", [&em](){
			std::unordered_set<EntityID> updated;
			em.forEach({EntityIDComponent::def()->id, TransformComponent::def()->id}, [&](byte** components){
				EntityID id = EntityIDComponent::fromVirtual(components[0])->id;
				if(!updated.count(id))
					updateTransform(id, updated, em);

			});
		}, "before main");

	}
}

#endif //BRANEENGINE_NATIVESYSTEMS_H
