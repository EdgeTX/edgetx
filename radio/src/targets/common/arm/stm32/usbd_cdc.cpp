/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#include "opentx.h"

#if defined(TELEMETRY_MAVLINK)
#include "telemetry/mavlink/mavlink_telem.h"
#endif

extern "C" {

/* Includes ------------------------------------------------------------------*/
#include "usb_conf.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/


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
  return USBD_OK;
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
    /* Not  needed for this driver */
    break;

  case GET_LINE_CODING:
    /* Not  needed for this driver */
    break;

  case SET_CONTROL_LINE_STATE:
    /* Not  needed for this driver */
    break;

  case SEND_BREAK:
    /* Not  needed for this driver */
    break;

  default:
    break;
  }

  return USBD_OK;
}

// some debug vars
uint16_t usbWraps = 0;
uint16_t charsWritten = 0;

void usbSerialPutc(uint8_t c)
{

  /*
      Apparently there is no reliable way to tell if the
      virtual serial port is opened or not.

      The cdcConnected variable only reports the state
      of the physical USB connection.
  */

  if (!cdcConnected) return;

  uint32_t prim = __get_PRIMASK();
  __disable_irq();
  uint32_t txDataLen = APP_RX_DATA_SIZE + APP_Rx_ptr_in - APP_Rx_ptr_out;
  if (!prim) __enable_irq();

  if (txDataLen >= APP_RX_DATA_SIZE) {
    txDataLen -= APP_RX_DATA_SIZE;
  }
  if (txDataLen > (APP_RX_DATA_SIZE - CDC_DATA_MAX_PACKET_SIZE)) return;    // buffer is too full, skip this write

  ++charsWritten;

  /*
    APP_Rx_Buffer and associated variables must be modified
    atomically, because they are used from the interrupt
  */

  /* Read PRIMASK register, check interrupt status before you disable them */
  /* Returns 0 if they are enabled, or non-zero if disabled */
  prim = __get_PRIMASK();
  __disable_irq();
  APP_Rx_Buffer[APP_Rx_ptr_in++] = c;
  if(APP_Rx_ptr_in >= APP_RX_DATA_SIZE)
  {
    APP_Rx_ptr_in = 0;
    ++usbWraps;
  }
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
  // TODO: try implementing inbound flow control:
  //        if the cliRxFifo does not have enough free space to receive all
  //        available characters, return VCP_FAIL. Maybe that will throttle down
  //        the sender and we will receive the same packet at a later time.

#if defined(CLI)
  //copy data to the application FIFO
  for (uint32_t i = 0; i < Len; i++) {
    cliRxFifo.push(Buf[i]);
  }
#endif
#if defined(LUA) && !defined(CLI)
  // copy data to the LUA FIFO
  if (luaRxFifo) {
    for (uint32_t i = 0; i < Len; i++) {
      luaRxFifo->push(Buf[i]);
    }
  }
#endif
#if defined(TELEMETRY_MAVLINK) && defined(USB_SERIAL)
  for (uint32_t i = 0; i < Len; i++) {
    mavlinkTelemUsbRxFifo.push(Buf[i]);
  }
#endif
  return USBD_OK;
}



// /************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
