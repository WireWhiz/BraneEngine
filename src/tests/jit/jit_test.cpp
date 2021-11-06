#include <testing.h>
#include <ecs/jit/virtualSystemCompiler.h>

TEST(ECS, BasicJITTest)
{
	VirtualSystemCompiler vsc;
	EXPECT_EQ(5, vsc.testFunction());
	EXPECT_EQ(5, vsc.testFunction2());
}