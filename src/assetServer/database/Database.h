//
// Created by eli on 1/10/2022.
//

#ifndef BRANEENGINE_DATABASE_H
#define BRANEENGINE_DATABASE_H
#include <sqlite/sqlite3.h>
#include <config/config.h>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include "databaseAsset.h"
#include "runtime/module.h"
#include "PreppedSQLCall.h"


class Database : public Module
{
	sqlite3* _db;
	PreppedSQLCall<const std::string&> _loginCall;
	PreppedSQLCall<const std::string&> _userIDCall;
	static int sqliteCallback(void *callback, int argc, char **argv, char **azColName);

	std::string randHex(size_t length);
	std::string hashPassword(const std::string& password, const std::string& salt);

	std::unordered_map<size_t, std::string> _permissions;
public:
	struct sqlColumn
	{
		char* name;
		char* value;
	};
	typedef std::function<void(const std::vector<sqlColumn>& columns)> sqlCallbackFunction;
	Database(Runtime& runtime);
	~Database();
	void rawSQLCall(const std::string& cmd, const sqlCallbackFunction& f);
	bool stringSafe(const std::string& str);

	bool authenticate(const std::string& username, const std::string& password);
	int64_t getUserID(const std::string& username);
	std::unordered_set<std::string> userPermissions(int64_t userID);

	AssetInfo loadAssetData(const AssetID& id);
	AssetPermission assetPermission(const uint32_t assetID, const uint32_t userID);
	std::string assetName(AssetID& id);

	std::vector<AssetInfo> listUserAssets(const uint32_t& userID);

	const char* name() override;
};


#endif //BRANEENGINE_DATABASE_H
