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

TEST(VirtualTypes, SerializationTest)
{
	OSerializedData sdata;
	float one = 1;
	float two = 2;
	std::vector<float> ott = {1,2,3};
	VirtualVariable<float> f(0);
	VirtualVariable<std::vector<float>> fv(0);

	f.serialize(sdata, (byte*)&one);
	fv.serialize(sdata, (byte*)&ott);
	f.serialize(sdata, (byte*)&two);

	ISerializedData serialized = sdata.toIMessage();

	serialized >> one;
	serialized >> ott;
	serialized >> two;

	EXPECT_EQ(one, 1);
	EXPECT_EQ(two, 2);
	EXPECT_EQ(ott[0], 1);
	EXPECT_EQ(ott[1], 2);
	EXPECT_EQ(ott[2], 3);
}