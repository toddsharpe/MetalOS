#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

#![no_std]

pub mod c_types {
    pub type c_schar = i8;
    pub type c_char = u8;
    pub type c_uchar = u8;
    pub type c_short = i16;
    pub type c_ushort = u16;
    pub type c_int = i32;
    pub type c_uint = u32;
    pub type c_longlong = i64;
    pub type c_ulonglong = u64;

    pub type c_void = *const u8;
}

include!("metalos_bindings.rs");

