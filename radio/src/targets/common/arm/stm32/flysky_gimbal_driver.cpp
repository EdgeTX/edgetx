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
  .txDMA = FLYSKY_HALL_SERIAL_DMA,
  .txDMA_Stream = FLYSKY_HALL_DMA_Stream_TX,
  .txDMA_Channel = FLYSKY_HALL_DMA_Channel,
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

static uint8_t _fs_hall_command[8] __DMA;

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
      if(hallBuffer->length > HALLSTICK_BUFF_SIZE - 5) { // buffer size - header size (1 byte header + 1 byte ID + 1 byte length + 2 bytes CRC = 5 bytes)
        hallBuffer->status = GET_START;
      } else if (0 == hallBuffer->length) {
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

void _fs_cmd_get_version()
{
  _fs_hall_command[0] = FLYSKY_HALL_PROTOLO_HEAD;
  _fs_hall_command[1] = 0xb1;
  _fs_hall_command[2] = 0x01;
  _fs_hall_command[3] = 0x00;

  unsigned short crc = crc16(CRC_1021, _fs_hall_command, 4, 0xffff);

  _fs_hall_command[4] = crc & 0xff;
  _fs_hall_command[5] = crc >>8 & 0xff ;

  STM32SerialDriver.sendBuffer(_fs_usart_ctx, _fs_hall_command, 6);
}

static volatile bool _fs_gimbal_detected;
static volatile uint8_t _fs_gimbal_version = GIMBAL_V1;
static volatile uint8_t versionInfo[4];

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
        case TRANSFER_DIR_RFMODULE:
          switch (HallProtocol.hallID.hall_Id.packetID) {
            case FLYSKY_PACKET_CHANNEL_ID:
              int16_t* p_values = (int16_t*)HallProtocol.data;
              uint16_t* adcValues = getAnalogValues();
              for (uint8_t i = 0; i < 4; i++) {
                adcValues[i] = FLYSKY_OFFSET_VALUE - p_values[i];
              }
              break;
            case FLYSKY_PACKET_VERSION_ID:
              int8_t* p_values = (int8_t*)HallProtocol.data;
              versionInfo[0] = p_values[12];
              versionInfo[1] = p_values[13];
              versionInfo[2] = p_values[14];
              versionInfo[3] = p_values[15];
              break;
          }
          break;
      }
      _fs_gimbal_detected = true;
    }
  }
}

void flysky_gimbal_deinit()
{
  if (_fs_usart_ctx != nullptr) {
    STM32SerialDriver.deinit(_fs_usart_ctx);
  }
  _fs_usart_ctx = 0;
}

static int flysky_gimbal_init_uart()
{
  etx_serial_init cfg = {
    .baudrate = FLYSKY_HALL_BAUDRATE,
    .encoding = ETX_Encoding_8N1,
    .direction = ETX_Dir_TX_RX,
    .polarity = ETX_Pol_Normal,
  };

  _fs_usart_ctx = STM32SerialDriver.init(REF_STM32_SERIAL_PORT(FSGimbal), &cfg);
  if (!_fs_usart_ctx) return -1;

  STM32SerialDriver.setIdleCb(_fs_usart_ctx, flysky_gimbal_loop, 0);
  return 0;
}

bool flysky_gimbal_init()
{
  if (flysky_gimbal_init_uart() != 0) return false;

  // Wait 70ms for FlySky gimbals to respond. According to LA trace, minimally
  // 23ms is required
  _fs_gimbal_detected = false;

  for (uint8_t i = 0; i < 70; i++) {
    delay_ms(1);
    if (_fs_gimbal_detected) {
      // Mask the first 4 inputs (sticks)
      stm32_hal_set_inputs_mask(0xF);
      return true;
    }
  }
  
  // Try to obtain the version of gimbal for operation mode selection
  _fs_cmd_get_version();

  flysky_gimbal_deinit();
  return false;
}

bool is_flysky_gimbal_sync_supported()
{
  return _fs_gimbal_version > GIMBAL_V1;
}

void flysky_gimbal_force_init()
{
  if (flysky_gimbal_init_uart() != 0) return;

  _fs_gimbal_detected = true;
  stm32_hal_set_inputs_mask(0xF);
}

const etx_serial_port_t* flysky_gimbal_get_port()
{
  return &_fs_gimbal_serial_port;
}
