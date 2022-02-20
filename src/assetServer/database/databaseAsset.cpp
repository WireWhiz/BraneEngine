//
// Created by eli on 2/5/2022.
//

#include "databaseAsset.h"
#include "Database.h"

AssetPermission::AssetPermission(uint32_t assetID, uint32_t userID, Database& db) : _db(db)
{
	_level = Level::none;
	_assetID = assetID;
	_userID = userID;
	_db.rawSQLCall("SELECT Level FROM AssetPermissions WHERE AssetID = " + std::to_string(assetID) + " AND UserID = " + std::to_string(userID), [&](std::vector<Database::sqlColumn> columns){
		_level = (Level)std::stoi(columns[0].value);
	});
}

AssetPermission::Level AssetPermission::level()
{
	return _level;
}

void AssetPermission::setLevel(Level level)
{
	if(level == Level::none && _level != Level::none) //If we want to remove this permission, and it exists, delete it.
	{
		_db.rawSQLCall("DELETE FROM AssetPermissions WHERE AssetID = " + std::to_string(_assetID) + " AND UserID = " + std::to_string(_userID), [&](std::vector<Database::sqlColumn> columns){
			_level = (Level)std::stoi(columns[0].value);
		});
		return;
	}

	if(_level != Level::none) //If this level exists, update it
		_db.rawSQLCall("UPDATE AssetPermissions SET Level = " + std::to_string((uint32_t)level) + " WHERE AssetID = " + std::to_string(_assetID) + " AND UserID = " + std::to_string(_userID), [&](std::vector<Database::sqlColumn> columns){
			_level = (Level)std::stoi(columns[0].value);
		});
	else //Otherwise, create it
		_db.rawSQLCall("INSERT INTO AssetPermissions (AssetID, UserID, Level) Values (" + std::to_string(_assetID) + ", " + std::to_string(_userID) + ", "  + std::to_string((uint32_t)level) + ")", [&](std::vector<Database::sqlColumn> columns){
			_level = (Level)std::stoi(columns[0].value);
		});
}

DatabaseAssetDependency::DatabaseAssetDependency(Database& db) : _db(db)
{
	_exists = false;
}

DatabaseAssetDependency::DatabaseAssetDependency(AssetID id, AssetID dep, Database& db) : _db(db)
{
	_exists = true;
	_db.rawSQLCall("SELECT DependencyID, DependencyDomain, Level, Type FROM AssetDependencies INNER JOIN Assets ON AssetDependencies.DependencyID = Assets.AssetID WHERE AssetDependencies.AssetID =" + std::to_string(id.id) +
	" AND AssetDependencies.DependencyID = " + std::to_string(dep.id) + " AND AssetDependencies.DependencyDomain = '" + dep.serverAddress + "';",
	                                        [this](const std::vector<Database::sqlColumn>& columns)
    {
        dependency.id.id = std::stoi(columns[0].value);
	    dependency.id.serverAddress = columns[1].value;
	    dependency.level = (AssetDependency::Level)std::stoi(columns[2].value);
	    dependency.type.set(columns[3].value);
		_exists = true;
    });
}

void DatabaseAssetDependency::save()
{
	if(_exists)
	{
		_db.rawSQLCall("UPDATE AssetDependencies SET DependencyID = " + std::to_string(dependency.id.id) + ", DependencyDomain = '" + dependency.id.serverAddress + "', Level = " + std::to_string((uint32_t)dependency.level) + " WHERE AssetID = " + std::to_string(id.id), [&](std::vector<Database::sqlColumn> columns){

		});
	}
	else
	{
		_db.rawSQLCall("INSERT INTO AssetDependencies (AssetID, DependencyID, DependencyDomain, Level) Values (" + std::to_string(id.id) + ", " + std::to_string(dependency.id.id) + ", '" + dependency.id.serverAddress + "', " + std::to_string((uint32_t)dependency.level) + ")", [&](std::vector<Database::sqlColumn> columns){

		});
		_exists = true;
	}
}

void DatabaseAssetDependency::del()
{
	if(!_exists)
		return;
	_db.rawSQLCall("DELETE FROM AssetDependencies WHERE AssetID = " + std::to_string(id.id), [&](std::vector<Database::sqlColumn> columns){
	});
	_exists = false;
}

AssetData::AssetData(Database& db) : _db(db)
{
	_exists = false;
}

AssetData::AssetData(AssetID id, Database& db) : _db(db)
{
	this->id = id;
	_exists = false;
	_db.rawSQLCall("SELECT Name, Type FROM Assets WHERE AssetID = " + std::to_string(id.id), [&](std::vector<Database::sqlColumn> columns){
		name = columns[0].value;
		type.set(columns[1].value);
		_exists = true;
	});
}

void AssetData::save()
{
	if(_exists)
		_db.rawSQLCall("UPDATE Assets SET Name = '" + name + "', Type = '" + type.string() + "' WHERE AssetID = " + std::to_string(id.id), [&](std::vector<Database::sqlColumn> columns){});
	else
		_db.rawSQLCall("INSERT INTO Assets (Name, Type) VALUES ('" + name + "', '" + type.string() + "'); SELECT last_insert_rowid()", [&](std::vector<Database::sqlColumn> columns){
			id.id = std::stoi(columns[0].value);
		});
}

void AssetData::del()
{
	if(_exists)
	{
		_db.rawSQLCall("DELETE FROM Assets WHERE AssetID = " + std::to_string(id.id), [&](std::vector<Database::sqlColumn> columns){
		});
	}
	_exists = false;
}



