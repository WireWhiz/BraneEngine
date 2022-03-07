#include "assetManager.h"
#include "ecs/ecs.h"
#include "networking/serializedData.h"
#include "networking/message.h"
#include "ecs/nativeTypes/transform.h"
#include "ecs/nativeTypes/meshRenderer.h"

/*void AssetManager::downloadAcceptorSystem(EntityManager* em, void* thisPtr)
{
	AssetManager* am = (AssetManager*)thisPtr;
	std::vector<EntityID> messages;
	em->forEach(am->_downloadAcceptorFE.id(), [&](byte** components) {
		net::IMessageComponent* message = net::IMessageComponent::fromVirtual(components[am->_downloadAcceptorFE.getComponentIndex(1)]);

		if (message->message.header.type == net::MessageType::assetData)
		{
			messages.push_back(EntityIDComponent::fromVirtual(components[am->_downloadAcceptorFE.getComponentIndex(0)])->id);
			AssetID id = message->message.body.peek<AssetID>();

			*//*switch (id.type.type())
			{
				case AssetType::mesh:
					am->addMesh(new MeshAsset(net::IMessageComponent::fromVirtual(components[am->_downloadAcceptorFE.getComponentIndex(1)])->message.body));
					std::cout << "Receved mesh asset" << std::endl;
					break;
				default:
					std::cout << "Receved unimplemented data type: " << id.type.string() << std::endl;
					assert(false && "Receved unimplemented data type");
					break;
			}*//*
		}
		
	});
	for (EntityID e : messages)
	{
		em->destroyEntity(e);
	}
}*/

/*void AssetManager::startDownloadAcceptorSystem(EntityManager* em)
{
	_downloadAcceptorFE = NativeForEach(std::vector<const ComponentAsset*>{EntityIDComponent::def(), net::IMessageComponent::def()}, em);
	em->addSystem(std::make_unique<FunctionPointerSystem>(AssetID("localhost/this/system/downloadAcceptor"), downloadAcceptorSystem, this));
}*/

/*void AssetManager::addComponent(ComponentAsset* component)
{
	assert(component->id().type == AssetType::component);
	_assets.emplace(component->id(), component);
}

ComponentAsset* AssetManager::getComponent(const AssetID& id)
{
	assert(id.type == AssetType::component);
	assert(_assets.count(id));
	return (ComponentAsset*)(_assets[id].get());
} 

void AssetManager::addMesh(MeshAsset* mesh)
{
	assert(mesh->id().type == AssetType::mesh);
	_assets[mesh->id()] = std::unique_ptr<Asset>(dynamic_cast<Asset*>(mesh));
}

MeshAsset* AssetManager::getMesh(const AssetID& id)
{
	assert(id.type == AssetType::mesh);
	assert(_assets.count(id));
	return (MeshAsset*)(_assets[id].get());
}*/


AssetManager::AssetManager(FileManager& fm, NetworkManager& nm) : _fm(fm), _nm(nm)
{
	addNativeComponent<EntityIDComponent>();
	addNativeComponent<comps::TransformComponent>();
	addNativeComponent<comps::LocalTransformComponent>();
	addNativeComponent<comps::ChildrenComponent>();
	addNativeComponent<comps::MeshRendererComponent>();
}

std::string AssetManager::getAssetName(AssetID& id)
{
	if(_assets.count(id))
	{
		return _assets[id]->name;
	}
	else if (id.serverAddress == "localhost")
	{
		std::string name;
		Asset* asset = _fm.readAsset<Asset>(id, *this);
		name = asset->name;
		delete asset;
		return name;
	}
	else
	{
		assert(false && "Ya need to code in fetching assets from a network");
		return "Not found";
	}
}
