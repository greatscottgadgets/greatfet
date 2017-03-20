#![no_std]
#![feature(lang_items)]

static mut i: u32 = 0;

#[no_mangle]
pub fn blinky_ratchet(led0_on: fn(),
                 led0_off: fn(),
                 led1_on: fn(),
                 led1_off: fn()) {
    match unsafe { i } {
        0 => led0_on(),
        1 => led1_on(),
        2 => led0_off(),
        3 => led1_off(),
        _ => {},
    }

    unsafe { i = (i+1) % 4 };
}

#[lang = "eh_personality"]
#[no_mangle]
pub extern fn rust_eh_personality() {
}

#[lang = "panic_fmt"]
#[no_mangle]
pub extern fn rust_begin_panic(_msg: core::fmt::Arguments,
                               _file: &'static str,
                               _line: u32) -> ! {
    loop {}
}

#[no_mangle]
pub extern fn abort() -> ! {
    loop {};
}

#[no_mangle]
pub extern fn _kill() -> ! {
    loop {};
}
