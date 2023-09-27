use std::ffi::c_char;
use brane_engine_api::{component, order_before, order_after, job};

#[link(wasm_import_module = "BraneEngine")]
extern "C" {
    pub fn test_external_function() -> i32;

    pub fn spawn_thread(func: Box<dyn Fn()>);

    pub fn extern_be_print(msg: *const u8, size: u32);
}

pub fn be_print(msg: &str) {
    unsafe {
        extern_be_print(msg.as_ptr(), msg.len() as u32);
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

#[order_before(test_extern_call)]
#[job]
pub fn test_function(ret: i32) -> i32 {
    be_print(format!("Test function passed {}", ret).as_str());
    ret
}

#[job]
pub fn test_extern_call() -> i32 {
    unsafe { test_external_function() }
}



#[job]
pub fn create_test_component() -> *mut TestComponent {
    let component = Box::new(TestComponent {
        a: false,
        b: 0,
        c: 0f32
    });
    Box::into_raw(component)
}

#[job]
pub fn run_thread(func: Box<dyn Fn()>)
{
    func();
}

#[job]
pub fn enqueue_job(func: Box<dyn Fn()>)
{
    unsafe{spawn_thread(func)};
}