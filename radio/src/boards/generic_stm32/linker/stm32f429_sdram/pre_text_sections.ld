/* TX16S / STM32F429 (EdgeTX#5899): on units with flash rev < 3,
   USB D+ (PA12) switching noise corrupts reads from flash bank 2
   (0x08100000 and above).  All code and rodata that may be used
   while a USB session is active must live in bank 1; this is
   guaranteed by the FLASH region being restricted to bank 1
   (overflow = link error).

   UI code and its rodata are relegated to bank 2 here: they only
   run while USB is inactive (the UI is frozen in USB modes).
   Claimed before the .text catch-all so these input sections land
   here; file-pattern matching requires non-LTO objects.  Shared
   C++ comdats are already pinned to bank 1 by .text.shared. */
/* Bank 1 pins: symbols living in UI objects but called from non-UI
   tasks (mixer pulses, telemetry/logs timers, audio), found by the
   bank1->bank2 call scan (bench/check_bank1.sh).  Section names
   follow the mangled symbol: a signature change silently un-pins,
   the scan is the gatekeeper.  Claimed before .uicode. */
.text.pinned :
{
  . = ALIGN(4);
  /* tick LVGL incremente par le hook SysTick (ISR, meme pendant l'USB) */
  *(.text.lv_tick_inc)
  /* helpers R9M inline (pulses, chaque cycle mixer) */
  *(.text._Z17isModuleR9MAccessh .text._Z20isModuleR9MNonAccessh)
  /* mailbox popup differee (timers telemetrie/logs) */
  *(.text._Z24POPUP_WARNING_ON_UI_TASKPKcS0_)
  /* popups bloquants, gardes par _usb_ui_frozen() (audio, PXX2) */
  *(.text._Z13POPUP_WARNINGPKcS0_ .text._Z17POPUP_INFORMATIONPKc)
  /* fonction speciale SET_SCREEN (mixer) */
  *(.text._Z20setRequestedMainViewh)
  /* appelee par pwrCheck (menusTask, toutes les 50 ms, gel compris) */
  *(.text._Z23cancelShutdownAnimationv)
  . = ALIGN(4);
} > REGION_TEXT AT> REGION_TEXT_STORAGE

.uicode :
{
  . = ALIGN(4);
  _suicode = .;
  *colorlcd*(.text .text.* .rodata .rodata.*)
  *lvgl*(.text .text.* .rodata .rodata.*)
  *libopenui*(.text .text.* .rodata .rodata.*)
  . = ALIGN(4);
  _euicode = .;
} > REGION_ASSETS
