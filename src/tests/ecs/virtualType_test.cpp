#include <testing.h>
#include <ecs/core/virtualType.h>

class ConstructionTestClass
{
	bool* deconstructed;
public:
	ConstructionTestClass()
	{
		deconstructed = nullptr;
	}
	ConstructionTestClass(bool* decon)
	{
		deconstructed = decon;
	}
	~ConstructionTestClass()
	{
		*deconstructed = true;
	}

};

typedef VirtualVariable<ConstructionTestClass> VirtualConstructionTest;

TEST(VirtualTypes, ConstructionTest)
{
	byte* var = new byte[sizeof(ConstructionTestClass)];

	bool deconstructed = false;
	VirtualConstructionTest vf(0);

	vf.construct(var, &deconstructed);

	vf.deconstruct(var);
	EXPECT_TRUE(deconstructed) << "Constructor and/or Deconstructor not called";
}