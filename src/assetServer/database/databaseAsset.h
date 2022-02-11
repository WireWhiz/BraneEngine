//
// Created by eli on 2/5/2022.
//

#ifndef BRANEENGINE_DATABASEASSET_H
#define BRANEENGINE_DATABASEASSET_H


#include <assets/asset.h>

class Database;

class AssetPermission
{

public:
	enum class Level
	{
		none = 0,
		view = 1,
		edit = 2,
		owner = 3
	};

private:
	uint32_t _assetID;
	uint32_t _userID;
	Level _level;
	Database& _db;

public:
	AssetPermission(uint32_t assetID, uint32_t userID, Database& db);
	Level level();
	void setLevel(Level level);
};

class AssetData
{
	Database& _db;
	bool _exists;
public:
	AssetID     id;
	std::string name;
	std::string sourceFile;
	AssetData(Database& db);
	AssetData(AssetID id, Database& db);
	void save();
	void del();
};


#endif //BRANEENGINE_DATABASEASSET_H
