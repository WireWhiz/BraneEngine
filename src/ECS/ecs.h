#pragma once
#include "Component.h"
#include "VirtualSystem.h"
#include "Archetype.h"
#include "Entity.h"


class ECSRuntime
{
	EntityManager _entityManager;
public:
	void RunSystems();
	void AddSystem(VirtualSystem newSystem);
	void AddArchetype();
	void RemoveArchetype();
};

