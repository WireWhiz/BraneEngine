#include "asset.h"
#include <networking/message.h>

const AssetID& Asset::id() const
{
	return _id;
}

AssetID& Asset::id()
{
	return _id;
}

void Asset::serialize(net::OMessage& message)
{
	message << _id;
}

void Asset::deserialize(net::IMessage& message)
{
	message >> _id;
}

size_t std::hash<Asset>::operator()(const Asset& k) const
{
	return std::hash<AssetID>()(k.id());
}