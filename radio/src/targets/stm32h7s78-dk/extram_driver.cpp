#include "extram_driver.h"
#include "stm32_hal_ll.h"
#include "delays_driver.h"

#include "stm32_hal.h"

  /* Aps256xx APMemory memory */
  
  /* Read Operations */
#define READ_CMD                                0x00
#define READ_LINEAR_BURST_CMD                   0x20
#define READ_HYBRID_BURST_CMD                   0x3F
  
  /* Write Operations */
#define WRITE_CMD                               0x80
#define WRITE_LINEAR_BURST_CMD                  0xA0
#define WRITE_HYBRID_BURST_CMD                  0xBF
  
  /* Reset Operations */
#define RESET_CMD                               0xFF

/* Registers definition */
#define MR0 0x00000000
#define MR1 0x00000001
#define MR2 0x00000002
#define MR3 0x00000003
#define MR4 0x00000004
#define MR8 0x00000008

/* Register Operations */
#define READ_REG_CMD                            0x40
#define WRITE_REG_CMD                           0xC0
  
/* Default dummy clocks cycles */
#define DUMMY_CLOCK_CYCLES_READ                 6
#define DUMMY_CLOCK_CYCLES_WRITE                6
  
/* Size of buffers */
#define BUFFERSIZE                              10240
#define KByte                                   1024


static XSPI_HandleTypeDef hxspi_ram;

/**
 * @brief  Initializes the XSPI MSP.
 * @param  hxspi XSPI handle
 * @retval None
 */
