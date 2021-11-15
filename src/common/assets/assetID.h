#pragma once
#include <string>
#include "assetType.h"

struct AssetID
{
	AssetID() = default;
	AssetID(const std::string& id);
	std::string name;
	AssetType type;
	std::string owner;
	std::string serverAddress;
	void parseString(const std::string& id);
	std::string string() const;

	bool operator==(const AssetID& other) const;
};


template <>
struct std::hash<AssetID>
{
	size_t operator()(const AssetID& k) const; 
};

