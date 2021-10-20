#pragma once
#include <string>
#include "assetType.h"

struct AssetID
{
	AssetID() = default;
	AssetID(const std::string& id);
	std::string serverAddress;
	std::string owner;
	AssetType type;
	std::string name;
	void parseString(const std::string& id);
	std::string string();

	bool operator==(const AssetID& other) const;
};


template <>
struct std::hash<AssetID>
{
	size_t operator()(const AssetID& k) const; 
};

