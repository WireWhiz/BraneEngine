extern crate proc_macro;
use proc_macro::{TokenStream};
use proc_macro_error::{emit_error, proc_macro_error};
use syn::{ItemStruct, ItemFn};
use syn::{parse_quote};
use quote::quote;

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
    let clone_method_sig = syn::Ident::new((String::from("be_clone_") + &ident_str).as_str(), component.ident.span());
    let drop_method_sig = syn::Ident::new((String::from("be_drop_") + &ident_str).as_str(), component.ident.span());


    let out = quote! {
        #component

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
