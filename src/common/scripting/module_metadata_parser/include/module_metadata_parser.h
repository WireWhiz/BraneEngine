#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <ostream>
#include <new>


namespace module_metadata_parser {

struct BEStr
{
  char *data;
};

struct ComponentDef
{
  BEStr name;
};

template<typename T>
struct BEVec
{
  T *data;
  size_t length;
  size_t capacity;
};

struct SystemDef
{
  BEStr name;
};

struct ModuleMetadata
{
  BEVec<ComponentDef> components;
  BEVec<SystemDef> systems;
};


extern "C" {

ModuleMetadata *build_module_metadata(const char *search_dir);

void free_be_string(BEStr *string);

void free_module_metadata(ModuleMetadata *metadata);

BEStr *new_be_string(const char *text);

} // extern "C"

} // namespace module_metadata_parser
