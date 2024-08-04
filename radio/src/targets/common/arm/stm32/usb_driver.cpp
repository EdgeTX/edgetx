/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#if defined(USBJ_EX)
#include "usb_joystick.h"
#else
#include "edgetx_helpers.h"
#include "dataconstants.h"
#include "globals.h"
#endif

extern "C" {
  #include "usbd_conf.h"
  #include "usbd_core.h"
  #include "usbd_msc.h"
  #include "usbd_desc.h"
  #include "usbd_hid.h"
  #include "usbd_cdc.h"
  #include "usbd_dfu.h"
}

#include "stm32_hal_ll.h"
#include "stm32_hal.h"

#include "stm32_gpio.h"

#include "hal/gpio.h"
#include "hal/usb_driver.h"

#include "hal.h"
#include "debug.h"

#if defined(USE_USB_HS)
  #define DEVICE_ID DEVICE_HS
#else
  #define DEVICE_ID DEVICE_FS
#endif

#if defined(BOOT)

// TODO: configured where needed
#define DEFAULT_USB_MODE USB_MASS_STORAGE_MODE;
static const USBD_DFU_MediaTypeDef* _dfu_media[USBD_DFU_MAX_ITF_NUM] = {nullptr};

#else
#define DEFAULT_USB_MODE USB_UNSELECTED_MODE;
#endif

static bool usbDriverStarted = false;
static usbMode selectedUsbMode = DEFAULT_USB_MODE;

USBD_HandleTypeDef hUsbDevice;

int getSelectedUsbMode()
{
  return selectedUsbMode;
}

void setSelectedUsbMode(int mode)
{
  selectedUsbMode = usbMode(mode);
}

#if defined(USB_GPIO_VBUS)
int usbPlugged()
{
#if defined(DEBUG_DISABLE_USB)
  return(false);
#endif

  static uint8_t debouncedState = 0;
  static uint8_t lastState = 0;

#if defined(USB_GPIO_VBUS_OPEN_DRAIN)
  uint8_t state = gpio_read(USB_GPIO_VBUS) ? 0 : 1;
#else
  uint8_t state = gpio_read(USB_GPIO_VBUS) ? 1 : 0;
#endif
  if (state == lastState)
    debouncedState = state;
  else
    lastState = state;

  return debouncedState;
}
#endif

extern PCD_HandleTypeDef hpcd_USB_OTG;

#if defined(USE_USB_HS)
extern "C" void OTG_HS_IRQHandler()
{
  DEBUG_INTERRUPT(INT_OTG_FS);
  HAL_PCD_IRQHandler(&hpcd_USB_OTG);
}
#else
extern "C" void OTG_FS_IRQHandler()
{
  DEBUG_INTERRUPT(INT_OTG_FS);
  HAL_PCD_IRQHandler(&hpcd_USB_OTG);
}
#endif

void usbInit()
{
  gpio_init_af(USB_GPIO_DM, USB_GPIO_AF, GPIO_PIN_SPEED_VERY_HIGH);
  gpio_init_af(USB_GPIO_DP, USB_GPIO_AF, GPIO_PIN_SPEED_VERY_HIGH);
  
#if defined(USB_GPIO_VBUS)
#if defined(USB_GPIO_VBUS_OPEN_DRAIN)
  gpio_init(USB_GPIO_VBUS, GPIO_IN_PU, GPIO_PIN_SPEED_LOW);
#else
  gpio_init(USB_GPIO_VBUS, GPIO_IN, GPIO_PIN_SPEED_LOW);
#endif
#endif

// TODO: seems this is only needed for USB wakeup,
//       which we do not support.
#if !defined(STM32H7RS)
#if defined(LL_APB2_GRP1_PERIPH_SYSCFG)
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
#elif defined(LL_APB4_GRP1_PERIPH_SYSCFG)
  LL_APB4_GRP1_EnableClock(LL_APB4_GRP1_PERIPH_SYSCFG);
#else
  #error "Unable to enable SYSCFG peripheral clock"
#endif
#endif

  usbDriverStarted = false;
}

extern void usbInitLUNs();
extern USBD_HandleTypeDef hUsbDevice;
extern "C" USBD_StorageTypeDef USBD_Storage_Interface_fops;
extern USBD_CDC_ItfTypeDef USBD_Interface_fops;
extern USBD_DFU_MediaTypeDef USBD_DFU_MEDIA_fops;

