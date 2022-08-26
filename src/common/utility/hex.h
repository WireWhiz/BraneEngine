//
// Created by eli on 2/4/2022.
//

#ifndef BRANEENGINE_HEX_H
#define BRANEENGINE_HEX_H
#include <string>
#include <unordered_map>
#include <cmath>
#include <vector>
#include <array>

const char numToHex[16] = {
		'0',
		'1',
		'2',
		'3',
		'4',
		'5',
		'6',
		'7',
		'8',
		'9',
		'A',
		'B',
		'C',
		'D',
		'E',
		'F'
};

template<typename T>
std::string toHex(T num)
{
	std::string hex;
	hex.resize(sizeof(T) * 2);

	for (uint8_t i = 0; i < sizeof(T); ++i)
	{
		uint8_t currentByte = ((uint8_t*)&num)[sizeof(T) - i - 1];
		hex[i * 2 + 1]     = numToHex[currentByte % 16];
		hex[i * 2] = numToHex[(currentByte / 16) % 16];
	}
	return hex;
}

template<typename T, size_t N>
std::string toHex(std::array<T, N>& array)
{
	std::string hex;
	hex.reserve(array.size() * sizeof(T)  * 2);

	for(auto& num : array)
		hex += toHex(num);
	return hex;
}

template<typename T>
std::string toHex(std::vector<T>& vector)
{
	std::string hex;
	hex.reserve(vector.size() * sizeof(T)  * 2);

	for(auto& num : vector)
		hex.append(toHex(num));
	return hex;
}

template<typename T>
T fromHex(std::string_view hex)
{
	T num = 0;

	for (uint8_t i = 0; i < hex.size(); ++i)
	{
		char c = std::toupper(hex[hex.size() - i - 1]); //Go from right to left
		T currentNum = 0;
		if(47 < c && c < 58) // if number character
			currentNum = c - 48;
		else if(64 < c && c < 71)
			currentNum = c - 65 + 10;
		num += currentNum * std::pow(16, i);
	}
	return num;
}



#endif //BRANEENGINE_HEX_H
