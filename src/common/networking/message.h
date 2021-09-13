#pragma once
#include <cstdint>
#include <vector>
#include <sstream>


namespace net
{
	typedef uint8_t byte;
	enum class MessageType
	{
		assetRequest, 
		assetData,
		one, 
		two, 
		three, 
		four
	};

	struct MessageHeader
	{
		MessageType type; //TODO define max size for different message types
		uint32_t size = 0;
	};

	class IMessage
	{
		size_t _ittr = 0;
	public:
		MessageHeader header;
		std::vector<byte> data;

		size_t size() const
		{
			return data.size();
		}

		friend std::ostream& operator << (std::ostream& os, const IMessage& msg)
		{
			os << "Type: " << int(msg.header.type) << " Size: " << msg.header.size;
			return os;
		}

		template <class T>
		friend IMessage& operator >> (IMessage& msg, T& data)
		{
			static_assert(std::is_standard_layout<T>::value, "Type cannot be used in message");
			assert(msg._ittr + sizeof(T) <= msg.data.size());

			std::memcpy(&data, msg.data.data() + msg._ittr, sizeof(T));

			msg._ittr += sizeof(T);

			return msg;
		}

		void read(void* dest, size_t size)
		{
			assert(_ittr + size <= data.size());

			std::memcpy(dest, &data.data()[_ittr], size);
			_ittr += size;

		}
	};

	class OMessage
	{
	public:
		MessageHeader header;
		std::vector<byte> data;

		size_t size() const
		{
			return data.size();
		}

		friend std::ostream& operator << (std::ostream& os, const OMessage& msg) 
		{
			os << "Type: " << int(msg.header.type) << " Size: " << msg.header.size;
			return os;
		}

		template <class T>
		friend OMessage& operator << (OMessage& msg, const T& data)
		{
			static_assert(std::is_standard_layout<T>::value, "Type cannot be used in message");

			size_t index = msg.data.size();
			msg.data.resize(index + sizeof(T));
			std::memcpy(&msg.data.data()[index], &data, sizeof(T));

			msg.header.size = msg.size();

			return msg;
		}

		void write(void* src, size_t size)
		{
			size_t index = data.size();
			data.resize(index + size);
			std::memcpy(&data.data()[index], src, size);
			
			header.size = this->size();
		}

		IMessage toIMessage() const
		{
			IMessage o{};
			o.header = header;
			o.data.resize(data.size());
			std::memcpy(o.data.data(), data.data(), data.size());
			return o;
		}
	};
}