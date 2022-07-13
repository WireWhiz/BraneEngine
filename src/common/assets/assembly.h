//
// Created by eli on 2/2/2022.
//

#ifndef BRANEENGINE_ASSEMBLY_H
#define BRANEENGINE_ASSEMBLY_H

#include "asset.h"
#include "ecs/core/component.h"
#include <json/json.h>

class EntityManager;



class Assembly : public Asset
{
public:
	struct WorldEntity
	{
		std::vector<VirtualComponent> components;
		std::vector<ComponentID> runtimeComponentIDs();
		void serialize(OSerializedData& message, Assembly& assembly);
		void deserialize(ISerializedData& message, Assembly& assembly, ComponentManager& cm, AssetManager& am);
		void writeToFile(MarkedSerializedData& sData, Assembly& assembly);
		void readFromFile(MarkedSerializedData& sData, Assembly& assembly, ComponentManager& cm, AssetManager& am);
	};

	Assembly();
	std::vector<AssetID> components;
	std::vector<AssetID> scripts; // Any systems in dependencies will be automatically loaded
	std::vector<AssetID> meshes; // We need to store these in a list, so we can tell witch asset entities are referring to
	std::vector<AssetID> textures;
	std::vector<WorldEntity> entities;
	void toFile(MarkedSerializedData& sData) override;
	void fromFile(MarkedSerializedData& sData) override;
	void serialize(OSerializedData& message) override;
	void deserialize(ISerializedData& message) override;
	Json::Value toJson(AssetManager& am) const;
	static Assembly* fromJson(Json::Value& json);


	void  inject(EntityManager& em, EntityID rootID);
};


#endif //BRANEENGINE_ASSEMBLY_H
