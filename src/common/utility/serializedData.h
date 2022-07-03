#pragma once
#include <cstdint>
#include <vector>
#include <sstream>
#include <byte.h>
#include <assets/assetID.h>
#include <cstring>
#include <regex>
#include <typeinfo>
#include <json/json.h>
#include <fstream>
#include <cassert>
#include <utility/inlineArray.h>

class SerializationError : virtual public std::runtime_error
{
public:
	explicit SerializationError(const std::type_info& t) : std::runtime_error(std::string("type: ") + t.name() + " could not be serialized")
	{

	}
};

class ISerializedData
{
	size_t _ittr;
public:
	ISerializedData(){
		_ittr = 0;
	}
	std::vector<byte> data;

	[[nodiscard]] size_t size() const
	{
		return data.size();
	}

	friend std::ostream& operator << (std::ostream& os, const ISerializedData& msg)
	{
		os << " Serialized Data: ";
		for (int i = 0; i < msg.data.size(); ++i)
		{
			if(i % 80 == 0)
				std::cout << "\n";
			std::cout << msg.data[i];
		}
		std::cout << std::endl;
		return os;
	}

	template <typename T>
	friend ISerializedData& operator >> (ISerializedData& msg, T& data)
	{
		if constexpr(!std::is_trivially_copyable<T>::value)
			throw SerializationError(typeid(T));
		if (msg._ittr + sizeof(T) > msg.data.size())
			throw std::runtime_error("Tried to read past end of serialized data");

		std::memcpy(&data, msg.data.data() + msg._ittr, sizeof(T));

		msg._ittr += sizeof(T);

		return msg;
	}

	template<typename T>
	void readSafeArraySize(T& index) // Call this instead of directly reading sizes to prevent buffer overruns
	{
		*this >> index;
		if(index + _ittr > data.size())
			throw std::runtime_error("invalid array length in serialized data");
	}

	friend ISerializedData& operator >> (ISerializedData& msg, std::vector<std::string>& strings)
	{
		uint32_t numStrings;
		msg >> numStrings;
		strings.resize(numStrings);
		for (uint32_t i = 0; i < numStrings; ++i)
		{
			msg >> strings[i];
		}
		return msg;
	}

	template <typename T>
	friend ISerializedData& operator >> (ISerializedData& msg, std::vector<T>& data)
	{
		if constexpr(!std::is_trivially_copyable<T>::value)
			throw SerializationError(typeid(T));
		uint32_t size;
		msg.readSafeArraySize(size);

		data.resize(size / sizeof(T));
		if(size > 0)
			std::memcpy(data.data(), &msg.data[msg._ittr], size);

		msg._ittr += size;

		return msg;
	}

	template <typename T, size_t Count>
	friend ISerializedData& operator >> (ISerializedData& msg, const InlineArray<T, Count>& data)
	{
		if constexpr(!std::is_trivially_copyable<T>::value)
			throw SerializationError(typeid(T));

		uint32_t arrLength;
		msg >> arrLength;
		size_t index = msg.data.size();
		msg.data.resize(index + arrLength);
		//TODO make InlineArray have SerializedData as a friend class and make this more efficient
		for (uint32_t i = 0; i < arrLength; ++i)
		{
			T d;
			msg >> d;
			data.push_back(d);
		}

		return msg;
	}

	friend ISerializedData& operator >> (ISerializedData& msg, ISerializedData& data)
	{
		msg >> data.data;
		return msg;
	}

	friend ISerializedData& operator >> (ISerializedData& msg, std::string& data)
	{
		uint32_t size;
		msg.readSafeArraySize(size);

		data.resize(size);
		if(size > 0)
			std::memcpy(data.data(), &msg.data[msg._ittr], size);

		msg._ittr += size;

		return msg;
	}

	friend ISerializedData& operator >> (ISerializedData& msg, AssetID& id)
	{
		std::string idString;
		msg >> idString;
		id.parseString(idString);

		return msg;
	}

	friend ISerializedData& operator >> (ISerializedData& msg, std::vector<AssetID>& ids)
	{
		uint32_t size;
		msg.readSafeArraySize(size);
		ids.resize(size);
		for (uint32_t i = 0; i < size; ++i)
		{
			std::string idString;
			msg >> idString;
			ids[i].parseString(idString);
		}


		return msg;
	}



	void read(void* dest, size_t size)
	{
		if (_ittr + size <= data.size())
			throw std::runtime_error("Tried to read past end of serialized data");

		std::memcpy(dest, &data[_ittr], size);
		_ittr += size;

	}

