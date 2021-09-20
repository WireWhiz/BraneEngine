#include "assetServer.h"

namespace net
{
	AssetServerInterface::AssetServerInterface(uint16_t port, uint16_t ssl_port) : ServerInterface(port, ssl_port)
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
		std::string text;
		text.resize(msg.header.size);
		msg.read(text.data(), msg.header.size);
		std::cout << "receved: " << text;

		OMessage response;
		switch (msg.header.type)
		{
			case MessageType::one:
				text = "General Kenobi!";
				response.write(text.data(), text.size());
				messageClient(client, response);
				
				break;
		}
	}
}