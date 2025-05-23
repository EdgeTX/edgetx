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

#include "vs1053b.h"

#include "os/sleep.h"
#include "stm32_hal_ll.h"
#include "stm32_gpio_driver.h"
#include "stm32_gpio.h"
#include "stm32_spi.h"
#include "hal/gpio.h"
#include "hal/audio_driver.h"

#include "board.h"
#include "edgetx.h"
#include "timers_driver.h"

#define VS_WRITE_COMMAND           0x02
#define VS_READ_COMMAND            0x03

#define SPI_MODE                   0x00
#define SPI_STATUS                 0x01
#define SPI_BASS                   0x02
#define SPI_CLOCKF                 0x03
#define SPI_DECODE_TIME            0x04
#define SPI_AUDATA                 0x05
#define SPI_WRAM                   0x06
#define SPI_WRAMADDR               0x07
#define SPI_HDAT0                  0x08
#define SPI_HDAT1                  0x09
#define SPI_AIADDR                 0x0a
#define SPI_VOL                    0x0b
#define SPI_AICTRL0                0x0c
#define SPI_AICTRL1                0x0d
#define SPI_AICTRL2                0x0e
#define SPI_AICTRL3                0x0f

#define SM_DIFF                    0x01
#define SM_LAYER12                 0x02
#define SM_RESET                   0x04
#define SM_CANCEL                  0x08
#define SM_EARSPEAKER_LO           0x10
#define SM_TESTS                   0x20
#define SM_STREAM                  0x40
#define SM_EARSPEAKER_HI           0x80
#define SM_DACT                   0x100
#define SM_SDIORD                 0x200
#define SM_SDISHARE               0x400
#define SM_SDINEW                 0x800
#define SM_ADPCM                 0x1000
#define SM_LINE1                 0x4000
#define SM_CLK_RANGE             0x8000

#define SPI_LOW_SPEED           1500000
#define SPI_HIGH_SPEED         12000000

#define VS1053_BUFFER_SIZE 32

#define DEFAULT_VOLUME 0xFEFE

#define XDCS_HIGH()  _xdcs_high()
#define XDCS_LOW()   _xdcs_low()
#define READ_DREQ()  _read_dreq()

static const vs1053b_t* _instance = nullptr;

// mute support
static bool _is_muted = false;
static uint32_t _last_play_ts = 0;

static uint8_t* _audio_buffer = nullptr;
static uint32_t _audio_buffer_len = 0;

static void(*_spi_transmit_data)(const uint8_t*, uint32_t) = nullptr;

// volume is set asynchronously:
// - first saved in _async_volume
// - then vs1053b_update_volume() is called periodically
//   to actually send the new setting to the device.
static int16_t _async_volume;

static inline void _xdcs_high() { gpio_set(_instance->XDCS); }
static inline void _xdcs_low() { gpio_clear(_instance->XDCS); }
static inline uint32_t _read_dreq() { return gpio_read(_instance->DREQ); }

static inline void _reset_high()
{
  if (_instance->set_rst_pin) {
    _instance->set_rst_pin(true);
  }
}

static inline void _reset_low()
{
  if (_instance->set_rst_pin) {
    _instance->set_rst_pin(false);
  }
}

static inline void _set_mute_pin(bool muted)
{
  if (_instance->set_mute_pin) {
    _is_muted = muted;
    _instance->set_mute_pin(muted);
  }
}

static void vs1053b_gpio_init(void)
{
  gpio_init(_instance->XDCS, GPIO_OUT, GPIO_PIN_SPEED_HIGH);
  gpio_init(_instance->DREQ, GPIO_IN, GPIO_PIN_SPEED_HIGH);

  _set_mute_pin(true);

  stm32_spi_init(_instance->spi, LL_SPI_DATAWIDTH_8BIT);
  stm32_spi_set_max_baudrate(_instance->spi, SPI_LOW_SPEED);
}

static uint8_t vs1053b_spi_rw_byte(uint8_t value)
{
  return stm32_spi_transfer_byte(_instance->spi, value);
}

