#pragma once
#include "assets/assetID.h"
#include <networking/message.h>
//namespace net
//{
//	class OMessage;
//	class IMessage;
//}


class Asset
{
protected:
	AssetID _id;

public:
	const AssetID& id() const;
	AssetID& id();

	virtual void serialize(net::OMessage& message);
	virtual void deserialize(net::IMessage& message);
};

template <>
struct std::hash<Asset>
{
	size_t operator()(const Asset& k) const;
};
