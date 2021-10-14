#pragma once
#include "VirtualSystem.h"
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
		SystemID _id;
	public:
		bool sorted;
		bool visited;
		std::vector<SystemNode*> after;
		std::vector<SystemID> afterConstraints;
		std::vector<SystemID> beforeConstraints;
		SystemNode(std::unique_ptr<VirtualSystem>& system);
		void setNext(SystemNode* next);
		SystemNode* next() const;
		void run(EntityManager* entities) const;
		VirtualSystem* system() const;
		SystemID id() const;
		void sort(SystemNode*& last);
	};

	SystemNode* _first;

	std::unordered_map<SystemID, std::unique_ptr<SystemNode>> _nodes;
	bool sort();

	void updateNodeConstraints(SystemNode* node);
	void removeNode(SystemID id);
public:
	SystemList();
	VirtualSystem* findSystem(SystemID id) const;
	bool addSystem(std::unique_ptr<VirtualSystem>& system);
	void removeSystem(SystemID id);

	bool addBeforeConstraint(SystemID id, SystemID before);
	bool addAfterConstraint(SystemID id, SystemID after);

	size_t size() const;
	void runSystems(EntityManager* em);

	class insertion_error : public std::runtime_error
	{
	public:
		insertion_error(const char* message) : runtime_error(message) {}
		insertion_error(const std::string& message) : runtime_error(message) {}
	};

};