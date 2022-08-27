#pragma once
#include <string>
#include <iostream>

class AssetID
{
public:
	AssetID() = default;
	AssetID(const std::string& id);
	AssetID(const std::string& serverAddress, uint32_t id);
	uint32_t id = 0;
	std::string serverAddress = "null";
	void parseString(const std::string& id);
	uint32_t size();
	std::string string() const;
    bool empty() const;

	bool operator==(const AssetID& other) const;
	bool operator!=(const AssetID& other) const;
	friend std::ostream& operator << (std::ostream& os, const AssetID& id);
	static const AssetID null;
};



template <>
struct std::hash<AssetID>
{
	size_t operator()(const AssetID& k) const; 
};

