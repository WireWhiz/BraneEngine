//
// Created by eli on 6/29/2022.
//

#ifndef BRANEENGINE_TRANSFORMS_H
#define BRANEENGINE_TRANSFORMS_H

#include <runtime/module.h>
#include <ecs/core/component.h>
#include <ecs/core/entity.h>

class Transform : public NativeComponent<Transform>
{
	REGISTER_MEMBERS_1("Transform", value);
public:
	glm::mat4 value;
};

class LocalTransform : public NativeComponent<LocalTransform>
{
	REGISTER_MEMBERS_2("Local Transform", value, parent);
public:
	glm::mat4 value;
	EntityID parent;
};

class TRS : public NativeComponent<TRS>
{
	REGISTER_MEMBERS_3("TRS", translation, rotation, scale);
public:
	glm::vec3 translation;//local translation
	glm::quat rotation;   //local rotation
	glm::vec3 scale;      //local scale
	bool dirty;
};

class Children : public NativeComponent<Children>
{
	REGISTER_MEMBERS_1("Children", children);
public:
	inlineUIntArray children;
};

class Transforms : public Module
{
	EntityManager* _em;
	void updateTRSFromMatrix(EntityID entity, glm::mat4 value);
	void updateChildTransforms(EntityID entity, glm::mat4 parentTransform);
public:
	void setParent(EntityID entity, EntityID parent);
	void removeParent(EntityID entity);
	void setGlobalTransform(EntityID entity, glm::mat4 value);
	void setLocalTransform(EntityID entity, glm::mat4 value);
	void setTRS(EntityID entity, glm::vec3 translation, glm::quat rotation, glm::vec3 scale);
	void start() override;

	static const char* name();
};


#endif //BRANEENGINE_TRANSFORMS_H
