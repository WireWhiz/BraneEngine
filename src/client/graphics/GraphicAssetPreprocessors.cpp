//
// Created by eli on 3/19/2022.
//

#include "graphicAssetPreprocessors.h"

void GraphicAssetPreprocessors::addAssetPreprocessors(AssetManager& am, graphics::VulkanRuntime& vkr)
{
	am.addAssetPreprocessor(AssetType::assembly, [&am, &vkr](Asset* asset){
		auto assembly = (Assembly*)asset;
		for(auto& entity : assembly->entities)
		{
			for(auto& component : entity.components)
			{
				if(component.def() == MeshRendererComponent::def())
				{
					MeshRendererComponent* mr = MeshRendererComponent::fromVirtual(component.data());
					auto* meshAsset = am.getAsset<MeshAsset>(AssetID(assembly->meshes[mr->mesh]));
					if(meshAsset->pipelineID == -1)
						vkr.addMesh(meshAsset);
					mr->mesh = meshAsset->pipelineID;

					//TODO: process materials here as well
				}
			}
		}
	});
}
