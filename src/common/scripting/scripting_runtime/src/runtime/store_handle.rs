use std::cell::RefCell;
use std::rc::Rc;

#[derive(Clone)]
pub struct StoreHandle {
    pub store: Rc<RefCell<wasmtime::Store<()>>>,
}


