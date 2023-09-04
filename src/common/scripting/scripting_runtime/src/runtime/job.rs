
pub enum SystemAttribute {
    OrderBefore(String),
    OrderAfter(String)
}

pub struct Job {
    pub name : String,
    pub attributes : Vec<SystemAttribute>,
    pub handle : dyn Fn() -> ()
}

impl Job {
    pub fn call(&self)
    {
        (self.handle)();
    }
}