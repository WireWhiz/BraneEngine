#pragma once
#include <networking/server.h>

namespace net
{
	class AssetServerInterface : public ServerInterface
	{
	public:
		AssetServerInterface(uint16_t port);
	protected:
		bool onClientConnect(std::shared_ptr<Connection> client) override;
		void onClientDissconnect(std::shared_ptr<Connection> client) override;
		void onMessage(std::shared_ptr<Connection> client, IMessage& msg) override;
	};
}