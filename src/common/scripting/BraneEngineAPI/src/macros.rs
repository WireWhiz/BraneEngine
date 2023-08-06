extern crate proc_macro;
use proc_macro_error::{proc_macro_error, emit_error};
use proc_macro::{Span, TokenStream, TokenTree, Ident};
use std::cell::RefCell;
use std::collections::HashMap;
use toml::{Table, Value};
use lazy_static::lazy_static;
use std::io::prelude::*;
use std::sync::Mutex;
use syn::{parse_macro_input};

#[proc_macro_attribute]
pub fn print_token_stream(_attr: TokenStream, item: TokenStream) -> TokenStream {
    let mut diagnostic_data = String::new();

    for token_tree in item.clone().into_iter() {
        //diagnostic_data += &parse_token_tree(token_tree);
    }

    panic!("Requested TokenStream: {}", diagnostic_data);
}

#[proc_macro_error]
#[proc_macro_attribute]
pub fn component(_attr: TokenStream, item: TokenStream) -> TokenStream {
    let mut is_pub = false;
    let mut is_extern_c = false;
    let mut next_token_is_ident = false;

    let mut identifier = String::new();

    for token_tree in item.clone().into_iter() {
        if let TokenTree::Ident(ident) = token_tree {

            if next_token_is_ident {
                if !is_pub {
                    emit_error!(ident.span(), "Components must be public!");
                    return item;
                }
                identifier = ident.to_string();
                break;
            }

            if ident.to_string() == "pub" {
                is_pub = true;
            } else if ident.to_string() == "fn" {
                let ident_span = ident.span();
                emit_error!(ident_span, "Components must be a struct!");
                return item;
            } else if ident.to_string() == "struct" {
                next_token_is_ident = true;
            } else if ident.to_string() == "extern" {
                is_extern_c = true;
            }
        }
    }

    let defs_ref = OPEN_DEFS.lock().unwrap();
    let mut defs = defs_ref.borrow_mut();
    let cf = file!().to_string();

    let script_def = if defs.contains_key(cf.as_str()) {
        defs.get_mut(cf.as_str()).unwrap()
    } else {
        defs.insert(cf.clone(), ScriptDefinition::new(cf.clone()));
        defs.get_mut(cf.as_str()).unwrap()
    };

    item
}

#[proc_macro_error]
#[proc_macro_attribute]
pub fn system(_attr: TokenStream, item: TokenStream) -> TokenStream {

    let mut is_pub = false;
    let mut is_extern_c = false;
    let mut next_token_is_ident = false;

    let mut identifier = String::new();

    for token_tree in item.clone().into_iter() {
        if let TokenTree::Ident(ident) = token_tree {

            if next_token_is_ident {
                if !is_pub {
                    emit_error!(ident.span(), "Systems must be public!");
                    return item;
                }
                identifier = ident.to_string();
                break;
            }

            if ident.to_string() == "pub" {
                is_pub = true;
            } else if ident.to_string() == "fn" {
                next_token_is_ident = true;
            } else if ident.to_string() == "struct" {
                let ident_span = ident.span();
                emit_error!(ident_span, "Systems must be a function!");
                return item;
            } else if ident.to_string() == "extern" {
                is_extern_c = true;
            }
        }
    }

    let defs_ref = OPEN_DEFS.lock().unwrap();
    let mut defs = defs_ref.borrow_mut();
    let cf = file!().to_string();

    let script_def = if defs.contains_key(cf.as_str()) {
        defs.get_mut(cf.as_str()).unwrap()
    } else {
        defs.insert(cf.clone(), ScriptDefinition::new(file!().to_string()));
        defs.get_mut(cf.as_str()).unwrap()
    };

    item
}
