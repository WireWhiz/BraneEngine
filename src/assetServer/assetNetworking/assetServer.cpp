//
// Created by eli on 3/3/2022.
//

#include "assetServer.h"

AssetServer::AssetServer(NetworkManager& nm, AssetManager& am) : _nm(nm), _am(am)
{
	if(!Config::json()["network"]["use_ssl"].asBool())
	{
		std::cout << "Started listening for asset requests on port: " << Config::json()["network"]["tcp_port"].asUInt() << std::endl;
		_nm.openAcceptor<net::tcp_socket>(Config::json()["network"]["tcp_port"].asUInt(), [this](std::unique_ptr<net::Connection>&& connection){
			std::cout << "User connected to tcp" << std::endl;
			std::scoped_lock lock(_cLock);
			_connections.push_back(std::move(connection));
		});
	}
	else
	{
		std::cout << "Started listening for asset requests on port: " << Config::json()["network"]["ssl_port"].asUInt() << std::endl;
		_nm.openAcceptor<net::ssl_socket>(Config::json()["network"]["ssl_port"].asUInt(), [this](std::unique_ptr<net::Connection>&& connection){
			std::cout << "User connected to ssl" << std::endl;
			std::scoped_lock lock(_cLock);
			_connections.push_back(std::move(connection));
		});
	}

}

void AssetServer::processMessages()
{
	std::scoped_lock lock(_cLock);
	for(size_t i = 0; i < _connections.size(); i++)
	{
		auto& connection = _connections[i];
		if(connection->connected())
		{
			std::shared_ptr<net::IMessage> message;
			while(connection->popIMessage(message))
			{
				std::cout << "received message: " << message;
				if(message->header.type == net::MessageType::assetRequest)
				{
					std::string requestedAssetString;
					message->body >> requestedAssetString;
					AssetID requestedAsset(requestedAssetString);
					std::cout << "request for: " << requestedAssetString << std::endl;
					std::shared_ptr<net::OMessage> res = std::make_shared<net::OMessage>();
					Asset* asset = _am.getAsset<Asset>(requestedAsset);
					if(asset)
					{
						asset->serialize(res->body);
						res->header.type = net::MessageType::assetData;
						connection->send(res);
					}

				}
			}
		}
	}
}
