#include "assetServer.h"

namespace net
{
	AssetServerInterface::AssetServerInterface(uint16_t port) : ServerInterface(port)
	{

	}
	bool AssetServerInterface::onClientConnect(std::shared_ptr<Connection> client)
	{
		std::cout << "Client connected\n";
		return true;
	}

	void AssetServerInterface::onClientDissconnect(std::shared_ptr<Connection> client)
	{
		std::cout << "Client disconnected\n";
	}

	void AssetServerInterface::onMessage(std::shared_ptr<Connection> client, IMessage& msg)
	{
		std::cout << "Receved message: " << msg << std::endl;
	}
}