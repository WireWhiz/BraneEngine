use std::ops::{Deref, DerefMut};
use wasmtime::{AsContext, Memory};

/// Struct that can be used as a member in a wasm struct to represent a pointer to a wasm memory buffer, allowing for chained pointers to be used
#[repr(C)]
pub struct WasmSlice<T> {
    ptr: u32,
    size: u32,
    phantom: std::marker::PhantomData<*const T>
}

pub struct WasmSliceRef<'a, T> {
    value: &'a [T]
}

pub struct MutWasmSliceRef<'a, T> {
    value: &'a mut [T]
}


impl<T> WasmSlice<T> {
    pub fn new(ptr: u32, size: u32) -> Self {
        WasmSlice { ptr, size, phantom: std::marker::PhantomData }
    }

    pub fn as_ref(&self, source: &Memory, store: impl AsContext) -> WasmSliceRef<T> {

        let raw_ptr = unsafe { source.data_ptr(store).offset(self.ptr as isize) as *const T };
        let value= unsafe { std::slice::from_raw_parts(raw_ptr, self.size as usize) };
        WasmSliceRef {
            value
        }
    }

    pub fn as_ref_mut(&mut self, source: &Memory, store: impl AsContext) -> MutWasmSliceRef<T> {
        let raw_ptr = unsafe { source.data_ptr(store).offset(self.ptr as isize) as *mut T };
        let value = unsafe { std::slice::from_raw_parts_mut(raw_ptr, self.size as usize) };
        MutWasmSliceRef {
            value
        }
    }

    pub fn size(&self) -> u32 {
        self.size
    }
}

impl WasmSlice<u8> {
    pub fn as_str(&self, source: &Memory, store: impl AsContext) -> &str {
        let raw_ptr = unsafe { source.data_ptr(store).offset(self.ptr as isize) as *const u8 };
        let value= unsafe { std::slice::from_raw_parts(raw_ptr, self.size as usize) };
        std::str::from_utf8(value).unwrap()
    }
}

impl<T> Deref for WasmSliceRef<'_, T> {
    type Target = [T];

    fn deref(&self) -> &Self::Target {
        self.value
    }
}

impl<T> Deref for MutWasmSliceRef<'_, T> {
    type Target = [T];

    fn deref(&self) -> &Self::Target {
        self.value
    }
}

impl<T> DerefMut for MutWasmSliceRef<'_, T> {
    fn deref_mut(&mut self) -> &mut Self::Target {
        self.value
    }
}