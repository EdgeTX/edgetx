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

#include "hal/gpio.h"

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

#define SPI_SPEED_2                    0
#define SPI_SPEED_4                    1
#define SPI_SPEED_8                    2
#define SPI_SPEED_16                   3
#define SPI_SPEED_32                   4
#define SPI_SPEED_64                   5
#define SPI_SPEED_128                  6
#define SPI_SPEED_256                  7

#define MP3_BUFFER_SIZE                32

#define CS_HIGH()    stm32_spi_unselect(&_audio_spi)
#define CS_LOW()     stm32_spi_select(&_audio_spi)
#define XDCS_HIGH()  do { AUDIO_XDCS_GPIO->BSRR = AUDIO_XDCS_GPIO_PIN; } while (0)
#define XDCS_LOW()   do { AUDIO_XDCS_GPIO->BSRR = AUDIO_XDCS_GPIO_PIN << 16; } while (0)
#define RST_HIGH()   do { AUDIO_RST_GPIO->BSRR = AUDIO_RST_GPIO_PIN; } while (0)
#define RST_LOW()    do { AUDIO_RST_GPIO->BSRR = AUDIO_RST_GPIO_PIN << 16; } while (0)

#define READ_DREQ()  (LL_GPIO_IsInputPinSet(AUDIO_DREQ_GPIO, AUDIO_DREQ_GPIO_PIN))

static const stm32_spi_t _audio_spi = {
  .SPIx = AUDIO_SPI,
  .SPI_GPIOx = AUDIO_SPI_SCK_GPIO,
  .SPI_Pins = AUDIO_SPI_SCK_GPIO_PIN | AUDIO_SPI_MISO_GPIO_PIN | AUDIO_SPI_MOSI_GPIO_PIN,
  .CS_GPIOx = AUDIO_CS_GPIO,
  .CS_Pin = AUDIO_CS_GPIO_PIN,
};

void audioSpiInit(void)
{
  stm32_gpio_enable_clock(AUDIO_XDCS_GPIO);
  stm32_gpio_enable_clock(AUDIO_RST_GPIO);
  stm32_gpio_enable_clock(AUDIO_DREQ_GPIO);
  
  LL_GPIO_InitTypeDef pinInit;
  LL_GPIO_StructInit(&pinInit);

  pinInit.Mode = LL_GPIO_MODE_OUTPUT;
  pinInit.Pin = AUDIO_XDCS_GPIO_PIN;
  LL_GPIO_Init(AUDIO_XDCS_GPIO, &pinInit);

  pinInit.Pin = AUDIO_RST_GPIO_PIN;
  LL_GPIO_Init(AUDIO_RST_GPIO, &pinInit);

  pinInit.Pin = AUDIO_DREQ_GPIO_PIN;
  pinInit.Mode = LL_GPIO_MODE_INPUT;
  LL_GPIO_Init(AUDIO_DREQ_GPIO, &pinInit);

  stm32_spi_init(&_audio_spi, LL_SPI_DATAWIDTH_8BIT);
}

void audioWaitReady()
{
  // The audio amp needs ~2s to start
  RTOS_WAIT_MS(2000); // 2s
}

void audioSpiSetSpeed(uint8_t speed)
{
  AUDIO_SPI->CR1 &= 0xFFC7; // Fsck=Fcpu/256
  switch (speed) {
    case SPI_SPEED_2:
      AUDIO_SPI->CR1 |= 0x00 << 3; // Fsck=Fpclk/2=36Mhz
      break;
    case SPI_SPEED_4:
      AUDIO_SPI->CR1 |= 0x01 << 3; // Fsck=Fpclk/4=18Mhz
      break;
    case SPI_SPEED_8:
      AUDIO_SPI->CR1 |= 0x02 << 3; // Fsck=Fpclk/8=9Mhz
      break;
    case SPI_SPEED_16:
      AUDIO_SPI->CR1 |= 0x03 << 3; // Fsck=Fpclk/16=4.5Mhz
      break;
    case SPI_SPEED_32:
      AUDIO_SPI->CR1 |= 0x04 << 3; // Fsck=Fpclk/32=2.25Mhz
      break;
    case SPI_SPEED_64:
      AUDIO_SPI->CR1 |= 0x05 << 3; // Fsck=Fpclk/16=1.125Mhz
      break;
    case SPI_SPEED_128:
      AUDIO_SPI->CR1 |= 0x06 << 3; // Fsck=Fpclk/16=562.5Khz
      break;
    case SPI_SPEED_256:
      AUDIO_SPI->CR1 |= 0x07 << 3; // Fsck=Fpclk/16=281.25Khz
      break;
  }
  AUDIO_SPI->CR1 |= 0x01 << 6;
}

uint8_t audioSpiReadWriteByte(uint8_t value)
{
  return stm32_spi_transfer_byte(&_audio_spi, value);
}

