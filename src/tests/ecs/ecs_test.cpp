#include <testing.h>
#include <ecs/ecs.h>
#include <utility/clock.h>

TEST(ECS, VirtualComponentTest)
{
	//Create a definiton for our virtual struct/component

	std::vector<std::unique_ptr<VirtualType>> variables;
	variables.push_back(std::make_unique<VirtualBool>());
	variables.push_back(std::make_unique<VirtualInt>());
	variables.push_back(std::make_unique<VirtualFloat>());
	AssetID aID("localhost/testRunner/component/testComponent");
	ComponentAsset vcd(variables, aID);

	//Create a component using that definition
	VirtualComponent vc(&vcd);
	vc.setVar(0, true);
	vc.setVar(1, 69);
	vc.setVar<float>(2, 420);
	EXPECT_EQ(true, *vc.getVar<bool>(0));
	EXPECT_EQ(69, *vc.getVar<int>(1));
	EXPECT_EQ(420, *vc.getVar<float>(2));

	EXPECT_EQ(true, vc.readVar<bool>(0));
	EXPECT_EQ(69, vc.readVar<int>(1));
	EXPECT_EQ(420, vc.readVar<float>(2));
}

TEST(ECS, VirtualComponentComplexTypesTest)
{
	//Create a definiton for our virtual struct/component

	std::vector<std::unique_ptr<VirtualType>> variables;
	variables.push_back(std::make_unique<VirtualVariable<std::string>>());

	AssetID aID("localhost/testRunner/component/testComponent");
	ComponentAsset vcd(variables, aID);

	//Create a component using that definition
	VirtualComponent vc(&vcd);
	vc.setVar<std::string>(0, "Hello there! General Kenobi!");
	EXPECT_EQ("Hello there! General Kenobi!", *vc.getVar<std::string>(0));
}

TEST(ECS, ComponentSetTest)
{
	ComponentSet cs;

	cs.add((ComponentAsset*)2);
	cs.add((ComponentAsset*)3);
	cs.add((ComponentAsset*)4);
	cs.add((ComponentAsset*)5);

	EXPECT_EQ(cs.components()[0], (ComponentAsset*)2);
	EXPECT_EQ(cs.components()[1], (ComponentAsset*)3);
	EXPECT_EQ(cs.components()[2], (ComponentAsset*)4);
	EXPECT_EQ(cs.components()[3], (ComponentAsset*)5);
	EXPECT_TRUE(cs.contains((ComponentAsset*)2));
	EXPECT_TRUE(cs.contains((ComponentAsset*)3));
	EXPECT_FALSE(cs.contains((ComponentAsset*)1));
	EXPECT_FALSE(cs.contains((ComponentAsset*)6));
	EXPECT_TRUE(cs.contains(cs));

	ComponentSet cs2 = cs;

	cs2.add((ComponentAsset*)1);
	EXPECT_EQ(cs2.components()[0], (ComponentAsset*)1);
	EXPECT_EQ(cs2.components()[1], (ComponentAsset*)2);
	EXPECT_EQ(cs2.components()[2], (ComponentAsset*)3);
	EXPECT_EQ(cs2.components()[3], (ComponentAsset*)4);
	EXPECT_EQ(cs2.components()[4], (ComponentAsset*)5);

	EXPECT_TRUE(cs2.contains(cs));

	cs2.remove((ComponentAsset*)2);
	EXPECT_EQ(cs2.components()[0], (ComponentAsset*)1);
	EXPECT_EQ(cs2.components()[1], (ComponentAsset*)3);

	EXPECT_TRUE(cs.contains(cs));
	EXPECT_FALSE(cs2.contains(cs));

}

TEST(ECS, ArchetypeTest)
{
	std::vector<std::unique_ptr<VirtualType>> variables;
	variables.push_back(std::make_unique<VirtualVariable<std::string>>());
	variables.push_back(std::make_unique<VirtualVariable<std::string>>());

	ComponentAsset helloWorldComponent(variables, AssetID("localhost/tests/component/helloWorld"));
	variables = std::vector<std::unique_ptr<VirtualType>>();
	variables.push_back(std::make_unique<VirtualVariable<std::string>>());
	variables.push_back(std::make_unique<VirtualVariable<std::string>>());
	ComponentAsset helloThereComponent(variables, AssetID("localhost/tests/component/helloThere"));

	variables = std::vector<std::unique_ptr<VirtualType>>();
	variables.push_back(std::make_unique<VirtualVariable<std::string>>());
	variables.push_back(std::make_unique<VirtualVariable<std::string>>());
	ComponentAsset generalKenobiComponent(variables, AssetID("localhost/tests/component/generalKenobi"));

	ComponentSet components;
	components.add(&helloWorldComponent);
	components.add(&helloThereComponent);
	components.add(&generalKenobiComponent);
	std::shared_ptr<ChunkPool> cp = std::make_shared<ChunkPool>();
	Archetype arch(components, cp);

	EXPECT_EQ(arch.entitySize(), sizeof(std::string) * 6);

	

}
TEST(ECS, ChunkTest)
{
	//Put stuff here later
}

