//
// Created by eli on 1/6/2022.
//

#include "loginDataAsset.h"

void LoginDataAsset::serialize(net::OMessage& message)
{
	Asset::serialize(message);
	message << username << email << password;
}

void LoginDataAsset::deserialize(net::IMessage& message)
{
	Asset::deserialize(message);
	message >> username >> email >> password;
}

LoginDataAsset::LoginDataAsset(net::IMessage& message)
{
	deserialize(message);
}