static void XSPI_RAM_MspInit()
{
  GPIO_InitTypeDef GPIO_InitStruct;

  /* Enable the SBS Clock */
  __HAL_RCC_SBS_CLK_ENABLE();

  /* Enable the XSPIM_P1 interface */  
  HAL_PWREx_EnableXSPIM1();

  /* SBS: Enable HSLV on XSPI1 */
  HAL_SBS_EnableIOSpeedOptimize(SBS_IO_XSPI1_HSLV);

  __HAL_RCC_XSPIM_CLK_ENABLE();
  __HAL_RCC_XSPI1_CLK_ENABLE();

  __HAL_RCC_GPIOO_CLK_ENABLE();
  __HAL_RCC_GPIOP_CLK_ENABLE();

  /**
   * XSPI1 GPIO Configuration
   * PO3     ------> XSPIM_P1_DQS1
   * PP10    ------> XSPIM_P1_IO10
   * PP12    ------> XSPIM_P1_IO12
   * PP14    ------> XSPIM_P1_IO14
   * PP2     ------> XSPIM_P1_IO2
   * PP5     ------> XSPIM_P1_IO5
   * PO2     ------> XSPIM_P1_DQS0
   * PP1     ------> XSPIM_P1_IO1
   * PP11    ------> XSPIM_P1_IO11
   * PP15    ------> XSPIM_P1_IO15
   * PP3     ------> XSPIM_P1_IO3
   * PP0     ------> XSPIM_P1_IO0
   * PP7     ------> XSPIM_P1_IO7
   * PP8     ------> XSPIM_P1_IO8
   * PP13    ------> XSPIM_P1_IO13
   * PP4     ------> XSPIM_P1_IO4
   * PO4     ------> XSPIM_P1_CLK
   * PP6     ------> XSPIM_P1_IO6
   * PO0     ------> XSPIM_P1_NCS1
   * PP9     ------> XSPIM_P1_IO9
   */
  GPIO_InitStruct.Pin = GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF9_XSPIM_P1;
  HAL_GPIO_Init(GPIOO, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_12 | GPIO_PIN_14 | GPIO_PIN_2 |
                        GPIO_PIN_5 | GPIO_PIN_1 | GPIO_PIN_11 | GPIO_PIN_15 |
                        GPIO_PIN_3 | GPIO_PIN_0 | GPIO_PIN_7 | GPIO_PIN_8 |
                        GPIO_PIN_13 | GPIO_PIN_4 | GPIO_PIN_6 | GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF9_XSPIM_P1;
  HAL_GPIO_Init(GPIOP, &GPIO_InitStruct);
}

/**
* @brief  Write mode register
* @param  Ctx Component object pointer
* @param  Address Register address
* @param  Value Register value pointer
* @retval error status
*/
uint32_t APS256_WriteReg(XSPI_HandleTypeDef *Ctx, uint32_t Address, uint8_t *Value)
{
  XSPI_RegularCmdTypeDef sCommand1={0};
  
  /* Initialize the write register command */
  sCommand1.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCommand1.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
  sCommand1.InstructionWidth    = HAL_XSPI_INSTRUCTION_8_BITS;
  sCommand1.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCommand1.Instruction        = WRITE_REG_CMD;
  sCommand1.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
  sCommand1.AddressWidth        = HAL_XSPI_ADDRESS_32_BITS;
  sCommand1.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
  sCommand1.Address            = Address;
  sCommand1.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCommand1.DataMode           = HAL_XSPI_DATA_8_LINES;
  sCommand1.DataDTRMode        = HAL_XSPI_DATA_DTR_ENABLE;
  sCommand1.DataLength         = 2;
  sCommand1.DummyCycles        = 0;
  sCommand1.DQSMode            = HAL_XSPI_DQS_DISABLE;
  
  /* Configure the command */
  if (HAL_XSPI_Command(Ctx, &sCommand1, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return HAL_ERROR;
  }
  
  /* Transmission of the data */
  if (HAL_XSPI_Transmit(Ctx, (uint8_t *)(Value), HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return HAL_ERROR;
  }
  
  return HAL_OK;
}

/**
  * @brief  Reset the memory
  * @param  Ctx Component object pointer
  * @retval error status
  */
int32_t APS256XX_Reset(XSPI_HandleTypeDef *Ctx)
{
  XSPI_RegularCmdTypeDef sCommand = {0};

  /* Initialize the command */
  sCommand.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCommand.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
  sCommand.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCommand.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCommand.Instruction        = RESET_CMD;
  sCommand.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
  sCommand.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  sCommand.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
  sCommand.Address            = 0;
  sCommand.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCommand.DataMode           = HAL_XSPI_DATA_NONE;
  sCommand.DataLength         = 0;
  sCommand.DummyCycles        = 0;
  sCommand.DQSMode            = HAL_XSPI_DQS_DISABLE;
 #if defined (XSPI_CCR_SIOO)
  sCommand.SIOOMode            = HAL_XSPI_SIOO_INST_EVERY_CMD;
 #endif

  /* Configure the command */
  if (HAL_XSPI_Command(Ctx, &sCommand, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return BSP_ERROR_PERIPH_FAILURE;
  }

  /* Need to wait tRST */
  delay_ms(1);
  return BSP_ERROR_NONE;
}

/**
* @brief  Read mode register value
* @param  Ctx Component object pointer
* @param  Address Register address
* @param  Value Register value pointer
* @param  LatencyCode Latency used for the access
* @retval error status
*/
uint32_t APS256_ReadReg(XSPI_HandleTypeDef *Ctx, uint32_t Address, uint8_t *Value, uint32_t LatencyCode)
{
  XSPI_RegularCmdTypeDef sCommand={0};
  
  /* Initialize the read register command */
  sCommand.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCommand.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
  sCommand.InstructionWidth    = HAL_XSPI_INSTRUCTION_8_BITS;
  sCommand.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCommand.Instruction        = READ_REG_CMD;
  sCommand.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
  sCommand.AddressWidth        = HAL_XSPI_ADDRESS_32_BITS;
  sCommand.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
  sCommand.Address            = Address;
  sCommand.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCommand.DataMode           = HAL_XSPI_DATA_8_LINES;
  sCommand.DataDTRMode        = HAL_XSPI_DATA_DTR_ENABLE;
  sCommand.DataLength            = 2;
  sCommand.DummyCycles        = (LatencyCode - 1U);
  sCommand.DQSMode            = HAL_XSPI_DQS_ENABLE;
  
  /* Configure the command */
  if (HAL_XSPI_Command(Ctx, &sCommand, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return HAL_ERROR;
  }
  
  /* Reception of the data */
  if (HAL_XSPI_Receive(Ctx, (uint8_t *)Value, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return HAL_ERROR;
  }
  
  return HAL_OK;
}

/**
* @brief  Switch from Octal Mode to Hexa Mode on the memory
* @param  None
* @retval None
*/
static int32_t Configure_APMemory()
{
  /* MR0 register for read and write */
  /* To configure AP memory Latency Type and drive Strength */
  uint8_t regW_MR0[2]={0x11,0x8D};
  uint8_t regR_MR0[2]={0};

  /* MR4 register for read and write */
  /* To configure AP memory write latency */
  uint8_t regW_MR4[2]={0x20,0};
  uint8_t regR_MR4[2]={0};
  
  /* MR8 register for read and write */
  /* To configure AP memory Burst Type */
  uint8_t regW_MR8[2]={0x45,0x08};
  uint8_t regR_MR8[2]={0};
  
  /*Read Latency */
  uint8_t latency=6;
  
  /* Configure Read Latency and drive Strength */
  if (APS256_WriteReg(&hxspi_ram, MR0, regW_MR0) != HAL_OK) {
    return BSP_ERROR_PERIPH_FAILURE;
  }

  /* Check MR0 configuration */
  if (APS256_ReadReg(&hxspi_ram, MR0, regR_MR0, latency) != HAL_OK) {
    return BSP_ERROR_PERIPH_FAILURE;
  }

  /* Check MR0 configuration */
  if (regR_MR0[0] != regW_MR0[0]) {
    return BSP_ERROR_PERIPH_FAILURE;
  }
  
  /* Configure write latency */
  if (APS256_WriteReg(&hxspi_ram, MR4, regW_MR4) != HAL_OK) {
    return BSP_ERROR_PERIPH_FAILURE;
  }

  /* Check MR4 configuration */
  if (APS256_ReadReg(&hxspi_ram, MR4, regR_MR4, latency) != HAL_OK) {
    return BSP_ERROR_PERIPH_FAILURE;
  }

  /* Check MR4 configuration */
  if (regR_MR4[0] != regW_MR4[0]) {
    return BSP_ERROR_PERIPH_FAILURE;
  }

  /* Configure Burst Length */
  if (APS256_WriteReg(&hxspi_ram, MR8, regW_MR8) != HAL_OK) {
    return BSP_ERROR_PERIPH_FAILURE;
  }

  /* Check MR8 configuration */
  if (APS256_ReadReg(&hxspi_ram, MR8, regR_MR8, 6) != HAL_OK) {
    return BSP_ERROR_PERIPH_FAILURE;
  }

  if (regR_MR8[0] != regW_MR8[0]) {
    return BSP_ERROR_PERIPH_FAILURE;
  }

  return BSP_ERROR_NONE;
}

static int32_t XSPI_RAM_ConfigureMemoryMappedMode()
{
  XSPI_RegularCmdTypeDef sCommand = {0};
  XSPI_MemoryMappedTypeDef sMemMappedCfg = {0};

  sCommand.OperationType      = HAL_XSPI_OPTYPE_WRITE_CFG;
  sCommand.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
  sCommand.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCommand.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCommand.Instruction        = WRITE_LINEAR_BURST_CMD; // WRITE_CMD;
  sCommand.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
  sCommand.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  sCommand.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
  sCommand.Address            = 0x0;
  sCommand.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCommand.DataMode           = HAL_XSPI_DATA_16_LINES;
  sCommand.DataDTRMode        = HAL_XSPI_DATA_DTR_ENABLE;
  sCommand.DataLength         = BUFFERSIZE;
  sCommand.DummyCycles        = DUMMY_CLOCK_CYCLES_WRITE;
  sCommand.DQSMode            = HAL_XSPI_DQS_ENABLE;

  if (HAL_XSPI_Command(&hxspi_ram, &sCommand, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) !=
      HAL_OK) {
    return BSP_ERROR_COMPONENT_FAILURE;
  }

  sCommand.OperationType = HAL_XSPI_OPTYPE_READ_CFG;
  sCommand.Instruction = READ_LINEAR_BURST_CMD; // READ_CMD;
  sCommand.DummyCycles = DUMMY_CLOCK_CYCLES_READ;
  sCommand.DQSMode = HAL_XSPI_DQS_ENABLE;

  if (HAL_XSPI_Command(&hxspi_ram, &sCommand, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) !=
      HAL_OK) {
    return BSP_ERROR_COMPONENT_FAILURE;
  }

  sMemMappedCfg.TimeOutActivation = HAL_XSPI_TIMEOUT_COUNTER_DISABLE;
  sMemMappedCfg.TimeoutPeriodClock = 0x50;

  if (HAL_XSPI_MemoryMapped(&hxspi_ram, &sMemMappedCfg) != HAL_OK) {
    return BSP_ERROR_PERIPH_FAILURE;
  }

  return BSP_ERROR_NONE;
}

/**
 * @brief  Initializes the XSPI interface.
 * @param  Instance   XSPI Instance
 * @param  Init       XSPI Init structure
 * @retval BSP status
 */
int32_t ExtRAM_Init()
{
  XSPIM_CfgTypeDef sXspiManagerCfg = {0};

  /* Msp XSPI initialization */
  XSPI_RAM_MspInit();

  /* XSPI1 parameter configuration*/
  hxspi_ram.Instance = XSPI1;
  hxspi_ram.Init.FifoThresholdByte = 4;
  hxspi_ram.Init.MemoryMode = HAL_XSPI_SINGLE_MEM;
  hxspi_ram.Init.MemoryType = HAL_XSPI_MEMTYPE_APMEM_16BITS;
  hxspi_ram.Init.MemorySize = HAL_XSPI_SIZE_256MB;
  hxspi_ram.Init.ChipSelectHighTimeCycle = 5;
  hxspi_ram.Init.FreeRunningClock = HAL_XSPI_FREERUNCLK_DISABLE;
  hxspi_ram.Init.ClockMode = HAL_XSPI_CLOCK_MODE_0;
  hxspi_ram.Init.WrapSize = HAL_XSPI_WRAP_NOT_SUPPORTED;
  hxspi_ram.Init.ClockPrescaler = 0;
  hxspi_ram.Init.SampleShifting = HAL_XSPI_SAMPLE_SHIFT_NONE;
  hxspi_ram.Init.DelayHoldQuarterCycle = HAL_XSPI_DHQC_DISABLE;
  hxspi_ram.Init.ChipSelectBoundary = HAL_XSPI_BONDARYOF_16KB;
  hxspi_ram.Init.MaxTran = 0;
  hxspi_ram.Init.Refresh = 396;
  hxspi_ram.Init.MemorySelect = HAL_XSPI_CSSEL_NCS1;

  if (HAL_XSPI_Init(&hxspi_ram) != HAL_OK) {
    return BSP_ERROR_PERIPH_FAILURE;
  }

  sXspiManagerCfg.nCSOverride = HAL_XSPI_CSSEL_OVR_NCS1;
  sXspiManagerCfg.IOPort = HAL_XSPIM_IOPORT_1;

  if (HAL_XSPIM_Config(&hxspi_ram, &sXspiManagerCfg,
                       HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
    return BSP_ERROR_PERIPH_FAILURE;
  }

  // Reset procedure uses delays...
  delaysInit();

  if (APS256XX_Reset(&hxspi_ram) != BSP_ERROR_NONE) {
    return BSP_ERROR_PERIPH_FAILURE;
  }

  if (Configure_APMemory() != BSP_ERROR_NONE) {
    return BSP_ERROR_PERIPH_FAILURE;
  }

  if (XSPI_RAM_ConfigureMemoryMappedMode() != BSP_ERROR_NONE) {
    return BSP_ERROR_PERIPH_FAILURE;
  }

  /* Return BSP status */
  return BSP_ERROR_NONE;
}
