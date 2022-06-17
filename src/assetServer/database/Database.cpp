//
// Created by eli on 1/10/2022.
//

#include "Database.h"
#include "utility/hex.h"
#include "openssl/rand.h"

#define WIN32_LEAN_AND_MEAN
#include <openssl/err.h>
#include <openssl/md5.h>
#include <openssl/ssl.h>
#include <openssl/x509v3.h>
#include <iomanip>

Database::Database()
{
	int rc;
	rc = sqlite3_open(Config::json()["data"]["database_path"].asCString(), &_db);
	if( rc ){
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(_db));
		sqlite3_close(_db);
		throw std::runtime_error("Can't open database");
	}
	_loginCall.initialize("SELECT Logins.Password, Logins.Salt, Logins.UserID FROM Users INNER JOIN Logins ON Logins.UserID = Users.UserID WHERE lower(Users.Username)=lower(?1);", _db);
	_userIDCall.initialize("SELECT UserID FROM Users WHERE lower(Username)=lower(?1);", _db);
	_directoryChildren.initialize("SELECT * FROM AssetDirectories WHERE ParentFolder = ?1;", _db);
	_directoryAssets.initialize("SELECT * FROM Assets WHERE DirectoryID = ?1;", _db);

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

bool Database::stringSafe(const std::string& str)
{
	for (int i = 0; i < str.size(); ++i)
	{
		if(str[i] == '"' || str[i] == '\'')
			return false;
	}
	return true;
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

AssetInfo Database::loadAssetData(const AssetID& id)
{
	return AssetInfo(id, *this);
}

AssetPermission Database::assetPermission(const uint32_t assetID, const uint32_t userID)
{
	return AssetPermission(assetID, userID, *this);
}

std::vector<AssetInfo> Database::listUserAssets(const uint32_t& userID)
{
	std::string sqlCall ="SELECT Assets.* FROM Assets JOIN AssetPermissions ON Assets.AssetID = AssetPermissions.AssetID WHERE AssetPermissions.UserID = " + std::to_string(userID);
	std::vector<AssetInfo> assets;
	rawSQLCall(sqlCall, [&](const std::vector<Database::sqlColumn>& columns){
		AssetInfo ad(*this);
		AssetID aid;
		aid.id = std::stoi(columns[0].value);
		ad.id = aid;
		ad.folderID = std::stoi(columns[1].value);
		ad.name = columns[2].value;
		ad.type.set(columns[3].value);
		assets.push_back(ad);
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
	_loginCall.run("WireWhiz", std::function([&passwordHashTarget, &salt](std::string h, std::string s){
		passwordHashTarget = h;
		salt = s;
	}));
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
	RAND_bytes(buffer, length);
	std::stringstream output;
	for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
	{
		output << std::hex << std::setw(2) << std::setfill('0') << (int)buffer[i];
	}
	return output.str();
}

Database::Directory Database::directoryTree()
{
	Directory tree;
	tree.id = 0;
	tree.name = "root";
	tree.parent = -1;
	populateDirectoryChildren(tree);

	return std::move(tree);
}

void Database::populateDirectoryChildren(Database::Directory& dir)
{
	_directoryChildren.run(dir.id, std::function([&dir](int FolderID, std::string Name, int ParentFolder){
		dir.children.push_back(Directory{FolderID, std::move(Name), ParentFolder});
	}));
	for(auto& child : dir.children)
		populateDirectoryChildren(child);
}

std::vector<Database::Asset> Database::directoryAssets(int directoryID)
{
	std::vector<Database::Asset> assets;
	_directoryAssets.run(directoryID, std::function([&assets](int AssetID, std::string Name, std::string Type, int DirectoryID){
		assets.push_back({AssetID, std::move(Name), std::move(Type), DirectoryID});
	}));
	return assets;
}


void Database::Directory::serialize(OSerializedData& sData)
{
	sData << (uint64_t)id << name << (uint16_t)children.size();
	for(auto& child : children){
		child.serialize(sData);
	}
}
