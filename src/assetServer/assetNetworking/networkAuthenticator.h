#pragma once
#include <ecs/ecs.h>
namespace net
{
	class NetworkAuthenticator
	{
		EntityManager* _em;
	public:
		NetworkAuthenticator(EntityManager* em);
	};
}