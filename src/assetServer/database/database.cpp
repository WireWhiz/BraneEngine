#include "database.h"

DatabaseInterface::DatabaseInterface()
{
	std::string ip = Config::json()["database"].get("ip", "127.0.0.1").asString();
	uint32_t port = Config::json()["database"].get("port", 33060).asUInt();
	std::string user = Config::json()["database"].get("user", "assetServer").asString();
	std::string password = Config::json()["database"]["password"].asString();

	try
	{
		std::cout << "Connecting to database: " << ip << ":" << port << " user: " << user << std::endl;
		_session = new mysqlx::Session(ip, port, user, password);

	}
	catch (const std::exception& e)
	{
		std::cerr << "Couldn't connect to database: " << e.what() << std::endl;
		throw e;
	}
	std::cout << "Conneced to database" << std::endl;


}

DatabaseInterface::~DatabaseInterface()
{
	if (_session)
		_session->close();
}
