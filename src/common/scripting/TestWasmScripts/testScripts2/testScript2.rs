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

#[derive(Clone)]
#[component]
pub struct TestComponent {
    pub a: bool,
    pub b: i32,
    pub c: f32
}

#[system]
pub fn test_component_access(component: &mut TestComponent) -> &mut TestComponent {
    *component = TestComponent {
        a: true,
        b: 42,
        c: 3.14
    };
    component
}
