#include "assetServer.h"

namespace net
{
	AssetServerInterface::AssetServerInterface(uint16_t port, uint16_t ssl_port) : ServerInterface(port, ssl_port)
	{
		_ssl_ctx.set_options(
			asio::ssl::context::default_workarounds
			| asio::ssl::context::no_sslv2
			| asio::ssl::context::single_dh_use

		);
		try
		{
			_ssl_ctx.use_certificate_chain_file("keys\\user.crt");
			_ssl_ctx.use_tmp_dh_file("keys\\dh2048.pem");
			_ssl_ctx.use_private_key_file("keys\\user.key", asio::ssl::context::file_format::pem);
			/*
			std::ifstream tmp_dh_file("keys/dh2048.pem", std::ios::binary | std::ios::ate);
			if (!tmp_dh_file.is_open())
				throw;
			size_t size = tmp_dh_file.tellg();
			tmp_dh_file.seekg(0);

			std::vector<char> tmp_dh(size);
			tmp_dh_file.read(tmp_dh.data(), tmp_dh.size());

			_ssl_ctx.use_tmp_dh(asio::const_buffer(tmp_dh.data(), tmp_dh.size()));
			*/
		}
		catch (const std::exception& e)
		{
			std::cerr << "Couldn't read file: " << e.what() << std::endl;
		}
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