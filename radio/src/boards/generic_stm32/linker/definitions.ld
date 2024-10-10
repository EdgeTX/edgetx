INCLUDE layout.ld

/* Entry Point */
ENTRY(Reset_Handler)

/* Stack address */
_estack = STACK_ADDRESS;

/* Main stack end */
_main_stack_start = _estack - MAIN_STACK_SIZE;

/* Higest heap address */
_heap_end = HEAP_ADDRESS;
