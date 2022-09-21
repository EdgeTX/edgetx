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

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
#pragma     data_alignment = 4
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */

// include STM32 headers and generic board defs
#include "board_common.h"
#include "usb_driver.h"

extern "C" {

/* Includes ------------------------------------------------------------------*/
#include "usb_conf.h"
#include "usbd_cdc_core.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* The following structures groups all needed parameters to be configured for the
   ComPort. These parameters can modified on the fly by the host through CDC class
   command class requests. */
typedef struct __attribute__ ((packed))
{
  uint32_t bitrate;
  uint8_t  format;
  uint8_t  paritytype;
  uint8_t  datatype;
} LINE_CODING;

static LINE_CODING g_lc;

/* These are external variables imported from CDC core to be used for IN
   transfer management. */
extern uint8_t  APP_Rx_Buffer []; /* Write CDC received data in this buffer.
                                     These data will be sent over USB IN endpoint
                                     in the CDC core functions. */
extern volatile uint32_t APP_Rx_ptr_in;    /* Increment this pointer or roll it back to
                                     start address when writing received data
                                     in the buffer APP_Rx_Buffer. */
extern volatile uint32_t APP_Rx_ptr_out;

/* Private function prototypes -----------------------------------------------*/
static uint16_t VCP_Init     (void);
static uint16_t VCP_DeInit   (void);
static uint16_t VCP_Ctrl     (uint32_t Cmd, uint8_t* Buf, uint32_t Len);
static uint16_t VCP_DataRx   (uint8_t* Buf, uint32_t Len);

extern "C" const CDC_IF_Prop_TypeDef VCP_fops =
{
  VCP_Init,
  VCP_DeInit,
  VCP_Ctrl,
  0,
  VCP_DataRx
};

}   // extern "C"

static void (*receiveDataCb)(uint8_t*, uint32_t) = nullptr;
// static void* receiveDataCbCtx;

//static void (*ctrlLineStateCb)(uint16_t ctrlLineState);

static void (*baudRateCb)(uint32_t) = nullptr;
// static void* baudRateCbCtx;

bool cdcConnected = false;

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  VCP_Init
  *         Initializes the Media on the STM32
  * @param  None
  * @retval Result of the opeartion (USBD_OK in all cases)
  */
static uint16_t VCP_Init(void)
{
  cdcConnected = true;
  // receiveDataCb = nullptr;
  // baudRateCb = nullptr;

  return USBD_OK;
}

/**
  * @brief  VCP_DeInit
  *         DeInitializes the Media on the STM32
  * @param  None
  * @retval Result of the opeartion (USBD_OK in all cases)
  */
static uint16_t VCP_DeInit(void)
{
  cdcConnected = false;
  receiveDataCb = nullptr;
  baudRateCb = nullptr;

  return USBD_OK;
}

void ust_cpy(LINE_CODING* plc2, const LINE_CODING* plc1)
{
   plc2->bitrate    = plc1->bitrate;
   plc2->format     = plc1->format;
   plc2->paritytype = plc1->paritytype;
   plc2->datatype   = plc1->datatype;
}

/**
  * @brief  VCP_Ctrl
  *         Manage the CDC class requests
  * @param  Cmd: Command code
  * @param  Buf: Buffer containing command data (request parameters)
  * @param  Len: Number of data to be sent (in bytes)
  * @retval Result of the opeartion (USBD_OK in all cases)
  */
static uint16_t VCP_Ctrl (uint32_t Cmd, uint8_t* Buf, uint32_t Len)
{
  LINE_CODING* plc = (LINE_CODING*)Buf;

  assert_param(Len>=sizeof(LINE_CODING));

  switch (Cmd)
  {
  case SEND_ENCAPSULATED_COMMAND:
    /* Not  needed for this driver */
    break;

  case GET_ENCAPSULATED_RESPONSE:
    /* Not  needed for this driver */
    break;

  case SET_COMM_FEATURE:
    /* Not  needed for this driver */
    break;

  case GET_COMM_FEATURE:
    /* Not  needed for this driver */
    break;

  case CLEAR_COMM_FEATURE:
    /* Not  needed for this driver */
    break;

  case SET_LINE_CODING:
    if (plc && (Len == sizeof (*plc))) {
      // If a callback is provided, tell the upper driver of changes in baud rate
      auto _cb = baudRateCb;
      // auto _ctx = baudRateCbCtx;
      if (_cb) {
        _cb(/*_ctx,*/ plc->bitrate);
      }
      // Copy into structure to save for later
      ust_cpy(&g_lc, plc);
    }
    break;

  case GET_LINE_CODING:
    if (plc && (Len == sizeof (*plc))) {
      ust_cpy(plc, &g_lc);
    }
    break;

  case SET_CONTROL_LINE_STATE:
    // If a callback is provided, tell the upper driver of changes in DTR/RTS state
    // if (plc && (Len == sizeof (uint16_t))) {
    //   if (ctrlLineStateCb) {
    //     ctrlLineStateCb(*((uint16_t *)Buf));
    //   }
    // }
    break;

  case SEND_BREAK:
    /* Not  needed for this driver */
    break;

  default:
    break;
  }

  return USBD_OK;
}

