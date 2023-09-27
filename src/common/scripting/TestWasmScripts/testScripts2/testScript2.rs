use std::ffi::c_void;
use std::thread::Thread;
use brane_engine_api::{component, job, order_before};


#[link(wasm_import_module = "module1")]
extern "C" {
    pub fn create_test_component() -> i32;

    pub fn enqueue_job(func: Box<dyn Fn()>);
}


#[link(wasm_import_module = "BraneEngine")]
extern "C" {
    pub fn extern_be_print(msg: *const u8, size: u32);
}

pub fn be_print(msg: &str) {
    unsafe {
        extern_be_print(msg.as_ptr(), msg.len() as u32);
    }
}


#[derive(Clone)]
#[component]
pub struct TestComponent2 {
    pub test: i32,
}

#[order_before(a, c, d)]
#[job]
pub fn test_function3() -> i32 {
    42
}

#[job]
pub fn test_function4() -> i32 {
    42
}

#[derive(Clone)]
#[component]
pub struct TestComponent {
    pub a: bool,
    pub b: i32,
    pub c: f32
}

#[job]
pub fn test_imported_create_test_component() -> *mut TestComponent
{
    unsafe { create_test_component() as *mut TestComponent }
}

#[job]
pub fn test_component_access(mut component: Box<TestComponent>) -> Box<TestComponent> {
    *component = TestComponent {
        a: true,
        b: 42,
        c: 3.14
    };
    component
}

#[derive(Copy, Clone)]
pub struct ThreadCtx {
    pub id: i32
}

pub extern "C" fn thread_runtime(data: Box<ThreadCtx>) {

    be_print(format!("Thread {} started", data.id).as_str());
    std::thread::sleep(std::time::Duration::from_millis(3000));
    be_print(format!("Thread {} finished", data.id).as_str());
}

#[job]
pub fn threading_test(n: i32) {
    be_print("Starting function and spawning threads");
    for i in 0..n {
        unsafe {
            let data = Box::new(ThreadCtx { id: i });
            enqueue_job(Box::new(move ||{
                thread_runtime(data.clone());
            }));
        }
    }
    be_print("Returning function");
}