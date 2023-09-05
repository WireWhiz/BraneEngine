use std::ffi::OsStr;
use std::fs;
use std::io::Read;
use std::path::{Path, PathBuf};
use crate::runtime::component;
use std::process::Command;

#[derive(Clone, Debug)]
pub struct BehaviourPack {
    pub name: String,
    pub module_data: Vec<u8>,
    pub jobs: Vec<String>,
    pub components: Vec<component::ComponentDef>
}

impl BehaviourPack {

    /// Extracts metadata from script files in a crate.
    pub fn extract_crate_metadata(path: &Path, pack: &mut BehaviourPack) -> Result<(), Box<dyn std::error::Error>>
    {
        let files = find_rs_files_in_crate(path)?;

        for file in files {
            let mut file = fs::File::open(file)?;
            let mut contents = String::new();

            file.read_to_string(&mut contents)?;
            let syntax = syn::parse_file(&contents)?;

            syntax.items.iter().for_each(|item| {
                match item {
                    syn::Item::Fn(item_fn) => {
                        if item_contains_attr(&item_fn.attrs, "system") {
                            let identifier = item_fn.sig.ident.to_string();
                            pack.jobs.push(identifier);
                        }
                    },
                    syn::Item::Struct(item_struct) => {
                        if item_contains_attr(&item_struct.attrs, "component") {
                            let identifier = item_struct.ident.to_string();

                            pack.components.push(component::ComponentDef {
                                name: identifier,
                            });
                        }
                    }
                    _ => {}
                }
            });
        }
        Ok(())
    }

    /// Compiles a crate into a behaviour pack.
    pub fn compile_crate(path: &str, release: bool) -> Result<BehaviourPack, Box<dyn std::error::Error>> {
        let mut behaviour_pack = BehaviourPack {
            name: String::new(),
            module_data: vec![],
            jobs: vec![],
            components: vec![]
        };

        let crate_root = Path::canonicalize(Path::new(path))?;
        BehaviourPack::extract_crate_metadata(&crate_root, &mut behaviour_pack)?;

        //Compile the crate
        let mut command = Command::new("cargo");
        command.arg("+nightly")
        .arg("build")
        .arg("--target")
        .arg("wasm32-unknown-unknown")
        .arg("-Z")
        .arg("build-std=std,panic_abort")
        .env("RUSTFLAGS", "--cfg=web_sys_unstable_apis -C target-feature=+atomics,+bulk-memory,+mutable-globals -C link-arg=--shared-memory -C link-arg=--import-memory -C link-arg=--max-memory=2147483648")
        .current_dir(crate_root.clone());

        if release {
            command.arg("--release");
        }

        let cmd_res = command.output()?;
        std::fs::write(crate_root.join("build.log"), cmd_res.stdout)?;

        if !cmd_res.status.success() {
            std::fs::write(crate_root.join("build_error.log"), cmd_res.stderr)?;
            return Err(format!("Failed to compile crate at '{:?}'!", crate_root).into());
        }

        let output_dir = crate_root.join("target").join("wasm32-unknown-unknown").join(if release { "release" } else { "debug" });
        if !Path::exists(&output_dir) {
            return Err(format!("Output directory at '{:?}' was not found!", output_dir).into());
        }

        for path in std::fs::read_dir(output_dir)? {
            if let Err(_) = path {
                continue;
            }

            let path = path.unwrap().path();
            if path.extension().unwrap_or_default() != "wasm" {
                continue;
            }

            behaviour_pack.name = path.file_stem().unwrap_or(OsStr::new("err_name")).to_str().unwrap().to_string();
            behaviour_pack.module_data = std::fs::read(path)?;
        }

        Ok(behaviour_pack)
    }
}


// Metadata helpers
fn find_rs_files_in_crate(path: &Path) -> Result<Vec<PathBuf>, Box<dyn std::error::Error>>
{
    let mut files = Vec::new();
    for entry in fs::read_dir(path)? {
        let entry = entry?;
        let path = entry.path();

        if path.is_dir() {
            let mut child_files = find_rs_files_in_crate(path.as_path())?;
            files.append(&mut child_files);
        } else if let Some(extension) = path.extension() {
            let e = extension.to_str().unwrap();
            if e == "rs" {
                files.push(PathBuf::from(path));
            }
        }
    };
    Ok(files)
}

fn item_contains_attr(attrs: &Vec<syn::Attribute>, attr_name: &str) -> bool {
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

fn parse_type(ty: &syn::Type) -> Option<component::VarType>
{
    match ty {
        syn::Type::Path(path) => {
            Some(component::VarType {
                type_name: path.path.segments.iter().map(|s|s.ident.to_string()).reduce(|a, b| {
                    a + "::" + b.as_str()
                }).unwrap()
            })
        },
        _ => None
    }
}