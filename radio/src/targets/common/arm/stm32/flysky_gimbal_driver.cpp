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

#include "timers_driver.h"
#include "debug.h"

#include "hal.h"
#include "crc.h"

#include <string.h>

#define SAMPLING_TIMEOUT_US             1000  // us
#define RESAMPLING_THRESHOLD            2500  // us, 400Hz freq = 2500us period
#define RESAMPLING_SWITCHING_THRESHOLD   200  // us
#define MODE_CHANGE_DELAY                 50  // ms

static const stm32_usart_t fsUSART = {
  .USARTx = FLYSKY_HALL_SERIAL_USART,
  .txGPIO = FLYSKY_HALL_SERIAL_TX_GPIO,
  .rxGPIO = FLYSKY_HALL_SERIAL_RX_GPIO,
  .IRQn = FLYSKY_HALL_SERIAL_USART_IRQn,
  .IRQ_Prio = 4,
  .txDMA = FLYSKY_HALL_SERIAL_DMA,
#if defined(FLYSKY_HALL_DMA_Stream_TX)
  .txDMA_Stream = FLYSKY_HALL_DMA_Stream_TX,
  .txDMA_Channel = FLYSKY_HALL_DMA_Channel,
#endif
  .rxDMA = FLYSKY_HALL_SERIAL_DMA,
  .rxDMA_Stream = FLYSKY_HALL_DMA_Stream_RX,
  .rxDMA_Channel = FLYSKY_HALL_DMA_Channel,
};

DEFINE_STM32_SERIAL_PORT(FSGimbal, fsUSART, HALLSTICK_BUFF_SIZE, HALLSTICK_CMD_BUFF_SIZE);

static const etx_serial_port_t _fs_gimbal_serial_port = {
  .name = "gimbals",
  .uart = &STM32SerialDriver,
  .hw_def = REF_STM32_SERIAL_PORT(FSGimbal),
  .set_pwr = nullptr,
};

static STRUCT_HALL HallProtocol = { 0 };

static void* _fs_usart_ctx = nullptr;

static volatile bool _fs_gimbal_detected;
static uint8_t _fs_gimbal_version;
static V2_GIMBAL_MODE _fs_gimbal_mode;
static V2_GIMBAL_MODE _fs_gimbal_mode_change;
static V2_GIMBAL_MODE _fs_gimbal_mode_detected;
static uint32_t _fs_gimbal_last_mode_change_tick;
static bool _fs_gimbal_cmd_finished;
static uint32_t _fs_gimbal_lastReadTick;
static uint32_t _fs_gimbal_readTick;
static uint32_t _fs_gimbal_sync_period;

void _fs_send_cmd(uint8_t id, uint8_t payload)
{
  if (!_fs_gimbal_cmd_finished) {
    // Skip command when last command not finished
    return;
  }

  FSGimbal_TXBuffer[0] = FLYSKY_HALL_PROTOLO_HEAD;
  FSGimbal_TXBuffer[1] = id;
  FSGimbal_TXBuffer[2] = 0x01;
  FSGimbal_TXBuffer[3] = payload;

  unsigned short crc = crc16(CRC_1021, FSGimbal_TXBuffer, 4, 0xffff);

  FSGimbal_TXBuffer[4] = crc & 0xff;
  FSGimbal_TXBuffer[5] = crc >>8 & 0xff ;

  _fs_gimbal_cmd_finished = false;
  STM32SerialDriver.sendBuffer(_fs_usart_ctx, FSGimbal_TXBuffer, 6);
//  TRACE("Flysky Gimbal: Sent command, id = %d, payload = %d", id, payload);
}

void _fs_cmd_get_version()
{
  _fs_send_cmd(0xb1, 0x00);
}

void _fs_cmd_set_mode(V2_GIMBAL_MODE mode)
{
  _fs_gimbal_mode_change = mode;
  _fs_send_cmd(0x41, mode);
}

void _fs_cmd_start_read()
{
  _fs_send_cmd(0xc1, 0x00);
}

static void _fs_handle_msg(STRUCT_HALL* hallBuffer)
{
  uint8_t length = hallBuffer->length;
  if (length == 0) return;

  hallBuffer->stickState = hallBuffer->data[length - 1];

  switch (hallBuffer->hallID.hall_Id.receiverID) {
    case TRANSFER_DIR_TXMCU:
    case TRANSFER_DIR_RFMODULE: {
      int16_t* p_values = (int16_t*)hallBuffer->data;
      if (hallBuffer->hallID.hall_Id.packetID == FLYSKY_PACKET_CHANNEL_ID) {
        if (length < 4 * 2) break;  // 4 channels of int16_t
        _fs_gimbal_cmd_finished = true;
        uint16_t* adcValues = getAnalogValues();
        for (uint8_t i = 0; i < 4; i++) {
          adcValues[i] = FLYSKY_OFFSET_VALUE - p_values[i];
        }
      } else if (hallBuffer->hallID.hall_Id.packetID == FLYSKY_PACKET_VERSION_ID) {
        if (length < 8 * 2) break;  // version words read at p_values[6] and [7]
        _fs_gimbal_cmd_finished = true;
        uint16_t minorVersion = p_values[6];
        uint16_t majorVersion = p_values[7];
        if (majorVersion == 2 && minorVersion >= 1) {
          _fs_gimbal_version = GIMBAL_V2;
        }
      } else if (hallBuffer->hallID.hall_Id.packetID == FLYSKY_PACKET_MODE_ID) {
        _fs_gimbal_cmd_finished = true;
        _fs_gimbal_mode = _fs_gimbal_mode_change;
        TRACE("Flysky Gimbal: Mode changed successfully, mode = %d", _fs_gimbal_mode);
      }
    } break;
  }
  _fs_gimbal_detected = true;
}

