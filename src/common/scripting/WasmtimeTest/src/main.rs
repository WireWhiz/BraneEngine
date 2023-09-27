use std::fs::File;
use std::io::Read;
use std::mem::size_of;
use std::ops::{DerefMut, Index};
use std::sync::{Arc, Mutex, RwLock};
use std::borrow::BorrowMut;
use std::collections::VecDeque;
use wasmtime::{Config, Engine, SharedMemory, Store, Module, MemoryType, Func, Extern, Linker, Caller, AsContextMut, AsContext};
use scripting_runtime::runtime::api_bindings::{WasmPtr, WasmSlice, ComponentInfo, ComponentFieldInfo};
use std::thread::ThreadId;

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

#[derive(Clone)]
struct BEState {
    be_state: Arc<BEStateInner>,
    wasi_ctx: wasmtime_wasi::WasiCtx
}

struct BEStateInner {
    engine: Engine,
    memory: SharedMemory,
    threads_module: Module,
    threads: Mutex<VecDeque<std::thread::JoinHandle<()>>>,
    linker: Mutex<Linker<BEState>>
}

fn be_print_external(caller: Caller<BEState>, text_ptr: u32, size: u32) {
    let text_ptr = WasmSlice::<u8>::new(text_ptr, size);
    let memory = &caller.data().be_state.memory;
    let text = text_ptr.as_shared_ref(memory);
    println!("Wasm Print: {}", text.as_str());
}

fn spawn_thread(caller: Caller<BEState>, func_a: u32) {
    let data = caller.data().clone();
    caller.data().be_state.threads.lock().expect("Couldn't lock threads").push_back(std::thread::spawn(move ||{
        let engine = &data.be_state.engine;
        let mut store = Store::<BEState>::new(engine, data.clone());
        let runtime_func;
        {
            let mut linker = data.be_state.linker.lock().unwrap();
            linker.module(&mut store, "module1", &data.be_state.threads_module).unwrap();
            runtime_func = linker.get(&mut store, "module1", "run_thread").unwrap().into_func().unwrap().typed::<(u32), ()>(&store).unwrap();
        }

        runtime_func.call(&mut store, (func_a)).unwrap();
    }));
}

fn main() {
    let mut engine_config = Config::new();
    engine_config.wasm_threads(true);
    engine_config.wasm_bulk_memory(true);
    engine_config.debug_info(true);

    let engine = Engine::new(&engine_config).unwrap();
    let module1 = load_module("wasm_crate1.wasm", &engine).unwrap();
    let main_memory = SharedMemory::new(&engine, MemoryType::shared(50, 32768)).unwrap();

    let data = BEState{
        be_state: Arc::new(BEStateInner {
            engine: engine.clone(),
            linker: Mutex::new(Linker::new(&engine)),
            threads_module: module1.clone(),
            threads: Mutex::new(VecDeque::new()),
            memory: main_memory.clone()
        }),
        wasi_ctx: wasmtime_wasi::WasiCtxBuilder::new().inherit_stdio().build()
    };

    wasmtime_wasi::snapshots::preview_1::add_wasi_snapshot_preview1_to_linker(&mut data.be_state.linker.lock().unwrap(),
                                 |data: &mut BEState| &mut data.wasi_ctx).expect("Was unable to link WASI");

    let mut store = Store::new(&engine, data.clone());


    let module2 = load_module("wasm_crate2.wasm", &engine).unwrap();


    let test_function;
    let test_extern_call;
    let create_test_component;
    let test_component_access;
    let test_component_info;
    let threading_test;

    {
        let mut linker = data.be_state.linker.lock().unwrap();
        linker.define(&mut store, "env", "memory", main_memory.clone()).unwrap();
        linker.func_wrap("BraneEngine", "extern_be_print", be_print_external).unwrap();
        linker.func_wrap("BraneEngine", "test_external_function", test_external_function).unwrap();
        linker.func_wrap("BraneEngine", "spawn_thread", spawn_thread).unwrap();


        let now = std::time::Instant::now();

        let instance1 = linker.module(&mut store, "module1", &module1);
        if instance1.is_err() {
            println!("Was unable to instantiate module1: {:?}", instance1.err().unwrap());
            return;
        }
        let instanceInstantiationTime = now.elapsed();
        println!("Instance1 instantiation time: {:?}", instanceInstantiationTime);

        let now = std::time::Instant::now();
        let instanceInstantiationTime = now.elapsed();
        println!("Instance2 instantiation time: {:?}", instanceInstantiationTime);

        test_function = linker.get(&mut store, "module1", "test_function").unwrap().into_func().unwrap().typed::<i32, i32, >(&store).unwrap();
        test_extern_call = linker.get(&mut store, "module1", "test_extern_call").unwrap().into_func().unwrap().typed::<(), i32, >(&store).unwrap();
        create_test_component = linker.get(&mut store, "module1", "create_test_component").unwrap().into_func().unwrap().typed::<(), u32, >(&store).unwrap();


        let instance2 = linker.module(&mut store, "module2", &module1);
        if instance2.is_err() {
            println!("Was unable to instantiate module2: {:?}", instance2.err().unwrap());
            return;
        }
        instance2.expect("Couldn't instantiate module2");
        test_component_access = linker.get(&mut store, "module2", "test_component_access").unwrap().into_func().unwrap().typed::<u32, u32, >(&store).unwrap();
        test_component_info = linker.get(&mut store, "module2", "be_info_TestComponent").unwrap().into_func().unwrap().typed::<(), u32, >(&store).unwrap();
        threading_test = linker.get(&mut store, "module2", "threading_test").unwrap().into_func().unwrap().typed::<i32, (), >(&store).unwrap();
    }

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


    let res = test_function.call(&mut store, 42).unwrap();
    println!("Test function returned {}", res);

    let res = test_extern_call.call(&mut store, ()).unwrap();
    println!("Test extern call returned {}", res);

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


    let test_component_info_ptr = test_component_info.call(&mut store, ()).unwrap();
    println!("Test component info returned {}", test_component_info_ptr);
    println!("Memory is of size: {}", main_memory.size() * 65536);
    assert!(test_component_info_ptr < main_memory.size() as u32 * 65536); //Make sure the pointer is within the bounds of the memory
    let test_component_info_ptr: WasmPtr<ComponentInfo> = WasmPtr::new(test_component_info_ptr);
    let test_component_info = test_component_info_ptr.as_shared_ref(&data.be_state.memory);
    println!("Test component of size {} contains fields:", test_component_info.size);

    let fields = test_component_info.fields.as_shared_ref(&data.be_state.memory);

    let mut i = 0;
    for f in fields.iter() {
        let ty = f.ty.as_shared_ref(&data.be_state.memory);
        println!("\t{}: offset = {}, size = {}, type = {}", i, f.offset, f.size, ty.as_str());
        i += 1;
    }


    threading_test.call(&mut store, 12).expect("Threading test failed");

    std::thread::sleep(std::time::Duration::from_millis(1000));
    let mut threads = data.be_state.threads.lock().expect("Couldn't lock threads");

    while !threads.is_empty() {
        threads.pop_back().expect("Couldn't pop thread").join().expect("Couldn't join thread");
    }
}
