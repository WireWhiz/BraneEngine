use brane_engine_api::{component, job, order_before};


#[link(wasm_import_module = "module1")]
extern "C" {
    pub fn create_test_component() -> i32;
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