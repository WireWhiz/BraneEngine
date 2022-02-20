//
// Created by eli on 1/10/2022.
//

#include "Database.h"

Database::Database()
{
	int rc;
	rc = sqlite3_open(Config::json()["database"]["path"].asCString(), &_db);
	if( rc ){
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(_db));
		sqlite3_close(_db);
		throw std::runtime_error("Can't open database");
	}

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

std::string Database::getUserID(const std::string& username)
{
	std::string userID;
	rawSQLCall("SELECT UserID FROM Users WHERE lower(Username)=lower('" + username + "');",
	           [&userID](const std::vector<Database::sqlColumn>& columns)
	           {
		           userID = columns[0].value;
	           });
	return userID;
}

std::unordered_set<std::string> Database::userPermissions(const std::string& userID)
{
	std::unordered_set<std::string> pems;
	rawSQLCall("SELECT PermissionID FROM UserPermissions WHERE UserID=" + userID + ";",
	           [&pems, this](const std::vector<Database::sqlColumn>& columns)
	           {
		           pems.insert(_permissions[std::stoi(columns[0].value)]);
	           });
	return pems;
}

AssetData Database::loadAssetData(const AssetID& id)
{
	return AssetData(id, *this);
}

AssetPermission Database::assetPermission(const uint32_t assetID, const uint32_t userID)
{
	return AssetPermission(assetID, userID, *this);
}

std::vector<AssetDependency> Database::assetDependencies(const uint32_t assetID)
{
	std::vector<AssetDependency> deps;

	rawSQLCall("SELECT DependencyID, DependencyDomain, Level, Type FROM AssetDependencies INNER JOIN Assets ON AssetDependencies.DependencyID = Assets.AssetID WHERE AssetDependencies.AssetID =" + std::to_string(assetID) + ";",
	                                                                        [&deps, this](const std::vector<Database::sqlColumn>& columns)
    {
		AssetDependency dep;
		dep.id.id = std::stoi(columns[0].value);
		dep.id.serverAddress = columns[1].value;
		dep.level = (AssetDependency::Level)std::stoi(columns[2].value);
        dep.type.set(columns[3].value);
		deps.push_back(dep);
    });

	return deps;
}

std::vector<AssetData> Database::listUserAssets(const uint32_t& userID, bool showDeps)
{
	std::string sqlCall = "";
	if(!showDeps)
		sqlCall ="SELECT Assets.* FROM Assets LEFT JOIN AssetDependencies ON Assets.AssetID = AssetDependencies.DependencyID JOIN AssetPermissions ON Assets.AssetID = AssetPermissions.AssetID WHERE AssetDependencies.DependencyID IS NULL";
	else
		sqlCall ="SELECT Assets.* FROM Assets JOIN AssetPermissions ON Assets.AssetID = AssetPermissions.AssetID";
	std::vector<AssetData> assets;
	rawSQLCall(sqlCall, [&](const std::vector<Database::sqlColumn>& columns){
		AssetData ad(*this);
		AssetID aid;
		aid.id = std::stoi(columns[0].value);
		ad.id = aid;
		ad.name = columns[1].value;
		ad.type.set(columns[2].value);
		assets.push_back(ad);
	});
	return assets;
}


