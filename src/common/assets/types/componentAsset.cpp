#include "componentAsset.h"
#include "common/ecs/component.h"
#include "utility/serializedData.h"

ComponentAsse::ComponentAsset()
{
    type.set(AssetType::Type::component);
}

ComponentAsse::ComponentAsset(const std::vector<VirtualType::Type>& members, const std::vector<std::string>& memberNames, AssetID&& id)
{
    this->id = std::move(id);
    type.set(AssetType::Type::component);
    _members = members;
    _memberNames = memberNames;
}

ComponentAsse::~ComponentAsse()
{
}

const std::vector<VirtualType::Type>& ComponentAsse::members() const
{
    return _members;
}

void ComponentAsse::serialize(OutputSerializer& s) const
{
    Asset::serialize(s);
    s << _members << _memberNames;
}

void ComponentAsse::deserialize(InputSerializer& s)
{
    Asset::deserialize(s);
    s >> _members >> _memberNames;
}

const std::vector<std::string>& ComponentAsse::memberNames() const
{
    return _memberNames;
}

std::vector<VirtualType::Type>& ComponentAsse::members()
{
    return _members;
}

std::vector<std::string>& ComponentAsse::memberNames()
{
    return _memberNames;
}




