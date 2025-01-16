#include "boot.h"
#include "hal/usb_driver.h"
#include "hal/storage.h"
#include "hal/fatfs_diskio.h"

#include "thirdparty/FatFs/diskio.h"

#include "board.h"
#include "lcd.h"

volatile tmr10ms_t g_tmr10ms;
volatile uint8_t tenms = 1;

void per5ms() {} // make linker happy

void per10ms()
{
  tenms |= 1u; // 10 mS has passed
  g_tmr10ms++;
}

void bootloaderUF2()
{
  BootloaderState state = ST_START;

  // TODO: register UF2 drive
  // fatfsRegisterDriver(drv, 0);

  storageInit();
  disk_initialize(0);

  for (;;) {

    if (tenms) {
      tenms = 0;

      if (state != ST_USB && state != ST_FLASHING
          && state != ST_FLASH_DONE && state != ST_RADIO_MENU) {
        if (usbPlugged()) {
          state = ST_USB;
#if !defined(SIMU)
          usbStart();
#endif
        }
      }

      if (state == ST_USB) {
        if (usbPlugged() == 0) {
#if !defined(SIMU)
          usbStop();
#endif
          state = ST_START;
        }
        bootloaderDrawScreen(state, 0);
      }

      if (state == ST_START) {
        bootloaderDrawScreen(state, 0);
      }

      lcdRefresh();
    }


    if (state == ST_REBOOT) {
#if !defined(SIMU)
      blExit();
      NVIC_SystemReset();
#else
      exit(1);
#endif
    }
  }  
}
