#pragma once
#include "virtualSystem.h"
//#include "entity.h"
#include <unordered_map>
#include <memory>
#include <stdexcept>

class EntityManager;

class SystemList
{

	class SystemNode
	{
		std::unique_ptr<VirtualSystem> _system;
		SystemNode* _next;
		AssetID _id;
	public:
		bool sorted;
		bool visited;
		std::vector<SystemNode*> after;
		std::vector<AssetID> afterConstraints;
		std::vector<AssetID> beforeConstraints;
		SystemNode(std::unique_ptr<VirtualSystem>&& system);
		void setNext(SystemNode* next);
		SystemNode* next() const;
		void run(EntityManager& entities) const;
		VirtualSystem* system() const;
		AssetID id() const;
		bool sort(SystemNode*& first, SystemNode*& last);
	};

	SystemNode* _first;

	std::unordered_map<AssetID, std::unique_ptr<SystemNode>> _nodes;
	bool sort();

	void updateNodeConstraints(SystemNode* node);
	void removeNode(AssetID id);
public:
	SystemList();
	VirtualSystem* findSystem(AssetID id) const;
	bool addSystem(std::unique_ptr<VirtualSystem>&& system);
	void removeSystem(AssetID id);

	bool addBeforeConstraint(AssetID id, AssetID before);
	bool addAfterConstraint(AssetID id, AssetID after);

	size_t size() const;
	void runSystems(EntityManager& em);

	class insertion_error : public std::runtime_error
	{
	public:
		insertion_error(const char* message) : runtime_error(message) {}
		insertion_error(const std::string& message) : runtime_error(message) {}
	};

};