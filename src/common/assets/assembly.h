//
// Created by eli on 2/2/2022.
//

#ifndef BRANEENGINE_ASSEMBLY_H
#define BRANEENGINE_ASSEMBLY_H

#include "asset.h"
#include "ecs/core/component.h"

struct WorldEntity
{
	std::vector<VirtualComponent> components;
	void serialize(OSerializedData& message);
	void deserialize(ISerializedData& message);
};

class Assembly : public Asset
{
public:
	std::vector<AssetID> dependencies; // Any systems in dependencies will be automatically loaded
	std::vector<WorldEntity> data;
	virtual void serialize(OSerializedData& message) override;
	virtual void deserialize(ISerializedData& message) override;
};


#endif //BRANEENGINE_ASSEMBLY_H
