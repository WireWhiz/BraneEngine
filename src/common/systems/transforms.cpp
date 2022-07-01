//
// Created by eli on 6/29/2022.
//

#include "transforms.h"

#include <glm/glm.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

void Transforms::start()
{
	_em = Runtime::getModule<EntityManager>();
}

void Transforms::setParent(EntityID entity, EntityID parent)
{
	if(!_em->entityHasComponent(entity, LocalTransform::def()->id))
		_em->addComponent(entity, LocalTransform::def()->id);
	VirtualComponent localTransform = _em->getEntityComponent(entity, LocalTransform::def()->id);
	localTransform.setVar(1, parent);
	_em->setEntityComponent(entity, localTransform);

	if(!_em->entityHasComponent(parent, Children::def()->id))
		_em->addComponent(entity, Children::def()->id);

	VirtualComponent children = _em->getEntityComponent(parent, Children::def()->id);
	children.getVar<inlineUIntArray>(0)->push_back(entity);
	_em->setEntityComponent(parent, children);
}

void Transforms::removeParent(EntityID entity)
{
	EntityID parent = *_em->getEntityComponent(entity, LocalTransform::def()->id).getVar<EntityID>(1);
	_em->removeComponent(entity, LocalTransform::def()->id);

	VirtualComponent children = _em->getEntityComponent(parent, Children::def()->id);
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
		_em->setEntityComponent(parent, children);
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
	_em->setEntityComponent(entity, trs.toVirtual());
}

void Transforms::updateChildTransforms(EntityID entity, glm::mat4 parentTransform)
{
	VirtualComponent children = _em->getEntityComponent(entity, Children::def()->id);
	for(EntityID child : *children.getVar<inlineUIntArray>(0)){
		VirtualComponent localTransform = _em->getEntityComponent(child, LocalTransform::def()->id);
		Transform worldTransform{};
		worldTransform.value = parentTransform * *localTransform.getVar<glm::mat4>(0);
		_em->setEntityComponent(child, worldTransform.toVirtual());

		if(_em->entityHasComponent(child, Children::def()->id))
			updateChildTransforms(entity, worldTransform.value);
	}
}

void Transforms::setGlobalTransform(EntityID entity, glm::mat4 value)
{
	Transform t{};
	t.value = value;
	_em->setEntityComponent(entity, t.toVirtual());

	if(_em->entityHasComponent(entity, LocalTransform::def()->id))
	{
		VirtualComponent localTransform = _em->getEntityComponent(entity, LocalTransform::def()->id);
		VirtualComponent parentTransform = _em->getEntityComponent(*localTransform.getVar<EntityID>(1), Transform::def()->id);
		glm::mat4 localTransformMatrix = glm::inverse(*parentTransform.getVar<glm::mat4>(0)) * value;
		localTransform.setVar(0, localTransformMatrix);

		if(_em->entityHasComponent(entity, TRS::def()->id))
			updateTRSFromMatrix(entity, localTransformMatrix);
	}
	else if(_em->entityHasComponent(entity, TRS::def()->id))
		updateTRSFromMatrix(entity, value);

	if(_em->entityHasComponent(entity, Children::def()->id))
		updateChildTransforms(entity, value);
}

void Transforms::setLocalTransform(EntityID entity, glm::mat4 value)
{
	if(_em->entityHasComponent(entity, LocalTransform::def()->id))
	{
		VirtualComponent localTransform = _em->getEntityComponent(entity, LocalTransform::def()->id);
		localTransform.setVar(0, value);
		VirtualComponent parentTransform = _em->getEntityComponent(entity, Transform::def()->id);
		Transform worldTransform{};
		worldTransform.value = *parentTransform.getVar<glm::mat4>(0) * value;
		_em->setEntityComponent(entity, worldTransform.toVirtual());
	}
	else
	{
		VirtualComponent transform = _em->getEntityComponent(entity, Transform::def()->id);
		transform.setVar(0, value);
	}
	if(_em->entityHasComponent(entity, TRS::def()->id))
		updateTRSFromMatrix(entity, value);

	if(_em->entityHasComponent(entity, Children::def()->id))
		updateChildTransforms(entity, value);
}

void Transforms::setTRS(EntityID entity, glm::vec3 translation, glm::quat rotation, glm::vec3 scale)
{
	TRS trs;
	trs.translation = translation;
	trs.rotation = rotation;
	trs.scale = scale;
	_em->setEntityComponent(entity, trs.toVirtual());
	glm::mat4 trsMatrix = glm::translate(glm::mat4_cast(rotation) * glm::scale(glm::mat4(1), scale), translation);
	if(_em->entityHasComponent(entity, LocalTransform::def()->id))
	{
		VirtualComponent localTransform = _em->getEntityComponent(entity, LocalTransform::def()->id);
		localTransform.setVar(0, trsMatrix);
		VirtualComponent parentTransform = _em->getEntityComponent(entity, Transform::def()->id);
		Transform transform{};
		transform.value = *parentTransform.getVar<glm::mat4>(0) * trsMatrix;
		_em->setEntityComponent(entity, transform.toVirtual());
	}
}

const char* Transforms::name()
{
	return "transforms";
}


