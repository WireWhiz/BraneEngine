
#[derive(Debug, Clone)]
pub struct ComponentField {
    pub name: String,
    pub offset: u32,
    pub size: u32,
    pub ty: String,
}

#[derive(Clone)]
pub struct ComponentDef {
    pub name: String,
    pub fields: Vec<ComponentField>,
    pub size: u32,
    pub clone_method: wasmtime::TypedFunc<(u32, u32), ()>,
    pub drop_method: wasmtime::TypedFunc<u32, ()>,
}

impl std::fmt::Debug for ComponentDef {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("ComponentDef")
            .field("name", &self.name)
            .field("fields", &self.fields)
            .field("size", &self.size)
            .finish()
    }
}