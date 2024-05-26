#include "stm32_hal_ll.h"
#include "delays_driver.h"

#include "bsp_errno.h"
#include "stm32_hal.h"


/* MX66UW1G45G size in bytes: 1GBit / 8 */
#define XSPI_NOR_FLASH_SIZE MX66UW1G45G_FLASH_SIZE

/* Flash commands */
#define OCTAL_IO_READ_CMD 0xEC13
#define OCTAL_IO_DTR_READ_CMD 0xEE11
#define OCTAL_PAGE_PROG_CMD 0x12ED
#define OCTAL_READ_STATUS_REG_CMD 0x05FA
#define OCTAL_SECTOR_ERASE_CMD 0x21DE
#define OCTAL_WRITE_ENABLE_CMD 0x06F9
#define READ_STATUS_REG_CMD 0x05
#define WRITE_CFG_REG_2_CMD 0x72
#define WRITE_ENABLE_CMD 0x06

/* Default dummy clocks cycles */
#define DUMMY_CLOCK_CYCLES_READ 20
#define DUMMY_CLOCK_CYCLES_WRITE 0

/* Auto-polling values */
#define WRITE_ENABLE_MATCH_VALUE 0x02
#define WRITE_ENABLE_MASK_VALUE 0x02

#define MEMORY_READY_MATCH_VALUE 0x00
#define MEMORY_READY_MASK_VALUE 0x01

#define AUTO_POLLING_INTERVAL 0x10

/* Memory registers address */
#define CONFIG_REG2_ADDR1 0x0000000
#define CR2_DTR_OPI_ENABLE 0x02

#define CONFIG_REG2_ADDR3 0x00000300
#define CR2_DUMMY_CYCLES_66MHZ 0x07

static XSPI_HandleTypeDef hxspi_nor;

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
 * @brief  This function configure the memory in Octal DTR mode.
 * @param  hxspi: XSPI handle
 * @retval None
 */
static int32_t XSPI_NOR_OctalDTRModeCfg(XSPI_HandleTypeDef *hxspi)
{
  uint8_t reg = 0;
  XSPI_RegularCmdTypeDef sCommand = {0};
  XSPI_AutoPollingTypeDef sConfig = {0};

  sCommand.OperationType = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCommand.InstructionMode = HAL_XSPI_INSTRUCTION_1_LINE;
  sCommand.InstructionWidth = HAL_XSPI_INSTRUCTION_8_BITS;
  sCommand.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCommand.AddressDTRMode = HAL_XSPI_ADDRESS_DTR_DISABLE;
  sCommand.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCommand.DataDTRMode = HAL_XSPI_DATA_DTR_DISABLE;
  sCommand.DummyCycles = 0;
  sCommand.DQSMode = HAL_XSPI_DQS_DISABLE;
  sConfig.MatchMode = HAL_XSPI_MATCH_MODE_AND;
  sConfig.AutomaticStop = HAL_XSPI_AUTOMATIC_STOP_ENABLE;
  sConfig.IntervalTime = 0x10;

  /* Enable write operations */
  sCommand.Instruction = WRITE_ENABLE_CMD;
  sCommand.DataMode = HAL_XSPI_DATA_NONE;
  sCommand.AddressMode = HAL_XSPI_ADDRESS_NONE;

  if (HAL_XSPI_Command(hxspi, &sCommand, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) !=
      HAL_OK) {
    return BSP_ERROR_COMPONENT_FAILURE;
  }

  /* Reconfigure XSPI to automatic polling mode to wait for write enabling */
  sConfig.MatchMask = 0x02;
  sConfig.MatchValue = 0x02;

  sCommand.Instruction = READ_STATUS_REG_CMD;
  sCommand.DataMode = HAL_XSPI_DATA_1_LINE;
  sCommand.DataLength = 1;

  if (HAL_XSPI_Command(hxspi, &sCommand, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) !=
      HAL_OK) {
    return BSP_ERROR_COMPONENT_FAILURE;
  }

  if (HAL_XSPI_AutoPolling(hxspi, &sConfig, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) !=
      HAL_OK) {
    return BSP_ERROR_COMPONENT_FAILURE;
  }

  /* Write Configuration register 2 (with Octal I/O SPI protocol) */
  sCommand.Instruction = WRITE_CFG_REG_2_CMD;
  sCommand.AddressMode = HAL_XSPI_ADDRESS_1_LINE;
  sCommand.AddressWidth = HAL_XSPI_ADDRESS_32_BITS;

  sCommand.Address = 0;
  reg = 0x2;

  if (HAL_XSPI_Command(hxspi, &sCommand, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) !=
      HAL_OK) {
    return BSP_ERROR_COMPONENT_FAILURE;
  }

  if (HAL_XSPI_Transmit(hxspi, &reg, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) !=
      HAL_OK) {
    return BSP_ERROR_COMPONENT_FAILURE;
  }

  sCommand.Instruction = READ_STATUS_REG_CMD;
  sCommand.DataMode = HAL_XSPI_DATA_1_LINE;
  sCommand.DataLength = 1;

  if (HAL_XSPI_Command(hxspi, &sCommand, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) !=
      HAL_OK) {
    return BSP_ERROR_COMPONENT_FAILURE;
  }

  if (HAL_XSPI_AutoPolling(hxspi, &sConfig, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) !=
      HAL_OK) {
    return BSP_ERROR_COMPONENT_FAILURE;
  }

  return BSP_ERROR_NONE;
}

