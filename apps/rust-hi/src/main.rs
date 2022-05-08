#![no_std]
#![no_main]

use core::panic::PanicInfo;

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    loop {}
}

#[no_mangle]
pub extern "C" fn _start() {
    main();
}

fn main() {
    metalos::print("Hi from Rust!\n");
    metalos::print("Its working!\n");
}
