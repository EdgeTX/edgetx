INCLUDE layout.ld

/* Entry Point */
ENTRY(Reset_Handler)

/* Stack address */
_estack = STACK_ADDRESS;

/* Main stack end */
_main_stack_start = _estack - MAIN_STACK_SIZE;

/* Higest heap address */
__heap_end = HEAP_ADDRESS;
__heap_size = __heap_end - __heap_start;

PHDRS
{
  text PT_LOAD;
  ram PT_LOAD;
  ram_init PT_LOAD;
  tls PT_TLS;
}
