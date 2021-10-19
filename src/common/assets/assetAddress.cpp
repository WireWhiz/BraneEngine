#include "assetAddress.h"

void AssetAddress::parseString(const std::string& id)
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

std::string AssetAddress::string()
{
	return serverAddress + '/' + owner + '/' + type.string() + '/' + name;
}


