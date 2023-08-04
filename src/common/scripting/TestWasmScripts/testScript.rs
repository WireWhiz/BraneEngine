use brane_engine::{component, system};

#[component]
pub struct TestComponent {
    pub test: i32,
}

#[system]
pub fn test_function() -> i32 { 42 }

#[system]
pub fn test_function2() -> i32 { 42 }