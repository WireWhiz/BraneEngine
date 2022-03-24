//
// Created by eli on 3/8/2022.
//

#ifndef BRANEENGINE_NATIVESYSTEMS_H
#define BRANEENGINE_NATIVESYSTEMS_H

#include "../core/virtualSystem.h"
#include "../core/entity.h"
#include "../../assets/assetID.h"
#include <unordered_set>
#include "../nativeTypes/transform.h"

namespace systems
{
	void updateTransform(EntityID id, std::unordered_set<EntityID>& updated, EntityManager& em)
	{
		if(em.entityHasComponent(id, LocalTransformComponent::def()))
		{
			VirtualComponent localTransform = em.getEntityComponent(id, LocalTransformComponent::def());
			EntityID parent = localTransform.readVar<EntityID>(1);
			if(!updated.count(parent))
				updateTransform(parent, updated, em);

			VirtualComponent parentTransform = em.getEntityComponent(parent, TransformComponent::def());

			TransformComponent transform{};
			transform.value =  parentTransform.readVar<glm::mat4>(0) * localTransform.readVar<glm::mat4>(0);
			em.setEntityComponent(id, transform.toVirtual());
		}

		updated.insert(id);
	}

	void addTransformSystem(EntityManager& em)
	{
		NativeForEach forEveryTransform( {EntityIDComponent::def(), TransformComponent::def()},&em);
		em.addSystem(std::make_unique<VirtualSystem>(AssetID("nativeSystem/0"), [forEveryTransform](EntityManager& em){
			std::unordered_set<EntityID> updated(em.forEachCount(forEveryTransform.id()));
			em.forEach(forEveryTransform.id(), [&](byte** components){
				EntityID id = EntityIDComponent::fromVirtual(components[forEveryTransform.getComponentIndex(0)])->id;
				if(!updated.count(id))
					updateTransform(id, updated, em);

			});
		}));

	}
}

#endif //BRANEENGINE_NATIVESYSTEMS_H
