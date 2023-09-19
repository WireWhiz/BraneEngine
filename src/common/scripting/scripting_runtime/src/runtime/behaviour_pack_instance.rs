use std::rc::Rc;
use crate::runtime::component::ComponentDef;
use crate::runtime::job::Job;

pub struct BehaviourPackInstance {
    pub id: u32,
    pub mod_instance: wasmtime::Instance,

    pub name: String,
    pub jobs: Vec<Rc<dyn Job>>,
    pub components: Vec<Rc<ComponentDef>>
}

impl std::fmt::Debug for BehaviourPackInstance {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("BehaviourPackInstance")
            .field("id", &self.id)
            .field("name", &self.name)
            .field("components", &self.components)
            .finish()
    }
}