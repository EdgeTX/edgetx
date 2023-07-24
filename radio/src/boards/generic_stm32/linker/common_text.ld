/* .text sections (code) */
*(.text .text* .gnu.linkonce.t.*)
*(.rodata .rodata* .gnu.linkonce.r.*)

*(.glue_7)  /* glue arm to thumb code */
*(.glue_7t) /* glue thumb to arm code */

*(.eh_frame)
*(.data.__global_locale)

/*
   C++ Runtime: initializers for static variables.
   C Runtime: designated constructors
*/
. = ALIGN(4);
KEEP(*(.init))
. = ALIGN(4);
__preinit_array_start = .;
KEEP (*(.preinit_array))
__preinit_array_end = .;

. = ALIGN(4);
__init_array_start = .;
KEEP (*(SORT(.init_array.*)))
KEEP (*(.init_array))
__init_array_end = .;

/*
   C++ runtime: destructors for static variables.
   C runtime: designated finializers
*/
. = ALIGN(4);
KEEP(*(.fini))

. = ALIGN(4);
__fini_array_start = .;
KEEP (*(.fini_array))
KEEP (*(SORT(.fini_array.*)))
__fini_array_end = .;

/*
   C++ runtime: static constructors
*/
. = ALIGN(4);
KEEP (*crtbegin.o(.ctors))
KEEP (*(EXCLUDE_FILE (*crtend.o) .ctors))
KEEP (*(SORT(.ctors.*)))
KEEP (*crtend.o(.ctors))
