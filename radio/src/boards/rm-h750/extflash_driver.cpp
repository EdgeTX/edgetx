#include "extflash_driver.h"

#include "stm32_hal.h"
#include "stm32_qspi.h"

#include "board.h"

#define QSPI_NOR_PAGE_SIZE 256

int32_t ExtFLASH_Init()
{
  if (stm32_qspi_nor_init() != 0) {
    return -1;
  }

  return stm32_qspi_nor_memory_mapped();
}

void ExtFLASH_InitRuntime() { stm32_qspi_no_init_runtime(); }

//
// ETX HAL flash interface
//

static uint32_t extflash_get_size_kb()
{
  return QSPI_FLASH_SIZE / 1024;
}

static uint32_t extflash_get_sector(uint32_t address)
{
  address -= QSPI_BASE;
  return address / (64 * 1024);
}

static uint32_t extflash_get_sector_size(uint32_t sector)
{
  (void)sector;
  return (64 * 1024);
}

static int extflash_erase_sector(uint32_t address)
{
  address -= QSPI_BASE;
  return stm32_qspi_nor_erase_sector(address);
}

static int extflash_program(uint32_t address, void* data, uint32_t len)
{
  address -= QSPI_BASE;

  int ret = 0;
  while (len > 0) {
    uint32_t size = (len > QSPI_NOR_PAGE_SIZE) ? QSPI_NOR_PAGE_SIZE : len;
    ret = stm32_qspi_nor_program(address, data, size);
    if (ret != 0) break;
    len -= size;
    address += size;
    data = (uint8_t*)data + size;
  }

  return ret;
}

static int extflash_read(uint32_t address, void* data, uint32_t len)
{
  address -= QSPI_BASE;
  return stm32_qspi_nor_read(address, data, len);
}

const etx_flash_driver_t extflash_driver = {
  .get_size_kb = extflash_get_size_kb,
  .get_sector = extflash_get_sector,
  .get_sector_size = extflash_get_sector_size,
  .erase_sector = extflash_erase_sector,
  .program = extflash_program,
  .read = extflash_read,
};

#if defined(BOOT)
//
// USB DFU interface
//
#include "usbd_dfu.h"

#define MEDIA_DESC_STR "@External Flash   /0x90000000/128*064Kg"

static uint16_t extflash_dfu_init()
{
  return USBD_OK;
}

static uint16_t extflash_dfu_deinit()
{
  return USBD_OK;
}

static uint16_t extflash_dfu_erase(uint32_t address)
{
  if (extflash_erase_sector(address) < 0) {
    return USBD_FAIL;
  }
  return USBD_OK;
}

static uint16_t extflash_dfu_write(uint8_t* src, uint8_t* dest, uint32_t len)
{
  if (extflash_program((intptr_t)dest, (void*)src, len) < 0) {
    return USBD_FAIL;
  }
  return USBD_OK;
}

static uint8_t* extflash_dfu_read(uint8_t* src, uint8_t* dest, uint32_t len)
{
  if (extflash_read((intptr_t)src, (void*)dest, len) < 0) {
    return nullptr;
  }
  return dest;
}

static uint16_t extflash_dfu_get_status(uint32_t address, uint8_t cmd, uint8_t *buffer)
{
  switch (cmd) {
    case DFU_MEDIA_PROGRAM:
      buffer[1] = 1;
      buffer[2] = 0;
      buffer[3] = 0;
      break;

    case DFU_MEDIA_ERASE:
    default:
      buffer[1] = 20;
      buffer[2] = 0;
      buffer[3] = 0;
      break;
  }

  return USBD_OK;
}

const USBD_DFU_MediaTypeDef _extflash_dfu_media = {
  .pStrDesc = (uint8_t*)MEDIA_DESC_STR,
  .Init = extflash_dfu_init,
  .DeInit = extflash_dfu_deinit,
  .Erase = extflash_dfu_erase,
  .Write = extflash_dfu_write,
  .Read = extflash_dfu_read,
  .GetStatus = extflash_dfu_get_status,
};

const void* extflash_dfu_media = &_extflash_dfu_media;

#endif // BOOT
