#pragma once
#include <cstdint>
#include <vector>
#include <sstream>
#include <byte.h>
#include <ecs/ecs.h>

namespace net
{
	enum class MessageType
	{
		assetRequest, 
		assetData,
		one, 
		two, 
		three, 
		four
	};

	struct alignas(8) MessageHeader
	{
		alignas(8) MessageType type;
		alignas(8) uint64_t size;
	};

	class IMessage
	{
		size_t _ittr = 0;
	public:
		 MessageHeader  header;
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

		friend OMessage& operator << (OMessage& msg, const std::string& data)
		{
			size_t index = msg.data.size();
			msg.data.resize(index + data.size());
			std::memcpy(&msg.data.data()[index], data.data(), data.size());

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

	typedef uint32_t ConnectionID;
	struct IMessageComponent : public NativeComponent<IMessageComponent>
	{
		REGESTER_MEMBERS_2(owner, message);
		ConnectionID owner;
		IMessage message;
	};

	struct OMessageComponent : public NativeComponent<OMessageComponent>
	{
		REGESTER_MEMBERS_2(owner, message);
		ConnectionID owner;
		OMessage message;
	};
}