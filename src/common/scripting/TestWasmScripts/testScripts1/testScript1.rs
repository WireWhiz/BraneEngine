use brane_engine::{component, system};

#[component]
pub struct TestComponent {
    pub test: i32,
}

#[system]
pub fn test_function(ret : i32) -> i32 {
    ret
}

#[system]
pub fn test_function2() -> i32 { 42 }