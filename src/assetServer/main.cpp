// Asset server
#include <iostream>
#include <networking/networking.h>
#include <common/config/config.h>
#include <json/json.h>
#include <fstream>
#include <string>

#include "assetNetworking/assetServer.h"

int main()
{
	Config::loadConfig();

	uint16_t serverPort = Config::json()["network"].get("tcp port", 80).asUInt();
	
	net::AssetServerInterface si(serverPort);
	si.start();

	while (true)
	{
		//std::this_thread::sleep_for(std::chrono::milliseconds(200));
		si.update();
	}

	return 0;
}