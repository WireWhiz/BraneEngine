use brane_engine::{component, system};

extern "C" {
    pub fn test_external_function() -> i32;
}

#[derive(Clone)]
#[component]
pub struct TestComponent {
    pub test: i32,
}

#[system]
pub fn test_function(ret: i32) -> i32 {
    ret
}

#[system]
pub fn test_extern_call() -> i32 {
    unsafe { test_external_function() }
}
