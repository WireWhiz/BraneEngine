use std::fs::File;
use std::io::Read;
use std::mem::size_of;
use std::ops::Index;
use std::sync::{Arc, Mutex};
use std::borrow::BorrowMut;
use wasmtime::{Config, Engine, Instance, Store, Module, MemoryType, Func, Extern, Linker, Caller, AsContextMut, AsContext};
use wasmtime_wasi::sync::WasiCtxBuilder;
use wasmtime_wasi_threads::WasiThreadsCtx;

use scripting_runtime::runtime::api_bindings::{WasmPtr, ComponentInfo, ComponentFieldInfo};

#[derive(Copy, Clone)]
#[repr(C)]
pub struct TestComponent {
    pub a: u8,
    pub b: i32,
    pub c: f32
}

fn load_module(path: &str, engine: &Engine) -> Result<Module, String> {
    let file = File::open(path);
    if let Err(e) = file {
        println!("Error opening file: {}", e.to_string());
        return Err(e.to_string());
    }
    let mut buf = Vec::new();
    let mut file = file.unwrap();
    let file_read = file.read_to_end(&mut buf);
    if let Err(e) = file_read {
        println!("Error reading file: {}", e.to_string());
        return Err(e.to_string());
    }
    let now = std::time::Instant::now();
    let module = Module::from_binary(&engine,&buf);
    if module.is_err() {
        let err = module.err().unwrap();
        println!("Error creating module: {}", err.to_string());
        return Err(err.to_string());
    }
    let module = module.unwrap();

    let module_instantiation_time = now.elapsed();
    println!("Module load time: {:?}", module_instantiation_time);
    Ok(module)
}

fn test_external_function() -> i32 {
    54
}

struct BEState {
    memory: Option<wasmtime::Memory>
}

fn be_print_external(text_ptr: u32) {
    /*let memory = caller.data().memory.as_ref().unwrap();
    let text;
    unsafe{
        let data = memory.data_ptr(caller.as_context()) as *const std::ffi::c_char;
        text = std::ffi::CStr::from_ptr(data.offset(text_ptr as isize));
    }
    println!("Wasm Print: {}", text.to_str().unwrap());*/
}

