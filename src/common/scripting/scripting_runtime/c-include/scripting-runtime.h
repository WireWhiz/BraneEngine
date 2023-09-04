#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <ostream>
#include <new>


namespace rust_script_runtime {

struct RUST_BehaviourPack;

struct RUST_ScriptingRuntime;

struct RUST_BEStr
{
  char *data;
};


extern "C" {

/// Create a new scripting runtime, and return the pointer to it. The caller is responsible for calling delete_scripting_runtime() on this object eventually
RUST_ScriptingRuntime *new_scripting_runtime(bool enable_debug);

/// Delete a scripting runtime allocated with new_scripting_runtime()
void delete_scripting_runtime(RUST_ScriptingRuntime *runtime);

RUST_BehaviourPack *compile_crate(const char *crate_root, bool release);

/// Get behaviour name
/// # Arguments
/// * `behaviour_pack` - Pointer to the behaviour pack
/// * `name` - Pointer to an uninitialized RawBuffer struct to write the name to
void behaviour_pack_name(const RUST_BehaviourPack *behaviour_pack, RUST_BEStr *name);

void new_be_string(RUST_BEStr *uninitalized);

void free_be_string(RUST_BEStr *string);

void new_be_string_from_c_str(RUST_BEStr *uninitalized, const char *text);

} // extern "C"

} // namespace rust_script_runtime
