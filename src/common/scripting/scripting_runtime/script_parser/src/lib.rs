use std::ffi::{c_char, CString};
use std::io::Read;
use std::error::Error;
use std::fs;
use std::path::PathBuf;

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



