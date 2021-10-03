#pragma once
#ifdef DEBUG
#include <common/ecs/ecs.h>
#include "JITTests.h"

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
		testAssert(*vc.getVar<bool>(0));
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
		vcvc.pushEmpty();

		vcvc.copy(&vcv, 0, 0);
		expectValue(42, vcvc.readComponentVar<int>(0, 1));
	}
#pragma region  native struct

	class TestNativeComponent : public NativeComponent<TestNativeComponent, 3>
	{
	public:
		bool var1;
		int var2;
		float var3;
		void getVariableIndicies(std::vector<NativeVarDef>& variables) override
		{
			variables.emplace_back(offsetof(TestNativeComponent, var1), virtualBool);
			variables.emplace_back(offsetof(TestNativeComponent, var2), virtualInt);
			variables.emplace_back(offsetof(TestNativeComponent, var3), virtualFloat);
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

		TestRunner::setTestCatagory("Native Component Vector");

		VirtualComponentVector vcv(nc.def());
		vcv.pushEmpty();
		vcv.pushEmpty(); // We want to test the second index to make sure that the calculated size() is correct
		TestNativeComponent* ncp = TestNativeComponent::fromVirtual(vcv.getComponentData(1));
		ncp->var1 = true;
		ncp->var2 = 69;
		ncp->var3 = 420;

		expectValue(true, vcv.readComponentVar<bool>(1, 0));
		expectValue(69, vcv.readComponentVar<int>(1, 1));
		expectValue(420, vcv.readComponentVar<float>(1, 2));

		vcv.setComponentVar(1, 0, false);
		vcv.setComponentVar(1, 1, 42);
		vcv.setComponentVar<float>(1, 2, 42 * 2);

		expectValue(false, vcv.readComponentVar<bool>(1, 0));
		expectValue(42, vcv.readComponentVar<int>(1, 1));
		expectValue(42 * 2, vcv.readComponentVar<float>(1, 2));

		expectValue(false, ncp->var1);
		expectValue(42, ncp->var2);
		expectValue(42 * 2, ncp->var3);
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
		VirtualComponentPtr compPtr = em.getEntityComponent(entity, 1);
		compPtr.setVar<bool>(0, true);
		expectValue(true, compPtr.readVar<bool>(0));

		// Adds a second component, this creates a second archetype and copies the data over,
		em.addComponent(entity, 2);

		//  make sure the copied data is still valid
		compPtr = em.getEntityComponent(entity, 1);
		expectValue(true, compPtr.readVar<bool>(0));

		// Get second component, make sure we can read and write
		compPtr = em.getEntityComponent(entity, 2);
		compPtr.setVar<float>(0, 42);
		expectValue(42, compPtr.readVar<float>(0));

		// make sure this throws no errors
		em.removeComponent(entity, 1);

		// Make sure that we still have the second component and can still read from it after the copy
		compPtr = em.getEntityComponent(entity, 2);
		expectValue(42, compPtr.readVar<float>(0));

		// just make sure these throw no errors
		em.addComponent(entity, 3);
		em.addComponent(entity, 1);
		em.removeComponent(entity, 3);
		em.removeComponent(entity, 2);
		em.removeComponent(entity, 1);

		//We now should have some archetypes that we can fetch
		std::vector<ComponentID> arch1components = {1, 2};
		std::vector<ComponentID> arch2components = {1, 2, 3};
		expectValue(true, (em.getArcheytpe(arch1components) != nullptr));
		expectValue(true, (em.getArcheytpe(arch2components) != nullptr));

	}

	class TestNativeComponent2 : public NativeComponent<TestNativeComponent2, 4>
	{
	public:
		bool var1;
		void getVariableIndicies(std::vector<NativeVarDef>& variables) override
		{
			variables.emplace_back(offsetof(TestNativeComponent2, var1), virtualBool);
		}

	};

	class TestNativeSystem : public VirtualSystem
	{
	public:
		TestNativeSystem(SystemID id)  : VirtualSystem(id)
		{
		}

		const std::vector<ComponentID> requiredComponents() const override
		{
			return {3, 4};
		};
		void run(const std::vector<byte*>& data, VirtualSystemGlobals* constants) const override
		{
			TestNativeComponent* c1 = TestNativeComponent::fromVirtual(data[0]);
			c1->var1 = true;
			c1->var2 = 42;
			c1->var3 = 32;
			TestNativeComponent2* c2 = TestNativeComponent2::fromVirtual(data[sizeof(void*)]);
			c2->var1 = true;

		};


	};

	

	void testForEach()
	{
		TestRunner::setTestCatagory("Native System");

		TestNativeSystem ts(1);


		EntityManager em;
		EnityForEachID forEachID = em.getForEachID({ 3, 4 });
		em.regesterComponent(*TestNativeComponent::def());
		em.regesterComponent(*TestNativeComponent2::def());
		
		for (size_t i = 0; i < 100; i++)
		{
			EntityID e = em.createEntity();
			em.addComponent(e, 3);
			// Create a new archetype
			if(i >= 49)
				em.addComponent(e, 4);
		}
		
		em.forEach(forEachID, [&](byte* components[]) {
			TestNativeComponent* c1 = TestNativeComponent::fromVirtual(components[0]);
			c1->var1 = true;
			c1->var2 += 42;
			c1->var3 += 32;
			TestNativeComponent2* c2 = TestNativeComponent2::fromVirtual(components[1]);
			c2->var1 = true;

		});

		int  testValue  = TestNativeComponent::fromVirtual( em.getEntityComponent(99, 3).data())->var2;
		bool testValue2 = TestNativeComponent2::fromVirtual(em.getEntityComponent(99, 4).data())->var1;
		expectValue(42, testValue);
		expectValue(true, testValue2);

	}

	void runECSTests()
	{  
		testVirtualStruct();
		testNativeStruct();
		testEntityManager();
		testForEach();
		runJITTests();
	}
}
#endif