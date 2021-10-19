#include <testing.h>
#include <ecs/ecs.h>


TEST(ECS, VirtualComponentTest)
{
	//Create a definiton for our virtual struct/component

	std::vector<std::shared_ptr<VirtualType>> components;
	components.push_back(std::make_unique<VirtualBool>());
	components.push_back(std::make_unique<VirtualInt>());
	components.push_back(std::make_unique<VirtualFloat>());

	ComponentDefinition vcd(components, 0);

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

	std::vector<std::shared_ptr<VirtualType>> components;
	components.push_back(std::make_unique<VirtualVariable<std::string>>());

	ComponentDefinition vcd(components, 0);

	//Create a component using that definition
	VirtualComponent vc(&vcd);
	vc.setVar<std::string>(0, "Hello there! General Kenobi!");
	EXPECT_EQ("Hello there! General Kenobi!", *vc.getVar<std::string>(0));
}

TEST(ECS, ComponentSetTest)
{
	ComponentSet cs;

	cs.add((ComponentDefinition*)2);
	cs.add((ComponentDefinition*)3);

	EXPECT_EQ(cs.components()[0], (ComponentDefinition*)2);
	EXPECT_EQ(cs.components()[1], (ComponentDefinition*)3);
	EXPECT_TRUE(cs.contains((ComponentDefinition*)2));
	EXPECT_TRUE(cs.contains((ComponentDefinition*)3));
	EXPECT_FALSE(cs.contains((ComponentDefinition*)1));
	EXPECT_FALSE(cs.contains((ComponentDefinition*)4));
	EXPECT_TRUE(cs.contains(cs));

	ComponentSet cs2 = cs;

	cs2.add((ComponentDefinition*)1);
	EXPECT_EQ(cs2.components()[0], (ComponentDefinition*)1);
	EXPECT_EQ(cs2.components()[1], (ComponentDefinition*)2);
	EXPECT_EQ(cs2.components()[2], (ComponentDefinition*)3);

	EXPECT_TRUE(cs2.contains(cs));

	cs2.remove((ComponentDefinition*)2);
	EXPECT_EQ(cs2.components()[0], (ComponentDefinition*)1);
	EXPECT_EQ(cs2.components()[1], (ComponentDefinition*)3);

	EXPECT_FALSE(cs2.contains(cs));

}

TEST(ECS, VirtualComponentChunkTest)
{
	//Put stuff here later
}

