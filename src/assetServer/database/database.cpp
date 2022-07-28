//
// Created by eli on 1/10/2022.
//

#include "database.h"
#include "config/config.h"
#include "assets/assetID.h"

#include "utility/hex.h"
#include "openssl/rand.h"

#define WIN32_LEAN_AND_MEAN
#include <openssl/err.h>
#include <openssl/md5.h>
#include <openssl/ssl.h>
#include <openssl/x509v3.h>
#include <iomanip>
#include "sqlite3.h"


Database::Database()
{
	int rc;
	rc = sqlite3_open(Config::json()["data"]["database_path"].asCString(), &_db);
	if( rc ){
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(_db));
		sqlite3_close(_db);
		throw std::runtime_error("Can't open database");
	}
	_getLastInserted.initialize("SELECT seq FROM sqlite_sequence WHERE name = ?1", _db);
	_loginCall.initialize("SELECT Logins.Password, Logins.Salt FROM Users "
						  "INNER JOIN Logins ON Logins.UserID = Users.UserID WHERE lower(Users.Username)=lower(?1);", _db);
	_userIDCall.initialize("SELECT UserID FROM Users WHERE lower(Username)=lower(?1);", _db);

	_getAssetInfo.initialize("SELECT AssetID, Name, Type, Filename FROM Assets WHERE AssetID = ?1", _db);
	_updateAssetInfo.initialize("UPDATE Assets SET Name = ?2, Type = ?3, Filename = ?4 WHERE AssetID = ?1", _db);
	_insertAssetInfo.initialize("INSERT INTO Assets (Name, Type, Filename) VALUES (?1, ?2, ?3);", _db);
    _moveAssets.initialize("UPDATE Assets SET Filename = ?2 || SUBSTR(Filename, LENGTH(?1), LENGTH(Filename)) WHERE Filename LIKE (?1 || '%')", _db);
	_deleteAsset.initialize("DELETE FROM Assets WHERE AssetID = ?1", _db);
	_fileToAssetID.initialize("SELECT AssetID FROM Assets WHERE Filename = ?1", _db);

	_getAssetPermission.initialize("SELECT Level FROM AssetPermissions WHERE AssetID = ?1 AND UserID = ?2", _db);
	_updateAssetPermission.initialize("INSERT INTO AssetPermissions (AssetID, UserID, Level) VALUES (?1, ?2, ?3) "
	                      " ON CONFLICT DO UPDATE SET Level = ?3 WHERE AssetID = ?1 AND UserID = ?2", _db);
	_deleteAssetPermission.initialize("DELETE FROM AssetPermissions WHERE AssetID = ?1 AND UserID = ?2", _db);

	rawSQLCall("SELECT * FROM Permissions;", [this](const std::vector<Database::sqlColumn>& columns)
	{
		_permissions.insert({std::stoi(columns[0].value), columns[1].value});
	});
}

Database::~Database()
{
	sqlite3_close(_db);
}

int Database::sqliteCallback(void *callback, int argc, char **argv, char **azColName)
{
	std::vector<sqlColumn> columns(argc);
	for (int i = 0; i < argc; ++i)
	{
		columns[i].name = azColName[i];
		columns[i].value = argv[i];
	}
	(*((sqlCallbackFunction*)(callback)))(columns);
	return 0;
}