static void vs1053b_spi_transmit_bytes(const uint8_t* data, uint32_t length)
{
  stm32_spi_transfer_bytes(_instance->spi, data, nullptr, length);
}

static void vs1053b_spi_dma_transmit_bytes(const uint8_t* data, uint32_t length)
{
#if __CORTEX_M >= 0x07
  SCB_CleanDCache_by_Addr((void*)data, length);
#endif
  stm32_spi_dma_transmit_bytes(_instance->spi, data, length);
}

static uint8_t vs1053b_wait_dreq(uint32_t timeout_us)
{
  uint32_t start = timersGetUsTick();
  while (READ_DREQ() == 0) {
    if (timersGetUsTick() - start >= timeout_us) return 0;
  }
  return 1;
}

static void _wait_us(uint32_t delay_us)
{
  uint32_t start = timersGetUsTick();
  while (timersGetUsTick() - start < delay_us) {}
}

static void _wait_ms(uint32_t delay_ms)
{
  uint32_t start = timersGetMsTick();
  while (timersGetMsTick() - start < delay_ms) {}
}

static uint8_t vs1053b_write_cmd(uint8_t address, uint16_t data)
{
  stm32_spi_set_max_baudrate(_instance->spi, SPI_LOW_SPEED);
  XDCS_HIGH();

  stm32_spi_select(_instance->spi);
  vs1053b_spi_rw_byte(VS_WRITE_COMMAND);
  vs1053b_spi_rw_byte(address);
  vs1053b_spi_rw_byte(data >> 8);
  vs1053b_spi_rw_byte(data);
  stm32_spi_unselect(_instance->spi);

  // wait for command to be processed
  _wait_us(20);
  vs1053b_wait_dreq(100);

  stm32_spi_set_max_baudrate(_instance->spi, SPI_HIGH_SPEED);
  return 1;
}

static void vs1053b_hard_reset()
{
  XDCS_HIGH();
  stm32_spi_unselect(_instance->spi);

  _reset_low();
  _wait_ms(100);
  _reset_high();

  // datasheet says 1.8ms at default clock speed
  vs1053b_wait_dreq(3000);
}

static uint32_t vs1053b_send_data(const uint8_t * buffer, uint32_t size)
{
  const uint8_t* data = buffer;
  while (size > 0 && READ_DREQ() != 0) {
    uint32_t xfer_size = size > VS1053_BUFFER_SIZE ? VS1053_BUFFER_SIZE : size;
    XDCS_LOW();
    _spi_transmit_data(data, xfer_size);
    XDCS_HIGH();
    size -= xfer_size;
    data += xfer_size;
  }
  return data - buffer;
}

static void vs1053b_send_buffer(const uint8_t * buffer, uint32_t size)
{
  const uint8_t * p = buffer;
  while (size > 0) {
    uint32_t written = vs1053b_send_data(p, size);
    p += written;
    size -= written;
  }
}

const uint8_t RiffHeader[] = {
  'R', 'I', 'F', 'F',
  0xff, 0xff, 0xff, 0xff, // infinite size
  'W', 'A', 'V', 'E',
  'f', 'm', 't', ' ',     // fmt chunk
  0x10, 0x00, 0x00, 0x00, // 16 bytes fmt chunk
  0x01, 0x00,             // PCM
  0x01, 0x00,             // 1 channel
  0x00, 0x7d, 0x00, 0x00, // 32 kHz 
  0x00, 0xfa, 0x00, 0x00, // 64 kB/s
  0x02, 0x00,             // 2 bytes per block
  0x10, 0x00,             // 16 bits per sample
  'd', 'a', 't', 'a',     // data chunk
  0xff, 0xff, 0xff, 0xff, // infinite size
  0x00, 0x00, 0x00, 0x00, // data
  0x00, 0x00, 0x00, 0x00, // data
  0x00, 0x00, 0x00, 0x00, // data
  0x00, 0x00, 0x00, 0x00, // data
  0x00, 0x00, 0x00, 0x00, // data
};

