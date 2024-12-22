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

#include "stm32_hal_ll.h"
#include "stm32_gpio_driver.h"
#include "stm32_gpio.h"
#include "stm32_spi.h"
//#include "bsp_io.h"
#include "hal/gpio.h"
#include "board.h"
#include "edgetx.h"

#if !defined(SIMU)

#define VS_WRITE_COMMAND 	           0x02
#define VS_READ_COMMAND 	           0x03

#define SPI_MODE        	           0x00
#define SPI_STATUS      	           0x01
#define SPI_BASS        	           0x02
#define SPI_CLOCKF      	           0x03
#define SPI_DECODE_TIME 	           0x04
#define SPI_AUDATA      	           0x05
#define SPI_WRAM        	           0x06
#define SPI_WRAMADDR    	           0x07
#define SPI_HDAT0       	           0x08
#define SPI_HDAT1       	           0x09
#define SPI_AIADDR      	           0x0a
#define SPI_VOL         	           0x0b
#define SPI_AICTRL0     	           0x0c
#define SPI_AICTRL1     	           0x0d
#define SPI_AICTRL2     	           0x0e
#define SPI_AICTRL3     	           0x0f

#define SM_DIFF         	           0x01
#define SM_LAYER12         	           0x02
#define SM_RESET        	           0x04
#define SM_CANCEL       	           0x08
#define SM_EARSPEAKER_LO  	           0x10
#define SM_TESTS        	           0x20
#define SM_STREAM       	           0x40
#define SM_EARSPEAKER_HI   	           0x80
#define SM_DACT         	           0x100
#define SM_SDIORD       	           0x200
#define SM_SDISHARE     	           0x400
#define SM_SDINEW       	           0x800
#define SM_ADPCM        	           0x1000
#define SM_LINE1         	           0x4000
#define SM_CLK_RANGE     	           0x8000

#define SPI_LOW_SPEED                  1755000
#define SPI_HIGH_SPEED                 9216000

#define MP3_BUFFER_SIZE                32

#define XDCS_HIGH()  gpio_set(AUDIO_XDCS_GPIO)
#define XDCS_LOW()   gpio_clear(AUDIO_XDCS_GPIO)

#define READ_DREQ()  gpio_read(AUDIO_DREQ_GPIO)

const AudioConfig_t* audioCfg = nullptr;

static const stm32_spi_t _audio_spi = {
  .SPIx = AUDIO_SPI,
  .SCK = AUDIO_SPI_SCK_GPIO,
  .MISO = AUDIO_SPI_MISO_GPIO,
  .MOSI = AUDIO_SPI_MOSI_GPIO,
  .CS = AUDIO_CS_GPIO,
};

static void audioSpiInit(void)
{
  gpio_init(AUDIO_XDCS_GPIO, GPIO_OUT, GPIO_PIN_SPEED_HIGH);
  gpio_init(AUDIO_DREQ_GPIO, GPIO_IN, GPIO_PIN_SPEED_HIGH);
//#ifdef(AUDIO_RST_GPIO)
//  stm32_gpio_enable_clock(AUDIO_RST_GPIO);
//#endif
  
//#ifdef(AUDIO_RST_GPIO)
//  pinInit.Pin = AUDIO_RST_GPIO_PIN;READ_DREQ
//  LL_GPIO_Init(AUDIO_RST_GPIO, &pinInit);
//#endif

  stm32_spi_init(audioCfg->spi, LL_SPI_DATAWIDTH_8BIT);
}

void audioWaitReady()
{
  // The audio amp needs ~2s to start
  RTOS_WAIT_MS(2000); // 2s
}

static uint8_t audioSpiReadWriteByte(uint8_t value)
{
  return stm32_spi_transfer_byte(audioCfg->spi, value);
}

static uint8_t audioWaitDreq(int32_t delay_us)
{
  while (READ_DREQ() == 0) {
    if (delay_us-- == 0) return 0;
    delay_01us(10);
  }
  return 1;
}

static uint16_t audioSpiReadReg(uint8_t address)
{
  if (!audioWaitDreq(100))
    return 0;

  stm32_spi_set_max_baudrate(audioCfg->spi, SPI_LOW_SPEED);
  XDCS_HIGH();
  stm32_spi_select(audioCfg->spi);
  audioSpiReadWriteByte(VS_READ_COMMAND);
  audioSpiReadWriteByte(address);
  volatile uint16_t result = audioSpiReadWriteByte(0xff) << 8;
  result += audioSpiReadWriteByte(0xff);
  delay_01us(100); // 10us
  stm32_spi_unselect(audioCfg->spi);

  stm32_spi_set_max_baudrate(audioCfg->spi, SPI_HIGH_SPEED);
  return result;
}

