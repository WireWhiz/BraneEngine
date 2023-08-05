use std::io::{Error, Read};
use std::fs;
use std::path::PathBuf;
use proc_macro2::TokenStream;
use syn::{DeriveInput, parse_macro_input};
use syn::__private::ToTokens;

fn find_rs_files_in_dir(path : &str) -> Result<Vec<PathBuf>, Error>
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

fn main() {
    println!("cargo:warning={}", "Extracting script definitions...");
    let definitions = find_rs_files_in_dir(std::env::current_dir().unwrap().to_str().unwrap()).unwrap();
    println!("cargo:warning={}", "Found script definitions:\n");
    for definition in definitions {
        println!("cargo:warning={}", definition.display());
        let mut file = fs::File::open(definition).unwrap();
        let mut contents = String::new();
        file.read_to_string(&mut contents).unwrap();

        let syntax = syn::parse_file(&contents).unwrap();
        syntax.items.iter().for_each(|item| {
            match item {
                syn::Item::Fn(item_fn) => {
                    println!("cargo:warning={}", format!("   Found function: {}", item_fn.sig.ident));
                },
                syn::Item::Struct(item_struct) => {
                    println!("cargo:warning={}", format!("   Found struct: {} (is component {})", item_struct.ident, item_struct.attrs.iter().find(|attr| {
                        if let syn::AttrStyle::Inner(t) = attr.style {
                            return false;
                        }
                        match attr.meta {
                            syn::Meta::Path(ref path) => {
                                if path.segments.len() == 1 && path.segments[0].ident == "component" {
                                    return true;
                                }
                            },
                            syn::Meta::List(ref list) => {
                                if list.path.segments.len() == 1 && list.path.segments[0].ident == "component" {
                                    return true;
                                }
                            },
                            _ => {}
                        }
                        false
                    }).is_some()));
                },
                syn::Item::Impl(item_impl) => {
                    println!("cargo:warning={}", format!("   Found impl: {}", item_impl.self_ty.to_token_stream().to_string()));
                },
                _ => {}
            }
        });
    }
}