TEST(ECS, VirtualComponentVectorTest)
{
	std::vector<std::unique_ptr<VirtualType>> variables;
	variables.push_back(std::make_unique<VirtualBool>());
	variables.push_back(std::make_unique<VirtualInt>());
	variables.push_back(std::make_unique<VirtualFloat>());
	// Create a definiton for our virtual struct/component

	AssetID aID("localhost/testRunner/component/testComponent");
	ComponentAsset vcd(variables, aID);

	// Create a test component
	VirtualComponent vc(&vcd);
	vc.setVar(0, true);
	vc.setVar(1, 69);
	vc.setVar<float>(2, 420);

	// Create a second test component
	VirtualComponent vc2(&vcd);
	vc2.setVar(0, false);
	vc2.setVar(1, 1234);
	vc2.setVar<float>(2, 42);

	// Create component vector
	VirtualComponentVector vcv(&vcd, 2);

	// Append in components
	vcv.pushBack(vc);
	vcv.pushBack(vc2);
	vcv.pushBack(vc);

	// Set some different values on one to see if set funcitons are working
	vcv.setComponentVar(2, 1, 42);

	EXPECT_EQ(69, vcv.readComponentVar<int>(0, 1));
	EXPECT_EQ(1234, vcv.readComponentVar<int>(1, 1));
	EXPECT_EQ(42, vcv.readComponentVar<int>(2, 1));

	// Test removal 
	vcv.remove(0);
	EXPECT_EQ(42, vcv.readComponentVar<int>(0, 1));
	EXPECT_EQ(1234, vcv.readComponentVar<int>(1, 1));
	EXPECT_EQ(42, vcv.readComponentVar<float>(1, 2));

	// Test copying from one vector to another
	VirtualComponentVector vcvc(&vcd);
	vcvc.pushEmpty();

	vcvc.copy(&vcv, 0, 0);
	EXPECT_EQ(42, vcvc.readComponentVar<int>(0, 1));
}

// Native component for testing 
class TestNativeComponent : public NativeComponent<TestNativeComponent>
{
public:
	bool var1;
	int64_t var2;
	float var3;
	void getComponentData(std::vector<std::unique_ptr<VirtualType>>& types, AssetID& id)
	{
		id.parseString("localhost/native/component/testNativeComponent");
		types.emplace_back(std::make_unique<VirtualBool>(offsetof(TestNativeComponent, var1)));
		types.emplace_back(std::make_unique<VirtualInt>(offsetof(TestNativeComponent, var2)));
		types.emplace_back(std::make_unique<VirtualFloat>(offsetof(TestNativeComponent, var3)));
	}

};

TEST(ECS, NativeComponentTest)
{
	TestNativeComponent nc;

	nc.var1 = true;
	nc.var2 = 69;
	nc.var3 = 420;

	VirtualComponentPtr vc = nc.toVirtual();
	EXPECT_EQ(true, vc.readVar<bool>(0));
	EXPECT_EQ(69, vc.readVar<int>(1));
	EXPECT_EQ(420, vc.readVar<float>(2));

	vc.setVar(0, false);
	vc.setVar(1, 42);
	vc.setVar<float>(2, 42 * 2);

	EXPECT_EQ(false, vc.readVar<bool>(0));
	EXPECT_EQ(42, vc.readVar<int>(1));
	EXPECT_EQ(42 * 2, vc.readVar<float>(2));

	EXPECT_EQ(false, nc.var1);
	EXPECT_EQ(42, nc.var2);
	EXPECT_EQ(42 * 2, nc.var3);
}

