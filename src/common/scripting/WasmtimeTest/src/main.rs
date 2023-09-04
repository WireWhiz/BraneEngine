use std::fs::File;
use std::io::Read;
use std::ops::Index;
use wasmtime::{Config, Engine, Instance, Store, Module, Memory, MemoryType, Func, Extern, Caller, AsContextMut, AsContext};

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
    memory: Option<Memory>
}

fn be_print_external(mut caller: Caller<'_, BEState>, text_ptr: u32) {
    let memory = caller.data().memory.as_ref().unwrap();
    let text;
    unsafe{
        let data = memory.data_ptr(caller.as_context()) as *const std::ffi::c_char;
        text = std::ffi::CStr::from_ptr(data.offset(text_ptr as isize));
    }
    println!("Wasm Print: {}", text.to_str().unwrap());
}

fn main() {
    let mut engine_config = Config::new();
    engine_config.wasm_threads(true);
    engine_config.wasm_bulk_memory(true);

    engine_config.debug_info(true);

    let engine = Engine::new(&engine_config).unwrap();


    let mut store = Store::new(&engine, BEState{
        memory: None
    });



    let module1 = load_module("wasm_crate1.wasm", &engine).unwrap();
    let module2 = load_module("wasm_crate2.wasm", &engine).unwrap();

    let main_memory = Memory::new(&mut store, MemoryType::shared(32, 32768)).unwrap();
    store.data_mut().memory = Some(main_memory.clone());

    let func1 = Func::wrap(&mut store, be_print_external);
    let func2 = Func::wrap(&mut store, test_external_function);

    let now = std::time::Instant::now();

    let imports1 = [Extern::Memory(main_memory.clone()), Extern::Func(func1), Extern::Func(func2)];
    let instance1 = Instance::new(&mut store, &module1, &imports1);
    if instance1.is_err() {
        println!("Was unable to instantiate module1: {:?}", instance1.err().unwrap());
        return;
    }
    let instanceInstantiationTime = now.elapsed();
    println!("Instance1 instantiation time: {:?}", instanceInstantiationTime);
    let instance1 = instance1.unwrap();

    let now = std::time::Instant::now();
    let imports2 = [Extern::Memory(main_memory.clone()), Extern::Func(instance1.get_typed_func::<(), i32>(&mut store, "create_test_component").unwrap().func().clone())];
    let instance2 = Instance::new(&mut store, &module2, &imports2);
    if instance2.is_err() {
        println!("Was unable to instantiate module1: {:?}", instance2.err().unwrap());
        return;
    }
    let instanceInstantiationTime = now.elapsed();
    println!("Instance2 instantiation time: {:?}", instanceInstantiationTime);
    let instance2 = instance2.unwrap();

    let test_function = instance1.get_typed_func::<i32, i32>(&mut store,"test_function").unwrap();
    let res = test_function.call(&mut store, 42).unwrap();
    println!("Test function returned {}", res);

    let test_extern_call = instance1.get_typed_func::<(), i32>(&mut store,"test_extern_call").unwrap();
    let res = test_extern_call.call(&mut store, ()).unwrap();
    println!("Test extern call returned {}", res);

    let test_component_ref;
    let create_test_component = instance2.get_typed_func::<(), i32>(&mut store,"test_imported_create_test_component").unwrap();
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

    let test_component_access = instance2.get_typed_func::<i32, i32>(&mut store,"test_component_access").unwrap();
    let res = test_component_access.call(&mut store, test_component_ref).unwrap();

    println!("Test component access returned {}", res);
    let test_component = unsafe { main_memory.data_ptr(&mut store).offset(res as isize) } as *mut TestComponent;
    unsafe {
        println!("Test component values: a = {}, b = {}, c = {}", (*test_component).a, (*test_component).b, (*test_component).c);
    }
}
