#include <sstream>
#include "assetID.h"
#include <utility/hex.h>
#include <cassert>

const AssetID AssetID::null("null");

AssetID::AssetID(const std::string& id)
{
	parseString(id);
}

void AssetID::parseString(const std::string& id)
{
	if(id == "null"){
		serverAddress = "null";
		return;
	}
	std::string strings[2] = {"", ""};
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
	this->id = fromHex<uint32_t>(strings[1]);
}

std::string AssetID::string() const
{
	if(serverAddress == "null")
		return "null";
	return serverAddress + '/' + toHex(id);
}

bool AssetID::operator==(const AssetID& other) const
{

	if (id != other.id)
		return false;
	return serverAddress == other.serverAddress;
}

bool AssetID::operator!=(const AssetID& other) const
{
	if (id != other.id)
		return true;
	return serverAddress != other.serverAddress;
}

std::ostream& operator<<(std::ostream& os, const AssetID& id)
{
	os << id.string();
	return os;
}

uint32_t AssetID::size()
{
	uint32_t size = 3; //Separating lines
	size += serverAddress.size();
	size += sizeof(id) * 2;
	return size;
}

AssetID::AssetID(const std::string& serverAddress, uint32_t id)
{
	this->serverAddress = serverAddress;
	this->id = id;
}

bool AssetID::isNull() const
{
    return this->serverAddress == "null";
}

std::size_t std::hash<AssetID>::operator()(const AssetID& k) const
{

	return (std::hash<string>()(k.serverAddress)
		^ (std::hash<uint64_t>()(k.id) << 1));
}