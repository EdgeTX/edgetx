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

#include "stm32_spi.h"
#include "delays_driver.h"

#include "hal.h"

#define CS_HIGH() stm32_spi_unselect(&_flash_spi)
#define CS_LOW()  stm32_spi_select(&_flash_spi)

#define MIN(a,b) (a < b ? a : b)

#define FLASH_SECTOR_SIZE 4096
#define FLASH_SECTOR_MASK (FLASH_SECTOR_SIZE - 1)

#define FLASH_BLOCK_SIZE 32768
#define FLASH_BLOCK_MASK (FLASH_BLOCK_SIZE - 1)

#define FLASH_PAGE_SIZE 256
#define FLASH_PAGE_MASK (FLASH_PAGE_SIZE - 1)

#define FLASH_CMD_READ_ID       0x90
#define FLASH_CMD_READ_JEDEC_ID 0x9f
#define FLASH_CMD_READ_SFDP     0x5a

#define FLASH_CMD_WRITE         0x02
#define FLASH_CMD_READ          0x03

#define FLASH_CMD_STATUS        0x05
#define FLASH_CMD_WRITE_ENABLE  0x06

#define FLASH_CMD_SECTOR_ERASE  0x20
#define FLASH_CMD_BLOCK_ERASE   0x52
#define FLASH_CMD_CHIP_ERASE    0xc7

#define FLASH_CMD_EN4B          0xb7


struct SpiFlashDescriptor {
  uint16_t id;
  uint8_t  log2Size;
  bool     use4BytesAddress;
};

// * RadioMaster/Eachine TX16S, RadioKing TX18S and Jumper T18 use GD25Q127C (16 MByte)
// * FlySky PL18, Paladin EV and NV14 use WinBond W25Q64JV (8 MByte)

// TODO:
// - fallback for flash size discovery (no JEDEC SFDP; see 'pico-bootrom')


const stm32_spi_t _flash_spi = {
  .SPIx = FLASH_SPI,
  .SPI_GPIOx = FLASH_SPI_GPIO,
  .SPI_Pins = FLASH_SPI_SCK_GPIO_PIN | FLASH_SPI_MISO_GPIO_PIN | FLASH_SPI_MOSI_GPIO_PIN,
  .CS_GPIOx = FLASH_SPI_CS_GPIO,
  .CS_Pin = FLASH_SPI_CS_GPIO_PIN,
  .DMA = FLASH_SPI_DMA,
  .DMA_Channel = FLASH_SPI_DMA_CHANNEL,
  .txDMA_Stream = FLASH_SPI_DMA_TX_STREAM,
  .rxDMA_Stream = FLASH_SPI_DMA_RX_STREAM,
  .DMA_FIFOMode = LL_DMA_FIFOMODE_ENABLE,
  .DMA_FIFOThreshold = LL_DMA_FIFOTHRESHOLD_1_2,
  .DMA_MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_HALFWORD,
  .DMA_MemBurst = LL_DMA_MBURST_INC4,
};

static SpiFlashDescriptor _flashDescriptor;

static inline void flash_select() { stm32_spi_select(&_flash_spi); }
static inline void flash_unselect() { stm32_spi_unselect(&_flash_spi); }

static inline uint8_t flash_rw_byte(uint8_t value)
{
  return stm32_spi_transfer_byte(&_flash_spi, value);
}

static inline uint8_t flash_read_byte()
{
  return flash_rw_byte(0xFF);
}

static inline void flash_rw_bytes(const uint8_t* tx, uint8_t* rx, uint16_t count)
{
  stm32_spi_transfer_bytes(&_flash_spi, tx, rx, count);
}

static inline void flash_dma_read_bytes(uint8_t* buffer, uint16_t count)
{
  stm32_spi_dma_receive_bytes(&_flash_spi, buffer, count);
}

static inline void flash_dma_write_bytes(const uint8_t* buffer, uint16_t count)
{
  stm32_spi_dma_transmit_bytes(&_flash_spi, buffer, count);
}