static void flysky_gimbal_loop(void*)
{
  // The idle callback fires once the line goes idle, i.e. after a complete frame
  // has landed in the DMA ring buffer. Copy it out in one shot and parse the
  // frame(s) directly, instead of feeding a byte-at-a-time state machine.
  //
  // Frame layout: [HEAD][ID][LENGTH][DATA x LENGTH][CRC_lo][CRC_hi]
  uint8_t buffer[HALLSTICK_BUFF_SIZE];

  int len = STM32SerialDriver.copyRxBuffer(_fs_usart_ctx, buffer, sizeof(buffer));

  int idx = 0;
  while (idx + 5 <= len) {  // smallest possible frame: HEAD + ID + LENGTH + CRC
    uint8_t* frame = &buffer[idx];
    if (frame[0] != FLYSKY_HALL_PROTOLO_HEAD) {
      idx++;  // resync on header
      continue;
    }

    uint8_t length = frame[2];
    if (length == 0 || length > HALLSTICK_BUFF_SIZE - 5) {
      idx++;  // bogus length: skip this header and resync
      continue;
    }
    if (idx + length + 5 > len) {
      break;  // frame not fully present in this batch
    }

    uint16_t checkSum = frame[3 + length] | (frame[4 + length] << 8);
    if (checkSum == crc16(CRC_1021, frame, length + 3, 0xffff)) {
      HallProtocol.hallID.ID = frame[1];
      HallProtocol.length = length;
      memcpy(HallProtocol.data, &frame[3], length);
      _fs_handle_msg(&HallProtocol);
      idx += length + 5;
      continue;
    }

    idx++;  // CRC mismatch: length is unverified, resync byte-by-byte
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

  // Init variables
  _fs_gimbal_version = GIMBAL_V1;
  _fs_gimbal_mode = V1_MODE;
  _fs_gimbal_mode_detected = V1_MODE;
  _fs_gimbal_last_mode_change_tick = 0;
  _fs_gimbal_cmd_finished = true;
  _fs_gimbal_lastReadTick = 0;
  _fs_gimbal_sync_period = 0;
  
  _fs_usart_ctx = STM32SerialDriver.init(REF_STM32_SERIAL_PORT(FSGimbal), &cfg);
  if (!_fs_usart_ctx) return -1;

  STM32SerialDriver.setIdleCb(_fs_usart_ctx, flysky_gimbal_loop, 0);
  return 0;
}

bool flysky_gimbal_init()
{
  if (flysky_gimbal_init_uart() != 0) return false;

  _fs_gimbal_detected = false;

  // Wait 70ms for serial gimbals to respond
  for (uint16_t i = 0; i < 70; i++) {
#if defined(HALL_SYNC) && !defined(SIMU)
    gpio_set(HALL_SYNC);
#endif
    delay_ms(1);
#if defined(HALL_SYNC) && !defined(SIMU)
    gpio_clear(HALL_SYNC);
#endif
    if (_fs_gimbal_detected) {
      // Try to obtain the version of gimbal for operation mode selection
      _fs_cmd_get_version();

      // Mask the first 4 inputs (sticks)
      stm32_hal_set_inputs_mask(0xF);
      return true;
    }
  }
  
  flysky_gimbal_deinit();
  return false;
}

void flysky_gimbal_start_read()
{
  if(_fs_gimbal_detected && _fs_gimbal_version > GIMBAL_V1) {
    _fs_gimbal_lastReadTick = _fs_gimbal_readTick;
    _fs_gimbal_readTick = timersGetUsTick();
    if (_fs_gimbal_lastReadTick != 0) {
      _fs_gimbal_sync_period = _fs_gimbal_readTick - _fs_gimbal_lastReadTick;
      V2_GIMBAL_MODE newMode = _fs_gimbal_mode_detected;
      switch (_fs_gimbal_mode_detected) {
        case V1_MODE:
        case SYNC_SAMPLING:
          newMode = _fs_gimbal_sync_period < RESAMPLING_THRESHOLD ? SYNC_RESAMPLING : SYNC_SAMPLING;
          break;
        case SYNC_RESAMPLING:
          newMode = _fs_gimbal_sync_period >= RESAMPLING_THRESHOLD + RESAMPLING_SWITCHING_THRESHOLD ? SYNC_SAMPLING : SYNC_RESAMPLING;
          break;
      }
      if (_fs_gimbal_mode_detected != newMode) {
        _fs_gimbal_mode_detected = newMode;
        _fs_gimbal_last_mode_change_tick = timersGetMsTick();
      } else if (_fs_gimbal_mode != _fs_gimbal_mode_detected) {
        uint32_t tick = timersGetMsTick();
        if (tick - _fs_gimbal_last_mode_change_tick >= MODE_CHANGE_DELAY) {
          // Update mode when mode is stable
          _fs_cmd_set_mode(_fs_gimbal_mode_detected);
        }
      }
    }
    if (_fs_gimbal_mode != V1_MODE) {
      _fs_cmd_start_read();
    }
  }
}

void flysky_gimbal_wait_completion()
{
  if(_fs_gimbal_detected && _fs_gimbal_version > GIMBAL_V1 && _fs_gimbal_mode != V1_MODE) {
      auto timeout = timersGetUsTick();
    while(!_fs_gimbal_cmd_finished) {
      // busy wait
      if ((uint32_t)(timersGetUsTick() - timeout) >= SAMPLING_TIMEOUT_US) {
//        TRACE("Gimbal timeout");
        return;
      }
    }
  }
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
