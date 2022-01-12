//
// Created by eli on 1/6/2022.
//

#ifndef BRANEENGINE_LOGINDATAASSET_H
#define BRANEENGINE_LOGINDATAASSET_H
#include "asset.h"

class LoginDataAsset : public Asset
{
public:
	LoginDataAsset() = default;
	LoginDataAsset(net::IMessage& message);
	std::string username;
	std::string email;
	std::string password;
	void serialize(net::OMessage& message) override;
	void deserialize(net::IMessage& message) override;
};


#endif //BRANEENGINE_LOGINDATAASSET_H
