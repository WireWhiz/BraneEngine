// Asset server
#include <iostream>
#include <networking/networking.h>
#include <common/config/config.h>
#include <json/json.h>
#include <fstream>
#include <string>

#include "assetNetworking/assetServer.h"
#include "database/database.h"

int main()
{
	Config::loadConfig();

	DatabaseInterface di;

	uint16_t tcpPort = Config::json()["network"].get("tcp port", 80).asUInt();
	uint16_t sslPort = Config::json()["network"].get("ssl port", 81).asUInt();
	
	net::AssetServerInterface si(tcpPort, sslPort);
	si.start();

	while (true)
	{
		//std::this_thread::sleep_for(std::chrono::milliseconds(200));
		si.update();
	}

	return 0;
}