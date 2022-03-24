#pragma once
#include <ecs/core/structMembers.h>
#include "serializedData.h"
#include <ecs/core/component.h>

namespace net
{
	enum class MessageType
	{
		acknowledge = 0,
		assetRequest = 1,
		assetData = 2,
		assetIncrementalHeader = 3,
		assetIncrementalData = 4,
	};

	struct alignas(4) MessageHeader
	{
		alignas(4) MessageType type;
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
