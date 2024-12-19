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

#include "serial_gimbal_driver.h"
#include "stm32_serial_driver.h"
#include "stm32_gpio.h"
#include "stm32_adc.h"

#include "delays_driver.h"
#include "hal/adc_driver.h"
#include "hal/gpio.h"

#include "hal.h"
#include "crc.h"

#include <string.h>

static const stm32_usart_t SGUSART = {
  .USARTx = SERIAL_GIMBAL_SERIAL_USART,
  .txGPIO = SERIAL_GIMBAL_SERIAL_TX_GPIO,
  .rxGPIO = SERIAL_GIMBAL_SERIAL_RX_GPIO,
  .IRQn = SERIAL_GIMBAL_SERIAL_USART_IRQn,
  .IRQ_Prio = 4,
  .txDMA = nullptr,
  .txDMA_Stream = 0,
  .txDMA_Channel = 0,
  .rxDMA = SERIAL_GIMBAL_SERIAL_DMA,
  .rxDMA_Stream = SERIAL_GIMBAL_DMA_Stream_RX,
  .rxDMA_Channel = SERIAL_GIMBAL_DMA_Channel,
};

DEFINE_STM32_SERIAL_PORT(SerGimbal, SGUSART, SERIAL_GIMBAL_BUFFER_SIZE, 0);

static const etx_serial_port_t _serial_gimbal_serial_port = {
  .name = "gimbals",
  .uart = &STM32SerialDriver,
  .hw_def = REF_STM32_SERIAL_PORT(SerGimbal),
  .set_pwr = nullptr,
};

static STRUCT_SERIAL_GIMBAL HallProtocol = { 0 };

static void* _serial_gimbal_usart_ctx = nullptr;

static int _serial_gimbal_get_byte(uint8_t* data)
{
  return STM32SerialDriver.getByte(_serial_gimbal_usart_ctx, data);
}

static void _fs_parse(STRUCT_SERIAL_GIMBAL *sgBuffer, unsigned char ch)
{
  switch (sgBuffer->status) {
    case GET_START:
      if (SERIAL_GIMBAL_PROTOLO_HEAD == ch) {
        sgBuffer->head = SERIAL_GIMBAL_PROTOLO_HEAD;
        sgBuffer->status = GET_ID;
        sgBuffer->msg_OK = 0;
      }
      break;

    case GET_ID:
      sgBuffer->serGimbalID.ID = ch;
      sgBuffer->status = GET_LENGTH;
      break;

    case GET_LENGTH:
      sgBuffer->length = ch;
      sgBuffer->dataIndex = 0;
      sgBuffer->status = GET_DATA;
      if (0 == sgBuffer->length) {
        sgBuffer->status = GET_CHECKSUM;
        sgBuffer->checkSum = 0;
      }
      break;

    case GET_DATA:
      sgBuffer->data[sgBuffer->dataIndex++] = ch;
      if (sgBuffer->dataIndex >= sgBuffer->length) {
        sgBuffer->checkSum = 0;
        sgBuffer->dataIndex = 0;
        sgBuffer->status = GET_STATE;
      }
      break;

    case GET_STATE:
      sgBuffer->checkSum = 0;
      sgBuffer->dataIndex = 0;
      sgBuffer->status = GET_CHECKSUM;
      // fall through!

    case GET_CHECKSUM:
      sgBuffer->checkSum |= ch << ((sgBuffer->dataIndex++) * 8);
      if (sgBuffer->dataIndex >= 2) {
        sgBuffer->dataIndex = 0;
        sgBuffer->status = CHECKSUM;
        // fall through!
      } else {
        break;
      }

    case CHECKSUM:
      if (sgBuffer->checkSum ==
          crc16(CRC_1021, &sgBuffer->head, sgBuffer->length + 3, 0xffff)) {
        sgBuffer->msg_OK = 1;
      }
      sgBuffer->status = GET_START;
      break;
  }
}

static volatile bool _serial_gimbal_detected;

static void serial_gimbal_loop(void*)
{
  uint8_t byte;

  while (_serial_gimbal_get_byte(&byte)) {
    HallProtocol.index++;

    _fs_parse(&HallProtocol, byte);
    if (HallProtocol.msg_OK) {
      HallProtocol.msg_OK = 0;
      HallProtocol.stickState = HallProtocol.data[HallProtocol.length - 1];

      switch (HallProtocol.serGimbalID.sg_Id.receiverID) {
        case TRANSFER_DIR_TXMCU:
          if (HallProtocol.serGimbalID.sg_Id.packetID ==
              SERIAL_GIMBAL_RESP_TYPE_VALUES) {
            int16_t* p_values = (int16_t*)HallProtocol.data;
            uint16_t* adcValues = getAnalogValues();
            for (uint8_t i = 0; i < 4; i++) {
              adcValues[i] = SERIAL_GIMBAL_OFFSET_VALUE - p_values[i];
            }
          }
          break;
      }
      _serial_gimbal_detected = true;
    }
  }
}

void serial_gimbal_deinit()
{
  STM32SerialDriver.deinit(_serial_gimbal_usart_ctx);
}

bool serial_gimbal_init()
{
  etx_serial_init cfg = {
    .baudrate = SERIAL_GIMBAL_BAUDRATE,
    .encoding = ETX_Encoding_8N1,
    .direction = ETX_Dir_RX,
    .polarity = ETX_Pol_Normal,
  };

  _serial_gimbal_detected = false;
  _serial_gimbal_usart_ctx = STM32SerialDriver.init(REF_STM32_SERIAL_PORT(SerGimbal), &cfg);
  STM32SerialDriver.setIdleCb(_serial_gimbal_usart_ctx, serial_gimbal_loop, 0);

  // Wait 70ms for Serial gimbals to respond. According to LA trace, minimally 23ms is required
  for (uint8_t i = 0; i < 70; i++) {
    delay_ms(1);
    if (_serial_gimbal_detected) {
      // Mask the first 4 inputs (sticks)
      stm32_hal_set_inputs_mask(0xF);
      return true;
    }
  }

  serial_gimbal_deinit();
  return false;
}

const etx_serial_port_t* serial_gimbal_get_port()
{
  return &_serial_gimbal_serial_port;
}
