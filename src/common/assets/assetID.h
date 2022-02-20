#pragma once
#include <string>
#include <iostream>

struct AssetID
{
	AssetID() = default;
	AssetID(const std::string& id);
	uint64_t id;
	std::string serverAddress;
	void parseString(const std::string& id);
	uint32_t size();
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

