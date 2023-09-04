use std::ffi::c_char;
use brane_engine_api::{component, system};

#[link(wasm_import_module = "BraneEngine")]
extern "C" {
    pub fn test_external_function() -> i32;
    pub fn extern_be_print(msg: *const c_char);
}

pub fn be_print(msg: &str) {
    let c_str = std::ffi::CString::new(msg).unwrap();
    unsafe {
        extern_be_print(c_str.as_ptr());
    }
}

pub fn be_malloc() -> *mut u8 {
    let mut vec = Vec::new();
    vec.push(0);
    vec.as_mut_ptr()
}

#[derive(Clone)]
#[component]
pub struct TestComponent {
    pub a: bool,
    pub b: i32,
    pub c: f32
}

#[system]
pub fn test_function(ret: i32) -> i32 {
    be_print(format!("Test function was passed: {}", ret).as_str());
    ret
}

#[system]
pub fn test_extern_call() -> i32 {
    unsafe { test_external_function() }
}



#[system]
pub fn create_test_component() -> *mut TestComponent {
    let mut component = Box::new(TestComponent {
        a: false,
        b: 0,
        c: 0f32
    });
    Box::into_raw(component)
}