static int32_t XSPI_WriteEnable(XSPI_HandleTypeDef *hxspi)
{
  XSPI_RegularCmdTypeDef sCommand = {0};
  uint8_t reg[2];

  /* Enable write operations ------------------------------------------ */
  sCommand.OperationType = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCommand.Instruction = OCTAL_WRITE_ENABLE_CMD;
  sCommand.InstructionMode = HAL_XSPI_INSTRUCTION_8_LINES;
  sCommand.InstructionWidth = HAL_XSPI_INSTRUCTION_16_BITS;
  sCommand.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
  sCommand.AddressMode = HAL_XSPI_ADDRESS_NONE;
  sCommand.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCommand.DataMode = HAL_XSPI_DATA_NONE;
  sCommand.DummyCycles = 0;
  sCommand.DQSMode = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(hxspi, &sCommand, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) !=
      HAL_OK) {
    return BSP_ERROR_COMPONENT_FAILURE;
  }

  /* Configure automatic polling mode to wait for write enabling ---- */
  sCommand.Instruction = OCTAL_READ_STATUS_REG_CMD;
  sCommand.Address = 0x0;
  sCommand.AddressMode = HAL_XSPI_ADDRESS_8_LINES;
  sCommand.AddressWidth = HAL_XSPI_ADDRESS_32_BITS;
  sCommand.AddressDTRMode = HAL_XSPI_ADDRESS_DTR_ENABLE;
  sCommand.DataMode = HAL_XSPI_DATA_8_LINES;
  sCommand.DataDTRMode = HAL_XSPI_DATA_DTR_ENABLE;
  sCommand.DataLength = 2;
  sCommand.DummyCycles = 6;
  sCommand.DQSMode = HAL_XSPI_DQS_ENABLE;

  do {
    if (HAL_XSPI_Command(hxspi, &sCommand, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) !=
        HAL_OK) {
      return BSP_ERROR_COMPONENT_FAILURE;
    }

    if (HAL_XSPI_Receive(hxspi, reg, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) !=
        HAL_OK) {
      return BSP_ERROR_COMPONENT_FAILURE;
    }
  } while ((reg[0] & 0x2) != 0x2);

  return BSP_ERROR_NONE;
}

static int32_t XSPI_NOR_ConfigureMemoryMappedMode()
{
  XSPI_RegularCmdTypeDef sCommand = {0};
  XSPI_MemoryMappedTypeDef sMemMappedCfg = {0};

  XSPI_NOR_OctalDTRModeCfg(&hxspi_nor);
  XSPI_WriteEnable(&hxspi_nor);

  /*Configure Memory Mapped mode*/

  sCommand.OperationType = HAL_XSPI_OPTYPE_WRITE_CFG;
  sCommand.InstructionMode = HAL_XSPI_INSTRUCTION_8_LINES;
  sCommand.InstructionWidth = HAL_XSPI_INSTRUCTION_16_BITS;
  sCommand.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
  sCommand.Instruction = OCTAL_PAGE_PROG_CMD;
  sCommand.AddressMode = HAL_XSPI_ADDRESS_8_LINES;
  sCommand.AddressWidth = HAL_XSPI_ADDRESS_32_BITS;
  sCommand.AddressDTRMode = HAL_XSPI_ADDRESS_DTR_ENABLE;
  sCommand.Address = 0x0;
  sCommand.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCommand.DataMode = HAL_XSPI_DATA_8_LINES;
  sCommand.DataDTRMode = HAL_XSPI_DATA_DTR_ENABLE;
  sCommand.DataLength = 0;
  sCommand.DummyCycles = DUMMY_CLOCK_CYCLES_WRITE;
  sCommand.DQSMode = HAL_XSPI_DQS_ENABLE;

  if (HAL_XSPI_Command(&hxspi_nor, &sCommand, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) !=
      HAL_OK) {
    return BSP_ERROR_COMPONENT_FAILURE;
  }

  sCommand.OperationType = HAL_XSPI_OPTYPE_READ_CFG;
  sCommand.Instruction = OCTAL_IO_DTR_READ_CMD;
  sCommand.DummyCycles = DUMMY_CLOCK_CYCLES_READ;
  sCommand.DQSMode = HAL_XSPI_DQS_ENABLE;

  if (HAL_XSPI_Command(&hxspi_nor, &sCommand, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) !=
      HAL_OK) {
    return BSP_ERROR_COMPONENT_FAILURE;
  }

  sMemMappedCfg.TimeOutActivation = HAL_XSPI_TIMEOUT_COUNTER_ENABLE;
  sMemMappedCfg.TimeoutPeriodClock = 0xFFFFU;

  if (HAL_XSPI_MemoryMapped(&hxspi_nor, &sMemMappedCfg) != HAL_OK) {
    return BSP_ERROR_PERIPH_FAILURE;
  }

  return BSP_ERROR_NONE;
}

/**
 * @brief  Initializes the XSPI interface.
 * @retval BSP status
 */
int32_t ExtFLASH_Init()
{
  XSPIM_CfgTypeDef sXspiManagerCfg = {0};

  /* Msp XSPI initialization */
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

  if (HAL_XSPI_Init(&hxspi_nor) != HAL_OK) {
    return BSP_ERROR_PERIPH_FAILURE;
  }

  sXspiManagerCfg.nCSOverride = HAL_XSPI_CSSEL_OVR_NCS1;
  sXspiManagerCfg.IOPort = HAL_XSPIM_IOPORT_2;
  if (HAL_XSPIM_Config(&hxspi_nor, &sXspiManagerCfg,
                       HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
    return BSP_ERROR_PERIPH_FAILURE;
  }

  if (XSPI_NOR_ConfigureMemoryMappedMode() != BSP_ERROR_NONE) {
    return BSP_ERROR_PERIPH_FAILURE;
  }

  /* Return BSP status */
  return BSP_ERROR_NONE;
}
