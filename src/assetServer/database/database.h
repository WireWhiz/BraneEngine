#pragma once
#include <config/config.h>
#include <mysqlx/xdevapi.h>
#include <string>

class DatabaseInterface
{
	mysqlx::Session* _session;
public:
	DatabaseInterface();
	~DatabaseInterface();
};