#include "testing.h"
#include "utility/threadPool.h"

TEST(Threading, ThreadPoolTest)
{
	ThreadPool::init();
	std::atomic_bool testBool = false;
	ThreadPool::enqueue([&]() {
		testBool = true;
	});

	ThreadPool::cleanup();
	EXPECT_TRUE(testBool);
}