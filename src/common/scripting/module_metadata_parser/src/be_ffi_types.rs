
pub mod be_ffi_types
{
    use std::ffi::{c_char, CStr, CString};
    use std::ops::{Index, IndexMut};
    use std::ptr::null_mut;
    use std::slice::SliceIndex;
    use std::str::Utf8Error;

    #[repr(C)]
    pub struct BEVec<T> {
        pub data: *mut T,
        pub length: usize,
        capacity: usize,
    }

    impl<T: Clone> BEVec<T> {
        pub fn new() -> BEVec<T> {
            let mut data = Vec::new();
            let new_vec = BEVec {
                data: data.as_mut_ptr(),
                length: data.len(),
                capacity: data.capacity()
            };
            std::mem::forget(data);
            new_vec
        }

        pub fn reserve(&mut self, new_capacity: usize) {
            let mut data = unsafe { Vec::from_raw_parts(self.data, self.length, self.capacity) };
            data.reserve(new_capacity);
            self.data = data.as_mut_ptr();
            self.length = data.len();
            self.capacity = data.capacity();
            std::mem::forget(data);
        }

        pub fn resize(&mut self, new_length: usize, value: T) {
            let mut data = unsafe { Vec::from_raw_parts(self.data, self.length, self.capacity) };
            data.resize(new_length, value);
            self.data = data.as_mut_ptr();
            self.length = data.len();
            self.capacity = data.capacity();
            std::mem::forget(data);
        }

        pub fn push(&mut self, value: T) {
            let mut data = unsafe { Vec::from_raw_parts(self.data, self.length, self.capacity) };
            data.push(value);
            self.data = data.as_mut_ptr();
            self.length = data.len();
            self.capacity = data.capacity();
            std::mem::forget(data);
        }
    }

    impl<T, I: SliceIndex<[T]>> Index<I> for BEVec<T> {
        type Output = I::Output;

        fn index(&self, index: I) -> &Self::Output {
            let data = unsafe { std::slice::from_raw_parts(self.data, self.length) };
            &data[index]
        }
    }

    impl<T, I: SliceIndex<[T]>> IndexMut<I> for BEVec<T> {
        fn index_mut(&mut self, index: I) -> &mut Self::Output {
            let data = unsafe { std::slice::from_raw_parts_mut(self.data, self.length) };
            &mut data[index]
        }
    }

    impl<T: Clone> Clone for BEVec<T> {
        fn clone(&self) -> Self {
            let mut data = unsafe { Vec::from_raw_parts(self.data, self.length, self.capacity) };
            let mut new_data : Vec<T> = data.clone();
            let new_vec = BEVec {
                data: new_data.as_mut_ptr(),
                length: new_data.len(),
                capacity: new_data.capacity()
            };
            std::mem::forget(data);
            new_vec
        }
    }

    impl<T> Drop for BEVec<T> {
        fn drop(&mut self) {
            unsafe {
                let data = Box::from_raw(self.data);
                drop(data);
            }
        }
    }

    #[repr(C)]
    pub struct BEStr {
        pub data: *mut c_char,
    }

    impl BEStr {
        pub fn new() -> BEStr {
            let new_string = BEStr{ data: CString::new("").unwrap().into_raw() };
            new_string
        }

        pub fn as_c_str(&self) -> &CStr {
            unsafe { CStr::from_ptr(self.data) }
        }
    }

    impl From<CString> for BEStr {
        fn from(string: CString) -> Self {
            let new_string = BEStr{ data: string.into_raw() };
            new_string
        }
    }

    impl Clone for BEStr {
        fn clone(&self) -> Self {
            let new_string = BEStr{ data: CString::from(self.as_c_str()).into_raw() };
            new_string
        }
    }

    impl Drop for BEStr {
        fn drop(&mut self) {
            unsafe {
                let data = CString::from_raw(self.data);
                drop(data);
            }
        }
    }

    #[no_mangle]
    pub extern fn free_be_string(string: *mut BEStr) {
        unsafe {
            let data = Box::from_raw(string);
            drop(data);
        }
    }

    #[no_mangle]
    pub extern fn new_be_string(text: *const c_char) -> *mut BEStr {
        let data = CString::from(unsafe { CStr::from_ptr(text) });
        let new_string = Box::new(BEStr{ data: data.into_raw()});
        Box::into_raw(new_string)
    }
}