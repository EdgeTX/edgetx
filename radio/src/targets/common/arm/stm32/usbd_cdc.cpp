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

extern "C" {

/* Includes ------------------------------------------------------------------*/
#include "usb_conf.h"
#include "usbd_cdc.h"

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

#if defined(USE_USB_HS)
uint8_t UserRxBufferFS[CDC_DATA_HS_OUT_PACKET_SIZE];
#else
uint8_t UserRxBufferFS[CDC_DATA_FS_OUT_PACKET_SIZE];
#endif

/** Data to send over USB CDC are stored in this buffer   */
uint8_t  UserTxBufferFS[APP_TX_DATA_SIZE];
volatile uint32_t APP_Tx_ptr_in = 0;
volatile uint32_t APP_Tx_ptr_out = 0;

/**
  * @}
  */

/** @defgroup USBD_VCP_IF_Exported_Variables USBD_VCP_IF_Exported_Variables
  * @brief Public variables.
  * @{
  */

extern USBD_HandleTypeDef hUsbDevice;

/* USER CODE BEGIN EXPORTED_VARIABLES */

/* USER CODE END EXPORTED_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_VCP_IF_Private_FunctionPrototypes USBD_VCP_IF_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

static int8_t VCP_Init_FS(void);
static int8_t VCP_DeInit_FS(void);
static int8_t VCP_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t VCP_Receive_FS(uint8_t* pbuf, uint32_t *Len);
static int8_t VCP_TransmitCplt_FS(uint8_t *pbuf, uint32_t *Len, uint8_t epnum);
static int8_t VCP_StartOfFrame_FS();

/* USER CODE BEGIN PRIVATE_FUNCTIONS_DECLARATION */

/* USER CODE END PRIVATE_FUNCTIONS_DECLARATION */

/**
  * @}
  */

USBD_CDC_ItfTypeDef USBD_Interface_fops =
{
  VCP_Init_FS,
  VCP_DeInit_FS,
  VCP_Control_FS,
  VCP_Receive_FS,
  VCP_TransmitCplt_FS,
  VCP_StartOfFrame_FS,
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
  * @brief  Initializes the CDC media low layer over the FS USB IP
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t VCP_Init_FS(void)
{
  cdcConnected = true;
  USBD_CDC_SetTxBuffer(&hUsbDevice, UserTxBufferFS, 0);
  USBD_CDC_SetRxBuffer(&hUsbDevice, UserRxBufferFS);
  return USBD_OK;
}

/**
  * @brief  DeInitializes the CDC media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t VCP_DeInit_FS(void)
{
  /* USER CODE BEGIN 4 */
  cdcConnected = false;
  receiveDataCb = nullptr;
  baudRateCb = nullptr;
  APP_Tx_ptr_in = 0;
  APP_Tx_ptr_out = 0;
  return (USBD_OK);
  /* USER CODE END 4 */
}

void ust_cpy(LINE_CODING* plc2, const LINE_CODING* plc1)
{
   plc2->bitrate    = plc1->bitrate;
   plc2->format     = plc1->format;
   plc2->paritytype = plc1->paritytype;
   plc2->datatype   = plc1->datatype;
}

/**
  * @brief  Manage the CDC class requests
  * @param  cmd: Command code
  * @param  pbuf: Buffer containing command data (request parameters)
  * @param  length: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t VCP_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length)
{
  LINE_CODING* plc = (LINE_CODING*)pbuf;

  /* USER CODE BEGIN 5 */
  assert_param(Len>=sizeof(LINE_CODING));
  
  switch(cmd)
  {
  case CDC_SEND_ENCAPSULATED_COMMAND:
    /* Not  needed for this driver */
    break;

  case CDC_GET_ENCAPSULATED_RESPONSE:
    /* Not  needed for this driver */
    break;

  case CDC_SET_COMM_FEATURE:
    /* Not  needed for this driver */
    break;

  case CDC_GET_COMM_FEATURE:
    /* Not  needed for this driver */
    break;

  case CDC_CLEAR_COMM_FEATURE:
    /* Not  needed for this driver */
    break;

  case CDC_SET_LINE_CODING:
    if (plc && (length == sizeof (*plc))) {
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

  case CDC_GET_LINE_CODING:
    if (plc && (length == sizeof (*plc))) {
      ust_cpy(plc, &g_lc);
    }
    break;

  case CDC_SET_CONTROL_LINE_STATE:
    // If a callback is provided, tell the upper driver of changes in DTR/RTS state
    // if (plc && (Len == sizeof (uint16_t))) {
    //   if (ctrlLineStateCb) {
    //     ctrlLineStateCb(*((uint16_t *)Buf));
    //   }
    // }
    break;

  case CDC_SEND_BREAK:
    /* Not  needed for this driver */
    break;

  default:
    break;
  }

  return (USBD_OK);
  /* USER CODE END 5 */
}

/**
  * @brief  VCP_TransmitCplt_FS
  *         Data transmitted callback
  *
  *         @note
  *         This function is IN transfer complete callback used to inform user that
  *         the submitted Data is successfully sent over USB.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t VCP_TransmitCplt_FS(uint8_t *Buf, uint32_t *Len, uint8_t epnum)
{
  uint8_t result = USBD_OK;
  /* USER CODE BEGIN 13 */
  UNUSED(Buf);
  UNUSED(Len);
  UNUSED(epnum);
  /* USER CODE END 13 */
  return result;
}

