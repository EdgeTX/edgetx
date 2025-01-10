#include "stm32_hal.h"

#include "boot.h"
#include "hal/usb_driver.h"
#include "lcd.h"

extern volatile uint8_t tenms;

void bootloaderDFU()
{
  usbPlugged();
  usbStart();

  bootloaderDrawDFUScreen();

  for (;;) {
    if (tenms) {
      if (!usbPlugged()) break;
      bootloaderDrawDFUScreen();
      lcdRefresh();
    }
  }

  blExit();
  NVIC_SystemReset();
}
