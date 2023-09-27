use std::ops::{Deref, DerefMut};
use wasmtime::{Memory, SharedMemory, AsContext};

/// Struct that can be used as a member in a wasm struct to represent a pointer to a wasm memory buffer, allowing for chained pointers to be used
#[repr(C)]
pub struct WasmPtr<T> {
    pub ptr: u32,
    phantom: std::marker::PhantomData<*const T>
}

pub struct WasmRef<'a, T> {
    value: &'a T
}

pub struct MutWasmRef<'a, T> {
    value: &'a mut T
}

impl<T> WasmPtr<T> {
    pub fn new(ptr: u32) -> Self {
        WasmPtr { ptr, phantom: std::marker::PhantomData }
    }

    pub fn as_ref(&self, source: &Memory, store: impl AsContext) -> WasmRef<T>
    {
        let raw_ptr = unsafe { source.data_ptr(store).offset(self.ptr as isize) as *const T };
        let value = unsafe { &*raw_ptr };
        WasmRef {
            value
        }
    }

    pub fn as_ref_mut(&mut self, source: &Memory, store: impl AsContext) -> MutWasmRef<T>
    {
        let raw_ptr = unsafe { source.data_ptr(store).offset(self.ptr as isize) as *mut T };
        let value = unsafe { &mut *raw_ptr };
        MutWasmRef {
            value
        }
    }

    pub fn as_shared_ref(&self, source: &SharedMemory) -> WasmRef<T> {

        let raw_ptr = unsafe { source.data()[self.ptr as usize].get() as *const T };
        let value= unsafe { &*raw_ptr };
        WasmRef {
            value
        }
    }

    pub fn as_shared_ref_mut(&mut self, source: &SharedMemory) -> MutWasmRef<T> {
        let raw_ptr = source.data()[self.ptr as usize].get() as *mut T;
        let value = unsafe { &mut *raw_ptr };
        MutWasmRef {
            value
        }
    }
}

impl<T> Deref for WasmRef<'_, T> {
    type Target = T;

    fn deref(&self) -> &Self::Target {
        self.value
    }
}

impl<T> Deref for MutWasmRef<'_, T> {
    type Target = T;

    fn deref(&self) -> &Self::Target {
        self.value
    }
}

impl<T> DerefMut for MutWasmRef<'_, T> {
    fn deref_mut(&mut self) -> &mut Self::Target {
        self.value
    }
}