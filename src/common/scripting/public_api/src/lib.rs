pub use brane_engine_api_macros::{component, job, order_after, order_before};
pub use memoffset::offset_of;

#[repr(align(4))]
#[repr(C)]
pub struct ComponentFieldInfo {
    pub offset: u32,
    pub size: u32,
    pub name: &'static str,
    pub ty: &'static str,

}

#[repr(align(4))]
#[repr(C)]
pub struct ComponentInfo {
    pub size: u32,
    pub fields: &'static [ComponentFieldInfo]
}