TEST(ECS, NativeComponentVectorTest)
{
	TestNativeComponent nc;
	nc.var1 = true;
	nc.var2 = 69;
	nc.var3 = 420;

	VirtualComponentVector vcv(nc.def());
	vcv.pushEmpty();
	vcv.pushEmpty(); 
	EXPECT_EQ(2, vcv.size());

	TestNativeComponent* ncp = TestNativeComponent::fromVirtual(vcv.getComponentData(1));
	ncp->var1 = true;
	ncp->var2 = 69;
	ncp->var3 = 420;

	EXPECT_EQ(true, vcv.readComponentVar<bool>(1, 0));
	EXPECT_EQ(69, vcv.readComponentVar<int>(1, 1));
	EXPECT_EQ(420, vcv.readComponentVar<float>(1, 2));

	vcv.setComponentVar(1, 0, false);
	vcv.setComponentVar(1, 1, 42);
	vcv.setComponentVar<float>(1, 2, 42 * 2);

	EXPECT_EQ(false, vcv.readComponentVar<bool>(1, 0));
	EXPECT_EQ(42, vcv.readComponentVar<int>(1, 1));
	EXPECT_EQ(42 * 2, vcv.readComponentVar<float>(1, 2));

	EXPECT_EQ(false, ncp->var1);
	EXPECT_EQ(42, ncp->var2);
	EXPECT_EQ(42 * 2, ncp->var3);
}


TEST(ECS, EntityManagerTest)
{
	std::vector<std::unique_ptr<VirtualType>> comps1;
	comps1.push_back(std::make_unique<VirtualBool>());

	AssetID aID1("localhost/testRunner/component/testComponent1");
	ComponentAsset vcd1(comps1, aID1);

	std::vector<std::unique_ptr<VirtualType>> comps2;
	comps2.push_back(std::make_unique<VirtualFloat>());
	AssetID aID2("localhost/testRunner/component/testComponent2");
	ComponentAsset vcd2(comps2, aID2);


	std::vector<std::unique_ptr<VirtualType>> comps3;
	comps3.push_back(std::make_unique<VirtualBool>());
	comps3.push_back(std::make_unique<VirtualFloat>());
	AssetID aID3("localhost/testRunner/component/testComponent3");
	ComponentAsset vcd3(comps3, aID3);


	EntityManager em;

	// Create entity
	EntityID entity = em.createEntity();

	// Add component, this creates first archetype
	em.addComponent(entity, &vcd1);
	// Get component, make sure we can read and write to it
	VirtualComponent comp = em.getEntityComponent(entity, &vcd1);
	comp.setVar<bool>(0, true);
	EXPECT_EQ(true, comp.readVar<bool>(0));
	em.setEntityComponent(entity, comp);

	// Adds a second component, this creates a second archetype and copies the data over,
	em.addComponent(entity, &vcd2);

	//  make sure the copied data is still valid
	comp = em.getEntityComponent(entity, &vcd1);
	EXPECT_EQ(true, comp.readVar<bool>(0));

	// Get second component, make sure we can read and write
	comp = em.getEntityComponent(entity, &vcd2);
	comp.setVar<float>(0, 42);
	EXPECT_EQ(42, comp.readVar<float>(0));
	em.setEntityComponent(entity, comp);

	// make sure this throws no errors
	em.removeComponent(entity, &vcd1);

	// Make sure that we still have the second component and can still read from it after the copy
	comp = em.getEntityComponent(entity, &vcd2);
	EXPECT_EQ(42, comp.readVar<float>(0));

	// just make sure these throw no errors
	em.addComponent(entity, &vcd3);
	em.addComponent(entity, &vcd1);
	em.removeComponent(entity, &vcd2);
	em.removeComponent(entity, &vcd3);
	em.removeComponent(entity, &vcd1);

	//We now should have some archetypes that we can fetch
	EXPECT_EQ(em._archetypes._archetypes.size(), 4);
	EXPECT_EQ(em._archetypes._archetypes[0].size(), 1);
	EXPECT_EQ(em._archetypes._archetypes[1].size(), 2);
	EXPECT_EQ(em._archetypes._archetypes[2].size(), 3);
	EXPECT_EQ(em._archetypes._archetypes[3].size(), 1);
}

//Classes for native system test
class TestNativeComponent2 : public NativeComponent<TestNativeComponent2>
{
public:
	bool var1;
	void getComponentData(std::vector<std::unique_ptr<VirtualType>>& types, AssetID& id)
	{
		id.parseString("localhost/native/component/testNativeComponent2");
		types.emplace_back(std::make_unique<VirtualVariable<bool>>(offsetof(TestNativeComponent2, var1)));
	}

};

