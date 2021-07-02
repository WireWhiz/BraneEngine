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
		TestRunner::setTestCatagory("Virtual Component");
		
		//Initalize a struct with one of every type of value and set them to test values, then read them back and make sure that they are correct
		ComponentDefinition vcd(3, 0);
		vcd.setIndexType(0, virtualBool);
		vcd.setIndexType(1, virtualInt);
		vcd.setIndexType(2, virtualFloat);
		vcd.initalize();
		VirtualComponent vc(&vcd);
		vc.setVar(0, true);
		vc.setVar(1, 69);
		vc.setVar<float>(2, 420);
		expectValue(true, *vc.getVar<bool> (0));
		expectValue(69,   *vc.getVar<int>  (1));
		expectValue(420,  *vc.getVar<float>(2));

		expectValue(true, vc.readVar<bool> (0));
		expectValue(69,   vc.readVar<int>  (1));
		expectValue(420,  vc.readVar<float>(2));


		TestRunner::setTestCatagory("Virtual Component Vector");

		VirtualComponent vc2(&vcd);
		vc2.setVar(0, false);
		vc2.setVar(1, 1234);
		vc2.setVar<float>(2, 42);

		VirtualComponentVector vcv(&vcd, 2);

		vcv.pushBack(vc);
		vcv.pushBack(vc2);
		vcv.pushBack(vc);
		vcv.setComponentVar(2, 1, 42);

		expectValue(69, vcv.readComponentVar<int>  (0, 1));
		expectValue(1234, vcv.readComponentVar<int>(1, 1));
		expectValue(42, vcv.readComponentVar<int>  (2, 1));
		
		vcv.swapRemove(0);
		expectValue(42, vcv.readComponentVar<int>  (0, 1));
		expectValue(1234, vcv.readComponentVar<int>(1, 1));
		expectValue(42, vcv.readComponentVar<float>(1, 2));

		VirtualComponentVector vcvc(&vcd);
		vcvc.pushBack();

		vcvc.copy(vcv, 0, 0);
		expectValue(42, vcvc.readComponentVar<int>(0, 1));
	}
#pragma region  native struct

	class TestNativeComponent : public NativeComponent<TestNativeComponent, 3>
	{
	public:
		bool var1;
		int var2;
		float var3;
	protected:
		void getVariables(std::vector<NativeVarDef>& variables) override
		{
			variables.push_back(NativeVarDef(getVarIndex(&var1), virtualBool));
			variables.push_back(NativeVarDef(getVarIndex(&var2), virtualInt));
			variables.push_back(NativeVarDef(getVarIndex(&var3), virtualFloat));
		}
	
	};

	void testNativeStruct()
	{
		TestRunner::setTestCatagory("Native Component");

		//Initalize a struct with one of every type of value and set them to test values, then read them back and make sure that they are correct
		TestNativeComponent nc;

		nc.var1 = true;
		nc.var2 = 69;
		nc.var3 = 420;

		VirtualComponentPtr vc = nc.toVirtual();
		expectValue(true, vc.readVar<bool>(0));
		expectValue(69, vc.readVar<int>(1));
		expectValue(420, vc.readVar<float>(2));

		vc.setVar(0, false);
		vc.setVar(1, 42);
		vc.setVar<float>(2, 42 * 2);

		expectValue(false, vc.readVar<bool>(0));
		expectValue(42, vc.readVar<int>(1));
		expectValue(42 * 2, vc.readVar<float>(2));

		expectValue(false, nc.var1);
		expectValue(42,   nc.var2);
		expectValue(42 * 2, nc.var3);
	}
#pragma endregion
	void testEntityManager()
	{
		TestRunner::setTestCatagory("Entity Manager");

		ComponentDefinition vcd1(1, 1);
		vcd1.setIndexType(0, virtualBool);
		vcd1.initalize();

		ComponentDefinition vcd2(1, 2);
		vcd2.setIndexType(0, virtualFloat);
		vcd2.initalize();

		ComponentDefinition vcd3(2, 3);
		vcd3.setIndexType(0, virtualBool);
		vcd3.setIndexType(1, virtualFloat);
		vcd3.initalize();

		
		EntityManager em;
		em.regesterComponent(vcd1);
		em.regesterComponent(vcd2);
		em.regesterComponent(vcd3);

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
		testNativeStruct();
		testEntityManager();
	}
}
#endif