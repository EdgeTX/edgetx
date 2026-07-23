INCLUDE layout.ld

/* Entry Point */
ENTRY(Reset_Handler)

/* Stack address */
_estack = STACK_ADDRESS;

/* Main stack end */
_main_stack_start = _estack - MAIN_STACK_SIZE;

/* Higest heap address */
_heap_end = HEAP_ADDRESS;

/* Fastcode defaults for scripts that do not define the section
   (bootloader): the startup copy loop becomes a no-op */
PROVIDE(_sfastcode = 0);
PROVIDE(_efastcode = 0);
PROVIDE(_fastcode_load = 0);

/* Same for scripts without a .text.shared section */
PROVIDE(_stext_shared = 0);
PROVIDE(_etext_shared = 0);
PROVIDE(_text_shared_load = 0);
