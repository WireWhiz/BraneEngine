//
// Created by eli on 2/20/2022.
//
#include "testing.h"
#include <assets/types/componentAsset.h>
#include <assets/assetManager.h>

TEST(assets, ComponentAssetSerializeTest)
{
	std::vector<std::unique_ptr<VirtualType>> types;
	types.push_back(std::make_unique<VirtualVariable<bool>>());
	types.push_back(std::make_unique<VirtualVariable<uint32_t>>());
	ComponentAsset testDef(types, AssetID("localhost/0"));

	OSerializedData oData;
	testDef.serialize(oData);
	std::cout << "Serialized component asset: " << oData << std::endl;

	ISerializedData iData = oData.toIMessage();
	ComponentAsset testDefResult;

	FileManager fm;
	NetworkManager nm(<#initializer#>);
	AssetManager am(fm, nm);
	testDefResult.deserialize(iData, am);

	for (int i = 0; i < testDef.types().size(); ++i)
	{
		auto& a = testDef.types()[i];
		auto& b = testDefResult.types()[i];
		EXPECT_EQ(a->getType(), b->getType());
		EXPECT_EQ(a->offset(), b->offset());
	}
}