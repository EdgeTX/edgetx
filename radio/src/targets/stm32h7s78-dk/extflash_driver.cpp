#include "extflash_driver.h"
#include "stm32_xspi_nor.h"


#define XSPI_NOR_PAGE_SIZE 256


static XSPI_HandleTypeDef hxspi_nor;

static const stm32_xspi_nor_t xspi_dev = {
  .flash_size = 128 * 1024 * 1024,
  .page_size = XSPI_NOR_PAGE_SIZE,
  .data_mode = XSPI_OCTO_MODE,
  .data_rate = XSPI_DTR_TRANSFER,
  .hxspi = &hxspi_nor, 
};

/**
 * @brief  Initializes the XSPI MSP.
 * @retval None
 */
static void XSPI_NOR_MspInit()
{
  GPIO_InitTypeDef GPIO_InitStruct;

  /* Enable the SBS Clock */
  __HAL_RCC_SBS_CLK_ENABLE();

  /* Enable the XSPIM_P2 interface */
  HAL_PWREx_EnableXSPIM2();

  /* SBS: Enable HSLV on XSPI2 */
  HAL_SBS_EnableIOSpeedOptimize(SBS_IO_XSPI2_HSLV);

  __HAL_RCC_XSPIM_CLK_ENABLE();
  __HAL_RCC_XSPI2_CLK_ENABLE();

  __HAL_RCC_GPION_CLK_ENABLE();
  /**
   * XSPI2 GPIO Configuration
   * PN1     ------> XSPIM_P2_NCS1
   * PN3     ------> XSPIM_P2_IO1
   * PN0     ------> XSPIM_P2_DQS0
   * PN11    ------> XSPIM_P2_IO7
   * PN10    ------> XSPIM_P2_IO6
   * PN9     ------> XSPIM_P2_IO5
   * PN2     ------> XSPIM_P2_IO0
   * PN6     ------> XSPIM_P2_CLK
   * PN8     ------> XSPIM_P2_IO4
   * PN4     ------> XSPIM_P2_IO2
   * PN5     ------> XSPIM_P2_IO3
   */
  GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_3 | GPIO_PIN_0 | GPIO_PIN_11 |
                        GPIO_PIN_10 | GPIO_PIN_9 | GPIO_PIN_2 | GPIO_PIN_6 |
                        GPIO_PIN_8 | GPIO_PIN_4 | GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF9_XSPIM_P2;
  HAL_GPIO_Init(GPION, &GPIO_InitStruct);
}

/**
 * @brief  Initializes the XSPI interface.
 * @retval BSP status
 */
int32_t ExtFLASH_Init(bool memory_mapped)
{
  // init pins
  XSPI_NOR_MspInit();

  hxspi_nor.Instance = XSPI2;
  hxspi_nor.Init.FifoThresholdByte = 4;
  hxspi_nor.Init.MemoryMode = HAL_XSPI_SINGLE_MEM;
  hxspi_nor.Init.MemoryType = HAL_XSPI_MEMTYPE_MACRONIX;
  hxspi_nor.Init.MemorySize = HAL_XSPI_SIZE_1GB;
  hxspi_nor.Init.ChipSelectHighTimeCycle = 1;
  hxspi_nor.Init.FreeRunningClock = HAL_XSPI_FREERUNCLK_DISABLE;
  hxspi_nor.Init.ClockMode = HAL_XSPI_CLOCK_MODE_0;
  hxspi_nor.Init.WrapSize = HAL_XSPI_WRAP_NOT_SUPPORTED;
  hxspi_nor.Init.ClockPrescaler = 1;
  hxspi_nor.Init.SampleShifting = HAL_XSPI_SAMPLE_SHIFT_NONE;
  hxspi_nor.Init.DelayHoldQuarterCycle = HAL_XSPI_DHQC_ENABLE;
  hxspi_nor.Init.ChipSelectBoundary = HAL_XSPI_BONDARYOF_NONE;
  hxspi_nor.Init.MaxTran = 0;
  hxspi_nor.Init.Refresh = 0;
  hxspi_nor.Init.MemorySelect = HAL_XSPI_CSSEL_NCS1;

  if (stm32_xspi_nor_init(&xspi_dev) != 0) {
    return BSP_ERROR_PERIPH_FAILURE;
  }

  if (!memory_mapped) {
    return BSP_ERROR_NONE;
  }

  if (stm32_xspi_nor_memory_mapped(&xspi_dev) != 0) {
    return BSP_ERROR_PERIPH_FAILURE;
  }

  /* Return BSP status */
  return BSP_ERROR_NONE;
}

//
// ETX HAL flash interface
//

static uint32_t extflash_get_size_kb()
{
  uint32_t size_bytes = 2 << hxspi_nor.Init.MemorySize;
  return size_bytes / 1024;
}

static uint32_t extflash_get_sector(uint32_t address)
{
  address -= XSPI2_BASE;
  return address / (64 * 1024);  
}

static uint32_t extflash_get_sector_size(uint32_t sector)
{
  (void)sector;
  return (64 * 1024);
}

static int extflash_erase_sector(uint32_t address)
{
  address -= XSPI2_BASE;
  if (stm32_xspi_nor_erase_sector(&xspi_dev, address) != BSP_ERROR_NONE) {
    return -1;
  }
  return 0;
}

static int extflash_program(uint32_t address, void* data, uint32_t len)
{
  address -= XSPI2_BASE;
  while (len > 0) {
    uint32_t size = (len > XSPI_NOR_PAGE_SIZE) ? XSPI_NOR_PAGE_SIZE : len;
    if (stm32_xspi_nor_program(&xspi_dev, address, data, size) != 0) {
      return -1;
    }
    len -= size;
    address += size;
    data = (uint8_t*)data + size;
  }

  return 0;
}

static int extflash_read(uint32_t address, void* data, uint32_t len)
{
  address -= XSPI2_BASE;
  if (stm32_xspi_nor_read(&xspi_dev, address, data, len) != 0) {
    return -1;
  }

  return 0;
}

const etx_flash_driver_t extflash_driver = {
  .get_size_kb = extflash_get_size_kb,
  .get_sector = extflash_get_sector,
  .get_sector_size = extflash_get_sector_size,
  .erase_sector = extflash_erase_sector,
  .program = extflash_program,
  .read = extflash_read,
  .unlock = nullptr,
  .lock = nullptr,
};

//
// USB DFU interface
//
#include "usbd_dfu.h"

#define MEDIA_DESC_STR "@External Flash   /0x70000000/2048*064Kg"

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
