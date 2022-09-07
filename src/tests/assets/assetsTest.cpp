#include "testing.h"
#include <assets/asset.h>
#include "assets/assetManager.h"


//Edit this function if we need to "load" any assets for testing
AsyncData<Asset*> AssetManager::fetchAssetInternal(const AssetID& id, bool incremental)
{
	AsyncData<Asset*> asset;
	asset.setData(nullptr);
	return asset;
}

TEST(assets, AssetIDTest)
{
	AssetID aa;
	aa.parseString("server.ip.goes.here/1234A");
	EXPECT_EQ(aa.serverAddress, "server.ip.goes.here");
	EXPECT_EQ(aa.id, 0x1234A);
	EXPECT_EQ(aa.string(), "server.ip.goes.here/1234A");
	EXPECT_TRUE(aa == aa);
	EXPECT_FALSE(aa.isNull());

	AssetID null;
	EXPECT_EQ(AssetID::null, null);
	EXPECT_TRUE(null.isNull());
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
