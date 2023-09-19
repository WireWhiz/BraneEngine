extern crate proc_macro;
use proc_macro::{Span, TokenStream};
use proc_macro_error::{emit_error, proc_macro_error};
use syn::{ItemStruct, ItemFn};
use quote::quote;

fn parse_type_name(ty: &syn::Type) -> String
{
    match ty {
        syn::Type::Path(path) => {
            parse_path(&path.path)
        },
        _ => {
            "unknown".to_string()
        }
    }
}


fn parse_path(path: &syn::Path) -> String
{
    path.segments.iter().map(|s|s.ident.to_string()).reduce(|a, b| {
        a + "::" + b.as_str()
    }).unwrap()
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
        let field_id = field.ident.as_ref().unwrap().clone();
        let field_name = syn::LitStr::new(field.ident.as_ref().unwrap().to_string().as_str(), field_id.span());
        let field_type = field.ty.clone();
        let field_type_name = syn::LitStr::new(parse_type_name(&field.ty).as_str(), field_id.span());
        quote! {
            brane_engine_api::ComponentFieldInfo{
                //offset: unsafe { (&(*(null() as *const #ident)).#field_name) as *const #field_type} as i32,
                offset: brane_engine_api::offset_of!(#ident, #field_id) as u32,
                size: std::mem::size_of::<#field_type>() as u32,
                name: #field_name,
                ty: #field_type_name
            }
        }
    }).reduce(|a, b| quote! {
        #a,
        #b
    }).unwrap();



    let out = quote! {
        #component

        #[allow(non_upper_case_globals)]
        static #info_global_sig : brane_engine_api::ComponentInfo = brane_engine_api::ComponentInfo {
            size: std::mem::size_of::<#ident>() as u32,
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

    TokenStream::from(out)
}

#[proc_macro_error]
#[proc_macro_attribute]
pub fn job(_attr: TokenStream, item: TokenStream) -> TokenStream {
    let job = syn::parse::<ItemFn>(item.clone());
    if let Err(err) = job {
        emit_error!(err.span(), "Failed to parse system: {}", err);
        return item;
    }

    let mut job = job.unwrap();

    match job.vis {
        syn::Visibility::Public(_) => { },
        _ => {
            emit_error!(job.sig.ident.span(), "Systems must be public!");
            return item;
        }
    }

    job.attrs.push(syn::parse_quote! {
        #[no_mangle]
    });

    job.sig.abi = Some(syn::parse_quote! {
        extern "C"
    });

    let out = quote! {
        #job


    };

    TokenStream::from(out)
}

#[proc_macro_error]
#[proc_macro_attribute]
pub fn order_before(_attr: TokenStream, item: TokenStream) -> TokenStream {
    if _attr.is_empty() {
        panic!("order_before expects a comma seperated list of functions! example: order_before(a, b, c)");
    }

    let mut last_was_ident = false;
    let mut last_span = Span::call_site();
    for a in _attr
    {
        match a {
            proc_macro::TokenTree::Ident(i) => {
                if last_was_ident {
                    emit_error!(i.span(), "unexpected id \"{}\" expecting \"{}\"", i, if last_was_ident {","} else {"identifier"});
                }
                last_was_ident = true;
            } ,
            proc_macro::TokenTree::Punct(p) => {
                if p.as_char() != ',' {
                    emit_error!(p.span(), "unexpected punctuation \"{}\" expecting \"{}\"", p.as_char(), if last_was_ident {","} else {"identifier"});
                }

                if !last_was_ident {
                    emit_error!(p.span(), "Unexpected comma, expecting identifier.")
                }

                last_was_ident = false;
                last_span = p.span();
            }
            _ => {
                emit_error!(a.span(), "unexpected \"{}\" expecting \"{}\"", a, if last_was_ident {","} else {"identifier"});
            }
        }
    }

    if !last_was_ident {
        emit_error!(last_span, "Expected identifier to follow this.")
    }

    item
}

#[proc_macro_error]
#[proc_macro_attribute]
pub fn order_after(_attr: TokenStream, item: TokenStream) -> TokenStream {
    if _attr.is_empty() {
        panic!("order_after expects a comma seperated list of functions! example: order_before(a, b, c)");
    }

    let mut last_was_ident = false;
    let mut last_span = Span::call_site();
    for a in _attr
    {
        match a {
            proc_macro::TokenTree::Ident(i) => {
                if last_was_ident {
                    emit_error!(i.span(), "unexpected id \"{}\" expecting \"{}\"", i, if last_was_ident {","} else {"identifier"});
                }
                last_was_ident = true;
            } ,
            proc_macro::TokenTree::Punct(p) => {
                if p.as_char() != ',' {
                    emit_error!(p.span(), "unexpected punctuation \"{}\" expecting \"{}\"", p.as_char(), if last_was_ident {","} else {"identifier"});
                }

                if !last_was_ident {
                    emit_error!(p.span(), "Unexpected comma, expecting identifier.")
                }

                last_was_ident = false;
                last_span = p.span();
            }
            _ => {
                emit_error!(a.span(), "unexpected \"{}\" expecting \"{}\"", a, if last_was_ident {","} else {"identifier"});
            }
        }
    }

    if !last_was_ident {
        emit_error!(last_span, "Expected identifier to follow this.")
    }

    item
}



