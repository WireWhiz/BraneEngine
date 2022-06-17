#pragma once
#include <memory>
#include <ecs/core/virtualType.h>
#include <vector>
#include "../asset.h"

typedef uint16_t ComponentID;
class ComponentAsset : public Asset
{
private:
	size_t _size;
	std::vector<VirtualType::Type> _members;
public:
	ComponentID componentID;

	ComponentAsset();
	ComponentAsset(const ComponentAsset&) = delete;
	ComponentAsset(const std::vector<VirtualType::Type>& members, AssetID id);
	~ComponentAsset();
	[[nodiscard]] size_t size() const;
	[[nodiscard]] const std::vector<VirtualType::Type>& members() const;

	void toFile(MarkedSerializedData& sData) override;
	void fromFile(MarkedSerializedData& sData) override;
	void serialize(OSerializedData& sdata) override;
	void deserialize(ISerializedData& sdata) override;
};