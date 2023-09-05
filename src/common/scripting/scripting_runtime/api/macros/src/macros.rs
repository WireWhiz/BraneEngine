extern crate proc_macro;
use proc_macro::{TokenStream};
use proc_macro_error::{emit_error, proc_macro_error};
use syn::{ItemStruct, ItemFn};
use quote::quote;

fn parse_type_name(ty: &syn::Type) -> String
{
    match ty {
        syn::Type::Path(path) => {
            path.path.segments.iter().map(|s|s.ident.to_string()).reduce(|a, b| {
                a + "::" + b.as_str()
            }).unwrap()
        },
        _ => {
            "unknown".to_string()
        }
    }
}

#[proc_macro_error]
#[proc_macro_attribute]
pub fn component(_attr: TokenStream, item: TokenStream) -> TokenStream {
    let component = syn::parse::<ItemStruct>(item.clone());
    if let Err(err) = component {
        emit_error!(err.span(), "Failed to parse component: {}", err);
        return item;
    }

    let mut component = component.unwrap();

    match component.vis {
        syn::Visibility::Public(_) => { },
        _ => {
            emit_error!(component.ident.span(), "Components must be public!");
            return item;
        }
    }

    component.attrs.push(syn::parse_quote! {
        #[repr(C)]
    });

    //TODO make sure member types are compatable with the ecs framework?

    let ident_str = component.ident.to_string();
    let ident = syn::Ident::new(ident_str.as_str(), component.ident.span());
    let info_global_sig = syn::Ident::new((String::from("be_info_data_") + &ident_str).as_str(), component.ident.span());
    let info_global_getter_sig = syn::Ident::new((String::from("be_info_") + &ident_str).as_str(), component.ident.span());
    let clone_method_sig = syn::Ident::new((String::from("be_clone_") + &ident_str).as_str(), component.ident.span());
    let drop_method_sig = syn::Ident::new((String::from("be_drop_") + &ident_str).as_str(), component.ident.span());

    let field_info_elements = component.fields.iter().map(|field| {
        let field_name = field.ident.as_ref().unwrap().clone();
        let field_type = field.ty.clone();
        let field_type_name = syn::LitStr::new(parse_type_name(&field.ty).as_str(), field_name.span());
        quote! {
            brane_engine_api::ComponentFieldInfo{
                //offset: unsafe { (&(*(null() as *const #ident)).#field_name) as *const #field_type} as i32,
                offset: brane_engine_api::offset_of!(#ident, #field_name) as i32,
                size: std::mem::size_of::<#field_type>() as i32,
                ty: #field_type_name
            }
        }
    }).reduce(|a, b| quote! {
        #a,
        #b
    }).unwrap();



    let out = quote! {
        #component

        static #info_global_sig : brane_engine_api::ComponentInfo = brane_engine_api::ComponentInfo {
            size: std::mem::size_of::<#ident>() as i32,
            fields: &[#field_info_elements]
        };

        #[no_mangle]
        pub extern "C" fn #info_global_getter_sig () -> *const brane_engine_api::ComponentInfo {
            unsafe {
                &#info_global_sig as *const brane_engine_api::ComponentInfo
            }
        }


        #[no_mangle]
        pub extern "C" fn #clone_method_sig (dest : *mut #ident, src : *const #ident) {
            unsafe {
                *dest = (*src).clone();
            }
        }

        #[no_mangle]
        pub extern "C" fn #drop_method_sig (component : *mut #ident) {
            unsafe {
                let data = Box::from_raw(component);
                drop(data);
            }
        }
    };

    eprintln!("{}", out);
    TokenStream::from(out)
}

#[proc_macro_error]
#[proc_macro_attribute]
pub fn system(_attr: TokenStream, item: TokenStream) -> TokenStream {
    let system = syn::parse::<ItemFn>(item.clone());
    if let Err(err) = system {
        emit_error!(err.span(), "Failed to parse system: {}", err);
        return item;
    }

    let mut system = system.unwrap();

    match system.vis {
        syn::Visibility::Public(_) => { },
        _ => {
            emit_error!(system.sig.ident.span(), "Systems must be public!");
            return item;
        }
    }

    system.attrs.push(syn::parse_quote! {
        #[no_mangle]
    });

    system.sig.abi = Some(syn::parse_quote! {
        extern "C"
    });

    let out = quote! {
        #system
    };

    TokenStream::from(out)
}
