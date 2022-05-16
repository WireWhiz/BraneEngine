//
// Created by eli on 4/27/2022.
//

#include "request.h"
namespace net
{
	Request::Request(const std::string& name, OSerializedData&& body)
	{
		_name = name;
		_body = body;
	}

	std::shared_ptr<net::OMessage> Request::message(uint32_t reqID)
	{
		std::shared_ptr<net::OMessage> message = std::make_shared<net::OMessage>();
		message->header.type = net::MessageType::request;
		message->body << _name << reqID << _body;
		return message;
	}


	RequestResponse::RequestResponse(net::Connection* connection, std::shared_ptr<net::IMessage> message)
	{
		assert(connection);
		assert(message->header.type == net::MessageType::request);
		_connection = connection;
		message->body >> _name >> _id >> _body;
	}

	const std::string& RequestResponse::name()
	{
		return _name;
	}

	ISerializedData& RequestResponse::body()
	{
		return _body;
	}

	OSerializedData& RequestResponse::res()
	{
		return _res;
	}

	void RequestResponse::send()
	{
		assert(_connection);
		std::shared_ptr<net::OMessage> message = std::make_shared<net::OMessage>();
		message->header.type = net::MessageType::response;
		message->body << _id << _res;
		_connection->send(message);
	}

	net::Connection* RequestResponse::sender()
	{
		return _connection;
	}
}