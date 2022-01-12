// Asset server
#include <iostream>
#include <networking/connectionAcceptor.h>
#include <common/config/config.h>
#include <json/json.h>
#include <fstream>
#include <string>
#include <ecs/ecs.h>
#include <assetNetworking/networkAuthenticator.h>
#include <fileManager/fileManager.h>
#include <assets/types/meshAsset.h>
#include <assets/types/shaderAsset.h>
#include <http/HTTPServer.h>
#include <database/Database.h>

struct SentMesh : public NativeComponent<SentMesh>
{
	REGISTER_MEMBERS_0();
};

/*void handleChallenge(const std::string& domain, const std::string& url, const std::string& keyAuthorization)
{
    std::cout << "A challenge approches! \n domain: " << domain << "\n url: " << url << "\n keyAuth: " << keyAuthorization << std::endl;
    serverThread = std::thread([domain, url, keyAuthorization](){
        try{
            httplib::Server svr;

            size_t domLength = domain.size() + 7; //Length of "http://domain.com"
            svr.Get( url.substr(domLength, url.size() - domLength), [keyAuthorization](const httplib::Request &, httplib::Response &res) {
                res.set_content(keyAuthorization, "text/plain");
            });

            svr.listen("0.0.0.0", 80);
        }
        catch(const std::exception& e){
            std::cerr<< "Problem starting server: " << e.what() << std::endl;
        }

    });

}*/

int main()
{
	Config::loadConfig();

	uint16_t tcpPort = Config::json()["network"].get("tcp port", 80).asUInt();
	uint16_t sslPort = Config::json()["network"].get("ssl port", 81).asUInt();
	FileManager fm;
	Database db;
    HTTPServer hs(Config::json()["network"]["domain"].asString(), fm, db, Config::json()["network"]["use_ssl"].asBool());
    hs.scanFiles();


	EntityManager em;


	net::ConnectionAcceptor ca(tcpPort, &em);
	net::NetworkAuthenticator na(&em);
	
	std::vector<const ComponentAsset*> components;
	components.push_back(EntityIDComponent::def());
	components.push_back(net::ConnectionComponent::def());
	ComponentSet exclude;
	exclude.add(SentMesh::def());
	NativeForEach nfe = NativeForEach(components, exclude, &em);

	
	MeshAsset quad = MeshAsset(AssetID("localhost/this/mesh/quad"), std::vector<uint32_t>({0, 1, 2, 2, 3, 0}),
													std::vector<Vertex>({
														{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
														{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
														{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
														{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
													}));

	AssetID vertexShaderID = AssetID("localhost/this/shader/vertex");
	std::vector<uint32_t> verticies;
	fm.readFile("localhost/this/shader/vertex.spirv", verticies);

	ShaderAsset vertexShader = ShaderAsset(vertexShaderID, ShaderType::vertex, verticies);
	

	fm.writeAsset(&quad);

    bool run = true;
	while (run)
	{
		em.runSystems();

		std::vector<EntityID> sent;
		em.forEach(nfe.id(), [&](byte** component) {
			EntityIDComponent* id = EntityIDComponent::fromVirtual(component[nfe.getComponentIndex(0)]);
			net::ConnectionComponent* cc = net::ConnectionComponent::fromVirtual(component[nfe.getComponentIndex(1)]);
			
			if (cc->connection && cc->connection->connected())
			{
				net::OMessage m;
				m.header.type = net::MessageType::assetData;
				quad.serialize(m);
				std::cout << "sending message: " << m;
				cc->connection->send(std::make_shared<net::OMessage>(std::move(m)));
			}
			sent.push_back(id->id);
		});

		for (EntityID id : sent)
		{
			em.addComponent(id, SentMesh::def());
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}

	return 0;
}