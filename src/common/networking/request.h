//
// Created by eli on 4/27/2022.
//

#ifndef BRANEENGINE_REQUEST_H
#define BRANEENGINE_REQUEST_H

#include <string>
#include <utility/threadPool.h>
#include "message.h"
#include "connection.h"
#include <utility/asyncData.h>

namespace net{
	class Connection;

	class Request
	{
		std::string _name;
		OSerializedData _body;
		AsyncData<ISerializedData> _data;
	public:
		Request(const std::string& name);
		std::shared_ptr<net::OMessage> message(uint32_t reqID);
		OSerializedData& body();
	};

	class RequestResponse
	{
		std::string _name;
		uint32_t _id;
		ISerializedData _body;
		OSerializedData _res;
		Connection* _connection;
	public:
		RequestResponse(Connection* connection, std::shared_ptr<net::IMessage> message);
		const std::string& name();
		ISerializedData& body();
		OSerializedData& res();
		void send();
		Connection* sender();
	};
}



#endif //BRANEENGINE_REQUEST_H
