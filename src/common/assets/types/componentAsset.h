#pragma once
#include <memory>
#include "common/ecs/virtualType.h"
#include <vector>
#include "../asset.h"

using ComponentID = uint16_t;
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
	~ComponentAsset() override;
	const std::vector<VirtualType::Type>& members() const;
	const std::vector<std::string>& memberNames() const;
	std::vector<VirtualType::Type>& members();
	std::vector<std::string>& memberNames();

    void serialize(OutputSerializer& s) override;
    void deserialize(InputSerializer& s) override;
};