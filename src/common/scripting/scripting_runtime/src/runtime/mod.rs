
pub mod api_bindings;
pub mod behaviour_pack;
pub mod behaviour_pack_instance;
pub mod component;
pub mod job;
pub mod store_handle;

use std::cell::RefCell;
use std::collections::HashMap;
use std::rc::{Rc, Weak};
use wasmtime::AsContextMut;
use behaviour_pack::BehaviourPack;
use behaviour_pack_instance::BehaviourPackInstance;
use store_handle::StoreHandle;
use crate::runtime::api_bindings::{ComponentInfo, WasmPtr};
use crate::runtime::component::ComponentField;
use crate::runtime::job::{Job};

pub struct ScriptingRuntime {
    engine: wasmtime::Engine,
    store: StoreHandle,
    main_memory: wasmtime::Memory,
    instance_id_counter: u32,

    exports: HashMap<String, wasmtime::Extern>,

    loaded_components: HashMap<String, component::ComponentDef>,
    loaded_jobs: HashMap<String, Weak<dyn job::Job>>,

    loaded_packs: HashMap<u32, BehaviourPackInstance>
}

impl std::fmt::Debug for ScriptingRuntime {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("ScriptingRuntime")
            .field("loaded_packs", &self.loaded_packs)
            .finish()
    }
}

impl ScriptingRuntime {
    pub fn new(enable_debug: bool) -> ScriptingRuntime {
        let mut engine_config = wasmtime::Config::new();
        engine_config.wasm_threads(true);
        engine_config.wasm_bulk_memory(true);
        engine_config.debug_info(enable_debug);
        
        let engine = wasmtime::Engine::new(&engine_config).unwrap();
        let mut store = StoreHandle { store: Rc::new(RefCell::new(wasmtime::Store::new(&engine, ()))) };

        let main_memory = wasmtime::Memory::new(store.store.borrow_mut().as_context_mut(), wasmtime::MemoryType::shared(32, 32768)).unwrap();
        
        ScriptingRuntime {
            engine,
            store,
            main_memory,
            instance_id_counter: 0,

            exports: HashMap::from([("env.memory".to_string(), wasmtime::Extern::Memory(main_memory.clone()))]),

            loaded_components: HashMap::new(),
            loaded_jobs: HashMap::new(),

            loaded_packs: HashMap::new()
        }
    }

    pub fn load_pack(&mut self, pack: &BehaviourPack) -> Result<u32, Box<dyn std::error::Error>> {
        let wasm_module = wasmtime::Module::new(&self.engine, &pack.module_data)?;

        let mut imports = vec![];
        for import in wasm_module.imports() {
            let import_key = format!("{}.{}", import.module(), import.name());
            if let Some(export) = self.exports.get(&import_key) {
                imports.push(export.clone());
            } else {
                return Err(format!("Missing import: {}", import_key).into());
            }
        }

        let mut store = self.store.store.borrow_mut();

        let mod_instance = wasmtime::Instance::new(store.as_context_mut(), &wasm_module, imports.as_slice())?;

        let mut jobs: Vec<Rc<dyn Job>> = vec![];
        for info in pack.jobs.iter() {
            let job = job::WasmJob {
                info: info.clone(),
                store_handle: self.store.clone(),
                handle: mod_instance.get_typed_func::<(), ()>(store.as_context_mut(), info.name.as_str())?
            };
            jobs.push(Rc::new(job));
        };

        let mut components: Vec<Rc<component::ComponentDef>> = vec![];
        for id in pack.component_ids.iter() {
            let info_getter = mod_instance.get_typed_func::<(), u32>(store.as_context_mut(), format!("be_info_{}", id).as_str())?;
            let info_ptr = WasmPtr::<ComponentInfo>::new(info_getter.call(store.as_context_mut(), ())?);

            let info = info_ptr.as_ref(&self.main_memory, store.as_context_mut());

            let mut fields = vec![];
            for field in info.fields.as_ref(&self.main_memory, store.as_context_mut()).iter() {
                fields.push(ComponentField {
                    name: field.name.as_str(&self.main_memory, store.as_context_mut()).to_string(),
                    offset: field.offset,
                    size: field.size,
                    ty: field.ty.as_str(&self.main_memory, store.as_context_mut()).to_string()
                });
            };
            let size = info.size;
            let clone_method = mod_instance.get_typed_func::<(u32, u32), ()>(store.as_context_mut(), format!("be_clone_{}", id).as_str())?;
            let drop_method = mod_instance.get_typed_func::<u32, ()>(store.as_context_mut(), format!("be_drop_{}", id).as_str())?;


            let component = component::ComponentDef {
                name: id.clone(),
                fields,
                size,
                clone_method,
                drop_method
            };
            components.push(Rc::new(component));
        }

        let id = self.instance_id_counter;
        let mut instance = BehaviourPackInstance {
            id,
            mod_instance,
            name: pack.name.clone(),
            jobs,
            components
        };


        //TODO: Throw stuff into the needed maps, sort jobs by dependencies, etc. Then run an actual runtime test.

        self.instance_id_counter += 1;

        Ok(id)
    }


}
