#pragma once
#ifdef DEBUG
#include <ecs.h>
#include <Component.h>
#include <Archetype.h>
#include <Entity.h>

namespace tests
{
	void testVirtualStruct()
	{
		TestRunner::setTestCatagory("Virtual Struct");
		
		//Initalize a struct with one of every type of value and set them to test values, then read them back and make sure that they are correct
		ComponentDefinition vsd(3, 0);
		vsd.setIndexType(0, virtualBool);
		vsd.setIndexType(1, virtualInt);
		vsd.setIndexType(2, virtualFloat);
		vsd.initalize();
		VirtualComponent vs(&vsd);
		vs.setVar(0, true);
		vs.setVar(1, 69);
		vs.setVar<float>(2, 420);
		expectValue(true, *vs.getVar<bool> (0));
		expectValue(69,   *vs.getVar<int>  (1));
		expectValue(420,  *vs.getVar<float>(2));

		expectValue(true, vs.readVar<bool> (0));
		expectValue(69,   vs.readVar<int>  (1));
		expectValue(420,  vs.readVar<float>(2));


		TestRunner::setTestCatagory("Virtual Struct Vector");

		VirtualComponent vs2(&vsd);
		vs2.setVar(0, false);
		vs2.setVar(1, 1234);
		vs2.setVar<float>(2, 42);

		VirtualComponentVector vsv(&vsd, 2);

		vsv.pushBack(vs);
		vsv.pushBack(vs2);
		vsv.pushBack(vs);
		vsv.setComponentVar(2, 1, 42);

		expectValue(69, vsv.readComponentVar<int>  (0, 1));
		expectValue(1234, vsv.readComponentVar<int>(1, 1));
		expectValue(42, vsv.readComponentVar<int>  (2, 1));
		
		vsv.swapRemove(0);
		expectValue(42, vsv.readComponentVar<int>  (0, 1));
		expectValue(1234, vsv.readComponentVar<int>(1, 1));
		expectValue(42, vsv.readComponentVar<float>(1, 2));

		VirtualComponentVector vsvc(&vsd);
		vsvc.pushBack();

		vsvc.copy(vsv, 0, 0);
		expectValue(42, vsvc.readComponentVar<int>(0, 1));
	}

	void testEntityManager()
	{
		TestRunner::setTestCatagory("Entity Manager");

		ComponentDefinition vsd1(1, 1);
		vsd1.setIndexType(0, virtualBool);
		vsd1.initalize();

		ComponentDefinition vsd2(1, 2);
		vsd2.setIndexType(0, virtualFloat);
		vsd2.initalize();

		ComponentDefinition vsd3(2, 3);
		vsd3.setIndexType(0, virtualBool);
		vsd3.setIndexType(1, virtualFloat);
		vsd3.initalize();

		
		EntityManager em;
		em.regesterComponent(vsd1);
		em.regesterComponent(vsd2);
		em.regesterComponent(vsd3);

		// Create entity
		EntityID entity = em.createEntity();

		// Add component, this creates first archetype
		em.addComponent(entity, 1);
		// Get component, make sure we can read and write to it
		VirtualComponentPtr compPtr = em.getComponent(entity, 1);
		compPtr.setVar<bool>(0, true);
		expectValue(true, compPtr.readVar<bool>(0));

		// Adds a second component, this creates a second archetype and copies the data over,
		em.addComponent(entity, 2);

		//  make sure the copied data is still valid
		compPtr = em.getComponent(entity, 1);
		expectValue(true, compPtr.readVar<bool>(0));

		// Get second component, make sure we can read and write
		compPtr = em.getComponent(entity, 2);
		compPtr.setVar<float>(0, 42);
		expectValue(42, compPtr.readVar<float>(0));

		// make sure this throws no errors
		em.removeComponent(entity, 1);

		// Make sure that we still have the second component and can still read from it after the copy
		compPtr = em.getComponent(entity, 2);
		expectValue(42, compPtr.readVar<float>(0));

		// just make sure these throw no errors
		em.addComponent(entity, 3);
		em.addComponent(entity, 1);
		em.removeComponent(entity, 3);
		em.removeComponent(entity, 2);
		em.removeComponent(entity, 1);
	}

	void runECSTests()
	{
		testVirtualStruct();
		testEntityManager();
	}
}
#endif