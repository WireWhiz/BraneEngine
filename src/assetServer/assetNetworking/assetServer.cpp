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
				std::cout << "received message: " << message << std::endl;
				if(message->header.type == net::MessageType::assetRequest)
				{
					AssetRequest ar{};
					ar.fromMessage(message);

					std::cout << "request for: " << ar.id.string() << std::endl;

					std::shared_ptr<net::OMessage> res = std::make_shared<net::OMessage>();
					Asset* asset = nullptr;
					try{
						asset = _am.getAsset<Asset>(ar.id);

					}
					catch(const std::exception& e)
					{
						std::cerr << "Failed to read asset: " << e.what();
					}

					if(asset)
					{
						if(!ar.incremental)
						{
							asset->serialize(res->body);
							res->header.type = net::MessageType::assetData;
							connection->send(res);
						}
						else
						{
							auto* ia = dynamic_cast<IncrementalAsset*>(asset);
							if(ia)
							{
								std::cout<< "Sending header for: " << ia->id << std::endl;
								ia->serializeHeader(res->body);
								res->header.type = net::MessageType::assetIncrementalHeader;
								connection->send(res);

								IncrementalAssetSender assetSender{};
								assetSender.asset = ia;
								assetSender.dest = connection.get();
								_senders.push_back(assetSender);
							}
							else
								std::cerr << "Tried to request non-incremental asset as incremental" << std::endl;
						}

					}
				}
			}
		}
	}
	//Send one increment from every incremental asset that we are sending, to create the illusion of them loading in parallel
	_senders.remove_if([&](auto& sender)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

		std::shared_ptr<net::OMessage> o = std::make_shared<net::OMessage>();
		o->header.type = net::MessageType::assetIncrementalData;
		bool moreData = sender.asset->serializeIncrement(o->body, sender.iteratorData);
		sender.dest->send(o);

		return !moreData;
	});
}

AssetServer::~AssetServer()
{
	for(auto& c : _connections)
		c->disconnect();
}