TEST(ECS, ForEachCachingTest)
{
	EntityManager em;

	std::vector<std::unique_ptr<VirtualType>> comps1;
	comps1.push_back(std::make_unique<VirtualBool>());

	AssetID aID1("localhost/testRunner/component/testComponent1");
	ComponentAsset ca1(comps1, aID1);

	std::vector<std::unique_ptr<VirtualType>> comps2;
	comps2.push_back(std::make_unique<VirtualFloat>());
	AssetID aID2("localhost/testRunner/component/testComponent2");
	ComponentAsset ca2(comps2, aID2);


	std::vector<std::unique_ptr<VirtualType>> comps3;
	comps3.push_back(std::make_unique<VirtualBool>());
	comps3.push_back(std::make_unique<VirtualFloat>());
	AssetID aID3("localhost/testRunner/component/testComponent3");
	ComponentAsset ca3(comps3, aID3);

	std::vector<std::unique_ptr<VirtualType>> comps4;
	AssetID aID4("localhost/testRunner/component/testComponent4");
	ComponentAsset ca4(comps4, aID4);


	// Create entity
	EntityID entity = em.createEntity();

	ComponentSet components;
	components.add(&ca1);
	EnityForEachID fe1 = em.getForEachID(components);
	EXPECT_EQ(em._archetypes._forEachData.size(), 1);

	em.addComponent(entity, &ca1); // archetype: ca1
	EXPECT_EQ(em._archetypes.getForEachArchetypes(fe1).size(), 1);
	EXPECT_EQ(em._archetypes._rootArchetypes[&ca1].size(), 1);

	em.addComponent(entity, &ca2); // archetype: ca1, ca2
	EXPECT_EQ(em._archetypes.getForEachArchetypes(fe1).size(), 1);
	EXPECT_EQ(em._archetypes._rootArchetypes[&ca1].size(), 1);
	EXPECT_EQ(em._archetypes._rootArchetypes[&ca2].size(), 1);

	em.addComponent(entity, &ca3); // archetype: ca1, ca2, ca3
	EXPECT_EQ(em._archetypes.getForEachArchetypes(fe1).size(), 1);
	EXPECT_EQ(em._archetypes._rootArchetypes[&ca1].size(), 1);
	EXPECT_EQ(em._archetypes._rootArchetypes[&ca2].size(), 1);
	EXPECT_EQ(em._archetypes._rootArchetypes[&ca3].size(), 1);

	em.addComponent(entity, &ca4); // archetype: ca1, ca2, ca3, ca4
	EXPECT_EQ(em._archetypes.getForEachArchetypes(fe1).size(), 1);
	EXPECT_EQ(em._archetypes._rootArchetypes[&ca1].size(), 1);
	EXPECT_EQ(em._archetypes._rootArchetypes[&ca2].size(), 1);
	EXPECT_EQ(em._archetypes._rootArchetypes[&ca3].size(), 1);
	EXPECT_EQ(em._archetypes._rootArchetypes[&ca4].size(), 1);

	em.removeComponent(entity, &ca2); // archetype: ca1, ca3, ca4
	EXPECT_EQ(em._archetypes.getForEachArchetypes(fe1).size(), 2);
	EXPECT_EQ(em._archetypes._rootArchetypes[&ca1].size(), 2);
	EXPECT_EQ(em._archetypes._rootArchetypes[&ca2].size(), 1);
	EXPECT_EQ(em._archetypes._rootArchetypes[&ca3].size(), 2);
	EXPECT_EQ(em._archetypes._rootArchetypes[&ca4].size(), 1);

	em.removeComponent(entity, &ca3); // archetype: ca1, ca4
	EXPECT_EQ(em._archetypes._archetypes[0][0]->_addEdges.size(), 1);

	EXPECT_EQ(em._archetypes.getForEachArchetypes(fe1).size(), 1);
	EXPECT_EQ(em._archetypes._rootArchetypes[&ca1].size(), 1);
	EXPECT_EQ(em._archetypes._rootArchetypes[&ca2].size(), 1);
	EXPECT_EQ(em._archetypes._rootArchetypes[&ca3].size(), 2);
	EXPECT_EQ(em._archetypes._rootArchetypes[&ca4].size(), 1);


}

