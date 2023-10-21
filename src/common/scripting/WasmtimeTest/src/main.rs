use std::fs::File;
use std::io::Read;
use std::sync::{Arc, Mutex};
use std::collections::VecDeque;
use wasmtime::{Config, Engine, SharedMemory, Store, Module, MemoryType, Linker, Caller};
use scripting_runtime::runtime::api_bindings::{WasmPtr, WasmSlice, ComponentInfo};

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

#[derive(Clone)]
struct BEState {
    be_state: Arc<BEStateInner>
}

struct BEStateInner {
    engine: Engine,
    memory: SharedMemory,
    linker: Mutex<Linker<BEState>>
}

fn be_print_external(caller: Caller<BEState>, text_ptr: u32, size: u32) {
    let text_ptr = WasmSlice::<u8>::new(text_ptr, size);
    let memory = &caller.data().be_state.memory;
    let text = text_ptr.as_shared_ref(memory);
    println!("Wasm Print: {}", text.as_str());
}

fn main() {
    let mut engine_config = Config::new();
    engine_config.wasm_threads(true);
    engine_config.wasm_bulk_memory(true);
    engine_config.debug_info(true);

    let engine = Engine::new(&engine_config).unwrap();
    let main_memory = SharedMemory::new(&engine, MemoryType::shared(50, 32768)).unwrap();

    let data = BEState{
        be_state: Arc::new(BEStateInner {
            engine: engine.clone(),
            linker: Mutex::new(Linker::new(&engine)),
            memory: main_memory.clone()
        })
    };

    let mut store = Store::new(&engine, data.clone());

    // Load two different modules importing the same shared memory
    let module1 = load_module("wasm_crate1.wasm", &engine).unwrap();
    let module2 = load_module("wasm_crate2.wasm", &engine).unwrap();


    let test_function;
    let create_test_component;
    let test_component_access;

    {
        let mut linker = data.be_state.linker.lock().unwrap();
        linker.define(&mut store, "env", "memory", main_memory.clone()).unwrap();
        linker.func_wrap("BraneEngine", "extern_be_print", be_print_external).unwrap();


        let now = std::time::Instant::now();

        let instance1 = linker.module(&mut store, "module1", &module1);
        if instance1.is_err() {
            println!("Was unable to instantiate module1: {:?}", instance1.err().unwrap());
            return;
        }
        let instanceInstantiationTime = now.elapsed();
        println!("Instance1 instantiation time: {:?}", instanceInstantiationTime);

        println!("-----Module 1-----");
        println!("Imports");
        let imports = module1.imports();
        for i in imports {
            println!("{:?}: {}", i.ty(), i.name());
        }

        println!("Exports");
        let exports = module1.exports();
        for e in exports {
            println!("{:?}: {}", e.ty(), e.name());
        }


        let now = std::time::Instant::now();

        test_function = linker.get(&mut store, "module1", "test_function").unwrap().into_func().unwrap().typed::<i32, i32, >(&store).unwrap();
        create_test_component = linker.get(&mut store, "module1", "create_test_component").unwrap().into_func().unwrap().typed::<(), u32, >(&store).unwrap();

        let instance2 = linker.module(&mut store, "module2", &module2);
        if instance2.is_err() {
            println!("Was unable to instantiate module2: {:?}", instance2.err().unwrap());
            return;
        }
        instance2.expect("Couldn't instantiate module2");
        let instanceInstantiationTime = now.elapsed();
        println!("Instance2 instantiation time: {:?}", instanceInstantiationTime);

        println!("-----Module 2-----");
        println!("Imports");
        let imports = module2.imports();
        for i in imports {
            println!("{:?}: {}", i.ty(), i.name());
        }

        println!("Exports");
        let exports = module2.exports();
        for e in exports {
            println!("{:?}: {}", e.ty(), e.name());
        }
        println!("-----end-----");

        test_component_access = linker.get(&mut store, "module2", "test_component_access").expect("could not find test_component_access").into_func().unwrap().typed::<u32, u32>(&store).unwrap();
    }


    let res = test_function.call(&mut store, 42).unwrap();
    println!("Test function returned {}", res);

    let test_component_ref;

    match create_test_component.call(&mut store, ()) {
        Ok(res) => {
            test_component_ref = WasmPtr::<TestComponent>::new(res);
            println!("Create test component returned {}", res);


            let test_component = test_component_ref.as_shared_ref(&main_memory);
            println!("Test component values: a = {}, b = {}, c = {}", (*test_component).a, (*test_component).b, (*test_component).c);

        },
        Err(err) => {
            eprintln!("create_test_component failed: {}", err.to_string());
            return;
        }
    }

    let res = test_component_access.call(&mut store, test_component_ref.ptr).unwrap();

    println!("Test component access returned {}", res);
    let test_component = test_component_ref.as_shared_ref(&main_memory);
    unsafe {
        println!("Test component values: a = {}, b = {}, c = {}", (*test_component).a, (*test_component).b, (*test_component).c);
    }
}
