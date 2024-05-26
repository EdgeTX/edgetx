#include "aps256xx.h"

/* Read/Write Array Commands *********************/
/**
 * @brief  Reads an amount of data from the memory.
 * @param  Ctx Component object pointer
 * @param  pData Pointer to data to be read
 * @param  ReadAddr Read start address
 * @param  Size Size of data to read
 * @param  LatencyCode Latency used for the access
 * @param  IOMode I/O Mode used for the access
 * @param  BurstType Type of burst used for the access
 * @retval Memory status
 */
int32_t APS256XX_Read(XSPI_HandleTypeDef *Ctx, uint8_t *pData,
                      uint32_t ReadAddr, uint32_t Size, uint32_t LatencyCode,
                      uint32_t IOMode, uint32_t BurstType)
{
  XSPI_RegularCmdTypeDef sCommand = {0};

  /* Initialize the read command */
  sCommand.OperationType = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCommand.InstructionMode = HAL_XSPI_INSTRUCTION_8_LINES;
  sCommand.InstructionWidth = HAL_XSPI_INSTRUCTION_8_BITS;
  sCommand.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCommand.Instruction =
      ((BurstType == 0U) ? APS256XX_READ_LINEAR_BURST_CMD : APS256XX_READ_CMD);
  sCommand.AddressMode = HAL_XSPI_ADDRESS_8_LINES;
  sCommand.AddressWidth = HAL_XSPI_ADDRESS_32_BITS;
  sCommand.AddressDTRMode = HAL_XSPI_ADDRESS_DTR_ENABLE;
  sCommand.Address = ReadAddr;
  sCommand.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCommand.DataMode =
      ((IOMode == 0U) ? HAL_XSPI_DATA_8_LINES : HAL_XSPI_DATA_16_LINES);
  sCommand.DataDTRMode = HAL_XSPI_DATA_DTR_ENABLE;
  sCommand.DataLength = Size;
  sCommand.DummyCycles = (LatencyCode - 1U);
  sCommand.DQSMode = HAL_XSPI_DQS_ENABLE;
#if defined(XSPI_CCR_SIOO)
  sCommand.SIOOMode = HAL_XSPI_SIOO_INST_EVERY_CMD;
#endif
  /* Configure the command */
  if (HAL_XSPI_Command(Ctx, &sCommand, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) !=
      HAL_OK) {
    return APS256XX_ERROR;
  }

  /* Reception of the data */
  if (HAL_XSPI_Receive(Ctx, pData, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
    return APS256XX_ERROR;
  }

  return APS256XX_OK;
}

/**
 * @brief  Reads an amount of data in DMA mode from the memory.
 * @param  Ctx Component object pointer
 * @param  pData Pointer to data to be read
 * @param  ReadAddr Read start address
 * @param  Size Size of data to read
 * @param  LatencyCode Latency used for the access
 * @param  IOMode I/O Mode used for the access
 * @param  BurstType Type of burst used for the access
 * @retval Memory status
 */
int32_t APS256XX_Read_DMA(XSPI_HandleTypeDef *Ctx, uint8_t *pData,
                          uint32_t ReadAddr, uint32_t Size,
                          uint32_t LatencyCode, uint32_t IOMode,
                          uint32_t BurstType)
{
  XSPI_RegularCmdTypeDef sCommand = {0};

  /* Initialize the read command */
  sCommand.OperationType = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCommand.InstructionMode = HAL_XSPI_INSTRUCTION_8_LINES;
  sCommand.InstructionWidth = HAL_XSPI_INSTRUCTION_8_BITS;
  sCommand.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCommand.Instruction =
      ((BurstType == 0U) ? APS256XX_READ_LINEAR_BURST_CMD : APS256XX_READ_CMD);
  sCommand.AddressMode = HAL_XSPI_ADDRESS_8_LINES;
  sCommand.AddressWidth = HAL_XSPI_ADDRESS_32_BITS;
  sCommand.AddressDTRMode = HAL_XSPI_ADDRESS_DTR_ENABLE;
  sCommand.Address = ReadAddr;
  sCommand.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCommand.DataMode =
      ((IOMode == 0U) ? HAL_XSPI_DATA_8_LINES : HAL_XSPI_DATA_16_LINES);
  sCommand.DataDTRMode = HAL_XSPI_DATA_DTR_ENABLE;
  sCommand.DataLength = Size;
  sCommand.DummyCycles = (LatencyCode - 1U);
  sCommand.DQSMode = HAL_XSPI_DQS_ENABLE;
#if defined(XSPI_CCR_SIOO)
  sCommand.SIOOMode = HAL_XSPI_SIOO_INST_EVERY_CMD;
#endif

  /* Configure the command */
  if (HAL_XSPI_Command(Ctx, &sCommand, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) !=
      HAL_OK) {
    return APS256XX_ERROR;
  }

  /* Reception of the data */
  if (HAL_XSPI_Receive_DMA(Ctx, pData) != HAL_OK) {
    return APS256XX_ERROR;
  }

  return APS256XX_OK;
}

/**
 * @brief  Writes an amount of data to the memory.
 * @param  Ctx Component object pointer
 * @param  pData Pointer to data to be written
 * @param  WriteAddr Write start address
 * @param  Size Size of data to write
 * @param  LatencyCode Latency used for the access
 * @param  IOMode I/O Mode used for the access
 * @param  BurstType Type of burst used for the access
 * @retval Memory status
 */
int32_t APS256XX_Write(XSPI_HandleTypeDef *Ctx, uint8_t *pData,
                       uint32_t WriteAddr, uint32_t Size, uint32_t LatencyCode,
                       uint32_t IOMode, uint32_t BurstType)
{
  XSPI_RegularCmdTypeDef sCommand = {0};

  /* Initialize the write command */
  sCommand.OperationType = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCommand.InstructionMode = HAL_XSPI_INSTRUCTION_8_LINES;
  sCommand.InstructionWidth = HAL_XSPI_INSTRUCTION_8_BITS;
  sCommand.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCommand.Instruction = ((BurstType == 0U) ? APS256XX_WRITE_LINEAR_BURST_CMD
                                            : APS256XX_WRITE_CMD);
  sCommand.AddressMode = HAL_XSPI_ADDRESS_8_LINES;
  sCommand.AddressWidth = HAL_XSPI_ADDRESS_32_BITS;
  sCommand.AddressDTRMode = HAL_XSPI_ADDRESS_DTR_ENABLE;
  sCommand.Address = WriteAddr;
  sCommand.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCommand.DataMode =
      ((IOMode == 0U) ? HAL_XSPI_DATA_8_LINES : HAL_XSPI_DATA_16_LINES);
  sCommand.DataDTRMode = HAL_XSPI_DATA_DTR_ENABLE;
  sCommand.DataLength = Size;
  sCommand.DummyCycles = (LatencyCode - 1U);
  sCommand.DQSMode = HAL_XSPI_DQS_ENABLE;
#if defined(XSPI_CCR_SIOO)
  sCommand.SIOOMode = HAL_XSPI_SIOO_INST_EVERY_CMD;
#endif

  /* Configure the command */
  if (HAL_XSPI_Command(Ctx, &sCommand, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) !=
      HAL_OK) {
    return APS256XX_ERROR;
  }

  /* Transmission of the data */
  if (HAL_XSPI_Transmit(Ctx, pData, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
    return APS256XX_ERROR;
  }

  return APS256XX_OK;
}

/**
 * @brief  Writes an amount of data in DMA mode to the memory.
 * @param  Ctx Component object pointer
 * @param  pData Pointer to data to be written
 * @param  WriteAddr Write start address
 * @param  Size Size of data to write
 * @param  LatencyCode Latency used for the access
 * @param  IOMode I/O Mode used for the access
 * @param  BurstType Type of burst used for the access
 * @retval Memory status
 */
int32_t APS256XX_Write_DMA(XSPI_HandleTypeDef *Ctx, uint8_t *pData,
                           uint32_t WriteAddr, uint32_t Size,
                           uint32_t LatencyCode, uint32_t IOMode,
                           uint32_t BurstType)
{
  XSPI_RegularCmdTypeDef sCommand = {0};

  /* Initialize the write command */
  sCommand.OperationType = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCommand.InstructionMode = HAL_XSPI_INSTRUCTION_8_LINES;
  sCommand.InstructionWidth = HAL_XSPI_INSTRUCTION_8_BITS;
  sCommand.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCommand.Instruction = ((BurstType == 0U) ? APS256XX_WRITE_LINEAR_BURST_CMD
                                            : APS256XX_WRITE_CMD);
  sCommand.AddressMode = HAL_XSPI_ADDRESS_8_LINES;
  sCommand.AddressWidth = HAL_XSPI_ADDRESS_32_BITS;
  sCommand.AddressDTRMode = HAL_XSPI_ADDRESS_DTR_ENABLE;
  sCommand.Address = WriteAddr;
  sCommand.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCommand.DataMode =
      ((IOMode == 0U) ? HAL_XSPI_DATA_8_LINES : HAL_XSPI_DATA_16_LINES);
  sCommand.DataDTRMode = HAL_XSPI_DATA_DTR_ENABLE;
  sCommand.DataLength = Size;
  sCommand.DummyCycles = (LatencyCode - 1U);
  sCommand.DQSMode = HAL_XSPI_DQS_ENABLE;
#if defined(XSPI_CCR_SIOO)
  sCommand.SIOOMode = HAL_XSPI_SIOO_INST_EVERY_CMD;
#endif

  /* Configure the command */
  if (HAL_XSPI_Command(Ctx, &sCommand, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) !=
      HAL_OK) {
    return APS256XX_ERROR;
  }

  /* Transmission of the data */
  if (HAL_XSPI_Transmit_DMA(Ctx, pData) != HAL_OK) {
    return APS256XX_ERROR;
  }

  return APS256XX_OK;
}

/**
 * @brief  Enable memory mapped mode for the memory.
 * @param  Ctx Component object pointer
 * @param  ReadLatencyCode Latency used for the read access
 * @param  WriteLatencyCode Latency used for the write access
 * @param  IOMode I/O Mode used for the access
 * @param  BurstType Type of burst used for the access
 * @retval Memory status
 */
int32_t APS256XX_EnableMemoryMappedMode(XSPI_HandleTypeDef *Ctx,
                                        uint32_t ReadLatencyCode,
                                        uint32_t WriteLatencyCode,
                                        uint32_t IOMode, uint32_t BurstType)
{
  XSPI_RegularCmdTypeDef sCommand = {0};
  XSPI_MemoryMappedTypeDef sMemMappedCfg = {0};

  /* Initialize the write command */
  sCommand.OperationType = HAL_XSPI_OPTYPE_WRITE_CFG;
  sCommand.InstructionMode = HAL_XSPI_INSTRUCTION_8_LINES;
  sCommand.InstructionWidth = HAL_XSPI_INSTRUCTION_8_BITS;
  sCommand.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCommand.Instruction = ((BurstType == 0U) ? APS256XX_WRITE_LINEAR_BURST_CMD
                                            : APS256XX_WRITE_CMD);
  sCommand.AddressMode = HAL_XSPI_ADDRESS_8_LINES;
  sCommand.AddressWidth = HAL_XSPI_ADDRESS_32_BITS;
  sCommand.AddressDTRMode = HAL_XSPI_ADDRESS_DTR_ENABLE;
  sCommand.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCommand.DataMode =
      ((IOMode == 0U) ? HAL_XSPI_DATA_8_LINES : HAL_XSPI_DATA_16_LINES);
  sCommand.DataDTRMode = HAL_XSPI_DATA_DTR_ENABLE;
  sCommand.DummyCycles = (WriteLatencyCode - 1U);
  sCommand.DQSMode = HAL_XSPI_DQS_ENABLE;
#if defined(XSPI_CCR_SIOO)
  sCommand.SIOOMode = HAL_XSPI_SIOO_INST_EVERY_CMD;
#endif

  if (HAL_XSPI_Command(Ctx, &sCommand, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) !=
      HAL_OK) {
    return APS256XX_ERROR;
  }

  /* Initialize the read command */
  sCommand.OperationType = HAL_XSPI_OPTYPE_READ_CFG;
  sCommand.Instruction =
      ((BurstType == 0U) ? APS256XX_READ_LINEAR_BURST_CMD : APS256XX_READ_CMD);
  sCommand.DummyCycles = (ReadLatencyCode - 1U);

  if (HAL_XSPI_Command(Ctx, &sCommand, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) !=
      HAL_OK) {
    return APS256XX_ERROR;
  }

  /* Activation of memory-mapped mode */
  sMemMappedCfg.TimeOutActivation = HAL_XSPI_TIMEOUT_COUNTER_DISABLE;

  if (HAL_XSPI_MemoryMapped(Ctx, &sMemMappedCfg) != HAL_OK) {
    return APS256XX_ERROR;
  }

  return APS256XX_OK;
}

/* Register/Setting Commands **************************************************/
/**
 * @brief  Read mode register value
 * @param  Ctx Component object pointer
 * @param  Address Register address
 * @param  Value Register value pointer
 * @param  LatencyCode Latency used for the access
 * @retval error status
 */
int32_t APS256XX_ReadReg(XSPI_HandleTypeDef *Ctx, uint32_t Address,
                         uint8_t *Value, uint32_t LatencyCode)
{
  XSPI_RegularCmdTypeDef sCommand = {0};

  /* Initialize the read register command */
  sCommand.OperationType = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCommand.InstructionMode = HAL_XSPI_INSTRUCTION_8_LINES;
  sCommand.InstructionWidth = HAL_XSPI_INSTRUCTION_8_BITS;
  sCommand.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCommand.Instruction = APS256XX_READ_REG_CMD;
  sCommand.AddressMode = HAL_XSPI_ADDRESS_8_LINES;
  sCommand.AddressWidth = HAL_XSPI_ADDRESS_32_BITS;
  sCommand.AddressDTRMode = HAL_XSPI_ADDRESS_DTR_ENABLE;
  sCommand.Address = Address;
  sCommand.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCommand.DataMode = HAL_XSPI_DATA_8_LINES;
  sCommand.DataDTRMode = HAL_XSPI_DATA_DTR_ENABLE;
  sCommand.DataLength = 2;
  sCommand.DummyCycles = (LatencyCode - 1U);
  sCommand.DQSMode = HAL_XSPI_DQS_ENABLE;
#if defined(XSPI_CCR_SIOO)
  sCommand.SIOOMode = HAL_XSPI_SIOO_INST_EVERY_CMD;
#endif

  /* Configure the command */
  if (HAL_XSPI_Command(Ctx, &sCommand, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) !=
      HAL_OK) {
    return APS256XX_ERROR;
  }

  /* Reception of the data */
  if (HAL_XSPI_Receive(Ctx, (uint8_t *)Value, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) !=
      HAL_OK) {
    return APS256XX_ERROR;
  }

  return APS256XX_OK;
}

/**
 * @brief  Write mode register
 * @param  Ctx Component object pointer
 * @param  Address Register address
 * @param  Value Value to write to register
 * @retval error status
 */
int32_t APS256XX_WriteReg(XSPI_HandleTypeDef *Ctx, uint32_t Address,
                          uint8_t Value)
{
  XSPI_RegularCmdTypeDef sCommand = {0};

  /* Initialize the write register command */
  sCommand.OperationType = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCommand.InstructionMode = HAL_XSPI_INSTRUCTION_8_LINES;
  sCommand.InstructionWidth = HAL_XSPI_INSTRUCTION_8_BITS;
  sCommand.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCommand.Instruction = APS256XX_WRITE_REG_CMD;
  sCommand.AddressMode = HAL_XSPI_ADDRESS_8_LINES;
  sCommand.AddressWidth = HAL_XSPI_ADDRESS_32_BITS;
  sCommand.AddressDTRMode = HAL_XSPI_ADDRESS_DTR_ENABLE;
  sCommand.Address = Address;
  sCommand.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCommand.DataMode = HAL_XSPI_DATA_8_LINES;
  sCommand.DataDTRMode = HAL_XSPI_DATA_DTR_ENABLE;
  sCommand.DataLength = 2;
  sCommand.DummyCycles = 0;
  sCommand.DQSMode = HAL_XSPI_DQS_DISABLE;
#if defined(XSPI_CCR_SIOO)
  sCommand.SIOOMode = HAL_XSPI_SIOO_INST_EVERY_CMD;
#endif

  /* Configure the command */
  if (HAL_XSPI_Command(Ctx, &sCommand, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) !=
      HAL_OK) {
    return APS256XX_ERROR;
  }

  /* Transmission of the data */
  if (HAL_XSPI_Transmit(Ctx, (uint8_t *)(&Value),
                        HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
    return APS256XX_ERROR;
  }

  return APS256XX_OK;
}

/* ID Commands ****************************************************************/
/**
 * @brief  Read Flash IDs.
 *         Vendor ID, Device ID, Device Density
 * @param  Ctx Component object pointer
 * @param  ID IDs pointer (2 * 8-bits value)
 * @param  LatencyCode Latency used for the access
 * @retval error status
 */
int32_t APS256XX_ReadID(XSPI_HandleTypeDef *Ctx, uint8_t *ID,
                        uint32_t LatencyCode)
{
  /* Read the Mode Register 1 and 2 */
  if (APS256XX_ReadReg(Ctx, APS256XX_MR1_ADDRESS, ID, LatencyCode) !=
      APS256XX_OK) {
    return APS256XX_ERROR;
  }

  /* Keep only Vendor ID from Mode Register 1 */
  *ID &= (APS256XX_MR1_VENDOR_ID);

  /* Keep only Device ID and Device Density from Mode Register 2 */
  *(ID + 1) &= (APS256XX_MR2_DEVICE_ID | APS256XX_MR2_DENSITY);

  return APS256XX_OK;
}

/* Power down Commands ********************************************************/
/**
 * @brief  Memory enter deep power-down command
 * @param  Ctx Component object pointer
 * @retval error status
 */
int32_t APS256XX_EnterPowerDown(XSPI_HandleTypeDef *Ctx)
{
  /* Update the deep power down value of the MR6 register */
  if (APS256XX_WriteReg(Ctx, APS256XX_MR6_ADDRESS,
                        APS256XX_MR6_HS_HALF_SPEED_MODE) != APS256XX_OK) {
    return APS256XX_ERROR;
  }

  /* ---         Memory enter deep power down as soon nCS goes high        ---
   */
  /* --- At least 500us should be respected before leaving deep power down ---
   */

  return APS256XX_OK;
}

/**
 * @brief  Flash leave deep power-down command
 * @param  Ctx Component object pointer
 * @param  LatencyCode Latency used for the access
 * @retval error status
 */
int32_t APS256XX_LeavePowerDown(XSPI_HandleTypeDef *Ctx)
{
  /* --- A dummy command is sent to the memory, as the nCS should be low for at
   * least 60 ns  --- */
  /* ---                  Memory takes 150us max to leave deep power down --- */

  XSPI_RegularCmdTypeDef sCommand = {0};

  /* Initialize the command */
  sCommand.OperationType = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCommand.InstructionMode = HAL_XSPI_INSTRUCTION_8_LINES;
  sCommand.InstructionWidth = HAL_XSPI_INSTRUCTION_8_BITS;
  sCommand.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCommand.Instruction = APS256XX_READ_CMD;
  sCommand.AddressMode = HAL_XSPI_ADDRESS_8_LINES;
  sCommand.AddressWidth = HAL_XSPI_ADDRESS_32_BITS;
  sCommand.AddressDTRMode = HAL_XSPI_ADDRESS_DTR_DISABLE;
  sCommand.Address = 0;
  sCommand.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCommand.DataMode = HAL_XSPI_DATA_NONE;
  sCommand.DataLength = 0;
  sCommand.DummyCycles = 0;
  sCommand.DQSMode = HAL_XSPI_DQS_DISABLE;
#if defined(XSPI_CCR_SIOO)
  sCommand.SIOOMode = HAL_XSPI_SIOO_INST_EVERY_CMD;
#endif

  /* Configure the command */
  if (HAL_XSPI_Command(Ctx, &sCommand, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) !=
      HAL_OK) {
    return APS256XX_ERROR;
  }

  return APS256XX_OK;
}

/* Reset Command **************************************************************/
/**
 * @brief  Reset the memory
 * @param  Ctx Component object pointer
 * @retval error status
 */
int32_t APS256XX_Reset(XSPI_HandleTypeDef *Ctx)
{
  XSPI_RegularCmdTypeDef sCommand = {0};

  /* Initialize the command */
  sCommand.OperationType = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCommand.InstructionMode = HAL_XSPI_INSTRUCTION_8_LINES;
  sCommand.InstructionWidth = HAL_XSPI_INSTRUCTION_8_BITS;
  sCommand.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCommand.Instruction = APS256XX_RESET_CMD;
  sCommand.AddressMode = HAL_XSPI_ADDRESS_8_LINES;
  sCommand.AddressWidth = HAL_XSPI_ADDRESS_24_BITS;
  sCommand.AddressDTRMode = HAL_XSPI_ADDRESS_DTR_DISABLE;
  sCommand.Address = 0;
  sCommand.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCommand.DataMode = HAL_XSPI_DATA_NONE;
  sCommand.DataLength = 0;
  sCommand.DummyCycles = 0;
  sCommand.DQSMode = HAL_XSPI_DQS_DISABLE;
#if defined(XSPI_CCR_SIOO)
  sCommand.SIOOMode = HAL_XSPI_SIOO_INST_EVERY_CMD;
#endif

  /* Configure the command */
  if (HAL_XSPI_Command(Ctx, &sCommand, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) !=
      HAL_OK) {
    return APS256XX_ERROR;
  }

  /* Need to wait tRST */
  HAL_Delay(1);
  return APS256XX_OK;
}
