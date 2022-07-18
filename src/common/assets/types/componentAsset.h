#pragma once
#include <memory>
#include <ecs/core/virtualType.h>
#include <vector>
#include "../asset.h"

typedef uint16_t ComponentID;
class ComponentAsset : public Asset
{
private:
	std::vector<VirtualType::Type> _members;
	std::vector<std::string> _memberNames;
public:
	ComponentID componentID;

	ComponentAsset();
	ComponentAsset(const ComponentAsset&) = delete;
	ComponentAsset(const std::vector<VirtualType::Type>& members, const std::vector<std::string>& memberNames, AssetID id);
	~ComponentAsset();
	const std::vector<VirtualType::Type>& members() const;
	const std::vector<std::string>& memberNames() const;
	std::vector<VirtualType::Type>& members();
	std::vector<std::string>& memberNames();

	void toFile(MarkedSerializedData& sData) override;
	void fromFile(MarkedSerializedData& sData) override;
	void serialize(OSerializedData& sdata) override;
	void deserialize(ISerializedData& sdata) override;
};