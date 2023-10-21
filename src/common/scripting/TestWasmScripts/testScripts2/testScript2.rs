
#[derive(Clone)]
#[repr(C)]
pub struct TestComponent {
    pub a: bool,
    pub b: i32,
    pub c: f32
}

#[no_mangle]
pub extern "C" fn test_component_access(mut component: Box<TestComponent>) -> Box<TestComponent> {
    *component = TestComponent {
        a: true,
        b: 42,
        c: 3.14
    };
    component
}