void Database::rawSQLCall(const std::string& cmd, const sqlCallbackFunction& f)
{
	char *zErrMsg = 0;
	int rc;
	rc = sqlite3_exec(_db, cmd.c_str(), sqliteCallback, (void*)&f, &zErrMsg);
	if( rc!=SQLITE_OK ){
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
}

int64_t Database::getUserID(const std::string& username)
{
	int64_t userID;
	_userIDCall.run(username, std::function([&userID](int64_t id)
	{
		userID = id;
    }));
	return userID;
}

std::unordered_set<std::string> Database::userPermissions(int64_t userID)
{
	std::unordered_set<std::string> pems;
	//TODO change this to prepared call
	rawSQLCall("SELECT PermissionID FROM UserPermissions WHERE UserID=" + std::to_string(userID) + ";",
	           [&pems, this](const std::vector<Database::sqlColumn>& columns)
	           {
		           pems.insert(_permissions[std::stoi(columns[0].value)]);
	           });
	return pems;
}

void Database::insertAssetInfo(AssetInfo& assetInfo)
{
	_insertAssetInfo.run(sqlTEXT(assetInfo.name), assetInfo.type.toString(), assetInfo.filename);
	_getLastInserted.run("Assets", [&assetInfo](sqlINT id){
		assetInfo.id = id;
	});
}

AssetInfo Database::getAssetInfo(uint32_t id)
{
	AssetInfo asset{};
	_getAssetInfo.run(static_cast<sqlINT>(id), [&asset](sqlINT assetID, sqlTEXT name, sqlTEXT type, sqlTEXT filename){
		asset.id = assetID;
		asset.name = std::move(name);
		asset.type.set(type);
		asset.filename = std::move(filename);
	});
	return asset;
}

void Database::insertAssetInfo(const AssetInfo& info)
{
	_updateAssetInfo.run(static_cast<sqlINT>(info.id), info.name, info.type.toString(), info.filename);
}

AssetPermissionLevel Database::getAssetPermission(uint32_t assetID, uint32_t userID)
{
	AssetPermissionLevel level = AssetPermissionLevel::none;
	_getAssetPermission.run(static_cast<sqlINT>(assetID), static_cast<sqlINT>(userID), [&level](sqlINT Level){
		level = (AssetPermissionLevel)Level;
	});
	return level;
}

void Database::setAssetPermission(uint32_t assetID, uint32_t userID, AssetPermissionLevel level)
{
	if(level != AssetPermissionLevel::none)
		_updateAssetPermission.run(static_cast<sqlINT>(assetID), static_cast<sqlINT>(userID), static_cast<sqlINT>(level));
	else
		_deleteAssetPermission.run(static_cast<sqlINT>(assetID), static_cast<sqlINT>(userID));
}

std::vector<AssetInfo> Database::listUserAssets(const uint32_t& userID)
{
	std::string sqlCall ="SELECT Assets.* FROM Assets JOIN AssetPermissions ON Assets.AssetID = AssetPermissions.AssetID WHERE AssetPermissions.UserID = " + std::to_string(userID);
	std::vector<AssetInfo> assets;
	rawSQLCall(sqlCall, [&](const std::vector<Database::sqlColumn>& columns){
		AssetInfo ai;
		ai.id = std::stoi(columns[0].value);
		ai.name = columns[1].value;
		ai.type.set(columns[2].value);
		ai.filename = columns[3].value;
		assets.push_back(ai);
	});
	return assets;
}

std::string Database::assetName(AssetID& id)
{
	std::string name = "name not found";
	if(id.serverAddress != "native")
	{
		std::string sqlCall ="SELECT Name FROM Assets WHERE AssetID = " + std::to_string(id.id);
		rawSQLCall(sqlCall, [&](const std::vector<Database::sqlColumn>& columns){
			name = columns[0].value;
		});
	}
	else
	{

	}
	return name;
}

const char* Database::name()
{
	return "database";
}

bool Database::authenticate(const std::string& username, const std::string& password)
{
	std::string passwordHashTarget;
	std::string salt;
	_loginCall.run(username, [&passwordHashTarget, &salt](std::string h, std::string s){
		passwordHashTarget = h;
		salt = s;
	});
	if(passwordHashTarget.empty())
		return false;
	std::string hashedPassword = hashPassword(password, salt);

	return hashedPassword == passwordHashTarget;
}

std::string Database::hashPassword(const std::string& password, const std::string& salt)
{
	size_t hashIterations = Config::json()["security"].get("hash_iterations", 10000).asLargestInt();
	unsigned char hash[SHA256_DIGEST_LENGTH];
	std::string output = password;
	for (int i = 0; i < hashIterations; ++i)
	{
		output += salt;
		SHA256_CTX sha256;
		SHA256_Init(&sha256);
		SHA256_Update(&sha256, output.c_str(), output.size());
		SHA256_Final(hash, &sha256);
		std::stringstream ss;
		for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
		{
			ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
		}
		output = ss.str();
	}
	return output;
}

std::string Database::randHex(size_t length)
{
	uint8_t* buffer = new uint8_t[length];
	RAND_bytes(buffer, static_cast<int>(length));
	std::stringstream output;
	for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
	{
		output << std::hex << std::setw(2) << std::setfill('0') << (int)buffer[i];
	}
	return output.str();
}

bool Database::fileToAssetID(const std::string& path, AssetID& id)
{
	bool found = false;
	_fileToAssetID.run(path, [&found, &id](sqlINT idInt){
		id.id = idInt;
		found = true;
	});
	return found;
}

void Database::moveAssets(const std::string& oldDir, const std::string& newDir)
{
    _moveAssets.run(oldDir, newDir);
}



