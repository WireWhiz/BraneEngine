#include "testing.h"
#include <ecs/ecs.h>

class CounterComponent : public NativeComponent<CounterComponent, 0>
{
public:
	virtual void getVariableTypes(std::vector<std::shared_ptr<VirtualType>>& types) override
	{
		types.push_back(std::make_shared<VirtualInt>(offsetof(CounterComponent, counter)));
	}
public:
	uint64_t counter = 0;
	
};

class AddSystem : public VirtualSystem
{
public:
	EnityForEachID _feid;
	int value;
	AddSystem(SystemID id, EnityForEachID feid, int value) : VirtualSystem(id)
	{
		_feid = feid;
		this->value = value;
	}
	virtual void run(EntityManager* em)
	{
		em->forEach(_feid, [this](byte** components) {
			CounterComponent* ptr = CounterComponent::fromVirtual(components[0]);
			ptr->counter += value;
		});
		
	}
};

class MulSystem : public VirtualSystem
{
public:
	EnityForEachID _feid;
	int value;
	MulSystem(SystemID id, EnityForEachID feid, int value) : VirtualSystem(id)
	{
		_feid = feid;
		this->value = value;
	}
	virtual void run(EntityManager* em)
	{
		em->forEach(_feid, [this](byte** components) {
			CounterComponent* ptr = CounterComponent::fromVirtual(components[0]);
			ptr->counter *= value;
		});

	}
};

TEST(Systems, NativeTest)
{
	EntityManager em;

	//Create entity and add test component
	CounterComponent cc;
	em.regesterComponent(*cc.def());

	em.createEntity();
	em.addComponent(0, 0);

	//Create test system
	ComponentSet components;
	components.add(em.componentDef(0));
	std::unique_ptr<VirtualSystem> aos = std::make_unique<AddSystem>(0, em.getForEachID(components), 1);
	EXPECT_TRUE(em.addSystem(aos));

	//Run system
	em.runSystems();
	em.runSystems();
	em.runSystems();

	EXPECT_EQ(CounterComponent::fromVirtual(em.getEntityComponent(0, 0).data())->counter, 3);
	EXPECT_EQ(em.getEntityComponent(0, 0).readVar<uint64_t>(0), 3);

}

TEST(Systems, BeforeConstraint)
{
	EntityManager em;

	//Create entity and add test component
	CounterComponent cc;
	em.regesterComponent(*cc.def());

	em.createEntity();
	em.addComponent(0, 0);

	//Create test systems
	ComponentSet components;
	components.add(em.componentDef(0));
	EnityForEachID feid = em.getForEachID(components);
	std::unique_ptr<VirtualSystem> mul = std::make_unique<MulSystem>(1, feid, 2);
	EXPECT_TRUE(em.addSystem(mul));

	std::unique_ptr<VirtualSystem> add = std::make_unique<AddSystem>(0, feid, 3);
	EXPECT_TRUE(em.addSystem(add));


	EXPECT_TRUE(em.addBeforeConstraint(0, 1));
	EXPECT_FALSE(em.addBeforeConstraint(1, 0));
	EXPECT_FALSE(em.addAfterConstraint(0, 1));


	//Run system
	em.runSystems();

	EXPECT_EQ(CounterComponent::fromVirtual(em.getEntityComponent(0, 0).data())->counter, 6);
	EXPECT_EQ(em.getEntityComponent(0, 0).readVar<uint64_t>(0), 6);
}

TEST(Systems, AfterConstraint)
{
	EntityManager em;

	//Create entity and add test component
	CounterComponent cc;
	em.regesterComponent(*cc.def());

	em.createEntity();
	em.addComponent(0, 0);

	//Create test systems
	ComponentSet components;
	components.add(em.componentDef(0));
	EnityForEachID feid = em.getForEachID(components);
	std::unique_ptr<VirtualSystem> mul = std::make_unique<MulSystem>(1, feid, 2);
	EXPECT_TRUE(em.addSystem(mul));

	std::unique_ptr<VirtualSystem> add = std::make_unique<AddSystem>(0, feid, 3);
	EXPECT_TRUE(em.addSystem(add));

	EXPECT_TRUE(em.addAfterConstraint(1, 0));
	EXPECT_FALSE(em.addAfterConstraint(0, 1));
	EXPECT_FALSE(em.addBeforeConstraint(1, 0));


	//Run system
	em.runSystems();

	EXPECT_EQ(CounterComponent::fromVirtual(em.getEntityComponent(0, 0).data())->counter, 6);
	EXPECT_EQ(em.getEntityComponent(0, 0).readVar<uint64_t>(0), 6);
}