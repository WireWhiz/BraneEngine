
pub mod runtime;
pub mod ffi_types;

use runtime::ScriptingRuntime;
use runtime::behaviour_pack::BehaviourPack;
use crate::ffi_types::BEStr;

// Runtime API

/// Create a new scripting runtime, and return the pointer to it. The caller is responsible for calling delete_scripting_runtime() on this object eventually
#[no_mangle]
pub extern "C" fn new_scripting_runtime(enable_debug: bool) -> *mut ScriptingRuntime
{
    Box::into_raw(Box::new(ScriptingRuntime::new(enable_debug)))
}

/// Delete a scripting runtime allocated with new_scripting_runtime()
#[no_mangle]
pub extern "C" fn delete_scripting_runtime(runtime : *mut ScriptingRuntime)
{
    unsafe { Box::from_raw(runtime) };
}

// Compile rust files into module (editor feature)
#[no_mangle]
pub extern "C" fn compile_crate(crate_root: *const std::ffi::c_char, release: bool) -> *mut BehaviourPack
{
    let crate_root = unsafe { std::ffi::CStr::from_ptr(crate_root).to_str().unwrap() };
    match BehaviourPack::compile_crate(crate_root, release) {
        Ok(pack) => Box::into_raw(Box::new(pack)),
        Err(e) => {
            eprintln!("Error compiling crate: {}", e.to_string());
            std::ptr::null_mut()
        }
    }
}

// Load a compiled module
#[no_mangle]
pub extern "C" fn runtime_load_pack(runtime: *mut runtime::ScriptingRuntime, pack: *const BehaviourPack) -> u32
{
    let runtime = unsafe { &mut *runtime };
    let pack = unsafe { &*pack };

    match runtime.load_pack(pack) {
        Ok(id) => id,
        Err(e) => {
            eprintln!("Error loading pack: {}", e.to_string());
            0xFFFFFFFF
        }
    }
}

// Unload a compiled module

// Add job

// Remove job

// Set job timing dependencies

// Allocate memory buffer in the scripting system (for ecs and other systems)
// Return handle, not raw pointer, as memory might change position when grown


// Module api

/// Get behaviour name
/// # Arguments
/// * `behaviour_pack` - Pointer to the behaviour pack
/// * `name` - Pointer to an uninitialized RawBuffer struct to write the name to
#[no_mangle]
pub extern "C" fn behaviour_pack_name(behaviour_pack: *const BehaviourPack, name: *mut BEStr)
{
    let behaviour_pack = unsafe { &*behaviour_pack };

    unsafe {
        *name = BEStr::from(behaviour_pack.name.as_str())
    };
}

#[no_mangle]
pub extern "C" fn free_behaviour_pack(behaviour_pack: *mut BehaviourPack)
{
    unsafe { drop(Box::from_raw(behaviour_pack)) }
}
