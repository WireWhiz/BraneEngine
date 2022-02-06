#include "networkAuthenticator.h"

net::NetworkAuthenticator::NetworkAuthenticator(EntityManager* em) : _em(em)
{ 
	std::vector<const ComponentAsset*> ca{ EntityIDComponent::def(), NewConnectionComponent::def(), ConnectionComponent::def() };
	_getNewConnections = NativeForEach(ca, em);

	em->addSystem(std::make_unique<FunctionPointerSystem>(AssetID("localhost/" + std::to_string(nativeComponentIDIttr)), networkAuthenticatorSystem, this));
}

void net::NetworkAuthenticator::networkAuthenticatorSystem(EntityManager* em, void* authenticator)
{
	NetworkAuthenticator* na = (NetworkAuthenticator*)authenticator;
	std::vector<EntityID> newConnections;
	em->forEach(na->_getNewConnections.id(), [&](byte** components) {
		newConnections.push_back(EntityIDComponent::fromVirtual(components[na->_getNewConnections.getComponentIndex(0)])->id);
		ConnectionComponent::fromVirtual(components[na->_getNewConnections.getComponentIndex(2)])->id = na->_connectionIdItterator++;
		std::cout << "Found new connection, entity: " << EntityIDComponent::fromVirtual(components[na->_getNewConnections.getComponentIndex(0)])->id << " id: " << na->_connectionIdItterator - 1 << std::endl;
	});

	for (size_t i = 0; i < newConnections.size(); i++)
	{
		em->removeComponent(newConnections[i], NewConnectionComponent::def());
	}
}
