#include "VirtualSystem.h"
#include "Entity.h"
#include <unordered_map>

class SystemBlock
{
	std::vector<std::unique_ptr<VirtualSystem>> _systems;
	SystemBlock* _next;
public:
	std::string identifier;
	std::string after;
	std::string before;
	SystemBlock(const std::string& identifier, const std::string& after, const std::string& before);
	void setNext(SystemBlock* next);
	SystemBlock* next() const;
	void runSystems(EntityManager& entities, VirtualSystemGlobals* constants) const;
	void addSystem(VirtualSystem* system);
	void removeSystem(SystemID id);
	VirtualSystem* getSystem(SystemID id) const;
};

class SystemBlockList
{
	SystemBlock* first;

	class SystemBlockNode
	{
		std::vector<SystemBlockNode*> _after;
	public:
		std::unique_ptr<SystemBlock> block;
		SystemBlockNode(SystemBlock* block)
		{
			this->block = std::unique_ptr<SystemBlock>(block);
			sorted = false;
			sorted = false;
		}
		bool sorted;
		bool visited;
		void sort(SystemBlock*& last, SystemBlock*& first);
		void addAfter(SystemBlockNode* node);

	};

	std::unordered_map<std::string, std::unique_ptr<SystemBlockNode>> _nodes;
	bool sort();
	SystemBlockNode* findNode(const std::string& identifier) const;
public:
	SystemBlock* operator[](size_t index) const;

	SystemBlock* find(const std::string& identifier);
	bool addBlock(const std::string& identifier, const std::string& after, const std::string& before);
	void removeBlock(const std::string& identifier);
	size_t size() const;

	class insertion_error : public std::runtime_error
	{
	public:
		insertion_error(const char* message) : runtime_error(message) {}
		insertion_error(const std::string& message) : runtime_error(message) {}
	};

};

class SystemManager
{
	SystemBlockList _systems;

public:
	void addSystemBlock(const std::string& identifier, const std::string& after, const std::string& before);
	void removeSystemBlock(const std::string& identifier);
	SystemBlock* getSystemBlock(const std::string& identifier);
	void runSystems(EntityManager& entities, VirtualSystemGlobals* constants) const;
};
