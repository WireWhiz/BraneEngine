#include "testing.h"
#include <ecs/ecs.h>

class CounterComponent : public NativeComponent<CounterComponent>
{
	REGISTER_MEMBERS_1(counter)
public:
	/*
	void getComponentData(std::vector<std::unique_ptr<VirtualType>>& types, AssetID& id)
	{
		id.parseString("localhost/native/component/CounterComponent");
		types.push_back(std::make_unique<VirtualInt>(offsetof(CounterComponent, counter)));
	}
	*/
public:
	uint64_t counter = 0;
	
};

class AddSystem : public VirtualSystem
{
public:
	EntityForEachID _feid;
	int value;
	AddSystem(SystemID id, EntityForEachID feid, int value) : VirtualSystem(id)
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
	EntityForEachID _feid;
	int value;
	MulSystem(SystemID id, EntityForEachID feid, int value) : VirtualSystem(id)
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

	em.createEntity();
	em.addComponent(0, CounterComponent::def());

	//Create test system
	ComponentSet components;
	components.add(cc.def());
	std::unique_ptr<VirtualSystem> aos = std::make_unique<AddSystem>(AssetID("localhost/1"), em.getForEachID(components), 1);
	EXPECT_TRUE(em.addSystem(std::move(aos)));

	//Run system
	em.runSystems();
	em.runSystems();
	em.runSystems();

	VirtualComponent counterTest = em.getEntityComponent(0, CounterComponent::def());
	EXPECT_EQ(CounterComponent::fromVirtual(counterTest.data())->counter, 3);
	EXPECT_EQ(em.getEntityComponent(0, CounterComponent::def()).readVar<uint64_t>(0), 3);

}

TEST(Systems, BeforeConstraint)
{
	EntityManager em;

	//Create entity and add test component
	CounterComponent cc;

	em.createEntity();
	em.addComponent(0, CounterComponent::def());

	//Create test systems
	ComponentSet components;
	components.add(cc.def());
	EntityForEachID feid = em.getForEachID(components);
	std::unique_ptr<VirtualSystem> mul = std::make_unique<MulSystem>(AssetID("localhost/0"), feid, 2);
	EXPECT_TRUE(em.addSystem(std::move(mul)));

	std::unique_ptr<VirtualSystem> add = std::make_unique<AddSystem>(AssetID("localhost/1"), feid, 3);
	EXPECT_TRUE(em.addSystem(std::move(add)));


	EXPECT_TRUE(em.addBeforeConstraint(AssetID("localhost/1"), AssetID("localhost/0")));
	EXPECT_FALSE(em.addBeforeConstraint(AssetID("localhost/0"), AssetID("localhost/1")));
	EXPECT_FALSE(em.addAfterConstraint(AssetID("localhost/1"), AssetID("localhost/0")));


	//Run system
	em.runSystems();

	VirtualComponent counterTest = em.getEntityComponent(0, CounterComponent::def());
	EXPECT_EQ(CounterComponent::fromVirtual(counterTest.data())->counter, 6);
	EXPECT_EQ(em.getEntityComponent(0, CounterComponent::def()).readVar<uint64_t>(0), 6);
}

TEST(Systems, AfterConstraint)
{
	EntityManager em;

	//Create entity and add test component
	CounterComponent cc;

	em.createEntity();
	em.addComponent(0, CounterComponent::def());

	//Create test systems
	ComponentSet components;
	components.add(cc.def());
	EntityForEachID feid = em.getForEachID(components);
	std::unique_ptr<VirtualSystem> mul = std::make_unique<MulSystem>(AssetID("localhost/0"), feid, 2);
	EXPECT_TRUE(em.addSystem(std::move(mul)));

	std::unique_ptr<VirtualSystem> add = std::make_unique<AddSystem>(AssetID("localhost/1"), feid, 3);
	EXPECT_TRUE(em.addSystem(std::move(add)));

	EXPECT_TRUE(em.addAfterConstraint(AssetID("localhost/0"), AssetID("localhost/1")));
	EXPECT_FALSE(em.addAfterConstraint(AssetID("localhost/1"), AssetID("localhost/0")));
	EXPECT_FALSE(em.addBeforeConstraint(AssetID("localhost/0"), AssetID("localhost/1")));


	//Run system
	em.runSystems();

	EXPECT_EQ(CounterComponent::fromVirtual(em.getEntityComponent(0, CounterComponent::def()).data())->counter, 6);
	EXPECT_EQ(em.getEntityComponent(0, CounterComponent::def()).readVar<uint64_t>(0), 6);
}