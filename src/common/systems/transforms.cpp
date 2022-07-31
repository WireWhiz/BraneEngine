//
// Created by eli on 6/29/2022.
//

#include "transforms.h"

#include <glm/glm.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

glm::mat4 TRS::toMat() const
{
	return glm::translate(glm::mat4(1), translation) * glm::mat4_cast(rotation) * glm::scale(glm::mat4(1), scale);
}

void Transforms::start()
{
	_em = Runtime::getModule<EntityManager>();
	_em->systems().addSystem("transform", std::make_unique<TransformSystem>());
}

void Transforms::setParent(EntityID entity, EntityID parent, EntityManager& em)
{
	if(!em.hasComponent<LocalTransform>(entity))
		em.addComponent<LocalTransform>(entity);
	auto* localTransform = em.getComponent<LocalTransform>(entity);
	localTransform->parent = parent;

	if(!em.hasComponent<Children>(parent))
		em.addComponent<Children>(parent);

	Children* children = em.getComponent<Children>(parent);
	children->children.push_back(entity);
}

void Transforms::removeParent(EntityID entity)
{
	EntityID parent = *_em->getComponent(entity, LocalTransform::def()->id).getVar<EntityID>(1);
	_em->removeComponent(entity, LocalTransform::def()->id);

	VirtualComponent children = _em->getComponent(parent, Children::def()->id);
	auto* carray = children.getVar<inlineUIntArray>(0);
	if(carray->size() > 1)
	{
		for(size_t i = 0; i < carray->size(); ++i)
		{
			if((*carray)[i] == entity)
			{
				carray->erase(entity);
				break;
			}
		}
		_em->setComponent(parent, children);
	}
	else
		_em->removeComponent(parent, Children::def()->id);

}

void Transforms::updateTRSFromMatrix(EntityID entity, glm::mat4 value)
{
	TRS trs{};
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::decompose(value, trs.scale, trs.rotation, trs.translation, skew, perspective);
	_em->setComponent(entity, trs.toVirtual());
}


const char* Transforms::name()
{
	return "transforms";
}

void Transforms::destroyRecursive(EntityID entity, bool updateParentChildren)
{
    assert(_em->entityExists(entity));
	if(_em->hasComponent<Children>(entity))
	{   auto* cc = _em->getComponent<Children>(entity);
		for(auto child : cc->children)
		{
			destroyRecursive(child, false);
		}
	}
	if(updateParentChildren && _em->hasComponent(entity, LocalTransform::def()->id)){
		VirtualComponent lt = _em->getComponent(entity, LocalTransform::def()->id);
		VirtualComponent cc = _em->getComponent(*lt.getVar<EntityID>(1), Children::def()->id);
		inlineUIntArray& children = *cc.getVar<inlineUIntArray>(0);
		for (int i = 0; i < children.size(); ++i)
		{
			if(children[i] == entity)
			{
				children.erase(i);
				_em->setComponent(*lt.getVar<EntityID>(1), cc);
				break;
			}
		}
	}
	_em->destroyEntity(entity);
}

glm::mat4 Transforms::getParentTransform(EntityID parent, EntityManager& em)
{
    if(!em.entityExists(parent))
        return glm::mat4(1);
	if(!em.hasComponent<LocalTransform>(parent))
		return em.getComponent<Transform>(parent)->value;
	auto* gt = em.getComponent<Transform>(parent);
	if(!gt->dirty)
		return gt->value;
	auto* lt = em.getComponent<LocalTransform>(parent);
	glm::mat4 pt = getParentTransform(lt->parent, em);
	gt->value = pt * lt->value;
	gt->dirty = false;
	return gt->value;
}


void TransformSystem::run(EntityManager& _em)
{
	//Update trs for TRS components on unparented entities
	ComponentFilter globalTRS(&_ctx);
	globalTRS.addComponent(TRS::def()->id, ComponentFilterFlags_Changed);
	globalTRS.addComponent(Transform::def()->id);
	globalTRS.addComponent(LocalTransform::def()->id, ComponentFilterFlags_Exclude);

	_em.getEntities(globalTRS).forEachNative([](byte** components){
		TRS* trs = TRS::fromVirtual(components[0]);
		Transform* t = Transform::fromVirtual(components[1]);
		t->value = trs->toMat();
        t->dirty = true;
	});

	//Update trs on parented entities
	ComponentFilter localTRS(&_ctx);
	localTRS.addComponent(TRS::def()->id, ComponentFilterFlags_Changed);
	localTRS.addComponent(LocalTransform::def()->id);
	localTRS.addComponent(Transform::def()->id);

	_em.getEntities(localTRS).forEachNative([](byte** components){
		TRS* trs = TRS::fromVirtual(components[0]);
		LocalTransform* t = LocalTransform::fromVirtual(components[1]);
		Transform* gt = Transform::fromVirtual(components[2]);
		t->value = trs->toMat();
		gt->dirty = true;
	});

	//Update transform for parented entities
	ComponentFilter localTransforms(&_ctx);
	localTransforms.addComponent(Transform::def()->id);
	localTransforms.addComponent(LocalTransform::def()->id, ComponentFilterFlags_Const);

	_em.getEntities(localTransforms).forEachNative([&_em](byte** components){
		auto* gt = Transform::fromVirtual(components[0]);
		auto* lt = LocalTransform::fromVirtual(components[1]);
		glm::mat4 pt = Transforms::getParentTransform(lt->parent, _em);

		gt->value = pt * lt->value;
		gt->dirty = false;
	});
}
