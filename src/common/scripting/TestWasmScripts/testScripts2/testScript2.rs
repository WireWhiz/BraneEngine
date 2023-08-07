use brane_engine::{component, system};

#[derive(Clone)]
#[component]
pub struct TestComponent2 {
    pub test: i32,
}

#[system]
pub fn test_function3() -> i32 {
    42
}

#[system]
pub fn test_function4() -> i32 {
    42
}