static uint16_t audioSpiReadCmd(uint8_t address)
{
  if (!audioWaitDreq(100))
    return 0;

  stm32_spi_set_max_baudrate(audioCfg->spi, SPI_LOW_SPEED);
  XDCS_HIGH();
  stm32_spi_select(audioCfg->spi);
  audioSpiReadWriteByte(VS_READ_COMMAND);
  audioSpiReadWriteByte(address);
  uint16_t result = audioSpiReadWriteByte(0) << 8;
  result |= audioSpiReadWriteByte(0);
  delay_01us(50); // 5us
  stm32_spi_unselect(audioCfg->spi);
  stm32_spi_set_max_baudrate(audioCfg->spi, SPI_HIGH_SPEED);
  return result;
}

static uint8_t audioSpiWriteCmd(uint8_t address, uint16_t data)
{
  if (!audioWaitDreq(100))
    return 0;

  stm32_spi_set_max_baudrate(audioCfg->spi, SPI_LOW_SPEED);
  XDCS_HIGH();
  stm32_spi_select(audioCfg->spi);
  audioSpiReadWriteByte(VS_WRITE_COMMAND);
  audioSpiReadWriteByte(address);
  audioSpiReadWriteByte(data >> 8);
  audioSpiReadWriteByte(data);
  delay_01us(50); // 5us
  stm32_spi_unselect(audioCfg->spi);
  stm32_spi_set_max_baudrate(audioCfg->spi, SPI_HIGH_SPEED);
  return 1;
}

static void audioResetDecodeTime()
{
  audioSpiWriteCmd(SPI_DECODE_TIME, 0x0000);
  audioSpiWriteCmd(SPI_DECODE_TIME, 0x0000);
}

static uint8_t audioHardReset()
{
  XDCS_HIGH();
  stm32_spi_unselect(audioCfg->spi);
  audioCfg->setResetLow();
  delay_ms(100); // 100ms
  audioCfg->setResetHigh();

  if (!audioWaitDreq(5000))
    return 0;

  delay_ms(20); // 20ms
  return 1;
}

static uint8_t audioSoftReset()
{
  stm32_spi_set_max_baudrate(audioCfg->spi, SPI_LOW_SPEED);
  if (!audioWaitDreq(100))
    return 0;

  audioSpiReadWriteByte(0x00); // start the transfer

  uint8_t retry = 0;
  uint16_t mode = SM_SDINEW;
  while (audioSpiReadReg(SPI_MODE) != mode && retry < 100) {
    retry++;
    audioSpiWriteCmd(SPI_MODE, mode | SM_RESET);
    delay_ms(2);
  }

  // wait for set up successful
  retry = 0;
  while (audioSpiReadReg(SPI_CLOCKF) != 0x9800 && retry < 100) {
    retry++;
    audioSpiWriteCmd(SPI_CLOCKF, 0x9800);
  }

  audioResetDecodeTime(); // reset the decoding time
  stm32_spi_set_max_baudrate(audioCfg->spi, SPI_HIGH_SPEED);
  XDCS_LOW();
  audioSpiReadWriteByte(0X0);
  audioSpiReadWriteByte(0X0);
  audioSpiReadWriteByte(0X0);
  audioSpiReadWriteByte(0X0);
  delay_01us(100); // 10us
  XDCS_HIGH();
  return 1;
}

static uint32_t audioSpiWriteData(const uint8_t * buffer, uint32_t size)
{
  XDCS_LOW();

  uint32_t index = 0;
  while (index < size && READ_DREQ() != 0) {
    for (int i=0; i<MP3_BUFFER_SIZE && index<size; i++) {
      audioSpiReadWriteByte(buffer[index++]);
    }
  }
  return index;
}

static void audioSpiWriteBuffer(const uint8_t * buffer, uint32_t size)
{
  const uint8_t * p = buffer;
  while (size > 0) {
    uint32_t written = audioSpiWriteData(p, size);
    p += written;
    size -= written;
  }
}

