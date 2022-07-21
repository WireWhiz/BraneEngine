//
// Created by eli on 6/29/2022.
//

#ifndef BRANEENGINE_TRANSFORMS_H
#define BRANEENGINE_TRANSFORMS_H

#include <runtime/module.h>
#include <ecs/core/entity.h>

class Transform : public NativeComponent<Transform>
{
	REGISTER_MEMBERS_2("Transform", value, "value", dirty, "dirty");
public:
	glm::mat4 value;
	bool dirty = true;
};

class LocalTransform : public NativeComponent<LocalTransform>
{
	REGISTER_MEMBERS_2("Local Transform", value, "value", parent, "parent id");
public:
	glm::mat4 value;
	EntityID parent;
};

class TRS : public NativeComponent<TRS>
{
	REGISTER_MEMBERS_4("TRS", translation, "translation", rotation, "rotation", scale, "scale", dirty, "dirty");
public:
	glm::vec3 translation;//local translation
	glm::quat rotation;   //local rotation
	glm::vec3 scale;      //local scale
	bool dirty = true;
	glm::mat4 toMat() const;
};

class Children : public NativeComponent<Children>
{
	REGISTER_MEMBERS_1("Children", children, "children");
public:
	inlineEntityIDArray children;
};

class Transforms : public Module
{
	EntityManager* _em;
	void updateTRSFromMatrix(EntityID entity, glm::mat4 value);
public:
	static void setParent(EntityID entity, EntityID parent, EntityManager& em);
	void removeParent(EntityID entity);
	void destroyRecursive(EntityID entity, bool updateParentChildren = true);
	static glm::mat4 getParentTransform(EntityID parent, EntityManager& em);
	void start() override;

	static const char* name();
};

class TransformSystem : public System
{
public:
	void run(EntityManager& _em) override;
};


#endif //BRANEENGINE_TRANSFORMS_H