void usbStart()
{
  USBD_Init(&hUsbDevice, &FS_Desc, DEVICE_ID);
  switch (getSelectedUsbMode()) {
    case USB_MASS_STORAGE_MODE:
      // initialize USB as MSC device
      usbInitLUNs();
      USBD_RegisterClass(&hUsbDevice, &USBD_MSC);
      USBD_MSC_RegisterStorage(&hUsbDevice, &USBD_Storage_Interface_fops);
      break;

#if defined(BOOT)
    case USB_DFU_MODE:
      USBD_RegisterClass(&hUsbDevice, &USBD_DFU);
      for (unsigned i = 0; i < USBD_DFU_MAX_ITF_NUM; i++){
        if (_dfu_media[i] != nullptr) {
          USBD_DFU_RegisterMedia(&hUsbDevice,
                                 (USBD_DFU_MediaTypeDef*)_dfu_media[i]);
        }
      }
      break;
#else
    case USB_JOYSTICK_MODE:
      // initialize USB as HID device
#if defined(USBJ_EX)
      setupUSBJoystick();
#endif
      USBD_RegisterClass(&hUsbDevice, &USBD_HID);
      break;
#if defined(USB_SERIAL)
    case USB_SERIAL_MODE:
      // initialize USB as CDC device (virtual serial port)
      USBD_RegisterClass(&hUsbDevice, &USBD_CDC);
      USBD_CDC_RegisterInterface(&hUsbDevice, &USBD_Interface_fops);
      break;
#endif
#endif

    default:
      return;
  }

  if (USBD_Start(&hUsbDevice) == USBD_OK) {
    usbDriverStarted = true;
  }
}

void usbStop()
{
  usbDriverStarted = false;
  USBD_DeInit(&hUsbDevice);
}


bool usbStarted()
{
  return usbDriverStarted;
}

#if defined(BOOT)

int usbRegisterDFUMedia(const void* dfu_media)
{
  for (unsigned i = 0; i < USBD_DFU_MAX_ITF_NUM; i++) {
    if (_dfu_media[i] == nullptr) {
      _dfu_media[i] = (const USBD_DFU_MediaTypeDef*)dfu_media;
      return 0;
    }
  }
  return -1;
}

#else // !BOOT

#if defined(USBJ_EX)
extern "C" void delay_ms(uint32_t count);
void usbJoystickRestart()
{
  if (!usbDriverStarted || getSelectedUsbMode() != USB_JOYSTICK_MODE) return;

  USBD_DeInit(&hUsbDevice);
  delay_ms(100);
  USBD_Init(&hUsbDevice, &FS_Desc, DEVICE_ID);
  USBD_RegisterClass(&hUsbDevice, &USBD_HID);
  USBD_Start(&hUsbDevice);
}
#endif


/*
  Prepare and send new USB data packet

  The format of HID_Buffer is defined by
  USB endpoint description can be found in
  file usb_hid_joystick.c, variable HID_JOYSTICK_ReportDesc
*/
void usbJoystickUpdate()
{
#if !defined(USBJ_EX)
   static uint8_t HID_Buffer[HID_IN_PACKET];

   //buttons
   HID_Buffer[0] = 0;
   HID_Buffer[1] = 0;
   HID_Buffer[2] = 0;
   for (int i = 0; i < 8; ++i) {
     if ( channelOutputs[i+8] > 0 ) {
       HID_Buffer[0] |= (1 << i);
     }
     if ( channelOutputs[i+16] > 0 ) {
       HID_Buffer[1] |= (1 << i);
     }
     if ( channelOutputs[i+24] > 0 ) {
       HID_Buffer[2] |= (1 << i);
     }
   }

   //analog values
   for (int i = 0; i < 8; ++i) {
     int16_t value = limit<int16_t>(0, channelOutputs[i] + 1024, 2048);;
     HID_Buffer[i*2 +3] = static_cast<uint8_t>(value & 0xFF);
     HID_Buffer[i*2 +4] = static_cast<uint8_t>(value >> 8);
   }
   USBD_HID_SendReport(&hUsbDevice, HID_Buffer, HID_IN_PACKET);
#else
  usbReport_t ret = usbReport();
  USBD_HID_SendReport(&hUsbDevice, ret.ptr, ret.size);
#endif
}
#endif
