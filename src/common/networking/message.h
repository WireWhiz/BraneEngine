#pragma once
#include <ecs/core/structMembers.h>
#include <utility/serializedData.h>
#include <ecs/core/component.h>

namespace net
{
	enum class MessageType : uint16_t
	{
		acknowledge = 0,
		request     = 1,
		response    = 2,
		streamData  = 3,
		endStream   = 4
	};

	struct alignas(4) MessageHeader
	{
		MessageType type;
		uint32_t size = 0;
	};

	struct OMessage
	{
		MessageHeader header;
		OSerializedData body;
	};

	struct IMessage
	{
		MessageHeader header;
		ISerializedData body;
	};
}
