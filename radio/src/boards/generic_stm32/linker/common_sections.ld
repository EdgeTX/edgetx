
/* C/C++ Exception handling (not used) */
.ARM.extab :
{
  *(.ARM.extab* .gnu.linkonce.armextab.*)
} > REGION_TEXT AT> REGION_TEXT_STORAGE

.ARM :
{
  PROVIDE(__exidx_start = .);
  *(.ARM.exidx* .gnu.linkonce.armexidx.*)
  PROVIDE(__exidx_end = .);
} > REGION_TEXT AT> REGION_TEXT_STORAGE

/* Bootstrap code to load ISR vector and code sections */
.bootstrap :
{
  *(.bootstrap)
} > REGION_TEXT_STORAGE

/* Initialized data sections goes into RAM, load LMA copy after code */
.data :
{
  . = ALIGN(4);
  _sdata = .;        /* create a global symbol at data start */
  *(.data .data*)    /* .data sections */
  . = ALIGN(4);
  _edata = .;        /* define a global symbol at data end */
} > REGION_DATA AT> REGION_TEXT_STORAGE

/* used by the startup to initialize data */
_sidata = LOADADDR(.data);

/* Uninitialized data section */
.bss(NOLOAD) : AT (ADDR(.bss))
{
  . = ALIGN(4);
  /* This is used by the startup in order to initialize the .bss secion */
  _sbss = .; /* define a global symbol at bss start */
  *(.bss .bss*)
  *(COMMON)
  . = ALIGN(4);
  _ebss = .;         /* define a global symbol at bss end */
} > REGION_BSS

/* collect all uninitialized .ram sections (force DMA accessible) */
.ram (NOLOAD) : AT (ADDR(.ram))
{
  . = ALIGN(4);
  _sram = .;
  *(.ram)
  . = ALIGN(4);
  _eram = .;
} > REGION_RAM

INCLUDE extra_sections.ld

/* Seems to be the safest place for all targets (see AN2606) */
.reboot_buffer (ORIGIN(REGION_DATA) + LENGTH(REGION_DATA) - 4) (NOLOAD) : AT (ADDR(.reboot_buffer))
{
  KEEP(*(.reboot*))
} > REGION_DATA

/* Remove information from the standard libraries */
/DISCARD/ :
{
  libc.a ( * )
  libm.a ( * )
  libgcc.a ( * )
}

.ARM.attributes 0 : { *(.ARM.attributes) }
