mod wasm_ptr;
mod wasm_slice;

pub use wasm_ptr::WasmPtr;
pub use wasm_slice::WasmSlice;

#[repr(align(4))]
#[repr(C)]
pub struct ComponentFieldInfo {
    pub offset: u32,
    pub size: u32,
    pub name: WasmSlice<u8>,
    pub ty: WasmSlice<u8>,
}

#[repr(C)]
pub struct ComponentInfo {
    pub size: u32,
    pub fields: WasmSlice<ComponentFieldInfo>
}
