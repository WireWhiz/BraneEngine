// Asset server
#include <iostream>
#include <common/config/config.h>
#include <json/json.h>
#include <fstream>
#include <string>
#include <ecs/ecs.h>
#include <fileManager/fileManager.h>
#include "assetNetworking/assetServer.h"
#include <assets/types/meshAsset.h>
#include <assets/types/shaderAsset.h>
#include "httpServer/assetHttpServer.h"
#include <database/Database.h>

/*struct SentMesh : public NativeComponent<SentMesh>
{
	REGISTER_MEMBERS_0("Sent Mesh");
};*/

int main()
{
	Config::loadConfig();
	ThreadPool::init(4);

	FileManager fm;
	NetworkManager nm;
	nm.configureServer();
	AssetManager am(fm, nm);
	am.isServer = true;
	AssetServer as(nm, am);
	nm.start();
	Database db;
    AssetHttpServer hs(Config::json()["network"]["domain"].asString(), Config::json()["network"]["use_ssl"].asBool(), db, am, fm);
    hs.scanFiles();




	EntityManager em;

	while(true)
	{
		as.processMessages();
	}

	//net::ConnectionAcceptor ca(tcpPort, &em);
	//net::NetworkAuthenticator na(&em);
	
	/*std::vector<const ComponentAsset*> components;
	components.push_back(EntityIDComponent::def());
	components.push_back(net::ConnectionComponent::def());
	ComponentSet exclude;
	exclude.add(SentMesh::def());
	NativeForEach nfe = NativeForEach(components, exclude, &em);*/

	
	/*MeshAsset quad = MeshAsset(AssetID("localhost/this/mesh/quad"), std::vector<uint32_t>({0, 1, 2, 2, 3, 0}),
													std::vector<Vertex>({
														{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
														{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
														{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
														{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
													}));*/

	/*AssetID vertexShaderID = AssetID("localhost/" + std::to_string(nativeComponentIDIttr));
	std::vector<uint32_t> verticies;
	fm.readFile("localhost/"  + std::to_string(nativeComponentIDIttr), verticies);

	ShaderAsset vertexShader = ShaderAsset(vertexShaderID, ShaderType::vertex, verticies);*/
	

	//fm.writeAsset(&quad);

    /*bool run = true;
	while (run)
	{
		em.runSystems();

		std::vector<EntityID> sent;
		em.forEach(nfe.id(), [&](byte** component) {
			EntityIDComponent* id = EntityIDComponent::fromVirtual(component[nfe.getComponentIndex(0)]);
			net::ConnectionComponent* cc = net::ConnectionComponent::fromVirtual(component[nfe.getComponentIndex(1)]);
			
			if (cc->connection && cc->connection->connected())
			{
				std::shared_ptr<net::OMessage> m = std::make_shared<net::OMessage>();
				m->header.type = net::MessageType::assetData;
				//quad.serialize(m->body);
				std::cout << "sending message: " << m->body;
				cc->connection->send(m);
			}
			sent.push_back(id->id);
		});

		for (EntityID id : sent)
		{
			em.addComponent(id, SentMesh::def());
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}*/
	ThreadPool::cleanup();
	return 0;
}