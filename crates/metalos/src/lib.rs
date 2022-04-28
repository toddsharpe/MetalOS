#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

#![no_std]

pub mod c_types {
    pub type c_uint = u32;
    pub type c_int = i32;
    pub type c_char = u8;
}

include!("bindings.rs");

pub fn _print(s: &str)
{
    unsafe
    {
        printf(s.as_ptr());
    }
}

