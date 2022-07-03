//
// Created by eli on 2/5/2022.
//

#ifndef BRANEENGINE_DATABASEASSET_H
#define BRANEENGINE_DATABASEASSET_H


#include <assets/asset.h>
#include "common/assets/assetType.h"

class Database;

enum class AssetPermissionLevel
{
	none = 0,
	view = 1,
	edit = 2,
	owner = 3
};

struct AssetInfo
{
	uint32_t id;
	std::string filename;
	std::string name;
	AssetType type;
};


#endif //BRANEENGINE_DATABASEASSET_H
