#include "testing.h"
#include "assets/assetAddress.h"

TEST(assets, AssetAddressTest)
{
	AssetAddress aa;
	aa.parseString("server.ip.goes.here/owner/model/test asset");
	EXPECT_EQ(aa.serverAddress, "server.ip.goes.here");
	EXPECT_EQ(aa.owner, "owner");
	EXPECT_EQ(aa.type.string(), "model");
	EXPECT_EQ(aa.name, "test asset");
	EXPECT_EQ(aa.string(), "server.ip.goes.here/owner/model/test asset");
}