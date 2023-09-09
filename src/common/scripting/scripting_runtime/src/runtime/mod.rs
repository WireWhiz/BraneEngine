
pub mod api_bindings;
pub mod behaviour_pack;
pub mod component;
pub mod job;


pub struct ScriptingRuntime {
    engine: wasmtime::Engine,
    store: wasmtime::Store<()>,
    loaded_packs: Vec<behaviour_pack::BehaviourPack>
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
        let store = wasmtime::Store::new(&engine, ());
        
        ScriptingRuntime {
            engine,
            store,
            loaded_packs: vec![]
        }
    }

    pub fn add_pack(&mut self, pack: behaviour_pack::BehaviourPack) {
        self.loaded_packs.push(pack);
    }


}
