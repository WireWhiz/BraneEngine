
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
#[repr(C)]
pub struct TestComponent {
    pub a: bool,
    pub b: i32,
    pub c: f32
}

#[no_mangle]
pub extern "C" fn test_function(ret: i32) -> i32 {
    be_print(format!("Test function passed {}", ret).as_str());
    ret
}

#[no_mangle]
pub extern "C"  fn create_test_component() -> *mut TestComponent {
    let component = Box::new(TestComponent {
        a: false,
        b: 0,
        c: 0f32
    });
    Box::into_raw(component)
}