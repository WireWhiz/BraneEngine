mod wasm_ptr;
mod wasm_slice;

pub use wasm_ptr::WasmPtr;
pub use wasm_slice::WasmSlice;

#[repr(C)]
pub struct ComponentFieldInfo {
    pub offset: i32,
    pub size: i32,
    pub ty: WasmSlice<u8>,
}

#[repr(C)]
pub struct ComponentInfo {
    pub size: i32,
    pub fields: WasmSlice<ComponentFieldInfo>
}
