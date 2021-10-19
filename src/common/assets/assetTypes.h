#pragma once
#include <string_view>
#include <string>
#include <cstdint>
#include <array>

class AssetType
{
	uint8_t _index;
	static const std::array<std::string, 3> _types;
public:
	AssetType();
	void set(const std::string& type);
	void set(uint8_t type);
	const std::string& string();
};