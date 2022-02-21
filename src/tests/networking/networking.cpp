//
// Created by eli on 1/31/2022.
//

#include "testing.h"
#include <networking/networking.h>
using namespace net;

TEST(networking, SerializedDataTest)
{
	OSerializedData oMessage;

	std::vector<uint32_t> first = {1,2,4,8,16};
	std::string second = "test string\n";
	uint64_t third = 42;

	oMessage << first << second << third;
	first.resize(0);
	second = "";
	third = 0;
	ISerializedData iMessage = oMessage.toIMessage();

	std::cout << oMessage << iMessage;

	iMessage >> first >> second >> third;

	EXPECT_EQ(first.size(), 5);
	EXPECT_EQ(first[0], 1);
	EXPECT_EQ(first[1], 2);
	EXPECT_EQ(first[2], 4);
	EXPECT_EQ(first[3], 8);
	EXPECT_EQ(first[4], 16);
	EXPECT_TRUE(second == "test string\n");
	EXPECT_EQ(third, 42);


	EXPECT_THROW(iMessage >> first, std::runtime_error);
	EXPECT_THROW(iMessage >> second, std::runtime_error);
	EXPECT_THROW(iMessage >> third, std::runtime_error);

	std::shared_ptr<int> testPtr = std::make_shared<int>(5);
	std::vector<std::shared_ptr<int>> testVectorPtr = {testPtr, testPtr, testPtr};
	EXPECT_THROW(oMessage << testPtr, SerializationError);
	EXPECT_THROW(iMessage >> testPtr, SerializationError);
	EXPECT_THROW(oMessage << testVectorPtr, SerializationError);
	EXPECT_THROW(iMessage >> testVectorPtr, SerializationError);
}