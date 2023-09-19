use std::ops::Deref;
use wasmtime::AsContextMut;
use crate::runtime::store_handle::StoreHandle;

#[derive(Clone, Debug)]
pub enum JobAttribute {
    OrderBefore(String),
    OrderAfter(String)
}

#[derive(Clone, Debug)]
pub struct JobInfo {
    pub name: String,
    pub attributes: Vec<JobAttribute>
}

pub trait Job {
    fn info(&self) -> &JobInfo;
    fn call(&self) -> Result<(), Box<dyn std::error::Error>>;
}

pub struct WasmJob {
    pub info: JobInfo,
    pub store_handle: StoreHandle,
    pub handle: wasmtime::TypedFunc<(), ()>
}

impl Job for WasmJob {
    fn info(&self) -> &JobInfo {
        &self.info
    }

    fn call(&self) -> Result<(), Box<dyn std::error::Error>>
    {
        let mut store = self.store_handle.store.borrow_mut();
        self.handle.call(store.as_context_mut(), ())?;
        Ok(())
    }
}

impl std::fmt::Debug for WasmJob {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("Job")
            .field("name", &self.info.name)
            .finish()
    }
}