	template<typename T>
	T peek()
	{
		if constexpr(!std::is_trivially_copyable<T>::value)
			throw SerializationError(typeid(T));
		if (_ittr + sizeof(T) > data.size())
			throw std::runtime_error("Tried to read past end of serialized data");
		size_t ittrPos = _ittr;
		T o;
		*this >> o;
		_ittr = ittrPos;
		return o;
	}

	[[nodiscard]] bool endOfData() const
	{
		return _ittr >= data.size();
	}

	void restart()
	{
		_ittr = 0;
	}
};

class OSerializedData
{
public:
	std::vector<byte> data;

	[[nodiscard]] size_t size() const
	{
		return data.size();
	}

	friend std::ostream& operator << (std::ostream& os, const OSerializedData& msg)
	{
		os << " Serialized Data: ";
		for (int i = 0; i < msg.data.size(); ++i)
		{
			if(i % 80 == 0)
				std::cout << "\n";
			std::cout << msg.data[i];
		}
		std::cout << std::endl;
		return os;
	}

	template <typename T>
	friend OSerializedData& operator << (OSerializedData& msg, const T& data)
	{
		if constexpr(!std::is_trivially_copyable<T>::value)
			throw SerializationError(typeid(T));

		size_t index = msg.data.size();
		msg.data.resize(index + sizeof(T));
		std::memcpy(&msg.data[index], &data, sizeof(T));

		return msg;
	}

	template <typename T>
	friend OSerializedData& operator << (OSerializedData& msg, const std::vector<T>& data)
	{
		if constexpr(!std::is_trivially_copyable<T>::value)
			throw SerializationError(typeid(T));

		uint32_t arrLength = data.size() * sizeof(T);
		msg << arrLength;
		size_t index = msg.data.size();
		msg.data.resize(index + arrLength);
		if(arrLength > 0)
			std::memcpy(&msg.data[index], data.data(), arrLength);


		return msg;
	}

	template <typename T, size_t Count>
	friend OSerializedData& operator << (OSerializedData& msg, const InlineArray<T, Count>& data)
	{
		if constexpr(!std::is_trivially_copyable<T>::value)
			throw SerializationError(typeid(T));

		uint32_t arrLength = data.size();
		msg << arrLength;
		size_t index = msg.data.size();
		msg.data.resize(index + arrLength);
		//TODO make InlineArray have SerializedData as a friend class and make this more efficient
		for (uint32_t i = 0; i < data.size(); ++i)
		{
			msg << data[i];
		}

		return msg;
	}

	friend OSerializedData& operator << (OSerializedData& msg, const OSerializedData& data)
	{
		msg << data.data;
		return msg;
	}

	friend OSerializedData& operator << (OSerializedData& msg, const std::string& data)
	{
		uint32_t arrLength = static_cast<uint32_t>(data.size());
		msg << arrLength;
		size_t index = msg.data.size();
		msg.data.resize(index + arrLength);
		if(arrLength > 0)
			std::memcpy(&msg.data[index], data.data(), data.size());

		return msg;
	}

	friend OSerializedData& operator << (OSerializedData& msg, const AssetID& id)
	{
		msg << id.string();

		return msg;
	}

	friend OSerializedData& operator << (OSerializedData& msg, const std::vector<std::string>& strings)
	{
		msg << (uint32_t)strings.size();
		for (uint32_t i = 0; i < strings.size(); ++i)
		{
			msg << strings[i];
		}
		return msg;
	}

	friend OSerializedData& operator << (OSerializedData& msg, const std::vector<AssetID>& ids)
	{
		msg << (uint32_t)ids.size();
		for (uint32_t i = 0; i < ids.size(); ++i)
		{
			msg << ids[i].string();
		}


		return msg;
	}

	void write(const void* src, size_t size)
	{
		size_t index = data.size();
		data.resize(index + size);
		std::memcpy(&data[index], src, size);
	}

	[[nodiscard]] ISerializedData toIMessage() const
	{
		ISerializedData o{};
		o.data.resize(data.size());
		std::memcpy(o.data.data(), data.data(), data.size());
		return o;
	}
};

//Indented for use in file serialization, thus static asserts and the like
class MarkedSerializedData
{
	Json::Value attributes;
	Json::Value* currentScope;
	std::stack<Json::Value*> scopes;
public:
	std::vector<byte> data;
	explicit MarkedSerializedData(std::ifstream& file);
	MarkedSerializedData();
	void writeToFile(std::ofstream& file);

