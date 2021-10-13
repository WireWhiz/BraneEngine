#include "testing.h"
#include <ecs/ecs.h>

class CounterComponent : public NativeComponent<CounterComponent, 0>
{
	virtual void getVariableTypes(std::vector<std::shared_ptr<VirtualType>>& types) override
	{
		types.push_back(std::make_shared<VirtualInt>(offsetof(CounterComponent, counter)));
	}
public:
	uint64_t counter;
	
};

class AddOneSystem : public VirtualSystem
{
public:
	EnityForEachID _feid;
	AddOneSystem(SystemID id) : VirtualSystem(id)
	{

	}
	virtual void run(EntityManager* em)
	{
		em->forEach(_feid, [this](byte** components) {
			CounterComponent* ptr = CounterComponent::fromVirtual(components[0]);
			ptr->counter += 1;
		});
		
	}
};

TEST(Systems, NativeTest)
{
	EntityManager em;
	em.addSystemBlock("testBlock", "", "");
	
	em.getSystemBlock("testBlock")->addSystem(new AddOneSystem(0));

	em.runSystems();
	em.runSystems();
	em.runSystems();

	EXPECT_EQ(em.getEntityComponent(0, 0).readVar<uint64_t>(0), 2);

}