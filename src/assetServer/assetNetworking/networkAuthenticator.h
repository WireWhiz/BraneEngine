#pragma once
#include <ecs/ecs.h>
#include <networking/connection.h>

namespace net
{
	class NetworkAuthenticator
	{
		EntityManager* _em;
		NativeForEach _getNewConnections;
		size_t _connectionIdItterator = 1000;
	public:
		NetworkAuthenticator(EntityManager* em);

		static void networkAuthenticatorSystem(EntityManager* em, void* authenticator);
	};
}