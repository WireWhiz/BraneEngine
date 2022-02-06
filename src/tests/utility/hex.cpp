//
// Created by eli on 2/4/2022.
//
#include "testing.h"
#include <utility/hex.h>

TEST(HEX, ConvertToHexTest)
{
	uint32_t testNum = 0xABC12345;
	std::string testHex = toHex(testNum);
	std::cout << "Number: " << testNum << " Hex: " << testHex << std::endl;
	EXPECT_EQ(testHex, "ABC12345");
}

TEST(HEX, ConvertFromHexTest)
{
	std::string testHex = "ABC12345";
	uint32_t testNum = fromHex<uint32_t>(testHex);
	std::cout  << " Hex: " << testHex << " Number: " << testNum << std::endl;
	EXPECT_EQ(testNum, 0xABC12345);
}