static void vs1053b_send_riff_header()
{
  vs1053b_send_buffer(RiffHeader, sizeof(RiffHeader));
}

static void vs1053b_update_volume()
{
  if (_async_volume >= 0) {
    uint8_t value = _async_volume;
    vs1053b_write_cmd(SPI_VOL, (value << 8) + value);
    _async_volume = -1;
  }
}

static void vs1053b_mute()
{
  if (_is_muted) return;

  if (_instance->mute_delay_ms) {
    uint32_t now = timersGetMsTick();
    if (!_last_play_ts) {
      // we start the mute delay now
      _last_play_ts = now;
      return;
    } else if (now - _last_play_ts < _instance->mute_delay_ms) {
      // delay not expired, we may not mute yet
      return;
    }
  }

  _set_mute_pin(true);
}

static void vs1053b_unmute()
{
  if(isFunctionActive(FUNCTION_DISABLE_AUDIO_AMP)) {
    _set_mute_pin(true);
    return;
  }

  if (_instance->unmute_delay_ms) {
    // if muted
    if (_is_muted) {
      // ..un-mute
      _set_mute_pin(false);
      sleep_ms(_instance->unmute_delay_ms);
    }
    // reset the mute delay
    _last_play_ts = 0;
  } else {
    _set_mute_pin(false);
  }
}

void vs1053b_init(const vs1053b_t* dev)
{
  _instance = dev;

  if (_instance->spi->DMA) {
    _spi_transmit_data = vs1053b_spi_dma_transmit_bytes;
  } else {
    _spi_transmit_data = vs1053b_spi_transmit_bytes;
  }

  vs1053b_gpio_init();
  vs1053b_hard_reset();

  vs1053b_write_cmd(SPI_MODE, SM_SDINEW);

  _async_volume = DEFAULT_VOLUME;
  vs1053b_update_volume();

  vs1053b_write_cmd(SPI_CLOCKF, 0x9800);
  vs1053b_wait_dreq(1000);

  stm32_spi_set_max_baudrate(_instance->spi, SPI_HIGH_SPEED);
  vs1053b_send_riff_header();
}

static void set_volume(uint8_t volume)
{
  _async_volume = volume;
}

static void audioSetCurrentBuffer(const AudioBuffer * buffer)
{
  if (buffer) {
    _audio_buffer = (uint8_t *)buffer->data;
    _audio_buffer_len = buffer->size * 2;
  }
  else {
    _audio_buffer = nullptr;
    _audio_buffer_len = 0;
  }
}

static const uint8_t nullBytes[32] = {0};

void audioConsumeCurrentBuffer()
{
  vs1053b_update_volume();

  if (!_audio_buffer) {
    audioSetCurrentBuffer(audioQueue.buffersFifo.getNextFilledBuffer());
  }

  if (_audio_buffer) {
    vs1053b_unmute();

    uint32_t written = vs1053b_send_data(_audio_buffer, _audio_buffer_len);
    _audio_buffer += written;
    _audio_buffer_len -= written;
    if (_audio_buffer_len == 0) {
      audioQueue.buffersFifo.freeNextFilledBuffer();
      _audio_buffer = nullptr;
      _audio_buffer_len = 0;
    }
  } else {
    if(READ_DREQ()) {
      vs1053b_send_data(nullBytes, sizeof(nullBytes));
    }
    vs1053b_mute();
  }
}

// adjust this value for a volume level just above the silence
// values is attenuation in dB, higher value - less volume
// max value is 126
#define VOLUME_MIN_DB     40

void audioSetVolume(uint8_t volume)
{
  if (volume > VOLUME_LEVEL_MAX) {
    volume = VOLUME_LEVEL_MAX;
  }
  // maximum volume is 0x00 and total silence is 0xFE
  if (volume == 0) {
    set_volume(0xFE);  // silence
  } else {
    uint32_t vol = (VOLUME_MIN_DB * 2) -
                   ((uint32_t)volume * (VOLUME_MIN_DB * 2)) / VOLUME_LEVEL_MAX;
    set_volume(vol);
  }
}
