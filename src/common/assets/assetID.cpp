#include "assetID.h"

AssetID::AssetID(const std::string& id)
{
	parseString(id);
}

void AssetID::parseString(const std::string& id)
{
	std::string* strings = new std::string[4];
	uint8_t strIndex = 0;
	for (size_t i = 0; i < id.size(); i++)
	{
		if (id[i] != '/')
			strings[strIndex] += id[i];
		else
		{
			strIndex++;
		}
	}
	serverAddress = strings[0];
	owner = strings[1];
	type.set(strings[2]);
	name = strings[3];
}

std::string AssetID::string() const
{
	return serverAddress + '/' + owner + '/' + type.string() + '/' + name;
}

bool AssetID::operator==(const AssetID& other) const
{
	if (name != other.name)
		return false;
	if (type != other.type)
		return false;
	if (owner != other.owner)
		return false;
	return serverAddress == other.serverAddress;
}

std::string AssetID::path() const
{
	return "assets/" + owner + "/" + type.string() + "/" + name + ".asset";
}

std::size_t std::hash<AssetID>::operator()(const AssetID& k) const
{
	using std::size_t;
	using std::hash;
	using std::string;

	// Compute individual hash values for first,
	// second and third and combine them using XOR
	// and bit shifting:

	return ((hash<string>()(k.serverAddress)
			 ^ (hash<string>()(k.owner) << 1)) >> 1)
		^ (hash<AssetType::Type>()(k.type.type()) << 1)
		^ (hash<string>()(k.name) << 1);
}