#pragma once
#include <iostream>
#include <unordered_map>

void testAssert(bool value)
{
	if (!value)
		exit(1);
}

template<typename T>
void testAssert(bool value, T message)
{
	if (!value)
	{
		std::cerr << message << std::endl;
		exit(1);
	}
}