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

#if defined(__cplusplus)
extern "C" {
#endif
#include "usb_dcd_int.h"
#include "usb_bsp.h"
#if defined(__cplusplus)
}
#endif

#include "opentx.h"
#include "debug.h"

#include "io/crsf/crsf.h"
#include "io/crsf/crossfire.h"

extern USB_OTG_CORE_HANDLE USB_OTG_dev;
extern usbMode selectedUsbMode;

#define USB_HID_FIFO_SIZE                       128
#define USB_HID_INEP1_NUM                       1
#define USB_HID_INEP1_TXFIFO_EMPTY              0x80U
#define USB_HID_INEP1_TXFIFO_FREE_WORD          0x80U
#define USB_HID_INEP1_TXFIFO_FREE_WORD_MASK     0xFFFFU
#define LIBCRSF_BF_LINK_STATISTICS              0x14

static Fifo<uint8_t, USB_HID_FIFO_SIZE> * hidTxFifo = 0;

void usbAgentWrite(uint8_t * pData)
{
  static uint8_t HID_Buffer[HID_AGENT_IN_PACKET];
  memcpy(HID_Buffer, pData, HID_AGENT_IN_PACKET);
  USBD_AGENT_SendReport(&USB_OTG_dev, HID_Buffer, HID_AGENT_IN_PACKET);
}

static uint8_t isUsbIdle()
{
  extern uint8_t ReportSent;
  return ReportSent;
  // workaround for usb stuck during send data but the host app is not ready to receive.
  uint8_t idle = (ReportSent &&
                  (USB_OTG_dev.regs.INEP_REGS[USB_HID_INEP1_NUM]->DIEPINT & USB_HID_INEP1_TXFIFO_EMPTY) &&
                  ((USB_OTG_dev.regs.INEP_REGS[USB_HID_INEP1_NUM]->DTXFSTS & USB_HID_INEP1_TXFIFO_FREE_WORD_MASK) == USB_HID_INEP1_TXFIFO_FREE_WORD)) ? 1 : 0;
  return idle;
}

void usbTX()
{
  static uint8_t isbusy = 0;
  if (!isbusy) {
    isbusy = 1;
    uint8_t sendData[HID_AGENT_IN_PACKET];
    memset(sendData, 0, HID_AGENT_IN_PACKET);
    if (hidTxFifo != 0 && hidTxFifo->size() > 0 && isUsbIdle()) {
      for (uint8_t i = 0; i < HID_AGENT_IN_PACKET; i++) {
        if (!hidTxFifo->pop(sendData[i])) {
          break;
        }
      }
      USBD_AGENT_SendReport(&USB_OTG_dev, sendData, HID_AGENT_IN_PACKET);
    }

    memset(sendData, 0, HID_AGENT_IN_PACKET);
    if (hidTxFifo != 0 && selectedUsbMode != USB_AGENT_MODE) {
      free(hidTxFifo);
      hidTxFifo = 0;
    }
    isbusy = 0;
  }
}

void crsfToUsbHid(uint8_t * pArr)
{
  *pArr = LIBCRSF_UART_SYNC;
  if (hidTxFifo == 0 && selectedUsbMode == USB_AGENT_MODE && usbStarted()) {
    hidTxFifo = (Fifo<uint8_t, USB_HID_FIFO_SIZE> *) malloc(sizeof(Fifo<uint8_t, USB_HID_FIFO_SIZE>));
    if (hidTxFifo != 0) {
      memset(hidTxFifo, 0, sizeof(Fifo<uint8_t, USB_HID_FIFO_SIZE>));
    }
  }

  // block sending telemetry and opentx related to usb
  if ((*(pArr + LIBCRSF_TYPE_ADD) != LIBCRSF_BF_LINK_STATISTICS) && (*(pArr + LIBCRSF_TYPE_ADD) != LIBCRSF_OPENTX_RELATED)) {
    if (hidTxFifo != 0 && hidTxFifo->hasSpace(pArr[LIBCRSF_LENGTH_ADD] + 2)) {
      for (uint8_t i = 0; i < HID_AGENT_IN_PACKET; i++) {
        hidTxFifo->push(pArr[i]);
      }
    }
  }
  usbTX();
}

void agentHandler()
{
  /* handle TBS Agent requests */
  extern uint8_t ReportReceived;
  extern uint8_t HidBuffer[HID_AGENT_OUT_PACKET];
  static libCrsfParseData hidCrsfData;
  uint8_t hidBuffer[HID_AGENT_OUT_PACKET];

  usbTX();

  while (ReportReceived) {
    // making a copy of HID_Buffer is a workaround for preventing changing data in hid_buffer during parsing crsf data,
    ReportReceived = 2;
    memcpy(hidBuffer, HidBuffer, HID_AGENT_OUT_PACKET);
    ReportReceived = 0;
    for (uint8_t i = 0; i < HID_AGENT_OUT_PACKET; i++) {
      if (libCrsfParse(&hidCrsfData, hidBuffer[i])) {
        libCrsfRouting(USB_HID, hidCrsfData.payload);
        break;
      }
    }
  }
}
