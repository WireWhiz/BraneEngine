#include "VirtualType.h"
#include <vector>
#include <string>
#include <assert.h>



class VirtualStructDefinition
{
private:
	size_t _numTypes;
	size_t _size;
#ifndef NDEBUG
	bool _initalized;
#endif
public:
	std::string identifier;
	VirtualType* types;

	VirtualStructDefinition(size_t _numTypes);
	~VirtualStructDefinition();

	void initalize();
	size_t size();
	
};

class VirtualStruct
{
private:
	VirtualStructDefinition* def;
	byte* data;
public:
	VirtualStruct(VirtualStructDefinition* definition);
	~VirtualStruct();
	
};

class VirtualStructVector
{
private:
	std::vector<byte> data;
};