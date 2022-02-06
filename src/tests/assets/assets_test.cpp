#include "testing.h"
#include <assets/asset.h>
#include "assets/assetManager.h"

TEST(assets, AssetIDTest)
{
	AssetID aa;
	aa.parseString("server.ip.goes.here/1234A");
	EXPECT_EQ(aa.serverAddress, "server.ip.goes.here");
	EXPECT_EQ(aa.id, 0x1234A);
	EXPECT_EQ(aa.string(), "server.ip.goes.here/000000000001234A");
	EXPECT_TRUE(aa == aa);
}

TEST(assets, AssetHeaderTest)
{
	AssetHeader ah;
	ah.id = AssetID("test/0");
	ah.dependencies.push_back({AssetID("test/2"), AssetDependencyLevel::requireFull});
	ah.dependencies.push_back({AssetID("test/2"), AssetDependencyLevel::loadProcedural});

	OSerializedData serialized;
	ah.serialize(serialized);
	std::cout << "Serialized asset header: " << serialized;

	ISerializedData deserializationStream = serialized.toIMessage();
	AssetHeader deserialized(deserializationStream);
	EXPECT_EQ(deserialized.id, ah.id);
	size_t ittr = 0;
	for(auto& dep1 : ah.dependencies)
	{
		auto& dep2 = deserialized.dependencies[ittr++];
		EXPECT_EQ(dep1.id, dep2.id);
		EXPECT_EQ(dep1.level, dep2.level);
	}

}

/*
TEST(assets, AssetManagerTest)
{
	AssetID aID;
	aID.parseString("localhost/tester/component/testComponent");
	std::vector<std::unique_ptr<VirtualType>> components;
	components.push_back(std::make_unique<VirtualBool>(0));
	ComponentAsset* ca = new ComponentAsset(components, aID);

	AssetManager am;
	am.addComponent(ca);

	EXPECT_EQ(am.getComponent(aID)->id(), aID);
}*/
