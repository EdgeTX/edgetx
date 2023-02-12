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
#include "delays_driver.h"
#include "timers_driver.h"

#include "hal.h"
#include "crc.h"

#include <string.h>

static const stm32_usart_t fsUSART = {
  .USARTx = FLYSKY_HALL_SERIAL_USART,
  .GPIOx = FLYSKY_HALL_SERIAL_GPIO,
  .GPIO_Pin = FLYSKY_HALL_SERIAL_RX_GPIO_PIN,
  .IRQn = FLYSKY_HALL_SERIAL_USART_IRQn,
  .IRQ_Prio = 6,
  .txDMA = nullptr,
  .txDMA_Stream = 0,
  .txDMA_Channel = 0,
  .rxDMA = FLYSKY_HALL_SERIAL_DMA,
  .rxDMA_Stream = FLYSKY_HALL_DMA_Stream_RX,
  .rxDMA_Channel = FLYSKY_HALL_DMA_Channel,
};

DEFINE_STM32_SERIAL_PORT(FSGimbal, fsUSART, HALLSTICK_BUFF_SIZE, 0);

static STRUCT_HALL HallProtocol = { 0 };

signed short hall_raw_values[FLYSKY_HALL_CHANNEL_COUNT];
unsigned short hall_adc_values[FLYSKY_HALL_CHANNEL_COUNT];

// Resampling variables
signed short hallRawSampleValues[FLYSKY_HALL_CHANNEL_COUNT]; // current sample
signed short hallRawLastSampleValues[FLYSKY_HALL_CHANNEL_COUNT]; // last sample
uint32_t rawSampleTime = 0;
uint32_t rawLastSampleTime = 0;
bool lastGetTimeValid = false;
uint16_t lastGetTime = 0;

static void* _fs_usart_ctx = nullptr;

uint16_t get_flysky_hall_adc_value(uint8_t ch)
{
 if (ch == 0)
  {
    // Resampling calculation
    uint32_t time = getTmr2MHz();
    uint16_t interpolateThreshold;    

    if (!lastGetTimeValid)
    {
      // No lastGetTime information
      lastGetTime = time;
      lastGetTimeValid = true;

      // No resampling
      for ( uint8_t channel = 0; channel < 4; channel++ )
      {
        hall_raw_values[channel] = hallRawSampleValues[channel];
      }
    }
    else
    {
      if (time < lastGetTime)
      {
        interpolateThreshold = ((65536 - lastGetTime) + time) >> 1;
      }
      else
      {
        interpolateThreshold = (time - lastGetTime) >> 1;
      }
      lastGetTime = time;

      while (time < rawSampleTime)
      {
        time += 65536;
      }
      if (time - rawSampleTime < interpolateThreshold)
      {
        // Linear interpolation
        int32_t timeDiff1 = time - rawLastSampleTime;
        int32_t timeDiff2 = rawSampleTime - rawLastSampleTime;
        for ( uint8_t channel = 0; channel < 4; channel++ )
        {
          int32_t value = hallRawSampleValues[channel];
          value -= hallRawLastSampleValues[channel];
          value *= timeDiff1;
          value /= timeDiff2;
          value += hallRawLastSampleValues[channel];
          hall_raw_values[channel] = value;
        }
      }
      else
      {
        // Linear extrapolation
        int32_t timeDiff1 = time - rawLastSampleTime;
        int32_t timeDiff2 = rawLastSampleTime - rawLastSampleTime;
        for ( uint8_t channel = 0; channel < 4; channel++ )
        {
          int32_t value = hallRawSampleValues[channel];
          value -= hallRawLastSampleValues[channel];
          value *= timeDiff1;
          value /= timeDiff2;
          value += hallRawSampleValues[channel];
          hall_raw_values[channel] = value;
        }
      }
    }

    for ( uint8_t channel = 0; channel < 4; channel++ )
    {
      hall_adc_values[channel] = FLYSKY_OFFSET_VALUE + hall_raw_values[channel];
    }
  }


  if (ch >= FLYSKY_HALL_CHANNEL_COUNT) {
    return 0;
  }

  return 2 * FLYSKY_OFFSET_VALUE - hall_adc_values[ch];
}

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

static void flysky_gimbal_loop()
{
    uint8_t byte;

    while(_fs_get_byte(&byte))
    {
        HallProtocol.index++;

        _fs_parse(&HallProtocol, byte);
        if ( HallProtocol.msg_OK )
        {
            HallProtocol.msg_OK = 0;
            HallProtocol.stickState = HallProtocol.data[HallProtocol.length - 1];

            switch ( HallProtocol.hallID.hall_Id.receiverID )
            {
            case TRANSFER_DIR_TXMCU:
                if(HallProtocol.hallID.hall_Id.packetID == FLYSKY_HALL_RESP_TYPE_VALUES) {
                  memcpy(hallRawLastSampleValues, hallRawSampleValues, sizeof(hallRawSampleValues));
                  memcpy(hallRawSampleValues, HallProtocol.data, sizeof(hallRawSampleValues));
                  rawLastSampleTime = rawSampleTime;
                  rawSampleTime = getTmr2MHz();
                  if (rawSampleTime < rawLastSampleTime)
                  {
                    rawSampleTime += 65536;
                  }
                  if (rawLastSampleTime >= 65536)
                  {
                    rawSampleTime -= 65536;
                    rawLastSampleTime -= 65536;
                  }
/*                  for ( uint8_t channel = 0; channel < 4; channel++ )
                  {
                    hall_adc_values[channel] = FLYSKY_OFFSET_VALUE + hall_raw_values[channel];
                  }*/
                }
                break;
            }
            _fs_gimbal_detected = true;
        }
    }
}

static void flysky_gimbal_deinit()
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
  STM32SerialDriver.setIdleCb(_fs_usart_ctx, flysky_gimbal_loop);

  // Wait 70ms for FlySky gimbals to respond. According to LA trace, minimally 23ms is required
  for (uint8_t i = 0; i < 70; i++) {
    delay_ms(1);
    if (_fs_gimbal_detected) {
      return true;
    }
  }

  flysky_gimbal_deinit();
  return false;
}
