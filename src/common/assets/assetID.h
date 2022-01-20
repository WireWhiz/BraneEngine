#pragma once
#include <string>
#include <iostream>
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
	std::string path() const;

	bool operator==(const AssetID& other) const;
	friend std::ostream& operator <<(std::ostream& os, const AssetID& id);
};



template <>
struct std::hash<AssetID>
{
	size_t operator()(const AssetID& k) const; 
};