static int8_t VCP_StartOfFrame_FS()
{
  uint8_t result = USBD_OK;
  static uint8_t FrameCount = 0;    // modified by OpenTX

  if (FrameCount++ >= CDC_IN_FRAME_INTERVAL)     // modified by OpenTX
  {
    /* Reset the frame counter */
    FrameCount = 0;

    /* Check the data to be sent through IN pipe */
    USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDevice.pClassData;

    if (hcdc->TxState != 0)
      return USBD_OK;

    if (APP_Tx_ptr_out == APP_TX_DATA_SIZE)
      APP_Tx_ptr_out = 0;

    if(APP_Tx_ptr_out == APP_Tx_ptr_in)
      return USBD_OK;

    size_t length = 0;

    if(APP_Tx_ptr_out > APP_Tx_ptr_in) /* rollback */
    {
      length = APP_TX_DATA_SIZE - APP_Tx_ptr_out;
    }
    else
    {
      length = APP_Tx_ptr_in - APP_Tx_ptr_out;
    }

    USBD_CDC_SetTxBuffer(&hUsbDevice, &UserTxBufferFS[APP_Tx_ptr_out], length);
    result = USBD_CDC_TransmitPacket(&hUsbDevice);
    if(result == USBD_OK)
      APP_Tx_ptr_out += length;
  }

  return result;
}


// return the bytes free in the circular buffer
uint32_t usbSerialFreeSpace()
{
  // functionally equivalent to:
  //
  //      (APP_Tx_ptr_out > APP_Tx_ptr_in ? APP_Tx_ptr_out - APP_Tx_ptr_in :
  //      APP_TX_DATA_SIZE - APP_Tx_ptr_in + APP_Tx_ptr_in)
  //
  //  but without the impact of the condition check.

  return ((APP_Tx_ptr_out - APP_Tx_ptr_in) +
          (-((int)(APP_Tx_ptr_out <= APP_Tx_ptr_in)) & APP_TX_DATA_SIZE)) -
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
    k and associated variables must be modified
    atomically, because they are used from the interrupt
  */

  /* Read PRIMASK register, check interrupt status before you disable them */
  /* Returns 0 if they are enabled, or non-zero if disabled */

  uint32_t prim = __get_PRIMASK();
  __disable_irq();

  UserTxBufferFS[APP_Tx_ptr_in] = c;
  APP_Tx_ptr_in = (APP_Tx_ptr_in + 1) % APP_TX_DATA_SIZE;

  if (!prim) __enable_irq();
}

/**
  * @brief  Data received over USB OUT endpoint are sent over CDC interface
  *         through this function.
  *
  *         @note
  *         This function will issue a NAK packet on any OUT packet received on
  *         USB endpoint until exiting this function. If you exit this function
  *         before transfer is complete on CDC interface (ie. using DMA controller)
  *         it will result in receiving more data while previous ones are still
  *         not sent.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t VCP_Receive_FS(uint8_t* Buf, uint32_t *Len)
{
  auto _rxCb = receiveDataCb;
  // auto _ctx = receiveDataCbCtx;

  if (_rxCb) _rxCb(/*_ctx,*/ Buf, *Len);

  USBD_CDC_SetRxBuffer(&hUsbDevice, &Buf[0]);
  USBD_CDC_ReceivePacket(&hUsbDevice);
  return USBD_OK;
}

#include "hal/usb_driver.h"

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

static void* usbSerialInit(void*, const etx_serial_init*)
{
  // always succeeds
  return (void*)1;
}

static const etx_serial_driver_t usbSerialDriver = {
  .init = usbSerialInit,
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
  nullptr,
};

// /************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
