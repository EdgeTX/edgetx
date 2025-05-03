#include "stm32_hal.h"

#include "boot.h"
#include "hal/usb_driver.h"
#include "os/time.h"

#include "lcd.h"

#define FRAME_INTERVAL_MS 20

void bootloaderDFU()
{
  usbPlugged();
  usbStart();

  bootloaderDrawDFUScreen();

  uint32_t next_frame = time_get_ms();
  for (;;) {
    if (time_get_ms() - next_frame >= FRAME_INTERVAL_MS) {
      next_frame += FRAME_INTERVAL_MS;

      if (!usbPlugged()) break;
      bootloaderDrawDFUScreen();
      lcdRefresh();
    }
  }

  blExit();
  NVIC_SystemReset();
}
