#![cfg_attr(not(test), no_std)]

use core::panic::PanicInfo;

#[panic_handler]
fn halt(info: &PanicInfo) -> ! {
    loop { }
}

extern "C" fn test_func() {

}
