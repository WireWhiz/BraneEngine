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
	return (*((sqlCallbackFunction*)(callback)))(columns);
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
