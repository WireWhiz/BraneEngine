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
	std::vector<AssetID> scripts; // Any systems in dependencies will be automatically loaded
	std::vector<AssetID> meshes; // We need to store these in a list, so we can tell witch asset entities are referring to
	std::vector<AssetID> textures;
	std::vector<WorldEntity> data;
	void serialize(OSerializedData& message) override;
	void deserialize(ISerializedData& message) override;
};


#endif //BRANEENGINE_ASSEMBLY_H
