

#[derive(Debug, Clone)]
pub struct VarType {
    pub type_name : String
}

#[derive(Debug, Clone)]
pub struct ComponentVar {
    pub name: String,
    pub value_type: VarType,
}

#[derive(Debug, Clone)]
pub struct ComponentDef {
    pub name: String,
    pub member_vars: Vec<ComponentVar>
}