TEST(ECS, ForEachTest)
{
	EntityManager em;
	//Create two for each ID one for entities with one component, another for those with two
	ComponentSet comps;
	comps.add(TestNativeComponent::def());
	EnityForEachID forEachID = em.getForEachID(comps);
	comps.add(TestNativeComponent2::def());
	EnityForEachID forEachID2 = em.getForEachID(comps);

	//Create 50 entities with one component, and 50 with two
	for (size_t i = 0; i < 100; i++)
	{
		EntityID e = em.createEntity();
		em.addComponent(e, TestNativeComponent::def());
		// Create a new archetype
		if (i > 49)
			em.addComponent(e, TestNativeComponent2::def());
	}

	EXPECT_EQ(em._archetypes._archetypes[0].size(), 1);
	EXPECT_EQ(em._archetypes._archetypes[1].size(), 1);

	//Set the variables on all the entites with component 3
	em.forEach(forEachID, [&](byte* components[]) {
		TestNativeComponent* c1 = TestNativeComponent::fromVirtual(components[0]);
		c1->var1 = false;
		c1->var2 += 32;
		c1->var3 += 42;
	});

	// Get index of components in set so that in the foreach we can get them from the array
	size_t firstComp  = comps.index(TestNativeComponent::def());
	size_t secondComp = comps.index(TestNativeComponent2::def());
	//Set the variables on all the entites with two components
	em.forEach(forEachID2, [&](byte* components[]) {
		TestNativeComponent2* c2 = TestNativeComponent2::fromVirtual(components[secondComp]);
		c2->var1 = true;
		TestNativeComponent* c1 = TestNativeComponent::fromVirtual(components[firstComp]);
		c1->var1 = true;
		c1->var2 += 42;
		c1->var3 += 32;
	});


	//Check that all the variables are what we set them too
	// 
	//Checking the variables on all the entites with two components
	em.constForEach(forEachID2, [&](const byte* components[]) {
		const TestNativeComponent2* c2 = TestNativeComponent2::fromVirtual(components[secondComp]);
		const TestNativeComponent* c1 = TestNativeComponent::fromVirtual(components[firstComp]);
		EXPECT_EQ(true, c1->var1);
		EXPECT_EQ(74, c1->var2);
		EXPECT_EQ(74, c1->var3);
		EXPECT_EQ(true, c2->var1);
	});


	for (size_t i = 0; i < 100; i++)
	{
		if (i > 49)
		{
			//Entites with two
			VirtualComponent vc1 = em.getEntityComponent(i, TestNativeComponent::def());
			TestNativeComponent*  ts1 = TestNativeComponent::fromVirtual(vc1.data());
			VirtualComponent vc2 = em.getEntityComponent(i, TestNativeComponent2::def());
			TestNativeComponent2* ts2 = TestNativeComponent2::fromVirtual(vc2.data());
			
			EXPECT_EQ(true, ts1->var1) << "entity: " << i;
			EXPECT_EQ(74, ts1->var2)   << "entity: " << i;
			EXPECT_EQ(74, ts1->var3)   << "entity: " << i;
			EXPECT_EQ(true, ts2->var1) << "entity: " << i;

			
		}
		else
		{
			//Entites with one
			VirtualComponent vc1 = em.getEntityComponent(i, TestNativeComponent::def());
			TestNativeComponent* ts1 = TestNativeComponent::fromVirtual(vc1.data());
			EXPECT_EQ(false, ts1->var1) << "entity: " << i;
			EXPECT_EQ(32, ts1->var2)    << "entity: " << i;
			EXPECT_EQ(42, ts1->var3)    << "entity: " << i;
		}
	}
	
	
}

TEST(ECS, ForEachParellelTest)
{
	EntityManager em;
	//Create two for each ID one for entities with one component, another for those with two
	
	std::vector<std::unique_ptr<VirtualType>> variables;
	variables.push_back(std::make_unique<VirtualVariable<size_t>>());

	ComponentAsset counterComponent(variables, AssetID("localhost/tests/component/counterComponent"));
	
	ComponentSet comps;
	comps.add(&counterComponent);
	EnityForEachID forEachID = em.getForEachID(comps);

	em.createEntities(comps, 2000000);

	Stopwatch sw;
	em.forEachParellel(forEachID, [&](byte* components[]) {
		VirtualComponentPtr counter = VirtualComponentPtr(&counterComponent, components[0]);
		counter.setVar(0, 420);
	}, 20000)->finish();
	long long time = sw.time();
	std::cout << "For Each Parellel took: " << time << std::endl;

	for (size_t i = 0; i < 2000000; i++)
	{
		VirtualComponent c = em.getEntityComponent(i, &counterComponent);
		EXPECT_EQ(*c.getVar<size_t>(0), 420) << "entitiy: " << i << std::endl;
	}
}