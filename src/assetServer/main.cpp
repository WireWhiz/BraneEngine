// Asset server
#include <iostream>
#include <networking/connectionAcceptor.h>
#include <common/config/config.h>
#include <json/json.h>
#include <fstream>
#include <string>
#include <ecs/ecs.h>
#include <assetNetworking/networkAuthenticator.h>


int main()
{
	Config::loadConfig();

	uint16_t tcpPort = Config::json()["network"].get("tcp port", 80).asUInt();
	uint16_t sslPort = Config::json()["network"].get("ssl port", 81).asUInt();

	EntityManager em;


	net::ConnectionAcceptor ca(tcpPort, &em);
	net::NetworkAuthenticator na(&em);
	
	ComponentSet components;
	//components.add(net::NewConnectionComponent::def());
	components.add(net::ConnectionComponent::def());
	EnityForEachID feid = em.getForEachID(components);

	while (true)
	{
		em.runSystems();
		em.forEach(feid, [&](byte** component) {
			net::ConnectionComponent* cc = net::ConnectionComponent::fromVirtual(component[0]);
			
			if (cc->connection && cc->connection->isConnected())
			{
				net::OMessage m;
				m << "Hello there connection " << std::to_string(cc->id) << "!\n\r";
				cc->connection->send(m);
			}
		});
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}

	return 0;
}