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

	struct alignas(2) MessageHeader
	{
		alignas(2) MessageType type;
		alignas(2) uint16_t size = 0;
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

	typedef uint32_t ConnectionID;
	struct IMessageComponent : public NativeComponent<IMessageComponent>
	{
		REGISTER_MEMBERS_2("Input Message", owner, message);
		ConnectionID owner;
		IMessage message;
	};

	struct OMessageComponent : public NativeComponent<OMessageComponent>
	{
		REGISTER_MEMBERS_2("Output Message", owner, message);
		ConnectionID owner;
		OMessage message;
	};
}
