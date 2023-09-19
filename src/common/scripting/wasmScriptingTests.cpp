#include <iostream>
#include <string_view>

#include "scripting-runtime.h"

int main()
{
    auto* pack = rust_script_runtime::compile_crate("test_crate", false);
    if (!pack)
    {
            std::cout << "Failed to compile crate" << std::endl;
            return 1;
    }

    rust_script_runtime::RUST_BEStr name{};
    rust_script_runtime::new_be_string(&name);
    rust_script_runtime::behaviour_pack_name(pack, &name);
    std::cout << "Compiled crate with name: " << name.data << std::endl;
    rust_script_runtime::free_be_string(&name);

    auto runtime = rust_script_runtime::new_scripting_runtime(true);

    runtime_load_pack(runtime, pack);

    rust_script_runtime::delete_scripting_runtime(runtime);
    rust_script_runtime::free_behaviour_pack(pack);
    return 0;
}
