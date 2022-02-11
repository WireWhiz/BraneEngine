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

AssetData::AssetData(Database& db) : _db(db)
{
	_exists = false;
}

AssetData::AssetData(AssetID id, Database& db) : _db(db)
{
	this->id = id;
	_exists = false;
	_db.rawSQLCall("SELECT Name, SourceFile FROM Assets WHERE AssetID = " + std::to_string(id.id), [&](std::vector<Database::sqlColumn> columns){
		name = columns[0].value;
		sourceFile = columns[1].value;
		_exists = true;
	});
}

void AssetData::save()
{
	if(_exists)
		_db.rawSQLCall("UPDATE Assets SET Name = '" + name + "', SourceFile = '" + sourceFile + "' WHERE AssetID = " + std::to_string(id.id), [&](std::vector<Database::sqlColumn> columns){

		});
	else
		_db.rawSQLCall("INSERT INTO Assets (Name, SourceFile) VALUES ('" + name + "', '" + sourceFile + "'); SELECT last_insert_rowid()", [&](std::vector<Database::sqlColumn> columns){
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






