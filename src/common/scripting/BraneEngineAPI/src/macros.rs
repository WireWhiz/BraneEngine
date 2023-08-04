extern crate proc_macro;
use proc_macro_error::{proc_macro_error, emit_error};
use proc_macro::{TokenStream, TokenTree};
use std::cell::RefCell;
use std::collections::HashMap;
use toml::{Table, Value};
use lazy_static::lazy_static;
use std::io::prelude::*;
use std::mem::swap;
use std::sync::Mutex;

fn parse_token_tree(token_tree: TokenTree) -> String {
    match token_tree {
        TokenTree::Ident(ident) => {
            format!("Ident: {}\n", ident)
        },
        TokenTree::Punct(punct) => {
            format!("Punct: {}\n", punct)
        },
        TokenTree::Literal(literal) => {
            format!("Literal: {}\n", literal)
        },
        TokenTree::Group(group) => {
            let delim;
            match group.delimiter() {
                proc_macro::Delimiter::Parenthesis => delim = "()",
                proc_macro::Delimiter::Brace => delim = "{}",
                proc_macro::Delimiter::Bracket => delim = "[]",
                proc_macro::Delimiter::None => delim = "None",
            }
            let mut out = String::new();

            out += &format!("Group: {} [\n", delim);
            for token_tree in group.stream().into_iter() {
                out += &parse_token_tree(token_tree);
            }
            out += "]\n";
            out
        },
    }
}

#[proc_macro_attribute]
pub fn print_token_stream(_attr: TokenStream, item: TokenStream) -> TokenStream {
    let mut diagnostic_data = String::new();

    for token_tree in item.clone().into_iter() {
        diagnostic_data += &parse_token_tree(token_tree);
    }

    panic!("Requested TokenStream: {}", diagnostic_data);
}


struct ScriptDefinition {
    components : Vec<String>,
    systems: Vec<String>,
}

impl ScriptDefinition {
    fn new() -> ScriptDefinition {
        ScriptDefinition {
            components: Vec::new(),
            systems: Vec::new(),
        }
    }

    fn save(&self) {
        println!("Components: {:?}", self.components);
        println!("Systems: {:?}", self.systems);

        let mut toml= Table::new();

        toml.insert("component".to_string(), Value::Array(self.components.iter().map(|s| {
            let mut component = Table::new();
            component.insert("name".to_string(), Value::String(s.clone()));
            Value::Table(component)
        }).collect()));

        toml.insert("system".to_string(), Value::Array(self.systems.iter().map(|s| {
            let mut system = Table::new();
            system.insert("name".to_string(), Value::String(s.clone()));
            Value::Table(system)
        }).collect()));

        println!("TOML: {}", toml::to_string(&toml).unwrap());

        let mut output_path = std::path::PathBuf::from(std::env::var("OUT_DIR").unwrap());
        //output_path.push("script_definitions");

        let file_path = std::path::PathBuf::from(file!().to_string());

        output_path.push(file_path.file_name().unwrap().to_str().unwrap().to_string() + ".toml");

        let mut binding_file = std::fs::File::create(output_path).unwrap();
        binding_file.write_all(toml::to_string(&toml).unwrap().as_bytes()).unwrap();
    }
}

lazy_static! {
    static ref OPEN_DEFS: Mutex<RefCell<HashMap<String, ScriptDefinition>>> = Mutex::new(RefCell::new(HashMap::new()));
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
        defs.insert(cf.clone(), ScriptDefinition::new());
        defs.get_mut(cf.as_str()).unwrap()
    };

    script_def.components.push(identifier);
    script_def.save();
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
        defs.insert(cf.clone(), ScriptDefinition::new());
        defs.get_mut(cf.as_str()).unwrap()
    };

    script_def.systems.push(identifier);
    script_def.save();
    item
}