TEST(ECS, VirtualComponentVectorTest)
{
	std::vector<std::shared_ptr<VirtualType>> components;
	components.push_back(std::make_unique<VirtualBool>());
	components.push_back(std::make_unique<VirtualInt>());
	components.push_back(std::make_unique<VirtualFloat>());
	// Create a definiton for our virtual struct/component
	ComponentDefinition vcd(components, 0);

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
class TestNativeComponent : public NativeComponent<TestNativeComponent, 3>
{
public:
	bool var1;
	int64_t var2;
	float var3;
	void getVariableTypes(std::vector<std::shared_ptr<VirtualType>>& types) override
	{
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
	std::vector<std::shared_ptr<VirtualType>> comps1;
	comps1.push_back(std::make_unique<VirtualBool>());
	ComponentDefinition vcd1(comps1, 1);

	std::vector<std::shared_ptr<VirtualType>> comps2;
	comps2.push_back(std::make_unique<VirtualFloat>());
	ComponentDefinition vcd2(comps2, 2);


	std::vector<std::shared_ptr<VirtualType>> comps3;
	comps3.push_back(std::make_unique<VirtualBool>());
	comps3.push_back(std::make_unique<VirtualFloat>());
	ComponentDefinition vcd3(comps3, 3);


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
	EXPECT_EQ(true, compPtr.readVar<bool>(0));

	// Adds a second component, this creates a second archetype and copies the data over,
	em.addComponent(entity, 2);

	//  make sure the copied data is still valid
	compPtr = em.getEntityComponent(entity, 1);
	EXPECT_EQ(true, compPtr.readVar<bool>(0));

	// Get second component, make sure we can read and write
	compPtr = em.getEntityComponent(entity, 2);
	compPtr.setVar<float>(0, 42);
	EXPECT_EQ(42, compPtr.readVar<float>(0));

	// make sure this throws no errors
	em.removeComponent(entity, 1);

	// Make sure that we still have the second component and can still read from it after the copy
	compPtr = em.getEntityComponent(entity, 2);
	EXPECT_EQ(42, compPtr.readVar<float>(0));

	// just make sure these throw no errors
	em.addComponent(entity, 3);
	em.addComponent(entity, 1);
	em.removeComponent(entity, 3);
	em.removeComponent(entity, 2);
	em.removeComponent(entity, 1);

	//We now should have some archetypes that we can fetch
	ComponentSet archComponents;
	archComponents.add(em.componentDef(1));
	archComponents.add(em.componentDef(2));
	EXPECT_EQ(true, (em.getArcheytpe(archComponents) != nullptr));
	archComponents.add(em.componentDef(3));
	EXPECT_EQ(true, (em.getArcheytpe(archComponents) != nullptr));
}

//Classes for native system test
class TestNativeComponent2 : public NativeComponent<TestNativeComponent2, 4>
{
public:
	bool var1;
	void getVariableTypes(std::vector<std::shared_ptr<VirtualType>>& types) override
	{
		types.emplace_back(std::make_unique<VirtualBool>(offsetof(TestNativeComponent2, var1)));
	}

};

TEST(ECS, ForEachTest)
{
	EntityManager em;
	//Create two for each ID one for entities with one component, another for those with two
	em.regesterComponent(*TestNativeComponent::def());
	em.regesterComponent(*TestNativeComponent2::def());
	ComponentSet comps;
	comps.add(em.componentDef(3));
	EnityForEachID forEachID = em.getForEachID(comps);
	comps.add(em.componentDef(4));
	EnityForEachID forEachID2 = em.getForEachID(comps);

	//Create 50 entities with one component, and 50 with two
	for (size_t i = 0; i < 100; i++)
	{
		EntityID e = em.createEntity();
		em.addComponent(e, 3);
		// Create a new archetype
		if (i > 49)
			em.addComponent(e, 4);
	}
	//Set the variables on all the entites with component 3
	size_t counter = 0;

	em.forEach(forEachID, [&](byte* components[]) {
		TestNativeComponent* c1 = TestNativeComponent::fromVirtual(components[0]);
		c1->var1 = false;
		c1->var2 += 32;
		c1->var3 += 42;
		counter++;
	});
	std::cout << "ran first for-each: " << counter << " times." << std::endl;

	size_t firstComp = comps.index(em.componentDef(3));
	size_t secondComp = comps.index(em.componentDef(4));

	//Set the variables on all the entites with two components
	counter = 0;
	em.forEach(forEachID2, [&](byte* components[]) {
		TestNativeComponent2* c2 = TestNativeComponent2::fromVirtual(components[secondComp]);
		c2->var1 = true;
		TestNativeComponent* c1 = TestNativeComponent::fromVirtual(components[firstComp]);
		c1->var1 = true;
		if (counter == 0)
		{
			std::cout << "Variable before: " << c1->var2 << std::endl;
			printf("Component %u address in foreach: %p\n", 0u, c1);
			printf("Component %u address in foreach: %p\n", 1u, c2);
		}
		c1->var2 += 42;
		if (counter == 0)
		{
			std::cout << "Variable after: " << c1->var2 << std::endl;
		}
		c1->var3 += 32;
		
		counter++;

	});
	std::cout << "ran second for-each " << counter << " times." << std::endl;


	//Check that all the variables are what we set them too
	for (size_t i = 0; i < 100; i++)
	{
		if (i > 49)
		{
			//Entites with two
			TestNativeComponent*  ts1 = TestNativeComponent::fromVirtual(em.getEntityComponent(i, 3).data());
			TestNativeComponent2* ts2 = TestNativeComponent2::fromVirtual(em.getEntityComponent(i, 4).data());
			if (i == 50)
			{
				printf("Component %u address expect check: %p\n", 0u, ts1);
				printf("Component %u address expect check: %p\n", 1u, ts2);
			}
			
			EXPECT_EQ(true, ts1->var1) << "entity: " << i;
			EXPECT_EQ(74, ts1->var2)   << "entity: " << i;
			EXPECT_EQ(74, ts1->var3)   << "entity: " << i;
			EXPECT_EQ(true, ts2->var1);

			
		}
		else
		{
			//Entites with one
			TestNativeComponent* ts1 = TestNativeComponent::fromVirtual(em.getEntityComponent(i, 3).data());
			EXPECT_EQ(false, ts1->var1) << "entity: " << i;
			EXPECT_EQ(32, ts1->var2)    << "entity: " << i;
			EXPECT_EQ(42, ts1->var3)    << "entity: " << i;
		}
	}
	
	
}