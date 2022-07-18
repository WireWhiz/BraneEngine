//
// Created by eli on 4/2/2022.
//
#include "serializedData.h"

MarkedSerializedData::MarkedSerializedData(std::ifstream& file)
{
	//Read json
	uint32_t jsonLength;
	file.read((char*)&jsonLength, sizeof(jsonLength));
	std::string jsonString;
	jsonString.resize(jsonLength);
	file.read(jsonString.data(), (std::streamsize)jsonLength);

	//Parse Json
	Json::CharReaderBuilder builder;
	builder["collectComments"] = false;
	std::string err;
	std::stringstream content(jsonString);
	if(!Json::parseFromStream(builder, content, &attributes, &err))
		throw std::runtime_error("could not read marked serialized data file");

	//read data
	uint32_t dataLength = static_cast<uint32_t>(data.size());
	file.read((char*)&dataLength, sizeof(dataLength));
	data.resize(dataLength);
	file.read((char*)data.data(), (std::streamsize)data.size());

	currentScope = &attributes;
}

MarkedSerializedData::MarkedSerializedData()
{
	currentScope = &attributes;
}

void MarkedSerializedData::writeToFile(std::ofstream& file)
{
	Json::StreamWriterBuilder builder;
	std::string jsonString = Json::writeString(builder, attributes);

	uint32_t jsonLength = static_cast<uint32_t>(jsonString.size());

	file.write((char*)&jsonLength, sizeof(jsonLength));
	file.write(jsonString.data(), (std::streamsize)jsonString.size());

	uint32_t dataLength = static_cast<uint32_t>(data.size());
	file.write((char*)&dataLength, sizeof(dataLength));
	file.write((char*)data.data(), (std::streamsize)data.size());
}

void MarkedSerializedData::enterScope(const std::string& scope)
{
	scopes.push(currentScope);
	currentScope = &(*currentScope)[scope];
}

void MarkedSerializedData::enterScope(int index)
{
	scopes.push(currentScope);
	currentScope = &(*currentScope)[index];
}

void MarkedSerializedData::exitScope()
{
	currentScope = &attributes;
	currentScope = scopes.top();
	scopes.pop();
}

void MarkedSerializedData::startIndex()
{
	scopes.push(currentScope);
	currentScope = new Json::Value;
}

void MarkedSerializedData::pushIndex()
{
	(*scopes.top()).append(std::move(*currentScope));
	delete currentScope;
	currentScope = scopes.top();
	scopes.pop();
}

size_t MarkedSerializedData::scopeSize()
{
	return currentScope->size();
}