const uint8_t RiffHeader[] = {
  0x52, 0x49, 0x46, 0x46, 0xff, 0xff, 0xff, 0xff, 0x57, 0x41, 0x56, 0x45, 0x66, 0x6d, 0x74, 0x20,
  0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x7d, 0x00, 0x00, 0x00, 0xfa, 0x00, 0x00,
  0x02, 0x00, 0x10, 0x00, 0x64, 0x61, 0x74, 0x61, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

void audioSendRiffHeader()
{
  audioSpiWriteBuffer(RiffHeader, sizeof(RiffHeader));
}

#if defined(AUDIO_MUTE_GPIO)
static bool isMuted = true;
static inline void setMutePin(bool enabled)
{
  isMuted = enabled;
#if defined(INVERTED_MUTE_PIN)
  enabled = !enabled;
#endif
  if(enabled)
    audioCfg->setMuteHigh();
  else
    audioCfg->setMuteLow();
}

void audioMute()
{
  if(isMuted)
    return;
#if defined(AUDIO_UNMUTE_DELAY)
  tmr10ms_t now = get_tmr10ms();
  if (!audioQueue.lastAudioPlayTime) {
    // we start the mute delay now
    audioQueue.lastAudioPlayTime = now;
  } else if (now - audioQueue.lastAudioPlayTime > AUDIO_MUTE_DELAY / 10) {
    // delay expired, we may mute
    setMutePin(true);
  }
#else
  // mute
  setMutePin(true);
#endif
}

void audioUnmute()
{
  if(isFunctionActive(FUNCTION_DISABLE_AUDIO_AMP)) {
    setMutePin(true);
    return;
  }

#if defined(AUDIO_UNMUTE_DELAY)
  // if muted
  if (isMuted) {
    // ..un-mute
    setMutePin(false);
    RTOS_WAIT_MS(AUDIO_UNMUTE_DELAY);
  }
  // reset the mute delay
  audioQueue.lastAudioPlayTime = 0;
#else
  setMutePin(false);
#endif
}
#endif

#if defined(PCBX12S)
void audioShutdownInit()
{
  gpio_init(AUDIO_SHUTDOWN_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);

  // we never RESET it, there is a 2s delay on STARTUP
  gpio_set(AUDIO_SHUTDOWN_GPIO);
}
#endif

void audioInit(const AudioConfig_t* cfg)
{
  audioCfg = cfg;
#if defined(AUDIO_MUTE_GPIO)
  // Mute before init anything
  setMutePin(true);
#endif

#if defined(PCBX12S)
  audioShutdownInit();
#endif

  audioSpiInit();
  audioHardReset();
  audioSoftReset();
  stm32_spi_set_max_baudrate(audioCfg->spi, SPI_HIGH_SPEED);
  delay_ms(1); // 1ms
  audioSendRiffHeader();
}

uint8_t * currentBuffer = nullptr;
uint32_t currentSize = 0;
int16_t newVolume = -1;

void audioSetCurrentBuffer(const AudioBuffer * buffer)
{
  if (buffer) {
    currentBuffer = (uint8_t *)buffer->data;
    currentSize = buffer->size * 2;
  }
  else {
    currentBuffer = nullptr;
    currentSize = 0;
  }
}

static const uint8_t nullBytes[32] = {0};

void audioConsumeCurrentBuffer()
{
  if (newVolume >= 0) {
    uint8_t value = newVolume;
    audioSpiWriteCmd(SPI_VOL, (value << 8) + value);
    // audioSendRiffHeader();
    newVolume = -1;
  }

  if (!currentBuffer) {
    audioSetCurrentBuffer(audioQueue.buffersFifo.getNextFilledBuffer());
  }

  if (currentBuffer) {
#if defined(AUDIO_MUTE_GPIO)
    audioUnmute();
#endif
    uint32_t written = audioSpiWriteData(currentBuffer, currentSize);
    currentBuffer += written;
    currentSize -= written;
    if (currentSize == 0) {
      audioQueue.buffersFifo.freeNextFilledBuffer();
      currentBuffer = nullptr;
      currentSize = 0;
    }
  }
#if defined(AUDIO_MUTE_GPIO)
  else {
    if(gpio_read(AUDIO_DREQ_GPIO) != 0)
      audioSpiWriteData(nullBytes, sizeof(nullBytes));
    audioMute();
  }
#endif
}

// adjust this value for a volume level just above the silence
// values is attenuation in dB, higher value - less volume
// max value is 126
#define VOLUME_MIN_DB     40

void setScaledVolume(uint8_t volume)
{
  if (volume > VOLUME_LEVEL_MAX) {
    volume = VOLUME_LEVEL_MAX;
  }
  // maximum volume is 0x00 and total silence is 0xFE
  if (volume == 0) {
    setVolume(0xFE);  // silence  
  }
  else {
    uint32_t vol = (VOLUME_MIN_DB * 2) - ((uint32_t)volume * (VOLUME_MIN_DB * 2)) / VOLUME_LEVEL_MAX;
    setVolume(vol);
  }
}

void setVolume(uint8_t volume)
{
  newVolume = volume;
}

int32_t getVolume()
{
  return -1; // TODO
}

#endif
