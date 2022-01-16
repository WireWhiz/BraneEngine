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
	/*
	sqlCall("CREATE TABLE Users (UserID int, Username varchar(16));", [this](int argc, char **argv, char **azColName){
		std::cout << "Created table: " << argc << std::endl;
		return 0;
	});
	sqlCall("INSERT INTO Users (UserID, Username) VALUES (0, \"WireWhiz\");", [this](int argc, char **argv, char **azColName){
		std::cout << "Inserted user: " << argc << std::endl;
		return 0;
	});
	sqlCall("SELECT UserID, Username FROM Users;", [this](int argc, char **argv, char **azColName){
		std::cout << "Selecting Users, arguments returned: " << argc << std::endl;
		for (int i = 0; i < argc; ++i)
		{
			std::cout << "arg " << i << " = " << argv[i] << std::endl;
		}
		return 0;
	});
	 */

	sqlCall("SELECT * FROM Permissions;", [this](const std::vector<Database::sqlColumn>& columns){
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

void Database::sqlCall(const std::string& cmd, const sqlCallbackFunction& f)
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
	sqlCall("SELECT UserID FROM Users WHERE lower(Username)=lower('" + username + "');", [&userID](const std::vector<Database::sqlColumn>& columns){
		userID = columns[0].value;
	});
	return userID;
}

std::unordered_set<std::string> Database::userPermissions(const std::string& userID)
{
	std::unordered_set<std::string> pems;
	sqlCall("SELECT PermissionID FROM UsersToPermissions WHERE UserID=" + userID + ";", [&pems, this](const std::vector<Database::sqlColumn>& columns){
		pems.insert(_permissions[std::stoi(columns[0].value)]);
	});
	return pems;
}
