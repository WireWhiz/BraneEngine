//
// Created by eli on 8/28/2022.
//

#include "testing.h"
#include "ecs/entity.h"
#include "assets/assetManager.h"
#include "unordered_set"
#include "utility/clock.h"

namespace std
{
	template <>
	struct hash<ComponentSet>
	{
		size_t operator()(const ComponentSet& componentSet) const
		{
			size_t h = 17;
			for(auto id : componentSet)
				h = h * 31 + hash<ComponentID>()(id);
			return h;
		}
	};
}


void allPossibleComponentSets(const ComponentSet& set, std::unordered_set<ComponentSet>& results)
{
	if(results.count(set))
		return;
	results.insert(set);
	if(set.size() == 1)
		return;
	for(auto cID : set)
	{
		ComponentSet remaining = set;
		remaining.remove(cID);
		allPossibleComponentSets(remaining, results);
	}
}

void printCallsPerFrame(size_t nanoseconds, size_t framerate)
{
	size_t timeInFrame = 1000000 / framerate;
	std::cout << "Can be called " << timeInFrame / nanoseconds << " times in a frame, assuming " << framerate << "fps" << std::endl;
}

TEST(ECS_Profiling, QueryCreation)
{
	Runtime::init();
	Runtime::timeline().addBlock("main");
	Runtime::addModule<EntityManager>();

	auto& em = *Runtime::getModule<EntityManager>();
	em.components().registerComponent(EntityIDComponent::constructDescription());

	std::set<ComponentDescription*> components;
	for (size_t i = 0; i < 12; ++i)
		components.insert(new ComponentDescription({VirtualType::virtualBool}));

	ComponentSet allComponents;
	for(auto c : components)
	{
		em.components().registerComponent(c);
		allComponents.add(c->id);
	}

	std::unordered_set<ComponentSet> allSets;
	allPossibleComponentSets(allComponents, allSets);
	//Create one archetype for every possible combination of components.
	std::cout << "Running test with " << components.size() << " components and " << allSets.size() + 1 << " archetypes" << std::endl;
	for(const ComponentSet& set : allSets)
		em.createEntity(set);


	SystemContext ctx;
	{
		std::cout << "Query with 1 component took:\n";
		Stopwatch smallTime1;
		ComponentFilter smallFilter1(&ctx);
		smallFilter1.addComponent(1);
		em.getEntities(smallFilter1);
		auto smallResult1 = smallTime1.time<std::chrono::microseconds>();
		std::cout << smallResult1 << " nanoseconds\n";

		Stopwatch smallTime2;
		ComponentFilter smallFilter2(&ctx);
		smallFilter2.addComponent(8);
		em.getEntities(smallFilter2);
		auto smallResult2 = smallTime2.time<std::chrono::microseconds>();
		std::cout << smallResult2 << " nanoseconds\n";

		Stopwatch smallTime3;
		ComponentFilter smallFilter3(&ctx);
		smallFilter3.addComponent(16);
		em.getEntities(smallFilter3);
		auto smallResult3 = smallTime3.time<std::chrono::microseconds>();
		std::cout << smallResult3 << " nanoseconds\n";

		auto average = (smallResult1 + smallResult2 + smallResult3) / 3;
		std::cout << "average: " << average << " nanoseconds" << std::endl;
		printCallsPerFrame(average, 144);
		std::cout << std::endl;
	}
	{
		std::cout << "Query with 3 components took:\n";
		Stopwatch mediumTime1;
		ComponentFilter medFilter(&ctx);
		for(auto id : {1, 2, 3})
			medFilter.addComponent(id);
		em.getEntities(medFilter);
		auto medResult1 = mediumTime1.time<std::chrono::microseconds>();
		std::cout << medResult1 << " nanoseconds\n";

		Stopwatch mediumTime2;
		ComponentFilter medFilter2(&ctx);
		for(auto id : {1, 5, 10})
			medFilter2.addComponent(id);
		em.getEntities(medFilter2);
		auto medResult2 = mediumTime2.time<std::chrono::microseconds>();
		std::cout << medResult2 << " nanoseconds\n";

		Stopwatch mediumTime3;
		ComponentFilter medFilter3(&ctx);
		for(auto id : {0, 9, 10})
			medFilter3.addComponent(id);
		em.getEntities(medFilter3);
		auto medResult3 = mediumTime3.time<std::chrono::microseconds>();
		std::cout << medResult3 << " nanoseconds\n";

		auto average = (medResult1 + medResult2 + medResult3) / 3;
		std::cout << "average: " << average << " nanoseconds" << std::endl;
		printCallsPerFrame(average, 144);
		std::cout << std::endl;
	}

	{
		std::cout << "Query with " << components.size() << " components took:\n";
		Stopwatch largeTime;
		ComponentFilter largeFilter(&ctx);
		for(auto id : allComponents)
			largeFilter.addComponent(id);
		em.getEntities(largeFilter);
		auto largeResult = largeTime.time<std::chrono::microseconds>();


		std::cout << largeResult << " nanoseconds" << std::endl;
		printCallsPerFrame(largeResult, 144);
		std::cout << std::endl;
	}

	Runtime::cleanup();

	for(auto c : components)
		delete c;
}