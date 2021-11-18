#include <ecs/core/structMembers.h>
#include "message.h"
#include <ecs/core/component.h>

namespace net
{
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