static inline void flash_put_cmd_addr(uint8_t cmd, uint32_t addr)
{
  addr |= cmd << 24;
  for (int i = 0; i < 4; i++) {
    flash_rw_byte(addr >> 24);
    addr <<= 8;
  }
}

static inline void flash_put_cmd_4b_addr(uint8_t cmd, uint32_t addr)
{
  flash_rw_byte(cmd);

  for (int i = 0; i < 4; i++) {
    flash_rw_byte(addr >> 24);
    addr <<= 8;
  }
}

static inline void flash_do_cmd(uint8_t cmd, const uint8_t* tx, uint8_t* rx, uint16_t count)
{
  flash_select();
  flash_rw_byte(cmd);
  flash_rw_bytes(tx, rx, count);
  flash_unselect();
}

static void flash_wait_for_not_busy()
{
  uint8_t status;
  do {
    flash_do_cmd(FLASH_CMD_STATUS, 0, &status, 1);
  } while (status & 0x01);
}

static void flash_enable_write()
{
  flash_do_cmd(FLASH_CMD_WRITE_ENABLE, 0, 0, 0);
}

static void flash_read_sfdp(uint32_t addr, uint8_t *rx, uint16_t count)
{
  flash_select();
  flash_put_cmd_addr(FLASH_CMD_READ_SFDP, addr);
  flash_read_byte(); // dummy byte
  flash_rw_bytes(0, rx, count);
  flash_unselect();
}

static inline uint32_t bytes_to_u32le(const uint8_t *b) {
  return b[0] | (b[1] << 8) | (b[2] << 16) | (b[3] << 24);
}

static bool flash_read_id(SpiFlashDescriptor* desc)
{
  uint8_t rxbuf[2];

  flash_select();
  flash_put_cmd_addr(FLASH_CMD_READ_ID, 0);
  flash_rw_bytes(0, rxbuf, 2);
  flash_unselect();

  uint16_t id = (rxbuf[0] << 8) | rxbuf[1];
  if (id == 0xFFFF) {
    return false;
  }

  desc->id = id;  
  return true;
}

static bool flash_read_sfdp(SpiFlashDescriptor* desc)
{
  uint8_t rxbuf[16];

  // check magic signature
  flash_read_sfdp(0, rxbuf, 16);
  if (rxbuf[0] != 'S' || rxbuf[1] != 'F' || rxbuf[2] != 'D' || rxbuf[3] != 'P') {
    return false;
  }

  // check JEDEC ID is 0
  if (rxbuf[8] != 0) {
    return false;
  }

  // read param table
  uint32_t param_table_ptr = bytes_to_u32le(rxbuf + 12) & 0xffffffu;
  flash_read_sfdp(param_table_ptr, rxbuf, 8);

  // 1st DWORD
  uint32_t param_table_dword = bytes_to_u32le(rxbuf);

  if ((param_table_dword & 3) != 1) {
    // 4 kilobyte erase is unavailable
    return false;
  }

  if (((param_table_dword >> 17) & 3) != 0) {
    // 4-byte addressing supported
    desc->use4BytesAddress = true;
    flash_do_cmd(FLASH_CMD_EN4B, 0, 0, 0);
  }

  // 2nd DWORD: flash memory density
  // - MSB set: array >= 2 Gbit, encoded as log2 of number of bits
  // - MSB clear: array < 2 Gbit, encoded as direct bit count
  
  param_table_dword = bytes_to_u32le(rxbuf + 4);
  if (param_table_dword & (1u << 31)) {
    param_table_dword &= ~(1u << 31);
  } else {
    uint32_t ctr = 0;
    param_table_dword += 1;
    while (param_table_dword >>= 1)
      ++ctr;
    param_table_dword = ctr;
  }
  desc->log2Size = param_table_dword - 3;
  
  return true;
}

void flashSpiSync()
{
  uint8_t status;
  do {
    flash_do_cmd(FLASH_CMD_STATUS, 0, &status, 1);
  } while (status & 0x01);
}

