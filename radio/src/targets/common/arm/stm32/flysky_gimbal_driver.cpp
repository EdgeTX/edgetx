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

#include "flysky_gimbal_driver.h"
#include "stm32_serial_driver.h"
#include "stm32_gpio.h"
#include "stm32_adc.h"

#include "delays_driver.h"
#include "hal/adc_driver.h"
#include "hal/gpio.h"

#include "hal.h"
#include "crc.h"

#include <string.h>

static const stm32_usart_t fsUSART = {
  .USARTx = FLYSKY_HALL_SERIAL_USART,
  .txGPIO = FLYSKY_HALL_SERIAL_TX_GPIO,
  .rxGPIO = FLYSKY_HALL_SERIAL_RX_GPIO,
  .IRQn = FLYSKY_HALL_SERIAL_USART_IRQn,
  .IRQ_Prio = 4,
  .txDMA = nullptr,
  .txDMA_Stream = 0,
  .txDMA_Channel = 0,
  .rxDMA = FLYSKY_HALL_SERIAL_DMA,
  .rxDMA_Stream = FLYSKY_HALL_DMA_Stream_RX,
  .rxDMA_Channel = FLYSKY_HALL_DMA_Channel,
};

DEFINE_STM32_SERIAL_PORT(FSGimbal, fsUSART, HALLSTICK_BUFF_SIZE, 0);

static const etx_serial_port_t _fs_gimbal_serial_port = {
  .name = "gimbals",
  .uart = &STM32SerialDriver,
  .hw_def = REF_STM32_SERIAL_PORT(FSGimbal),
  .set_pwr = nullptr,
};

static STRUCT_HALL HallProtocol = { 0 };

static void* _fs_usart_ctx = nullptr;

static int _fs_get_byte(uint8_t* data)
{
  return STM32SerialDriver.getByte(_fs_usart_ctx, data);
}

static void _fs_parse(STRUCT_HALL *hallBuffer, unsigned char ch)
{
  switch (hallBuffer->status) {
    case GET_START:
      if (FLYSKY_HALL_PROTOLO_HEAD == ch) {
        hallBuffer->head = FLYSKY_HALL_PROTOLO_HEAD;
        hallBuffer->status = GET_ID;
        hallBuffer->msg_OK = 0;
      }
      break;

    case GET_ID:
      hallBuffer->hallID.ID = ch;
      hallBuffer->status = GET_LENGTH;
      break;

    case GET_LENGTH:
      hallBuffer->length = ch;
      hallBuffer->dataIndex = 0;
      hallBuffer->status = GET_DATA;
      if (0 == hallBuffer->length) {
        hallBuffer->status = GET_CHECKSUM;
        hallBuffer->checkSum = 0;
      }
      break;

    case GET_DATA:
      hallBuffer->data[hallBuffer->dataIndex++] = ch;
      if (hallBuffer->dataIndex >= hallBuffer->length) {
        hallBuffer->checkSum = 0;
        hallBuffer->dataIndex = 0;
        hallBuffer->status = GET_STATE;
      }
      break;

    case GET_STATE:
      hallBuffer->checkSum = 0;
      hallBuffer->dataIndex = 0;
      hallBuffer->status = GET_CHECKSUM;
      // fall through!

    case GET_CHECKSUM:
      hallBuffer->checkSum |= ch << ((hallBuffer->dataIndex++) * 8);
      if (hallBuffer->dataIndex >= 2) {
        hallBuffer->dataIndex = 0;
        hallBuffer->status = CHECKSUM;
        // fall through!
      } else {
        break;
      }

    case CHECKSUM:
      if (hallBuffer->checkSum ==
          crc16(CRC_1021, &hallBuffer->head, hallBuffer->length + 3, 0xffff)) {
        hallBuffer->msg_OK = 1;
      }
      hallBuffer->status = GET_START;
      break;
  }
}

static volatile bool _fs_gimbal_detected;

static void flysky_gimbal_loop(void*)
{
  uint8_t byte;

  while (_fs_get_byte(&byte)) {
    HallProtocol.index++;

    _fs_parse(&HallProtocol, byte);
    if (HallProtocol.msg_OK) {
      HallProtocol.msg_OK = 0;
      HallProtocol.stickState = HallProtocol.data[HallProtocol.length - 1];

      switch (HallProtocol.hallID.hall_Id.receiverID) {
        case TRANSFER_DIR_TXMCU:
          if (HallProtocol.hallID.hall_Id.packetID ==
              FLYSKY_HALL_RESP_TYPE_VALUES) {
            int16_t* p_values = (int16_t*)HallProtocol.data;
            uint16_t* adcValues = getAnalogValues();
            for (uint8_t i = 0; i < 4; i++) {
              adcValues[i] = FLYSKY_OFFSET_VALUE - p_values[i];
            }
          }
          break;
      }
      _fs_gimbal_detected = true;
    }
  }
}

void flysky_gimbal_deinit()
{
  STM32SerialDriver.deinit(_fs_usart_ctx);
}

bool flysky_gimbal_init()
{
  etx_serial_init cfg = {
    .baudrate = FLYSKY_HALL_BAUDRATE,
    .encoding = ETX_Encoding_8N1,
    .direction = ETX_Dir_RX,
    .polarity = ETX_Pol_Normal,
  };

  _fs_gimbal_detected = false;
  _fs_usart_ctx = STM32SerialDriver.init(REF_STM32_SERIAL_PORT(FSGimbal), &cfg);
  STM32SerialDriver.setIdleCb(_fs_usart_ctx, flysky_gimbal_loop, 0);

  // Wait 70ms for FlySky gimbals to respond. According to LA trace, minimally 23ms is required
  for (uint8_t i = 0; i < 70; i++) {
    delay_ms(1);
    if (_fs_gimbal_detected) {
      // Mask the first 4 inputs (sticks)
      stm32_hal_set_inputs_mask(0xF);
      return true;
    }
  }

  flysky_gimbal_deinit();
  return false;
}

const etx_serial_port_t* flysky_gimbal_get_port()
{
  return &_fs_gimbal_serial_port;
}
