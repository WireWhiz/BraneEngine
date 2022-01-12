//
// Created by eli on 1/10/2022.
//

#ifndef BRANEENGINE_DATABASE_H
#define BRANEENGINE_DATABASE_H
#include <sqlite/sqlite3.h>
#include <config/config.h>
#include <functional>

class Database
{

	sqlite3* _db;
	static int sqliteCallback(void *callback, int argc, char **argv, char **azColName);
public:
	struct sqlColumn
	{
		char* name;
		char* value;
	};
	typedef std::function<int(const std::vector<sqlColumn>& columns)> sqlCallbackFunction;
	Database();
	~Database();
	void sqlCall(const std::string& cmd, const sqlCallbackFunction& f);
	bool stringSafe(const std::string& str);
};


#endif //BRANEENGINE_DATABASE_H