// return the bytes free in the circular buffer
uint32_t usbSerialFreeSpace()
{
  // functionally equivalent to:
  //
  //      (APP_Rx_ptr_out > APP_Rx_ptr_in ? APP_Rx_ptr_out - APP_Rx_ptr_in :
  //      APP_RX_DATA_SIZE - APP_Rx_ptr_in + APP_Rx_ptr_in)
  //
  //  but without the impact of the condition check.

  return ((APP_Rx_ptr_out - APP_Rx_ptr_in) +
          (-((int)(APP_Rx_ptr_out <= APP_Rx_ptr_in)) & APP_RX_DATA_SIZE)) -
         1;
}

void usbSerialPutc(void*, uint8_t c)
{
  /*
    Apparently there is no reliable way to tell if the
    virtual serial port is opened or not.

    The cdcConnected variable only reports the state
    of the physical USB connection.
  */

  if (!cdcConnected) return;

  /*
    APP_Rx_Buffer and associated variables must be modified
    atomically, because they are used from the interrupt
  */

  /* Read PRIMASK register, check interrupt status before you disable them */
  /* Returns 0 if they are enabled, or non-zero if disabled */

  uint32_t prim = __get_PRIMASK();
  __disable_irq();

  APP_Rx_Buffer[APP_Rx_ptr_in] = c;
  APP_Rx_ptr_in = (APP_Rx_ptr_in + 1) % APP_RX_DATA_SIZE;

  if (!prim) __enable_irq();
}

/**
  * @brief  VCP_DataRx
  *         Data received over USB OUT endpoint is available here
  *
  *         @note
  *         This function will block any OUT packet reception on USB endpoint
  *         until exiting this function. If you exit this function before transfer
  *         is complete on CDC interface (ie. using DMA controller) it will result
  *         in receiving more data while previous ones are still not sent.
  *
  *         @note
  *         This function is executed inside the USBD_OTG_ISR_Handler() interrupt handler!

  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the opeartion: USBD_OK if all operations are OK else VCP_FAIL
  */
static uint16_t VCP_DataRx (uint8_t* Buf, uint32_t Len)
{
  auto _rxCb = receiveDataCb;
  // auto _ctx = receiveDataCbCtx;

  if (_rxCb) _rxCb(/*_ctx,*/ Buf, Len);
  return USBD_OK;
}

uint32_t usbSerialBaudRate(void*)
{
    return g_lc.bitrate;
}

void usbSerialSetReceiveDataCb(void*, void (*cb)(uint8_t*, uint32_t))
{
  // receiveDataCb = nullptr;
  // receiveDataCbCtx = cb_ctx;
  receiveDataCb = cb;
}

void usbSerialSetBaudRateCb(void*, void (*cb)(uint32_t))
{
  // baudRateCb = nullptr;
  // baudRateCbCtx = cb_ctx;
  baudRateCb = cb;
}

// void usbSerialSetCtrlLineStateCb(void (*cb)(uint16_t ctrlLineState))
// {
//   ctrlLineStateCb = cb;
// }

static const etx_serial_driver_t usbSerialDriver = {
  .init = nullptr,
  .deinit = nullptr,
  .sendByte = usbSerialPutc,
  .sendBuffer = nullptr,
  .waitForTxCompleted = nullptr,
  .getByte = nullptr,
  .clearRxBuffer = nullptr,
  .getBaudrate = usbSerialBaudRate,
  .setReceiveCb = usbSerialSetReceiveDataCb,
  .setBaudrateCb = usbSerialSetBaudRateCb,
};

const etx_serial_port_t UsbSerialPort = {
  "USB-VCP",
  &usbSerialDriver,
  nullptr,
};

// /************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
