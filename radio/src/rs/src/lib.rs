#![no_std]
use core::panic::PanicInfo;

mod bindings;

#[panic_handler]
fn halt(info: &PanicInfo) -> ! {
    loop { }
}

#[no_mangle]
pub unsafe extern "C" fn this_function_exists_in_c() -> u32 {
    bindings::cliGetMode()
}
