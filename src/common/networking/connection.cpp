#include "connection.h"

#include <utility>

namespace net
{
	Connection::Connection()
	{
		_exists = std::make_shared<bool>(true);
	}

	Connection::~Connection()
	{
		*_exists = false;
	}

	bool Connection::messageAvailable()
	{
		return !_ibuffer.empty();
	}

	std::shared_ptr<IMessage> Connection::popMessage()
	{
		return _ibuffer.pop_front();
	}

	void Connection::sendStreamData(uint32_t id, OSerializedData&& sData)
	{
		auto message = std::make_shared<net::OMessage>();
		message->body << id << sData;
		message->header.type = net::MessageType::streamData;
		send(message);
	}

	void Connection::sendStreamEnd(uint32_t id)
	{
		auto message = std::make_shared<net::OMessage>();
		message->body << id;
		message->header.type = net::MessageType::endStream;
		send(message);
	}

	void Connection::addStreamListener(uint32_t id, std::function<void(ISerializedData&)> callback)
	{
		_streamLock.lock();
		assert(!_streamListeners.count(id));
		_streamListeners.insert({id, callback});
		_streamLock.unlock();
	}

	void Connection::eraseStreamListener(uint32_t id)
	{
		_streamLock.lock();
		_streamListeners.erase(id);
		_streamLock.unlock();
	}

	AsyncData<ISerializedData> Connection::sendRequest(Request&& req)
	{
		return sendRequest(req);
	}

	AsyncData<ISerializedData> Connection::sendRequest(Request& req)
	{
		uint32_t id = _reqIDCounter++;
		AsyncData<ISerializedData> res;
		_responseLock.lock();
		_responseListeners.insert({id, res});
		_responseLock.unlock();

		send(req.message(id));
		return res;
	}



	void Connection::onDisconnect(std::function<void()> f)
	{
		_onDisconnect.push_back(std::move(f));
	}

	template<>
	void ServerConnection<tcp_socket>::connectToClient()
	{
		_address = _socket.remote_endpoint().address().to_string();
		async_readHeader();
	}

	template<>
	void ServerConnection<ssl_socket>::connectToClient()
	{
		_socket.async_handshake(asio::ssl::stream_base::server, [this](std::error_code ec) {
			if (!ec)
			{
				_address = _socket.lowest_layer().remote_endpoint().address().to_string();
				async_readHeader();
			}
			else
			{
				std::cout << "SSL handshake failed: " << ec.message() << std::endl;
			}
		});

	}

	template<>
	void ClientConnection<tcp_socket>::connectToServer(const asio::ip::tcp::resolver::results_type& endpoints, std::function<void()> onConnect, std::function<void()> onFail)
	{

		std::shared_ptr<bool> exists = _exists;
		asio::async_connect(_socket.lowest_layer(), endpoints, [this, onConnect, onFail, exists](std::error_code ec, asio::ip::tcp::endpoint endpoint) {
			if (!ec && *exists)
			{
				_address = _socket.remote_endpoint().address().to_string();
				async_readHeader();
				onConnect();
			}
			else
			{
				std::cerr << "Failed to connect to server." << std::endl;
				onFail();
			}
		});
	}

	template<>
	void ClientConnection<ssl_socket>::connectToServer(const asio::ip::tcp::resolver::results_type& endpoints, std::function<void()> onConnect, std::function<void()> onFail)
	{
		std::shared_ptr<bool> exists = _exists;
		asio::async_connect(_socket.lowest_layer(), endpoints, [this, exists, onConnect , onFail](std::error_code ec, asio::ip::tcp::endpoint endpoint) {
			if (!ec && *exists)
			{
				_address = _socket.lowest_layer().remote_endpoint().address().to_string();
				_socket.async_handshake(asio::ssl::stream_base::client, [this, exists, onConnect, onFail](std::error_code ec) {
					if (!ec && *exists)
					{
						async_readHeader();
						onConnect();
					}
					else
					{
						std::cout << "SSL handshake failed: " << ec.message() << std::endl;
						onFail();
					}
				});
			}
			else
			{
				std::cerr << "Failed to connect to server." << std::endl;
				onFail();
			}
		});
	}
}