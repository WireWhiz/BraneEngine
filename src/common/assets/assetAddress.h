#pragma once
#include <string>
#include "assetTypes.h"

struct AssetAddress
{
	std::string serverAddress;
	std::string owner;
	AssetType type;
	std::string name;
	void parseString(const std::string& id);
	std::string string();
};