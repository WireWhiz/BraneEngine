pub use brane_engine_api_macros::{component, system};
pub use memoffset::offset_of;

#[repr(C)]
pub struct ComponentFieldInfo {
    pub offset: i32,
    pub size: i32,
    pub ty: &'static str,

}

#[repr(C)]
pub struct ComponentInfo {
    pub size: i32,
    pub fields: &'static [ComponentFieldInfo]
}