uint8_t audioWaitDreq(int32_t delay_us)
{
  while (READ_DREQ() == 0) {
    if (delay_us-- == 0) return 0;
    delay_01us(10);
  }
  return 1;
}

uint16_t audioSpiReadReg(uint8_t address)
{
  if (!audioWaitDreq(100))
    return 0;

  audioSpiSetSpeed(SPI_SPEED_64);
  XDCS_HIGH();
  CS_LOW();
  audioSpiReadWriteByte(VS_READ_COMMAND);
  audioSpiReadWriteByte(address);
  uint16_t result = audioSpiReadWriteByte(0xff) << 8;
  result += audioSpiReadWriteByte(0xff);
  delay_01us(100); // 10us
  CS_HIGH();
  audioSpiSetSpeed(SPI_SPEED_8);
  return result;
}

uint16_t audioSpiReadCmd(uint8_t address)
{
  if (!audioWaitDreq(100))
    return 0;

  audioSpiSetSpeed(SPI_SPEED_64);
  XDCS_HIGH();
  CS_LOW();
  audioSpiReadWriteByte(VS_READ_COMMAND);
  audioSpiReadWriteByte(address);
  uint16_t result = audioSpiReadWriteByte(0) << 8;
  result |= audioSpiReadWriteByte(0);
  delay_01us(50); // 5us
  CS_HIGH();
  audioSpiSetSpeed(SPI_SPEED_8);
  return result;
}

uint8_t audioSpiWriteCmd(uint8_t address, uint16_t data)
{
  if (!audioWaitDreq(100))
    return 0;

  audioSpiSetSpeed(SPI_SPEED_64);
  XDCS_HIGH();
  CS_LOW();
  audioSpiReadWriteByte(VS_WRITE_COMMAND);
  audioSpiReadWriteByte(address);
  audioSpiReadWriteByte(data >> 8);
  audioSpiReadWriteByte(data);
  delay_01us(50); // 5us
  CS_HIGH();
  audioSpiSetSpeed(SPI_SPEED_8);
  return 1;
}

void audioResetDecodeTime(void)
{
  audioSpiWriteCmd(SPI_DECODE_TIME, 0x0000);
  audioSpiWriteCmd(SPI_DECODE_TIME, 0x0000);
}

uint8_t audioHardReset(void)
{
  XDCS_HIGH();
  CS_HIGH();
  RST_LOW();
  delay_ms(100); // 100ms
  RST_HIGH();

  if (!audioWaitDreq(5000))
    return 0;

  delay_ms(20); // 20ms
  return 1;
}

uint8_t audioSoftReset(void)
{
  audioSpiSetSpeed(SPI_SPEED_64);
  if (!audioWaitDreq(100))
    return 0;

  audioSpiReadWriteByte(0x00); // start the transfer

  uint8_t retry = 0;
  uint16_t mode = SM_SDINEW;
  while (audioSpiReadReg(SPI_MODE) != mode && retry < 100) {
    retry++;
    audioSpiWriteCmd(SPI_MODE, mode | SM_RESET);
  }

  // wait for set up successful
  retry = 0;
  while (audioSpiReadReg(SPI_CLOCKF) != 0x9800 && retry < 100) {
    retry++;
    audioSpiWriteCmd(SPI_CLOCKF, 0x9800);
  }

  audioResetDecodeTime(); // reset the decoding time
  audioSpiSetSpeed(SPI_SPEED_8);
  XDCS_LOW();
  audioSpiReadWriteByte(0X0);
  audioSpiReadWriteByte(0X0);
  audioSpiReadWriteByte(0X0);
  audioSpiReadWriteByte(0X0);
  delay_01us(100); // 10us
  XDCS_HIGH();
  return 1;
}

uint32_t audioSpiWriteData(const uint8_t * buffer, uint32_t size)
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

void audioSpiWriteBuffer(const uint8_t * buffer, uint32_t size)
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
static inline void setMutePin(bool enabled)
{
#if defined(INVERTED_MUTE_PIN)
  enabled = !enabled;
#endif
  gpio_write(AUDIO_MUTE_GPIO, enabled);
}

static inline bool getMutePin(void)
{
  bool muted = gpio_read(AUDIO_MUTE_GPIO) ? 1 : 0;
#if defined(INVERTED_MUTE_PIN)
  muted = !muted;
#endif
  return muted;
}

void audioMute()
{
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
  if (getMutePin()) {
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

void audioInit()
{
#if defined(AUDIO_MUTE_GPIO)
  // Mute before init anything
  gpio_init(AUDIO_MUTE_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  setMutePin(true);
#endif

#if defined(PCBX12S)
  audioShutdownInit();
#endif

  audioSpiInit();
  audioHardReset();
  audioSoftReset();
  audioSpiSetSpeed(SPI_SPEED_8);
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
