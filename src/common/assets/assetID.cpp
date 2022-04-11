#include <sstream>
#include "assetID.h"
#include <utility/hex.h>
#include <cassert>

AssetID::AssetID(const std::string& id)
{
	parseString(id);
}

void AssetID::parseString(const std::string& id)
{
	std::string* strings = new std::string[2];
	uint8_t strIndex = 0;
	for (size_t i = 0; i < id.size(); i++)
	{
		if (id[i] != '/')
			strings[strIndex] += id[i];
		else
		{
			strIndex++;
			assert(strIndex < 2);
		}
	}
	serverAddress = strings[0];
	this->id = fromHex<uint64_t>(strings[1]);
}

std::string AssetID::string() const
{
	std::stringstream stream;
	stream << serverAddress << '/' << toHex(id);
	return stream.str();
}

bool AssetID::operator==(const AssetID& other) const
{

	if (id != other.id)
		return false;
	return serverAddress == other.serverAddress;
}

std::ostream& operator<<(std::ostream& os, const AssetID& id)
{
	os << id.string();
	return os;
}
#include <config/config.h>
std::string AssetID::path() const
{
	return Config::json()["data"]["asset_path"].asString() + "/" + toHex(id) + ".asset";
}

uint32_t AssetID::size()
{
	uint32_t size = 3; //Separating lines
	size += serverAddress.size();
	size += sizeof(id) * 2;
	return size;
}

std::size_t std::hash<AssetID>::operator()(const AssetID& k) const
{
	using std::size_t;
	using std::hash;
	using std::string;

	// Compute individual hash values for first,
	// second and third and combine them using XOR
	// and bit shifting:

	return (hash<string>()(k.serverAddress)
		^ (hash<uint64_t>()(k.id) << 1));
}