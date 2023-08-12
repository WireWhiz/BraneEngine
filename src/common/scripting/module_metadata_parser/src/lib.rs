mod be_ffi_types;

pub mod module_metadata_parser {
    use std::ffi::{c_char, CString};
    use std::io::Read;
    use std::error::Error;
    use std::fs;
    use std::path::PathBuf;
    use proc_macro2::TokenStream;
    use syn::{DeriveInput};
    use syn::__private::ToTokens;
    use crate::be_ffi_types::be_ffi_types::{BEVec, BEStr};

    #[repr(C)]
    #[derive(Clone)]
    pub struct ComponentDef {
        pub name: BEStr
    }

    #[repr(C)]
    #[derive(Clone)]
    pub struct SystemDef {
        pub name: BEStr
    }

    #[repr(C)]
    pub struct ModuleMetadata {
        pub components: BEVec<ComponentDef>,
        pub systems: BEVec<SystemDef>
    }

    impl ModuleMetadata {
        pub fn new() -> ModuleMetadata {
            let metadata = ModuleMetadata {
                systems: BEVec::new(),
                components: BEVec::new(),
            };
            metadata
        }
    }

    #[no_mangle]
    pub extern fn free_module_metadata(metadata: *mut ModuleMetadata) {
        unsafe {
            let data = Box::from_raw(metadata);
            drop(data);
        }
    }

    fn find_rs_files_in_dir(path : &str) -> Result<Vec<PathBuf>, Box<dyn Error>>
    {
        let mut files = Vec::new();
        for entry in fs::read_dir(path)? {
            let entry = entry?;
            let path = entry.path();

            if path.is_dir() {
                find_rs_files_in_dir(path.to_str().unwrap())?;
            } else if let Some(extension) = path.extension() {
                let e = extension.to_str().unwrap();
                if e == "rs" {
                    files.push(PathBuf::from(path));
                }
            }
        };
        Ok(files)
    }

    fn contains_attr(attrs: &Vec<syn::Attribute>, attr_name: &str) -> bool {
        attrs.iter().find(|attr| {
            if let syn::AttrStyle::Inner(t) = attr.style {
                return false;
            }
            match attr.meta {
                syn::Meta::Path(ref path) => {
                    if path.segments.len() == 1 && path.segments[0].ident == attr_name {
                        return true;
                    }
                },
                syn::Meta::List(ref list) => {
                    if list.path.segments.len() == 1 && list.path.segments[0].ident == attr_name {
                        return true;
                    }
                },
                _ => {}
            }
            false
        }).is_some()
    }

    #[no_mangle]
    pub extern fn build_module_metadata(search_dir: *const c_char) -> *mut ModuleMetadata {

        let c_search_dir = unsafe { std::ffi::CStr::from_ptr(search_dir) }.to_str();

        if c_search_dir.is_err() {
            return std::ptr::null_mut();
        }

        let metadata = build_module_metadata_internal(c_search_dir.unwrap());
        if metadata.is_err() {
            return std::ptr::null_mut();
        }
        Box::into_raw(Box::new(metadata.unwrap()))
    }

    pub fn build_module_metadata_internal(search_dir: &str) -> Result<ModuleMetadata, Box<dyn Error>> {
        let mut metadata = ModuleMetadata::new();

        let definitions = find_rs_files_in_dir(search_dir)?;
        for definition in definitions {
            let mut file = fs::File::open(definition)?;
            let mut contents = String::new();

            file.read_to_string(&mut contents)?;
            let syntax = syn::parse_file(&contents)?;

            syntax.items.iter().for_each(|item| {
                match item {
                    syn::Item::Fn(item_fn) => {
                        if contains_attr(&item_fn.attrs, "system") {
                            let identifier = CString::new(item_fn.sig.ident.to_string());
                            if identifier.is_err() {
                                eprint!("Failed to convert identifier for system \"{}\" to CString!", item_fn.sig.ident);
                                return;
                            }
                            metadata.systems.push(SystemDef {
                                name: BEStr::from(identifier.unwrap())
                            });
                        }
                    },
                    syn::Item::Struct(item_struct) => {
                        if contains_attr(&item_struct.attrs, "component") {
                            let identifier = CString::new(item_struct.ident.to_string());
                            if identifier.is_err() {
                                eprint!("Failed to convert identifier for component \"{}\" to CString!", item_struct.ident);
                                return;
                            }
                            metadata.components.push(ComponentDef {
                                name: BEStr::from(identifier.unwrap())
                            });
                        }
                    }
                    _ => {}
                }
            });
        }
        Ok(metadata)
    }
}