	void enterScope(const std::string& scope);
	void enterScope(int index);
	void exitScope();
	void startIndex();
	void pushIndex();
	size_t scopeSize();

	template<typename T>
	void writeAttribute(const std::string& name, const T& value)
	{
		static_assert(std::is_trivially_copyable<T>::value);
		assert(!(*currentScope).isMember(name));

		size_t index = data.size();
		size_t size = sizeof(value);
		data.resize(index + size);
		std::memcpy(&data[index], &value, size);


		(*currentScope)[name]["index"] = index;
	}

	template<typename T>
	void writeAttribute(const std::string& name, const std::vector<T>& value)
	{
		static_assert(std::is_trivially_copyable<T>::value);
		assert(!(*currentScope).isMember(name));

		size_t index = data.size();
		size_t size = value.size() * sizeof(T);
		data.resize(index + size);
		if(size != 0)
			std::memcpy(&data[index], value.data(), size);


		(*currentScope)[name]["index"] = index;
		(*currentScope)[name]["size"] = size;
	}

	void writeAttribute(const std::string& name, const std::string& value)
	{
		assert(!(*currentScope).isMember(name));

		size_t index = data.size();
		size_t size = value.size();
		data.resize(index + size);
		if(size != 0)
			std::memcpy(&data[index], value.data(), size);


		(*currentScope)[name]["index"] = index;
		(*currentScope)[name]["size"] = size;
	}

	void writeAttribute(const std::string& name, const AssetID& value)
	{
		assert(!(*currentScope).isMember(name));

		writeAttribute(name, value.string());
	}

	void writeAttribute(const std::string& name, const std::vector<AssetID>& value)
	{
		assert(!(*currentScope).isMember(name));

		std::vector<byte> data;
		Json::Value idSizes;
		for(auto& id : value)
		{
			size_t index = data.size();
			std::string s = id.string();
			data.resize(index + s.size());
			std::memcpy(&data[index], s.data(), s.size());
			idSizes.append(s.size());
		}
		writeAttribute(name, data);
		(*currentScope)[name]["sizes"] = idSizes;

	}
	
	template<typename T>
	void readAttribute(const std::string& name, T& value)
	{
		static_assert(std::is_trivially_copyable<T>::value);
		if(!(*currentScope).isMember(name))
			throw std::runtime_error(name + " was not found");

		size_t index = (*currentScope)[name]["index"].asLargestUInt();
		size_t size  = sizeof(T);
		if(index + size > data.size())
			throw std::runtime_error(name + " has invalid index or size");
		if(size != 0)
			std::memcpy(&value, &data[index], size);
	}

	template<typename T>
	void readAttribute(const std::string& name, std::vector<T>& value)
	{
		static_assert(std::is_trivially_copyable<T>::value);
		if(!(*currentScope).isMember(name))
			throw std::runtime_error(name + " was not found");

		size_t index = (*currentScope)[name]["index"].asLargestUInt();
		size_t size  = (*currentScope)[name]["size"].asLargestUInt();
		if(index + size > data.size())
			throw std::runtime_error(name + " has invalid index or size");

		value.resize(size / sizeof(T));
		if(size != 0)
			std::memcpy(value.data(), &data[index], size);
	}

	void readAttribute(const std::string& name, std::string& value)
	{
		if(!(*currentScope).isMember(name))
			throw std::runtime_error(name + " was not found");

		size_t index = (*currentScope)[name]["index"].asLargestUInt();
		size_t size  = (*currentScope)[name]["size"].asLargestUInt();
		if(index + size > data.size())
			throw std::runtime_error(name + " has invalid index or size");

		value.resize(size);
		if(size != 0)
			std::memcpy(value.data(), &data[index], size);
	}

	void readAttribute(const std::string& name, AssetID& value)
	{
		assert((*currentScope).isMember(name));

		std::string id;
		readAttribute(name, id);
		value.parseString(id);
	}

	void readAttribute(const std::string& name, std::vector<AssetID>& value)
	{
		assert((*currentScope).isMember(name));

		std::vector<byte> data;
		readAttribute(name, data);
		size_t sIndex = 0;
		size_t idIndex = 0;
		value.resize((*currentScope)[name]["sizes"].size());
		for(auto& jsize : (*currentScope)[name]["sizes"])
		{
			size_t size = jsize.asUInt();
			std::string s;
			s.resize(size);
			if(size != 0)
				std::memcpy(s.data(), &data[sIndex], s.size());
			sIndex += size;
			value[idIndex++].parseString(s);
		}



	}
};


