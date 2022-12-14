#pragma once
#include <memory>
#include "common/ecs/virtualType.h"
#include <vector>
#include "../asset.h"

using ComponentID = uint32_t;
class ComponentAsse : public Asset
{
private:
    std::vector<VirtualType::Type> _members;
    std::vector<std::string> _memberNames;
public:
    ComponentID componentID = -1;

    ComponentAsse();
    ComponentAsse(const ComponentAsse&) = delete;
    ComponentAsse(const std::vector<VirtualType::Type>& members, const std::vector<std::string>& memberNames, AssetID&& id);
    ~ComponentAsse() override;
    const std::vector<VirtualType::Type>& members() const;
    const std::vector<std::string>& memberNames() const;
    std::vector<VirtualType::Type>& members();
    std::vector<std::string>& memberNames();

    void serialize(OutputSerializer& s) const override;
    void deserialize(InputSerializer& s) override;
};