// Asset server
#include <iostream>
#include <networking/networking.h>
#include <common/config/config.h>
#include <json/json.h>
#include <fstream>
#include <string>

int main()
{
	Config::loadConfig();

	std::cout << Config::json()["network"].get("port", 80).asUInt();
	net::NetworkManager nm;


	return 0;
}