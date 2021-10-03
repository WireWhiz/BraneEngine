#include <iostream>
#include <gtest/gtest.h>
#include <ecs/ecs.h>

TEST(ECS, BasicAssertions)
{
	ComponentDefinition vcd(3, 0);
	vcd.setIndexType(0, virtualBool);
	vcd.setIndexType(1, virtualInt);
	vcd.setIndexType(2, virtualFloat);
	vcd.initalize();
	VirtualComponent vc(&vcd);
	vc.setVar(0, true);
	vc.setVar(1, 69);
	vc.setVar<float>(2, 420);
	EXPECT_EQ(!*vc.getVar<bool>(0), *vc.getVar<bool>(0));//This will fail
	EXPECT_EQ(true, *vc.getVar<bool>(0));
	EXPECT_EQ(69, *vc.getVar<int>(1));
	EXPECT_EQ(420, *vc.getVar<float>(2));

	EXPECT_EQ(true, vc.readVar<bool>(0));
	EXPECT_EQ(69, vc.readVar<int>(1));
	EXPECT_EQ(420, vc.readVar<float>(2));
    EXPECT_STRNE("hello", "world");
    EXPECT_EQ(7 * 6, 42);
}