bool flashSpiInit(void)
{
  stm32_spi_init(&_flash_spi, LL_SPI_DATAWIDTH_8BIT);
  delay_ms(1);
  flashSpiSync();

  if (!flash_read_id(&_flashDescriptor)) {
    return false;
  }

  if (!flash_read_sfdp(&_flashDescriptor)) {
    return false;
  }

  return true;  
}

uint32_t flashSpiGetSize()
{
  return (1UL << _flashDescriptor.log2Size);
}

bool flashSpiIsErased(uint32_t address)
{
  // verify sector alignment
  if((address & FLASH_SECTOR_MASK) != 0)
    return false;

  flash_wait_for_not_busy();
  flash_select();

  if (_flashDescriptor.use4BytesAddress) {
    flash_put_cmd_4b_addr(FLASH_CMD_READ, address);
  } else {
    flash_put_cmd_addr(FLASH_CMD_READ, address);
  }

  bool ret = true;
  for(uint32_t i = 0; i < FLASH_SECTOR_SIZE; i++) {
    uint8_t byte = flash_rw_byte(0xFF);
    if (byte != 0xff) {
      ret = false;
      break;
    }
  }

  flash_unselect();
  return ret;
}

uint32_t flashSpiRead(uint32_t address, uint8_t* data, uint32_t size)
{
  size = MIN(size, (uint32_t)(flashSpiGetSize() - address));

  flash_wait_for_not_busy();
  flash_select();

  if (_flashDescriptor.use4BytesAddress) {
    flash_put_cmd_4b_addr(FLASH_CMD_READ, address);
  } else {
    flash_put_cmd_addr(FLASH_CMD_READ, address);
  }
  flash_dma_read_bytes(data, size);

  flash_unselect();
  return size;
}

uint32_t flashSpiWrite(uint32_t address, const uint8_t* data, uint32_t size)
{
  size = MIN(size, (uint32_t)(flashSpiGetSize() - address));

  // verify page alignment and size
  if (size != FLASH_PAGE_SIZE || (address & FLASH_PAGE_MASK) != 0) {
    return 0;
  }

  flash_wait_for_not_busy();
  flash_enable_write();

  flash_select();
  if (_flashDescriptor.use4BytesAddress) {
    flash_put_cmd_4b_addr(FLASH_CMD_WRITE, address);
  } else {
    flash_put_cmd_addr(FLASH_CMD_WRITE, address);
  }
  flash_dma_write_bytes(data, size);
  flash_unselect();
  
  flash_wait_for_not_busy();
  return size;
}

int flashSpiErase(uint32_t address)
{
  if((address & FLASH_SECTOR_MASK) != 0)
    return -1;

  flash_wait_for_not_busy();
  flash_enable_write();

  flash_select();
  if (_flashDescriptor.use4BytesAddress) {
    flash_put_cmd_4b_addr(FLASH_CMD_SECTOR_ERASE, address);
  } else {
    flash_put_cmd_addr(FLASH_CMD_SECTOR_ERASE, address);
  }
  flash_unselect();

  flash_wait_for_not_busy();
  return 0;
}

int flashSpiBlockErase(uint32_t address)
{
  // verify block alignment
  if((address & FLASH_BLOCK_MASK) != 0)
    return -1;

  flash_wait_for_not_busy();
  flash_enable_write();
  
  flash_select();
  if (_flashDescriptor.use4BytesAddress) {
    flash_put_cmd_4b_addr(FLASH_CMD_BLOCK_ERASE, address);
  } else {
    flash_put_cmd_addr(FLASH_CMD_BLOCK_ERASE, address);
  }
  flash_unselect();

  flash_wait_for_not_busy();
  return 0;
}

void flashSpiEraseAll()
{
  flash_wait_for_not_busy();
  flash_enable_write();
  flash_do_cmd(FLASH_CMD_CHIP_ERASE, 0, 0, 0);
  flash_wait_for_not_busy();
}

uint16_t flashSpiGetPageSize()
{
  return FLASH_PAGE_SIZE;
}