fn main() {
    let mut engine_config = Config::new();
    engine_config.wasm_threads(true);
    engine_config.wasm_bulk_memory(true);

    engine_config.debug_info(true);

    let engine = Engine::new(&engine_config).unwrap();
    let module1 = load_module("wasm_crate1.wasm", &engine).unwrap();

    let mut linker = Linker::new(&engine);

    let wasi_ctx = WasiCtxBuilder::new().inherit_stdio().build();
    let mut store = Store::new(&engine, wasi_ctx);

    wasmtime_wasi::add_to_linker(&mut linker, |s| s).unwrap();


    //wasmtime_wasi_threads::add_to_linker(&mut linker, &store, &module1, |s| s).unwrap();
    let module2 = load_module("wasm_crate2.wasm", &engine).unwrap();

    let main_memory = wasmtime::Memory::new(&mut store, MemoryType::shared(32, 32768)).unwrap();
    linker.define(&mut store, "env", "memory", Extern::Memory(main_memory.clone())).unwrap();
    let func1 = Func::wrap(&mut store, be_print_external);
    linker.define(&mut store, "BraneEngine", "extern_be_print", func1).unwrap();
    let func2 = Func::wrap(&mut store, test_external_function);
    linker.define(&mut store, "BraneEngine", "test_external_function", func2).unwrap();

    let now = std::time::Instant::now();

    let instance1 = linker.module(&mut store, "module1", &module1);
    if instance1.is_err() {
        println!("Was unable to instantiate module1: {:?}", instance1.err().unwrap());
        return;
    }
    let instanceInstantiationTime = now.elapsed();
    println!("Instance1 instantiation time: {:?}", instanceInstantiationTime);

    let now = std::time::Instant::now();
    let instance2 = linker.module(&mut store, "module2", &module2);
    if instance2.is_err() {
        println!("Was unable to instantiate module2: {:?}", instance2.err().unwrap());
        return;
    }
    let instanceInstantiationTime = now.elapsed();
    println!("Instance2 instantiation time: {:?}", instanceInstantiationTime);

    /*let threads_test = linker.get(&mut store, "module1","threads_test").unwrap().into_func().unwrap().typed::<(), (),>(&store).unwrap();
    println!("Starting threads test");
    threads_test.call(&mut store, ()).unwrap();
    println!("Threads test returned to host env");
    std::thread::sleep(std::time::Duration::from_millis(5000));*/

    let test_function = linker.get(&mut store, "module1","test_function").unwrap().into_func().unwrap().typed::<i32, i32,>(&store).unwrap();
    let res = test_function.call(&mut store, 42).unwrap();
    println!("Test function returned {}", res);

    let test_extern_call = linker.get(&mut store, "module1","test_extern_call").unwrap().into_func().unwrap().typed::<(), i32,>(&store).unwrap();
    let res = test_extern_call.call(&mut store, ()).unwrap();
    println!("Test extern call returned {}", res);

    let test_component_ref;
    let create_test_component = linker.get(&mut store, "module1","create_test_component").unwrap().into_func().unwrap().typed::<(), i32,>(&store).unwrap();

    match create_test_component.call(&mut store, ()) {
        Ok(res) => {
            test_component_ref = res;
            println!("Create test component returned {}", res);

            let test_component = unsafe { main_memory.data_ptr(&mut store).offset(res as isize) } as *mut TestComponent;
            unsafe {
                println!("Test component values: a = {}, b = {}, c = {}", (*test_component).a, (*test_component).b, (*test_component).c);
            }

        },
        Err(err) => {
            eprintln!("create_test_component failed: {}", err.to_string());
            return;
        }
    }

    let test_component_access = linker.get(&mut store, "module2","test_component_access").unwrap().into_func().unwrap().typed::<i32, i32,>(&store).unwrap();
    let res = test_component_access.call(&mut store, test_component_ref).unwrap();

    println!("Test component access returned {}", res);
    let test_component = unsafe { main_memory.data_ptr(&mut store).offset(res as isize) } as *mut TestComponent;
    unsafe {
        println!("Test component values: a = {}, b = {}, c = {}", (*test_component).a, (*test_component).b, (*test_component).c);
    }
    let imports = module1.imports();
    for i in imports {
        println!("Instance2 imports: {} of type {:?}", i.name(), i.ty());
    }

    let exports = module1.exports();

    for e in exports {
        println!("Instance1 exports: {} of type {:?}", e.name(), e.ty());
    }

    let test_component_info = linker.get(&mut store, "module2","be_info_TestComponent").unwrap().into_func().unwrap().typed::<(), u32,>(&store).unwrap();

    let test_component_info_ptr = test_component_info.call(&mut store, ()).unwrap();
    println!("Test component info returned {}", test_component_info_ptr);
    println!("Memory is of size: {}", main_memory.size(&mut store) * 65536);
    assert!(test_component_info_ptr < main_memory.size(&mut store) as u32 * 65536); //Make sure the pointer is within the bounds of the memory
    let test_component_info_ptr: WasmPtr<ComponentInfo> = WasmPtr::new(test_component_info_ptr);
    let test_component_info = test_component_info_ptr.as_ref(&main_memory, &store);
    println!("Test component of size {} contains fields:", test_component_info.size);

    let fields = test_component_info.fields.as_ref(&main_memory, &store);

    let mut i = 0;
    for f in fields.iter() {
        let ty = f.ty.as_str(&main_memory, &store);
        println!("\t{}: offset = {}, size = {}, type = {}", i, f.offset, f.size, ty);
        i += 1;
    }

    let mut threads = vec![];
    let mut store = Arc::new(Mutex::new(store));
    for i in 0..10 {
        let ctx = store.lock().unwrap().as_context_mut();
        let ctx2 = store.lock().unwrap().as_context_mut();
        let test_function = linker.get(ctx, "module1","test_function").unwrap().into_func().unwrap().typed::<i32, i32,>(ctx2).unwrap();
        let mut store_ref = store.clone();
        threads.push(std::thread::spawn(move ||{
            let ctx;
            {
                let mut store_borrow = store_ref.borrow_mut().lock().unwrap();
                ctx = store_borrow.as_context_mut();
            }
            test_function.call(ctx, i).unwrap();
        }));
    }
    for t in threads {
        t.join().unwrap();
    }
}
