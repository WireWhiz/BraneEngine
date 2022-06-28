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
