#pragma once
#include <string>
#include <iostream>

class AssetID
{
public:
	AssetID() = default;
	AssetID(const std::string& id);
	AssetID(const std::string& serverAddress, uint32_t id);
	uint32_t id;
	std::string serverAddress;
	void parseString(const std::string& id);
	uint32_t size();
	std::string string() const;

	bool operator==(const AssetID& other) const;
	friend std::ostream& operator <<(std::ostream& os, const AssetID& id);
};



template <>
struct std::hash<AssetID>
{
	size_t operator()(const AssetID& k) const; 
};

