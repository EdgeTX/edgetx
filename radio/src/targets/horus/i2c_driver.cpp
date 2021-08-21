/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   OpenTX - https://github.com/opentx/opentx
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
 *
 * The content of this file partially stems from STM32F4 HAL by STMicroelectronics.
 */

#include "opentx.h"
#include "i2c_driver.h"

/* I2C MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hi2c: I2C handle pointer
 * @retval None
 */
void I2C_MspInit(I2C_HandleTypeDef* hi2c)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  if (I2C_TP_GPIO == GPIOA)
      __HAL_RCC_GPIOA_CLK_ENABLE();
  else if (I2C_TP_GPIO == GPIOB)
      __HAL_RCC_GPIOB_CLK_ENABLE();
  else if (I2C_TP_GPIO == GPIOC)
      __HAL_RCC_GPIOC_CLK_ENABLE();
  else if (I2C_TP_GPIO == GPIOH)
      __HAL_RCC_GPIOH_CLK_ENABLE();
  else
      TRACE("I2C ERROR: HAL_I2C_MspInit() I2C_GPIO misconfiguration");

  if ((hi2c->Instance==I2C1) || (hi2c->Instance==I2C3)) // TP
  {
    GPIO_PinAFConfig(I2C_TP_GPIO, I2C_TP_SCL_GPIO_PinSource, I2C_TP_GPIO_AF);
    GPIO_PinAFConfig(I2C_TP_GPIO, I2C_TP_SDA_GPIO_PinSource, I2C_TP_GPIO_AF);

    GPIO_InitStruct.GPIO_Pin = I2C_TP_SCL_GPIO_PIN | I2C_TP_SDA_GPIO_PIN;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(I2C_TP_GPIO, &GPIO_InitStruct);
  }

  if (hi2c->Instance==I2C2) // IMU
  {
    GPIO_PinAFConfig(I2C_IMU_GPIO, I2C_IMU_SCL_GPIO_PinSource, I2C_IMU_GPIO_AF);
    GPIO_PinAFConfig(I2C_IMU_GPIO, I2C_IMU_SDA_GPIO_PinSource, I2C_IMU_GPIO_AF);

    GPIO_InitStruct.GPIO_Pin = I2C_IMU_SCL_GPIO_PIN | I2C_IMU_SDA_GPIO_PIN;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(I2C_IMU_GPIO, &GPIO_InitStruct);
  }

  /* Peripheral clock enable */
  if (I2C_TP == I2C1)
      __HAL_RCC_I2C1_CLK_ENABLE();
  else if (I2C_TP == I2C2)
      __HAL_RCC_I2C2_CLK_ENABLE();
  else if (I2C_TP == I2C3)
      __HAL_RCC_I2C3_CLK_ENABLE();
  else
      TRACE("I2C ERROR: HAL_I2C_MspInit() I2C misconfiguration");
}

/* De-initializes the GPIOx peripheral registers to their default reset values.
 * @param  GPIOx where x can be (A..K) to select the GPIO peripheral for STM32F429X device or
 *                      x can be (A..I) to select the GPIO peripheral for STM32F40XX and STM32F427X devices.
 * @param  GPIO_Pin specifies the port bit to be written.
 *          This parameter can be one of GPIO_PIN_x where x can be (0..15).
 * @retval None
 */
void HAL_GPIO_DeInit(GPIO_TypeDef  *GPIOx, uint32_t GPIO_Pin)
{
  uint32_t position;
  uint32_t ioposition = 0x00U;
  uint32_t iocurrent = 0x00U;
  uint32_t tmp = 0x00U;

  /* Check the parameters */
  assert_param(IS_GPIO_ALL_INSTANCE(GPIOx));

  /* Configure the port pins */
  for(position = 0U; position < GPIO_NUMBER; position++)
  {
    /* Get the IO position */
    ioposition = 0x01U << position;
    /* Get the current IO position */
    iocurrent = (GPIO_Pin) & ioposition;

    if(iocurrent == ioposition)
    {
      /*------------------------- EXTI Mode Configuration --------------------*/
      tmp = SYSCFG->EXTICR[position >> 2U];
      tmp &= (0x0FU << (4U * (position & 0x03U)));
      if(tmp == ((uint32_t)(GPIO_GET_INDEX(GPIOx)) << (4U * (position & 0x03U))))
      {
        /* Clear EXTI line configuration */
        EXTI->IMR &= ~((uint32_t)iocurrent);
        EXTI->EMR &= ~((uint32_t)iocurrent);

        /* Clear Rising Falling edge configuration */
        EXTI->RTSR &= ~((uint32_t)iocurrent);
        EXTI->FTSR &= ~((uint32_t)iocurrent);

        /* Configure the External Interrupt or event for the current IO */
        tmp = 0x0FU << (4U * (position & 0x03U));
        SYSCFG->EXTICR[position >> 2U] &= ~tmp;
      }

      /*------------------------- GPIO Mode Configuration --------------------*/
      /* Configure IO Direction in Input Floating Mode */
      GPIOx->MODER &= ~(GPIO_MODER_MODER0 << (position * 2U));

      /* Configure the default Alternate Function in current IO */
      GPIOx->AFR[position >> 3U] &= ~(0xFU << ((uint32_t)(position & 0x07U) * 4U)) ;

      /* Deactivate the Pull-up and Pull-down resistor for the current IO */
      GPIOx->PUPDR &= ~(GPIO_PUPDR_PUPDR0 << (position * 2U));

      /* Configure the default value IO Output Type */
      GPIOx->OTYPER  &= ~(GPIO_OTYPER_OT_0 << position) ;

      /* Configure the default value for IO Speed */
      GPIOx->OSPEEDR &= ~(GPIO_OSPEEDER_OSPEEDR0 << (position * 2U));
    }
  }
}

/* I2C MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hi2c: I2C handle pointer
 * @retval None
 */
void HAL_I2C_MspDeInit(I2C_HandleTypeDef* hi2c)
{
  if(hi2c->Instance==I2C1)
  {
    __HAL_RCC_I2C1_CLK_DISABLE();
  }
  else
  {
      if(hi2c->Instance==I2C2)
      {
          __HAL_RCC_I2C2_CLK_DISABLE();
            if(hi2c->Instance==I2C3)
                __HAL_RCC_I2C3_CLK_DISABLE();
            else
                TRACE("I2C ERROR: HAL_I2C_MspDeInit() I2C misconfiguration");
      }
  }

  GPIO_InitTypeDef GPIO_InitStructure;

  /* Configure the default Alternate Function in current IO */
  GPIO_PinAFConfig(I2C_TP_GPIO, I2C_TP_SCL_GPIO_PinSource, 0);
  GPIO_PinAFConfig(I2C_TP_GPIO, I2C_TP_SDA_GPIO_PinSource, 0);

  GPIO_InitStructure.GPIO_Pin = I2C_TP_SCL_GPIO_PIN | I2C_TP_SDA_GPIO_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;   /* Configure a low value for IO Speed */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; /* Configure IO Direction in Input Floating Mode */
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD; /* Leave the configuration to Open Drain */
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; /* Deactivate the Pull-up and Pull-down resistor for the current IO */
  GPIO_Init(I2C_TP_GPIO, &GPIO_InitStructure);
}

/* Initializes the I2C according to the specified parameters
 * in the HAL_I2C_InitTypeDef and initialize the associated handle.
 * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
 *              the configuration information for the specified I2C.
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_I2C_Init(I2C_HandleTypeDef *hi2c)
{
  uint32_t freqrange;
  uint32_t pclk1;

  /* Check the I2C handle allocation */
  if (hi2c == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the parameters */
  assert_param(IS_I2C_ALL_INSTANCE(hi2c->Instance));
  assert_param(IS_I2C_CLOCK_SPEED(hi2c->Init.ClockSpeed));
  assert_param(IS_I2C_DUTY_CYCLE(hi2c->Init.DutyCycle));
  assert_param(IS_I2C_OWN_ADDRESS1(hi2c->Init.OwnAddress1));
  assert_param(IS_I2C_ADDRESSING_MODE(hi2c->Init.AddressingMode));
  assert_param(IS_I2C_DUAL_ADDRESS(hi2c->Init.DualAddressMode));
  assert_param(IS_I2C_OWN_ADDRESS2(hi2c->Init.OwnAddress2));
  assert_param(IS_I2C_GENERAL_CALL(hi2c->Init.GeneralCallMode));
  assert_param(IS_I2C_NO_STRETCH(hi2c->Init.NoStretchMode));

  if (hi2c->State == HAL_I2C_STATE_RESET)
  {
    /* Allocate lock resource and initialize it */
    hi2c->Lock = HAL_UNLOCKED;

    /* Init the low level hardware : GPIO, CLOCK, NVIC */
    I2C_MspInit(hi2c);
  }

  hi2c->State = HAL_I2C_STATE_BUSY;

  /* Disable the selected I2C peripheral */
  __HAL_I2C_DISABLE(hi2c);

  /*Reset I2C*/
  hi2c->Instance->CR1 |= I2C_CR1_SWRST;
  hi2c->Instance->CR1 &= ~I2C_CR1_SWRST;

  /* Get PCLK1 frequency */
  RCC_ClocksTypeDef rccClocks;
  RCC_GetClocksFreq(&rccClocks);
  pclk1 = rccClocks.PCLK1_Frequency;

  /* Check the minimum allowed PCLK1 frequency */
  if (I2C_MIN_PCLK_FREQ(pclk1, hi2c->Init.ClockSpeed) == 1U)
  {
    return HAL_ERROR;
  }

  /* Calculate frequency range */
  freqrange = I2C_FREQRANGE(pclk1);

  /*---------------------------- I2Cx CR2 Configuration ----------------------*/
  /* Configure I2Cx: Frequency range */
  MODIFY_REG(hi2c->Instance->CR2, I2C_CR2_FREQ, freqrange);

  /*---------------------------- I2Cx TRISE Configuration --------------------*/
  /* Configure I2Cx: Rise Time */
  MODIFY_REG(hi2c->Instance->TRISE, I2C_TRISE_TRISE, I2C_RISE_TIME(freqrange, hi2c->Init.ClockSpeed));

  /*---------------------------- I2Cx CCR Configuration ----------------------*/
  /* Configure I2Cx: Speed */
  MODIFY_REG(hi2c->Instance->CCR, (I2C_CCR_FS | I2C_CCR_DUTY | I2C_CCR_CCR), I2C_SPEED(pclk1, hi2c->Init.ClockSpeed, hi2c->Init.DutyCycle));

  /*---------------------------- I2Cx CR1 Configuration ----------------------*/
  /* Configure I2Cx: Generalcall and NoStretch mode */
  MODIFY_REG(hi2c->Instance->CR1, (I2C_CR1_ENGC | I2C_CR1_NOSTRETCH), (hi2c->Init.GeneralCallMode | hi2c->Init.NoStretchMode));

  /*---------------------------- I2Cx OAR1 Configuration ---------------------*/
  /* Configure I2Cx: Own Address1 and addressing mode */
  MODIFY_REG(hi2c->Instance->OAR1, (I2C_OAR1_ADDMODE | I2C_OAR1_ADD8_9 | I2C_OAR1_ADD1_7 | I2C_OAR1_ADD0), (hi2c->Init.AddressingMode | hi2c->Init.OwnAddress1));

  /*---------------------------- I2Cx OAR2 Configuration ---------------------*/
  /* Configure I2Cx: Dual mode and Own Address2 */
  MODIFY_REG(hi2c->Instance->OAR2, (I2C_OAR2_ENDUAL | I2C_OAR2_ADD2), (hi2c->Init.DualAddressMode | hi2c->Init.OwnAddress2));

  /* Enable the selected I2C peripheral */
  __HAL_I2C_ENABLE(hi2c);

  hi2c->ErrorCode = HAL_I2C_ERROR_NONE;
  hi2c->State = HAL_I2C_STATE_READY;
  hi2c->PreviousState = I2C_STATE_NONE;
  hi2c->Mode = HAL_I2C_MODE_NONE;

  return HAL_OK;
}

/* DeInitialize the I2C peripheral.
 * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
 *         the configuration information for the specified I2C.
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_I2C_DeInit(I2C_HandleTypeDef *hi2c)
{
  /* Check the I2C handle allocation */
  if (hi2c == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the parameters */
  assert_param(IS_I2C_ALL_INSTANCE(hi2c->Instance));

  hi2c->State = HAL_I2C_STATE_BUSY;

  /* Disable the I2C Peripheral Clock */
  __HAL_I2C_DISABLE(hi2c);

  /* DeInit the low level hardware: GPIO, CLOCK, NVIC */
  HAL_I2C_MspDeInit(hi2c);

  hi2c->ErrorCode     = HAL_I2C_ERROR_NONE;
  hi2c->State         = HAL_I2C_STATE_RESET;
  hi2c->PreviousState = I2C_STATE_NONE;
  hi2c->Mode          = HAL_I2C_MODE_NONE;

  /* Release Lock */
  __HAL_UNLOCK(hi2c);

  return HAL_OK;
}

/* Configures I2C Analog noise filter.
 * @param  hi2c pointer to a I2C_HandleTypeDef structure that contains
 *              the configuration information for the specified I2Cx peripheral.
 * @param  AnalogFilter new state of the Analog filter.
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_I2CEx_ConfigAnalogFilter(I2C_HandleTypeDef *hi2c, uint32_t AnalogFilter)
{
  /* Check the parameters */
  assert_param(IS_I2C_ALL_INSTANCE(hi2c->Instance));
  assert_param(IS_I2C_ANALOG_FILTER(AnalogFilter));

  if (hi2c->State == HAL_I2C_STATE_READY)
  {
    hi2c->State = HAL_I2C_STATE_BUSY;

    /* Disable the selected I2C peripheral */
    __HAL_I2C_DISABLE(hi2c);

    /* Reset I2Cx ANOFF bit */
    hi2c->Instance->FLTR &= ~(I2C_FLTR_ANOFF);

    /* Disable the analog filter */
    hi2c->Instance->FLTR |= AnalogFilter;

    __HAL_I2C_ENABLE(hi2c);

    hi2c->State = HAL_I2C_STATE_READY;

    return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }
}

/* Configures I2C Digital noise filter.
 * @param  hi2c pointer to a I2C_HandleTypeDef structure that contains
 *              the configuration information for the specified I2Cx peripheral.
 * @param  DigitalFilter Coefficient of digital noise filter between 0x00 and 0x0F.
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_I2CEx_ConfigDigitalFilter(I2C_HandleTypeDef *hi2c, uint32_t DigitalFilter)
{
  uint16_t tmpreg = 0;

  /* Check the parameters */
  assert_param(IS_I2C_ALL_INSTANCE(hi2c->Instance));
  assert_param(IS_I2C_DIGITAL_FILTER(DigitalFilter));

  if (hi2c->State == HAL_I2C_STATE_READY)
  {
    hi2c->State = HAL_I2C_STATE_BUSY;

    /* Disable the selected I2C peripheral */
    __HAL_I2C_DISABLE(hi2c);

    /* Get the old register value */
    tmpreg = hi2c->Instance->FLTR;

    /* Reset I2Cx DNF bit [3:0] */
    tmpreg &= ~(I2C_FLTR_DNF);

    /* Set I2Cx DNF coefficient */
    tmpreg |= DigitalFilter;

    /* Store the new register value */
    hi2c->Instance->FLTR = tmpreg;

    __HAL_I2C_ENABLE(hi2c);

    hi2c->State = HAL_I2C_STATE_READY;

    return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }
}

/* This function handles I2C Communication Timeout.
 * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
 *         the configuration information for I2C module
 * @param  Flag specifies the I2C flag to check.
 * @param  Status The new Flag status (SET or RESET).
 * @param  Timeout Timeout duration
 * @param  Tickstart Tick start value
 * @retval HAL status
 */
static HAL_StatusTypeDef I2C_WaitOnFlagUntilTimeout(I2C_HandleTypeDef *hi2c, uint32_t Flag, FlagStatus Status, uint32_t Timeout, uint32_t Tickstart)
{
  /* Wait until flag is set */
  while (I2C_GetFlagStatus(hi2c->Instance, Flag) == Status)
  {
    /* Check for the Timeout */
    if (Timeout != HAL_MAX_DELAY)
    {
      if (((xTaskGetTickCount() - Tickstart) > Timeout) || (Timeout == 0U))
      {
        hi2c->PreviousState     = I2C_STATE_NONE;
        hi2c->State             = HAL_I2C_STATE_READY;
        hi2c->Mode              = HAL_I2C_MODE_NONE;
        hi2c->ErrorCode         |= HAL_I2C_ERROR_TIMEOUT;

        /* Process Unlocked */
        __HAL_UNLOCK(hi2c);

        return HAL_ERROR;
      }
    }
  }
  return HAL_OK;
}

/* This function handles I2C Communication Timeout for Master addressing phase.
 * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
 *         the configuration information for I2C module
 * @param  Flag specifies the I2C flag to check.
 * @param  Timeout Timeout duration
 * @param  Tickstart Tick start value
 * @retval HAL status
 */
static HAL_StatusTypeDef I2C_WaitOnMasterAddressFlagUntilTimeout(I2C_HandleTypeDef *hi2c, uint32_t Flag, uint32_t Timeout, uint32_t Tickstart)
{
  while (I2C_GetFlagStatus(hi2c->Instance, Flag) == RESET)
  {
    if (I2C_GetFlagStatus(hi2c->Instance, I2C_FLAG_AF) == SET)
    {
      /* Generate Stop */
      SET_BIT(hi2c->Instance->CR1, I2C_CR1_STOP);

      /* Clear AF Flag */
      __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_AF);

      hi2c->PreviousState       = I2C_STATE_NONE;
      hi2c->State               = HAL_I2C_STATE_READY;
      hi2c->Mode                = HAL_I2C_MODE_NONE;
      hi2c->ErrorCode           |= HAL_I2C_ERROR_AF;

      /* Process Unlocked */
      __HAL_UNLOCK(hi2c);

      return HAL_ERROR;
    }

    /* Check for the Timeout */
    if (Timeout != HAL_MAX_DELAY)
    {
      if (((xTaskGetTickCount() - Tickstart) > Timeout) || (Timeout == 0U))
      {
        hi2c->PreviousState       = I2C_STATE_NONE;
        hi2c->State               = HAL_I2C_STATE_READY;
        hi2c->Mode                = HAL_I2C_MODE_NONE;
        hi2c->ErrorCode           |= HAL_I2C_ERROR_TIMEOUT;

        /* Process Unlocked */
        __HAL_UNLOCK(hi2c);

        return HAL_ERROR;
      }
    }
  }
  return HAL_OK;
}

/* hi2c Pointer to a I2C_HandleTypeDef structure that contains
 *         the configuration information for I2C module
 * @param  DevAddress Target device address: The device 7 bits address value
 *         in datasheet must be shifted to the left before calling the interface
 * @param  Timeout Timeout duration
 * @param  Tickstart Tick start value
 * @retval HAL status
 */
static HAL_StatusTypeDef I2C_MasterRequestWrite(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Timeout, uint32_t Tickstart)
{
  /* Declaration of temporary variable to prevent undefined behavior of volatile usage */
  uint32_t CurrentXferOptions = hi2c->XferOptions;

  /* Generate Start condition if first transfer */
  if ((CurrentXferOptions == I2C_FIRST_AND_LAST_FRAME) || (CurrentXferOptions == I2C_FIRST_FRAME) || (CurrentXferOptions == I2C_NO_OPTION_FRAME))
  {
    /* Generate Start */
    SET_BIT(hi2c->Instance->CR1, I2C_CR1_START);
  }
  else if (hi2c->PreviousState == I2C_STATE_MASTER_BUSY_RX)
  {
    /* Generate ReStart */
    SET_BIT(hi2c->Instance->CR1, I2C_CR1_START);
  }
  else
  {
    /* Do nothing */
  }

  /* Wait until SB flag is set */
  if (I2C_WaitOnFlagUntilTimeout(hi2c, I2C_FLAG_SB, RESET, Timeout, Tickstart) != HAL_OK)
  {
    if (READ_BIT(hi2c->Instance->CR1, I2C_CR1_START) == I2C_CR1_START)
    {
      hi2c->ErrorCode = HAL_I2C_WRONG_START;
    }
    return HAL_TIMEOUT;
  }

  if (hi2c->Init.AddressingMode == I2C_ADDRESSINGMODE_7BIT)
  {
    /* Send slave address */
    hi2c->Instance->DR = I2C_7BIT_ADD_WRITE(DevAddress);
  }
  else
  {
    /* Send header of slave address */
    hi2c->Instance->DR = I2C_10BIT_HEADER_WRITE(DevAddress);

    /* Wait until ADD10 flag is set */
    if (I2C_WaitOnMasterAddressFlagUntilTimeout(hi2c, I2C_FLAG_ADD10, Timeout, Tickstart) != HAL_OK)
    {
      return HAL_ERROR;
    }

    /* Send slave address */
    hi2c->Instance->DR = I2C_10BIT_ADDRESS(DevAddress);
  }

  /* Wait until ADDR flag is set */
  if (I2C_WaitOnMasterAddressFlagUntilTimeout(hi2c, I2C_FLAG_ADDR, Timeout, Tickstart) != HAL_OK)
  {
    return HAL_ERROR;
  }

  return HAL_OK;
}

/* This function handles Acknowledge failed detection during an I2C Communication.
 * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
 *              the configuration information for the specified I2C.
 * @retval HAL status
 */
static HAL_StatusTypeDef I2C_IsAcknowledgeFailed(I2C_HandleTypeDef *hi2c)
{
  if ( I2C_GetFlagStatus(hi2c->Instance, I2C_FLAG_AF) == SET)
  {
    /* Clear NACKF Flag */
    __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_AF);

    hi2c->PreviousState       = I2C_STATE_NONE;
    hi2c->State               = HAL_I2C_STATE_READY;
    hi2c->Mode                = HAL_I2C_MODE_NONE;
    hi2c->ErrorCode           |= HAL_I2C_ERROR_AF;

    /* Process Unlocked */
    __HAL_UNLOCK(hi2c);

    return HAL_ERROR;
  }
  return HAL_OK;
}

/* This function handles I2C Communication Timeout for specific usage of TXE flag.
 * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
 *              the configuration information for the specified I2C.
 * @param  Timeout Timeout duration
 * @param  Tickstart Tick start value
 * @retval HAL status
 */
static HAL_StatusTypeDef I2C_WaitOnTXEFlagUntilTimeout(I2C_HandleTypeDef *hi2c, uint32_t Timeout, uint32_t Tickstart)
{
  while ( I2C_GetFlagStatus(hi2c->Instance, I2C_FLAG_TXE) == RESET)
  {
    /* Check if a NACK is detected */
    if (I2C_IsAcknowledgeFailed(hi2c) != HAL_OK)
    {
      return HAL_ERROR;
    }

    /* Check for the Timeout */
    if (Timeout != HAL_MAX_DELAY)
    {
      if (((xTaskGetTickCount() - Tickstart) > Timeout) || (Timeout == 0U))
      {
        hi2c->PreviousState       = I2C_STATE_NONE;
        hi2c->State               = HAL_I2C_STATE_READY;
        hi2c->Mode                = HAL_I2C_MODE_NONE;
        hi2c->ErrorCode           |= HAL_I2C_ERROR_TIMEOUT;

        /* Process Unlocked */
        __HAL_UNLOCK(hi2c);

        return HAL_ERROR;
      }
    }
  }
  return HAL_OK;
}

/* This function handles I2C Communication Timeout for specific usage of BTF flag.
 * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
 *              the configuration information for the specified I2C.
 * @param  Timeout Timeout duration
 * @param  Tickstart Tick start value
 * @retval HAL status
 */
static HAL_StatusTypeDef I2C_WaitOnBTFFlagUntilTimeout(I2C_HandleTypeDef *hi2c, uint32_t Timeout, uint32_t Tickstart)
{
  while ( I2C_GetFlagStatus(hi2c->Instance, I2C_FLAG_BTF) == RESET)
  {
    /* Check if a NACK is detected */
    if (I2C_IsAcknowledgeFailed(hi2c) != HAL_OK)
    {
      return HAL_ERROR;
    }

    /* Check for the Timeout */
    if (Timeout != HAL_MAX_DELAY)
    {
      if (((xTaskGetTickCount() - Tickstart) > Timeout) || (Timeout == 0U))
      {
        hi2c->PreviousState       = I2C_STATE_NONE;
        hi2c->State               = HAL_I2C_STATE_READY;
        hi2c->Mode                = HAL_I2C_MODE_NONE;
        hi2c->ErrorCode           |= HAL_I2C_ERROR_TIMEOUT;

        /* Process Unlocked */
        __HAL_UNLOCK(hi2c);

        return HAL_ERROR;
      }
    }
  }
  return HAL_OK;
}

/* Transmits in master mode an amount of data in blocking mode.
 * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
 *              the configuration information for the specified I2C.
 * @param  DevAddress Target device address: The device 7 bits address value
 *         in datasheet must be shifted to the left before calling the interface
 * @param  pData Pointer to data buffer
 * @param  Size Amount of data to be sent
 * @param  Timeout Timeout duration
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_I2C_Master_Transmit(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
    /* Init tickstart for timeout management*/
     uint32_t tickstart = xTaskGetTickCount();

     if (hi2c->State == HAL_I2C_STATE_READY)
     {
       /* Wait until BUSY flag is reset */
       if (I2C_WaitOnFlagUntilTimeout(hi2c, I2C_FLAG_BUSY, SET, I2C_TIMEOUT_BUSY_FLAG, tickstart) != HAL_OK)
       {
         return HAL_BUSY;
       }

       /* Process Locked */
       __HAL_LOCK(hi2c);

       /* Check if the I2C is already enabled */
       if ((hi2c->Instance->CR1 & I2C_CR1_PE) != I2C_CR1_PE)
       {
         /* Enable I2C peripheral */
         __HAL_I2C_ENABLE(hi2c);
       }

       /* Disable Pos */
       CLEAR_BIT(hi2c->Instance->CR1, I2C_CR1_POS);

       hi2c->State       = HAL_I2C_STATE_BUSY_TX;
       hi2c->Mode        = HAL_I2C_MODE_MASTER;
       hi2c->ErrorCode   = HAL_I2C_ERROR_NONE;

       /* Prepare transfer parameters */
       hi2c->pBuffPtr    = pData;
       hi2c->XferCount   = Size;
       hi2c->XferSize    = hi2c->XferCount;
       hi2c->XferOptions = I2C_NO_OPTION_FRAME;

       /* Send Slave Address */
       if (I2C_MasterRequestWrite(hi2c, DevAddress, Timeout, tickstart) != HAL_OK)
       {
         return HAL_ERROR;
       }

       /* Clear ADDR flag */
       __HAL_I2C_CLEAR_ADDRFLAG(hi2c);

       while (hi2c->XferSize > 0U)
       {
         /* Wait until TXE flag is set */
         if (I2C_WaitOnTXEFlagUntilTimeout(hi2c, Timeout, tickstart) != HAL_OK)
         {
           if (hi2c->ErrorCode == HAL_I2C_ERROR_AF)
           {
             /* Generate Stop */
             SET_BIT(hi2c->Instance->CR1, I2C_CR1_STOP);
           }
           return HAL_ERROR;
         }

         /* Write data to DR */
         hi2c->Instance->DR = *hi2c->pBuffPtr;

         /* Increment Buffer pointer */
         hi2c->pBuffPtr++;

         /* Update counter */
         hi2c->XferCount--;
         hi2c->XferSize--;

         if (( I2C_GetFlagStatus(hi2c->Instance, I2C_FLAG_BTF) == SET) && (hi2c->XferSize != 0U))
         {
           /* Write data to DR */
           hi2c->Instance->DR = *hi2c->pBuffPtr;

           /* Increment Buffer pointer */
           hi2c->pBuffPtr++;

           /* Update counter */
           hi2c->XferCount--;
           hi2c->XferSize--;
         }

         /* Wait until BTF flag is set */
         if (I2C_WaitOnBTFFlagUntilTimeout(hi2c, Timeout, tickstart) != HAL_OK)
         {
           if (hi2c->ErrorCode == HAL_I2C_ERROR_AF)
           {
             /* Generate Stop */
             SET_BIT(hi2c->Instance->CR1, I2C_CR1_STOP);
           }
           return HAL_ERROR;
         }
       }

       /* Generate Stop */
       SET_BIT(hi2c->Instance->CR1, I2C_CR1_STOP);

       hi2c->State = HAL_I2C_STATE_READY;
       hi2c->Mode = HAL_I2C_MODE_NONE;

       /* Process Unlocked */
       __HAL_UNLOCK(hi2c);

       return HAL_OK;
     }
     else
     {
       return HAL_BUSY;
     }
}

/* Master sends target device address for read request.
 * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
 *         the configuration information for I2C module
 * @param  DevAddress Target device address: The device 7 bits address value
 *         in datasheet must be shifted to the left before calling the interface
 * @param  Timeout Timeout duration
 * @param  Tickstart Tick start value
 * @retval HAL status
 */
static HAL_StatusTypeDef I2C_MasterRequestRead(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Timeout, uint32_t Tickstart)
{
  /* Declaration of temporary variable to prevent undefined behavior of volatile usage */
  uint32_t CurrentXferOptions = hi2c->XferOptions;

  /* Enable Acknowledge */
  SET_BIT(hi2c->Instance->CR1, I2C_CR1_ACK);

  /* Generate Start condition if first transfer */
  if ((CurrentXferOptions == I2C_FIRST_AND_LAST_FRAME) || (CurrentXferOptions == I2C_FIRST_FRAME)  || (CurrentXferOptions == I2C_NO_OPTION_FRAME))
  {
    /* Generate Start */
    SET_BIT(hi2c->Instance->CR1, I2C_CR1_START);
  }
  else if (hi2c->PreviousState == I2C_STATE_MASTER_BUSY_TX)
  {
    /* Generate ReStart */
    SET_BIT(hi2c->Instance->CR1, I2C_CR1_START);
  }
  else
  {
    /* Do nothing */
  }

  /* Wait until SB flag is set */
  if (I2C_WaitOnFlagUntilTimeout(hi2c, I2C_FLAG_SB, RESET, Timeout, Tickstart) != HAL_OK)
  {
    if (READ_BIT(hi2c->Instance->CR1, I2C_CR1_START) == I2C_CR1_START)
    {
      hi2c->ErrorCode = HAL_I2C_WRONG_START;
    }
    return HAL_TIMEOUT;
  }

  if (hi2c->Init.AddressingMode == I2C_ADDRESSINGMODE_7BIT)
  {
    /* Send slave address */
    hi2c->Instance->DR = I2C_7BIT_ADD_READ(DevAddress);
  }
  else
  {
    /* Send header of slave address */
    hi2c->Instance->DR = I2C_10BIT_HEADER_WRITE(DevAddress);

    /* Wait until ADD10 flag is set */
    if (I2C_WaitOnMasterAddressFlagUntilTimeout(hi2c, I2C_FLAG_ADD10, Timeout, Tickstart) != HAL_OK)
    {
      return HAL_ERROR;
    }

    /* Send slave address */
    hi2c->Instance->DR = I2C_10BIT_ADDRESS(DevAddress);

    /* Wait until ADDR flag is set */
    if (I2C_WaitOnMasterAddressFlagUntilTimeout(hi2c, I2C_FLAG_ADDR, Timeout, Tickstart) != HAL_OK)
    {
      return HAL_ERROR;
    }

    /* Clear ADDR flag */
    __HAL_I2C_CLEAR_ADDRFLAG(hi2c);

    /* Generate Restart */
    SET_BIT(hi2c->Instance->CR1, I2C_CR1_START);

    /* Wait until SB flag is set */
    if (I2C_WaitOnFlagUntilTimeout(hi2c, I2C_FLAG_SB, RESET, Timeout, Tickstart) != HAL_OK)
    {
      if (READ_BIT(hi2c->Instance->CR1, I2C_CR1_START) == I2C_CR1_START)
      {
        hi2c->ErrorCode = HAL_I2C_WRONG_START;
      }
      return HAL_TIMEOUT;
    }

    /* Send header of slave address */
    hi2c->Instance->DR = I2C_10BIT_HEADER_READ(DevAddress);
  }

  /* Wait until ADDR flag is set */
  if (I2C_WaitOnMasterAddressFlagUntilTimeout(hi2c, I2C_FLAG_ADDR, Timeout, Tickstart) != HAL_OK)
  {
    return HAL_ERROR;
  }

  return HAL_OK;
}

/* This function handles I2C Communication Timeout for specific usage of RXNE flag.
 * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
 *              the configuration information for the specified I2C.
 * @param  Timeout Timeout duration
 * @param  Tickstart Tick start value
 * @retval HAL status
 */
static HAL_StatusTypeDef I2C_WaitOnRXNEFlagUntilTimeout(I2C_HandleTypeDef *hi2c, uint32_t Timeout, uint32_t Tickstart)
{

  while ( I2C_GetFlagStatus(hi2c->Instance, I2C_FLAG_RXNE) == RESET)
  {
    /* Check if a STOPF is detected */
    if ( I2C_GetFlagStatus(hi2c->Instance, I2C_FLAG_STOPF) == SET)
    {
      /* Clear STOP Flag */
      __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_STOPF);

      hi2c->PreviousState       = I2C_STATE_NONE;
      hi2c->State               = HAL_I2C_STATE_READY;
      hi2c->Mode                = HAL_I2C_MODE_NONE;
      hi2c->ErrorCode           |= HAL_I2C_ERROR_NONE;

      /* Process Unlocked */
      __HAL_UNLOCK(hi2c);

      return HAL_ERROR;
    }

    /* Check for the Timeout */
    if (((xTaskGetTickCount() - Tickstart) > Timeout) || (Timeout == 0U))
    {
      hi2c->PreviousState       = I2C_STATE_NONE;
      hi2c->State               = HAL_I2C_STATE_READY;
      hi2c->Mode                = HAL_I2C_MODE_NONE;
      hi2c->ErrorCode           |= HAL_I2C_ERROR_TIMEOUT;

      /* Process Unlocked */
      __HAL_UNLOCK(hi2c);

      return HAL_ERROR;
    }
  }
  return HAL_OK;
}

/* Receives in master mode an amount of data in blocking mode.
 * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
 *              the configuration information for the specified I2C.
 * @param  DevAddress Target device address: The device 7 bits address value
 *         in datasheet must be shifted to the left before calling the interface
 * @param  pData Pointer to data buffer
 * @param  Size Amount of data to be sent
 * @param  Timeout Timeout duration
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_I2C_Master_Receive(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
      /* Init tickstart for timeout management*/
      uint32_t tickstart = xTaskGetTickCount();

      if (hi2c->State == HAL_I2C_STATE_READY)
      {
        /* Wait until BUSY flag is reset */
        if (I2C_WaitOnFlagUntilTimeout(hi2c, I2C_FLAG_BUSY, SET, I2C_TIMEOUT_BUSY_FLAG, tickstart) != HAL_OK)
        {
          return HAL_BUSY;
        }

        /* Process Locked */
        __HAL_LOCK(hi2c);

        /* Check if the I2C is already enabled */
        if ((hi2c->Instance->CR1 & I2C_CR1_PE) != I2C_CR1_PE)
        {
          /* Enable I2C peripheral */
          __HAL_I2C_ENABLE(hi2c);
        }

        /* Disable Pos */
        CLEAR_BIT(hi2c->Instance->CR1, I2C_CR1_POS);

        hi2c->State       = HAL_I2C_STATE_BUSY_RX;
        hi2c->Mode        = HAL_I2C_MODE_MASTER;
        hi2c->ErrorCode   = HAL_I2C_ERROR_NONE;

        /* Prepare transfer parameters */
        hi2c->pBuffPtr    = pData;
        hi2c->XferCount   = Size;
        hi2c->XferSize    = hi2c->XferCount;
        hi2c->XferOptions = I2C_NO_OPTION_FRAME;

        /* Send Slave Address */
        if (I2C_MasterRequestRead(hi2c, DevAddress, Timeout, tickstart) != HAL_OK)
        {
          return HAL_ERROR;
        }

        if (hi2c->XferSize == 0U)
        {
          /* Clear ADDR flag */
          __HAL_I2C_CLEAR_ADDRFLAG(hi2c);

          /* Generate Stop */
          SET_BIT(hi2c->Instance->CR1, I2C_CR1_STOP);
        }
        else if (hi2c->XferSize == 1U)
        {
          /* Disable Acknowledge */
          CLEAR_BIT(hi2c->Instance->CR1, I2C_CR1_ACK);

          /* Clear ADDR flag */
          __HAL_I2C_CLEAR_ADDRFLAG(hi2c);

          /* Generate Stop */
          SET_BIT(hi2c->Instance->CR1, I2C_CR1_STOP);
        }
        else if (hi2c->XferSize == 2U)
        {
          /* Disable Acknowledge */
          CLEAR_BIT(hi2c->Instance->CR1, I2C_CR1_ACK);

          /* Enable Pos */
          SET_BIT(hi2c->Instance->CR1, I2C_CR1_POS);

          /* Clear ADDR flag */
          __HAL_I2C_CLEAR_ADDRFLAG(hi2c);
        }
        else
        {
          /* Enable Acknowledge */
          SET_BIT(hi2c->Instance->CR1, I2C_CR1_ACK);

          /* Clear ADDR flag */
          __HAL_I2C_CLEAR_ADDRFLAG(hi2c);
        }

        while (hi2c->XferSize > 0U)
        {
          if (hi2c->XferSize <= 3U)
          {
            /* One byte */
            if (hi2c->XferSize == 1U)
            {
              /* Wait until RXNE flag is set */
              if (I2C_WaitOnRXNEFlagUntilTimeout(hi2c, Timeout, tickstart) != HAL_OK)
              {
                return HAL_ERROR;
              }

              /* Read data from DR */
              *hi2c->pBuffPtr = (uint8_t)hi2c->Instance->DR;

              /* Increment Buffer pointer */
              hi2c->pBuffPtr++;

              /* Update counter */
              hi2c->XferSize--;
              hi2c->XferCount--;
            }
            /* Two bytes */
            else if (hi2c->XferSize == 2U)
            {
              /* Wait until BTF flag is set */
              if (I2C_WaitOnFlagUntilTimeout(hi2c, I2C_FLAG_BTF, RESET, Timeout, tickstart) != HAL_OK)
              {
                return HAL_ERROR;
              }

              /* Generate Stop */
              SET_BIT(hi2c->Instance->CR1, I2C_CR1_STOP);

              /* Read data from DR */
              *hi2c->pBuffPtr = (uint8_t)hi2c->Instance->DR;

              /* Increment Buffer pointer */
              hi2c->pBuffPtr++;

              /* Update counter */
              hi2c->XferSize--;
              hi2c->XferCount--;

              /* Read data from DR */
              *hi2c->pBuffPtr = (uint8_t)hi2c->Instance->DR;

              /* Increment Buffer pointer */
              hi2c->pBuffPtr++;

              /* Update counter */
              hi2c->XferSize--;
              hi2c->XferCount--;
            }
            /* 3 Last bytes */
            else
            {
              /* Wait until BTF flag is set */
              if (I2C_WaitOnFlagUntilTimeout(hi2c, I2C_FLAG_BTF, RESET, Timeout, tickstart) != HAL_OK)
              {
                return HAL_ERROR;
              }

              /* Disable Acknowledge */
              CLEAR_BIT(hi2c->Instance->CR1, I2C_CR1_ACK);

              /* Read data from DR */
              *hi2c->pBuffPtr = (uint8_t)hi2c->Instance->DR;

              /* Increment Buffer pointer */
              hi2c->pBuffPtr++;

              /* Update counter */
              hi2c->XferSize--;
              hi2c->XferCount--;

              /* Wait until BTF flag is set */
              if (I2C_WaitOnFlagUntilTimeout(hi2c, I2C_FLAG_BTF, RESET, Timeout, tickstart) != HAL_OK)
              {
                return HAL_ERROR;
              }

              /* Generate Stop */
              SET_BIT(hi2c->Instance->CR1, I2C_CR1_STOP);

              /* Read data from DR */
              *hi2c->pBuffPtr = (uint8_t)hi2c->Instance->DR;

              /* Increment Buffer pointer */
              hi2c->pBuffPtr++;

              /* Update counter */
              hi2c->XferSize--;
              hi2c->XferCount--;

              /* Read data from DR */
              *hi2c->pBuffPtr = (uint8_t)hi2c->Instance->DR;

              /* Increment Buffer pointer */
              hi2c->pBuffPtr++;

              /* Update counter */
              hi2c->XferSize--;
              hi2c->XferCount--;
            }
          }
          else
          {
            /* Wait until RXNE flag is set */
            if (I2C_WaitOnRXNEFlagUntilTimeout(hi2c, Timeout, tickstart) != HAL_OK)
            {
              return HAL_ERROR;
            }

            /* Read data from DR */
            *hi2c->pBuffPtr = (uint8_t)hi2c->Instance->DR;

            /* Increment Buffer pointer */
            hi2c->pBuffPtr++;

            /* Update counter */
            hi2c->XferSize--;
            hi2c->XferCount--;

            if ( I2C_GetFlagStatus(hi2c->Instance, I2C_FLAG_BTF) == SET)
            {
              /* Read data from DR */
              *hi2c->pBuffPtr = (uint8_t)hi2c->Instance->DR;

              /* Increment Buffer pointer */
              hi2c->pBuffPtr++;

              /* Update counter */
              hi2c->XferSize--;
              hi2c->XferCount--;
            }
          }
        }

        hi2c->State = HAL_I2C_STATE_READY;
        hi2c->Mode = HAL_I2C_MODE_NONE;

        /* Process Unlocked */
        __HAL_UNLOCK(hi2c);

        return HAL_OK;
      }
      else
      {
        return HAL_BUSY;
      }
}

/* Slave Tx Transfer completed callback.
 * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
 *                the configuration information for the specified I2C.
 * @retval None
 */
__weak void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hi2c);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_I2C_SlaveTxCpltCallback could be implemented in the user file
   */
}

/* Slave Rx Transfer completed callback.
 * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
 *                the configuration information for the specified I2C.
 * @retval None
 */
__weak void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hi2c);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_I2C_SlaveRxCpltCallback could be implemented in the user file
   */
}

/* I2C error callback.
 * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
 *                the configuration information for the specified I2C.
 * @retval None
 */
__weak void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hi2c);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_I2C_ErrorCallback could be implemented in the user file
   */
}

/* Memory Tx Transfer completed callback.
 * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
 *                the configuration information for the specified I2C.
 * @retval None
 */
__weak void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hi2c);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_I2C_MemTxCpltCallback could be implemented in the user file
   */
}

/* Memory Rx Transfer completed callback.
 * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
 *                the configuration information for the specified I2C.
 * @retval None
 */
__weak void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hi2c);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_I2C_MemRxCpltCallback could be implemented in the user file
   */
}

/* Master Rx Transfer completed callback.
 * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
 *                the configuration information for the specified I2C.
 * @retval None
 */
__weak void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hi2c);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_I2C_MasterRxCpltCallback could be implemented in the user file
   */
}

/* DMA I2C process complete callback.
 * @param  hdma DMA handle
 * @retval None
 */
static void I2C_DMAXferCplt(DMA_HandleTypeDef *hdma)
{
  I2C_HandleTypeDef *hi2c = (I2C_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent; /* Derogation MISRAC2012-Rule-11.5 */

  /* Declaration of temporary variable to prevent undefined behavior of volatile usage */
  HAL_I2C_StateTypeDef CurrentState = hi2c->State;
  HAL_I2C_ModeTypeDef CurrentMode   = hi2c->Mode;
  uint32_t CurrentXferOptions       = hi2c->XferOptions;

  /* Disable EVT and ERR interrupt */
  __HAL_I2C_DISABLE_IT(hi2c, I2C_IT_EVT | I2C_IT_ERR);

  /* Clear Complete callback */
  if (hi2c->hdmatx != NULL)
  {
    hi2c->hdmatx->XferCpltCallback = NULL;
  }
  if (hi2c->hdmarx != NULL)
  {
    hi2c->hdmarx->XferCpltCallback = NULL;
  }

  if ((((uint32_t)CurrentState & (uint32_t)HAL_I2C_STATE_BUSY_TX) == (uint32_t)HAL_I2C_STATE_BUSY_TX) || ((((uint32_t)CurrentState & (uint32_t)HAL_I2C_STATE_BUSY_RX) == (uint32_t)HAL_I2C_STATE_BUSY_RX) && (CurrentMode == HAL_I2C_MODE_SLAVE)))
  {
    /* Disable DMA Request */
    CLEAR_BIT(hi2c->Instance->CR2, I2C_CR2_DMAEN);

    hi2c->XferCount = 0U;

    if (CurrentState == HAL_I2C_STATE_BUSY_TX_LISTEN)
    {
      /* Set state at HAL_I2C_STATE_LISTEN */
      hi2c->PreviousState = I2C_STATE_SLAVE_BUSY_TX;
      hi2c->State = HAL_I2C_STATE_LISTEN;

      /* Call the corresponding callback to inform upper layer of End of Transfer */
#if (USE_HAL_I2C_REGISTER_CALLBACKS == 1)
      hi2c->SlaveTxCpltCallback(hi2c);
#else
      HAL_I2C_SlaveTxCpltCallback(hi2c);
#endif /* USE_HAL_I2C_REGISTER_CALLBACKS */
    }
    else if (CurrentState == HAL_I2C_STATE_BUSY_RX_LISTEN)
    {
      /* Set state at HAL_I2C_STATE_LISTEN */
      hi2c->PreviousState = I2C_STATE_SLAVE_BUSY_RX;
      hi2c->State = HAL_I2C_STATE_LISTEN;

      /* Call the corresponding callback to inform upper layer of End of Transfer */
#if (USE_HAL_I2C_REGISTER_CALLBACKS == 1)
      hi2c->SlaveRxCpltCallback(hi2c);
#else
      HAL_I2C_SlaveRxCpltCallback(hi2c);
#endif /* USE_HAL_I2C_REGISTER_CALLBACKS */
    }
    else
    {
      /* Do nothing */
    }

    /* Enable EVT and ERR interrupt to treat end of transfer in IRQ handler */
    __HAL_I2C_ENABLE_IT(hi2c, I2C_IT_EVT | I2C_IT_ERR);
  }
  /* Check current Mode, in case of treatment DMA handler have been preempted by a prior interrupt */
  else if (hi2c->Mode != HAL_I2C_MODE_NONE)
  {
    if (hi2c->XferCount == (uint16_t)1)
    {
      /* Disable Acknowledge */
      CLEAR_BIT(hi2c->Instance->CR1, I2C_CR1_ACK);
    }

    /* Disable EVT and ERR interrupt */
    __HAL_I2C_DISABLE_IT(hi2c, I2C_IT_EVT | I2C_IT_ERR);

    /* Prepare next transfer or stop current transfer */
    if ((CurrentXferOptions == I2C_NO_OPTION_FRAME) || (CurrentXferOptions == I2C_FIRST_AND_LAST_FRAME) || (CurrentXferOptions == I2C_OTHER_AND_LAST_FRAME) || (CurrentXferOptions == I2C_LAST_FRAME))
    {
      /* Generate Stop */
      SET_BIT(hi2c->Instance->CR1, I2C_CR1_STOP);
    }

    /* Disable Last DMA */
    CLEAR_BIT(hi2c->Instance->CR2, I2C_CR2_LAST);

    /* Disable DMA Request */
    CLEAR_BIT(hi2c->Instance->CR2, I2C_CR2_DMAEN);

    hi2c->XferCount = 0U;

    /* Check if Errors has been detected during transfer */
    if (hi2c->ErrorCode != HAL_I2C_ERROR_NONE)
    {
#if (USE_HAL_I2C_REGISTER_CALLBACKS == 1)
      hi2c->ErrorCallback(hi2c);
#else
      HAL_I2C_ErrorCallback(hi2c);
#endif /* USE_HAL_I2C_REGISTER_CALLBACKS */
    }
    else
    {
      hi2c->State = HAL_I2C_STATE_READY;

      if (hi2c->Mode == HAL_I2C_MODE_MEM)
      {
        hi2c->Mode = HAL_I2C_MODE_NONE;
        hi2c->PreviousState = I2C_STATE_NONE;

#if (USE_HAL_I2C_REGISTER_CALLBACKS == 1)
        hi2c->MemRxCpltCallback(hi2c);
#else
        HAL_I2C_MemRxCpltCallback(hi2c);
#endif /* USE_HAL_I2C_REGISTER_CALLBACKS */
      }
      else
      {
        hi2c->Mode = HAL_I2C_MODE_NONE;
        hi2c->PreviousState = I2C_STATE_MASTER_BUSY_RX;

#if (USE_HAL_I2C_REGISTER_CALLBACKS == 1)
        hi2c->MasterRxCpltCallback(hi2c);
#else
        HAL_I2C_MasterRxCpltCallback(hi2c);
#endif /* USE_HAL_I2C_REGISTER_CALLBACKS */
      }
    }
  }
  else
  {
    /* Do nothing */
  }
}

/* Return the DMA error code
 * @param  hdma  pointer to a DMA_HandleTypeDef structure that contains
 *              the configuration information for the specified DMA Stream.
 * @retval DMA Error Code
 */
uint32_t HAL_DMA_GetError(DMA_HandleTypeDef *hdma)
{
  return hdma->ErrorCode;
}

/* DMA I2C communication error callback.
 * @param  hdma DMA handle
 * @retval None
 */
static void I2C_DMAError(DMA_HandleTypeDef *hdma)
{
  I2C_HandleTypeDef *hi2c = (I2C_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent; /* Derogation MISRAC2012-Rule-11.5 */

  /* Clear Complete callback */
  if (hi2c->hdmatx != NULL)
  {
    hi2c->hdmatx->XferCpltCallback = NULL;
  }
  if (hi2c->hdmarx != NULL)
  {
    hi2c->hdmarx->XferCpltCallback = NULL;
  }

  /* Ignore DMA FIFO error */
  if (HAL_DMA_GetError(hdma) != HAL_DMA_ERROR_FE)
  {
    /* Disable Acknowledge */
    hi2c->Instance->CR1 &= ~I2C_CR1_ACK;

    hi2c->XferCount = 0U;

    hi2c->State = HAL_I2C_STATE_READY;
    hi2c->Mode = HAL_I2C_MODE_NONE;

    hi2c->ErrorCode |= HAL_I2C_ERROR_DMA;

#if (USE_HAL_I2C_REGISTER_CALLBACKS == 1)
    hi2c->ErrorCallback(hi2c);
#else
    HAL_I2C_ErrorCallback(hi2c);
#endif /* USE_HAL_I2C_REGISTER_CALLBACKS */
  }
}

/* Sets the DMA Transfer parameter.
 * @param  hdma       pointer to a DMA_HandleTypeDef structure that contains
 *                     the configuration information for the specified DMA Stream.
 * @param  SrcAddress The source memory Buffer address
 * @param  DstAddress The destination memory Buffer address
 * @param  DataLength The length of data to be transferred from source to destination
 * @retval HAL status
 */
static void DMA_SetConfig(DMA_HandleTypeDef *hdma, uint32_t SrcAddress, uint32_t DstAddress, uint32_t DataLength)
{
  /* Clear DBM bit */
  hdma->Instance->CR &= (uint32_t)(~DMA_SxCR_DBM);

  /* Configure DMA Stream data length */
  hdma->Instance->NDTR = DataLength;

  /* Memory to Peripheral */
  if((hdma->Init.DMA_DIR) == DMA_MEMORY_TO_PERIPH)
  {
    /* Configure DMA Stream destination address */
    hdma->Instance->PAR = DstAddress;

    /* Configure DMA Stream source address */
    hdma->Instance->M0AR = SrcAddress;
  }
  /* Peripheral to Memory */
  else
  {
    /* Configure DMA Stream source address */
    hdma->Instance->PAR = SrcAddress;

    /* Configure DMA Stream destination address */
    hdma->Instance->M0AR = DstAddress;
  }
}

/* Start the DMA Transfer with interrupt enabled.
 * @param  hdma       pointer to a DMA_HandleTypeDef structure that contains
 *                     the configuration information for the specified DMA Stream.
 * @param  SrcAddress The source memory Buffer address
 * @param  DstAddress The destination memory Buffer address
 * @param  DataLength The length of data to be transferred from source to destination
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_DMA_Start_IT(DMA_HandleTypeDef *hdma, uint32_t SrcAddress, uint32_t DstAddress, uint32_t DataLength)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* calculate DMA base and stream number */
  DMA_Base_Registers *regs = (DMA_Base_Registers *)hdma->StreamBaseAddress;

  /* Check the parameters */
  assert_param(IS_DMA_BUFFER_SIZE(DataLength));

  /* Process locked */
  __HAL_LOCK(hdma);

  if(HAL_DMA_STATE_READY == hdma->State)
  {
    /* Change DMA peripheral state */
    hdma->State = HAL_DMA_STATE_BUSY;

    /* Initialize the error code */
    hdma->ErrorCode = HAL_DMA_ERROR_NONE;

    /* Configure the source, destination address and the data length */
    DMA_SetConfig(hdma, SrcAddress, DstAddress, DataLength);

    /* Clear all interrupt flags at correct offset within the register */
    regs->IFCR = 0x3FU << hdma->StreamIndex;

    /* Enable Common interrupts*/
    hdma->Instance->CR  |= DMA_IT_TC | DMA_IT_TE | DMA_IT_DME;

    if(hdma->XferHalfCpltCallback != NULL)
    {
      hdma->Instance->CR  |= DMA_IT_HT;
    }

    /* Enable the Peripheral */
    __HAL_DMA_ENABLE(hdma);
  }
  else
  {
    /* Process unlocked */
    __HAL_UNLOCK(hdma);

    /* Return error status */
    status = HAL_BUSY;
  }

  return status;
}

/* Master sends target device address followed by internal memory address for read request.
 * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
 *         the configuration information for I2C module
 * @param  DevAddress Target device address: The device 7 bits address value
 *         in datasheet must be shifted to the left before calling the interface
 * @param  MemAddress Internal memory address
 * @param  MemAddSize Size of internal memory address
 * @param  Timeout Timeout duration
 * @param  Tickstart Tick start value
 * @retval HAL status
 */
static HAL_StatusTypeDef I2C_RequestMemoryRead(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint32_t Timeout, uint32_t Tickstart)
{
  /* Enable Acknowledge */
  SET_BIT(hi2c->Instance->CR1, I2C_CR1_ACK);

  /* Generate Start */
  SET_BIT(hi2c->Instance->CR1, I2C_CR1_START);

  /* Wait until SB flag is set */
  if (I2C_WaitOnFlagUntilTimeout(hi2c, I2C_FLAG_SB, RESET, Timeout, Tickstart) != HAL_OK)
  {
    if (READ_BIT(hi2c->Instance->CR1, I2C_CR1_START) == I2C_CR1_START)
    {
      hi2c->ErrorCode = HAL_I2C_WRONG_START;
    }
    return HAL_TIMEOUT;
  }

  /* Send slave address */
  hi2c->Instance->DR = I2C_7BIT_ADD_WRITE(DevAddress);

  /* Wait until ADDR flag is set */
  if (I2C_WaitOnMasterAddressFlagUntilTimeout(hi2c, I2C_FLAG_ADDR, Timeout, Tickstart) != HAL_OK)
  {
    return HAL_ERROR;
  }

  /* Clear ADDR flag */
  __HAL_I2C_CLEAR_ADDRFLAG(hi2c);

  /* Wait until TXE flag is set */
  if (I2C_WaitOnTXEFlagUntilTimeout(hi2c, Timeout, Tickstart) != HAL_OK)
  {
    if (hi2c->ErrorCode == HAL_I2C_ERROR_AF)
    {
      /* Generate Stop */
      SET_BIT(hi2c->Instance->CR1, I2C_CR1_STOP);
    }
    return HAL_ERROR;
  }

  /* If Memory address size is 8Bit */
  if (MemAddSize == I2C_MEMADD_SIZE_8BIT)
  {
    /* Send Memory Address */
    hi2c->Instance->DR = I2C_MEM_ADD_LSB(MemAddress);
  }
  /* If Memory address size is 16Bit */
  else
  {
    /* Send MSB of Memory Address */
    hi2c->Instance->DR = I2C_MEM_ADD_MSB(MemAddress);

    /* Wait until TXE flag is set */
    if (I2C_WaitOnTXEFlagUntilTimeout(hi2c, Timeout, Tickstart) != HAL_OK)
    {
      if (hi2c->ErrorCode == HAL_I2C_ERROR_AF)
      {
        /* Generate Stop */
        SET_BIT(hi2c->Instance->CR1, I2C_CR1_STOP);
      }
      return HAL_ERROR;
    }

    /* Send LSB of Memory Address */
    hi2c->Instance->DR = I2C_MEM_ADD_LSB(MemAddress);
  }

  /* Wait until TXE flag is set */
  if (I2C_WaitOnTXEFlagUntilTimeout(hi2c, Timeout, Tickstart) != HAL_OK)
  {
    if (hi2c->ErrorCode == HAL_I2C_ERROR_AF)
    {
      /* Generate Stop */
      SET_BIT(hi2c->Instance->CR1, I2C_CR1_STOP);
    }
    return HAL_ERROR;
  }

  /* Generate Restart */
  SET_BIT(hi2c->Instance->CR1, I2C_CR1_START);

  /* Wait until SB flag is set */
  if (I2C_WaitOnFlagUntilTimeout(hi2c, I2C_FLAG_SB, RESET, Timeout, Tickstart) != HAL_OK)
  {
    if (READ_BIT(hi2c->Instance->CR1, I2C_CR1_START) == I2C_CR1_START)
    {
      hi2c->ErrorCode = HAL_I2C_WRONG_START;
    }
    return HAL_TIMEOUT;
  }

  /* Send slave address */
  hi2c->Instance->DR = I2C_7BIT_ADD_READ(DevAddress);

  /* Wait until ADDR flag is set */
  if (I2C_WaitOnMasterAddressFlagUntilTimeout(hi2c, I2C_FLAG_ADDR, Timeout, Tickstart) != HAL_OK)
  {
    return HAL_ERROR;
  }

  return HAL_OK;
}

/* Aborts the DMA Transfer in Interrupt mode.
 * @param  hdma   pointer to a DMA_HandleTypeDef structure that contains
 *                 the configuration information for the specified DMA Stream.
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_DMA_Abort_IT(DMA_HandleTypeDef *hdma)
{
  if(hdma->State != HAL_DMA_STATE_BUSY)
  {
    hdma->ErrorCode = HAL_DMA_ERROR_NO_XFER;
    return HAL_ERROR;
  }
  else
  {
    /* Set Abort State  */
    hdma->State = HAL_DMA_STATE_ABORT;

    /* Disable the stream */
    __HAL_DMA_DISABLE(hdma);
  }

  return HAL_OK;
}

/* Master sends target device address followed by internal memory address for write request.
 * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
 *         the configuration information for I2C module
 * @param  DevAddress Target device address: The device 7 bits address value
 *         in datasheet must be shifted to the left before calling the interface
 * @param  MemAddress Internal memory address
 * @param  MemAddSize Size of internal memory address
 * @param  Timeout Timeout duration
 * @param  Tickstart Tick start value
 * @retval HAL status
 */
static HAL_StatusTypeDef I2C_RequestMemoryWrite(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint32_t Timeout, uint32_t Tickstart)
{
  /* Generate Start */
  SET_BIT(hi2c->Instance->CR1, I2C_CR1_START);

  /* Wait until SB flag is set */
  if (I2C_WaitOnFlagUntilTimeout(hi2c, I2C_FLAG_SB, RESET, Timeout, Tickstart) != HAL_OK)
  {
    if (READ_BIT(hi2c->Instance->CR1, I2C_CR1_START) == I2C_CR1_START)
    {
      hi2c->ErrorCode = HAL_I2C_WRONG_START;
    }
    return HAL_TIMEOUT;
  }

  /* Send slave address */
  hi2c->Instance->DR = I2C_7BIT_ADD_WRITE(DevAddress);

  /* Wait until ADDR flag is set */
  if (I2C_WaitOnMasterAddressFlagUntilTimeout(hi2c, I2C_FLAG_ADDR, Timeout, Tickstart) != HAL_OK)
  {
    return HAL_ERROR;
  }

  /* Clear ADDR flag */
  __HAL_I2C_CLEAR_ADDRFLAG(hi2c);

  /* Wait until TXE flag is set */
  if (I2C_WaitOnTXEFlagUntilTimeout(hi2c, Timeout, Tickstart) != HAL_OK)
  {
    if (hi2c->ErrorCode == HAL_I2C_ERROR_AF)
    {
      /* Generate Stop */
      SET_BIT(hi2c->Instance->CR1, I2C_CR1_STOP);
    }
    return HAL_ERROR;
  }

  /* If Memory address size is 8Bit */
  if (MemAddSize == I2C_MEMADD_SIZE_8BIT)
  {
    /* Send Memory Address */
    hi2c->Instance->DR = I2C_MEM_ADD_LSB(MemAddress);
  }
  /* If Memory address size is 16Bit */
  else
  {
    /* Send MSB of Memory Address */
    hi2c->Instance->DR = I2C_MEM_ADD_MSB(MemAddress);

    /* Wait until TXE flag is set */
    if (I2C_WaitOnTXEFlagUntilTimeout(hi2c, Timeout, Tickstart) != HAL_OK)
    {
      if (hi2c->ErrorCode == HAL_I2C_ERROR_AF)
      {
        /* Generate Stop */
        SET_BIT(hi2c->Instance->CR1, I2C_CR1_STOP);
      }
      return HAL_ERROR;
    }

    /* Send LSB of Memory Address */
    hi2c->Instance->DR = I2C_MEM_ADD_LSB(MemAddress);
  }

  return HAL_OK;
}

/* Write an amount of data in non-blocking mode with DMA to a specific memory address
 * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
 *                the configuration information for the specified I2C.
 * @param  DevAddress Target device address: The device 7 bits address value
 *         in datasheet must be shifted to the left before calling the interface
 * @param  MemAddress Internal memory address
 * @param  MemAddSize Size of internal memory address
 * @param  pData Pointer to data buffer
 * @param  Size Amount of data to be sent
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_I2C_Mem_Write_DMA(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size)
{
  __IO uint32_t count = 0U;
  HAL_StatusTypeDef dmaxferstatus;

  /* Init tickstart for timeout management*/
  uint32_t tickstart = xTaskGetTickCount();

  /* Check the parameters */
  assert_param(IS_I2C_MEMADD_SIZE(MemAddSize));

  if (hi2c->State == HAL_I2C_STATE_READY)
  {
    /* Wait until BUSY flag is reset */
    count = I2C_TIMEOUT_BUSY_FLAG * (SystemCoreClock / 25U / 1000U);
    do
    {
      count--;
      if (count == 0U)
      {
        hi2c->PreviousState       = I2C_STATE_NONE;
        hi2c->State               = HAL_I2C_STATE_READY;
        hi2c->Mode                = HAL_I2C_MODE_NONE;
        hi2c->ErrorCode           |= HAL_I2C_ERROR_TIMEOUT;

        /* Process Unlocked */
        __HAL_UNLOCK(hi2c);

        return HAL_ERROR;
      }
    }
    while (I2C_GetFlagStatus(hi2c->Instance, I2C_FLAG_BUSY) != RESET);

    /* Process Locked */
    __HAL_LOCK(hi2c);

    /* Check if the I2C is already enabled */
    if ((hi2c->Instance->CR1 & I2C_CR1_PE) != I2C_CR1_PE)
    {
      /* Enable I2C peripheral */
      __HAL_I2C_ENABLE(hi2c);
    }

    /* Disable Pos */
    CLEAR_BIT(hi2c->Instance->CR1, I2C_CR1_POS);

    hi2c->State     = HAL_I2C_STATE_BUSY_TX;
    hi2c->Mode      = HAL_I2C_MODE_MEM;
    hi2c->ErrorCode = HAL_I2C_ERROR_NONE;

    /* Prepare transfer parameters */
    hi2c->pBuffPtr    = pData;
    hi2c->XferCount   = Size;
    hi2c->XferSize    = hi2c->XferCount;
    hi2c->XferOptions = I2C_NO_OPTION_FRAME;

    if (hi2c->XferSize > 0U)
    {
      if (hi2c->hdmatx != NULL)
      {
        /* Set the I2C DMA transfer complete callback */
        hi2c->hdmatx->XferCpltCallback = I2C_DMAXferCplt;

        /* Set the DMA error callback */
        hi2c->hdmatx->XferErrorCallback = I2C_DMAError;

        /* Set the unused DMA callbacks to NULL */
        hi2c->hdmatx->XferHalfCpltCallback = NULL;
        hi2c->hdmatx->XferM1CpltCallback = NULL;
        hi2c->hdmatx->XferM1HalfCpltCallback = NULL;
        hi2c->hdmatx->XferAbortCallback = NULL;

        /* Enable the DMA stream */
        dmaxferstatus = HAL_DMA_Start_IT(hi2c->hdmatx, (uint32_t)hi2c->pBuffPtr, (uint32_t)&hi2c->Instance->DR, hi2c->XferSize);
      }
      else
      {
        /* Update I2C state */
        hi2c->State     = HAL_I2C_STATE_READY;
        hi2c->Mode      = HAL_I2C_MODE_NONE;

        /* Update I2C error code */
        hi2c->ErrorCode |= HAL_I2C_ERROR_DMA_PARAM;

        /* Process Unlocked */
        __HAL_UNLOCK(hi2c);

        return HAL_ERROR;
      }

      if (dmaxferstatus == HAL_OK)
      {
        /* Send Slave Address and Memory Address */
        if (I2C_RequestMemoryWrite(hi2c, DevAddress, MemAddress, MemAddSize, I2C_TIMEOUT_FLAG, tickstart) != HAL_OK)
        {
          /* Abort the ongoing DMA */
          dmaxferstatus = HAL_DMA_Abort_IT(hi2c->hdmatx);

          /* Prevent unused argument(s) compilation and MISRA warning */
          UNUSED(dmaxferstatus);

          /* Set the unused I2C DMA transfer complete callback to NULL */
          hi2c->hdmatx->XferCpltCallback = NULL;

          /* Disable Acknowledge */
          CLEAR_BIT(hi2c->Instance->CR1, I2C_CR1_ACK);

          hi2c->XferSize = 0U;
          hi2c->XferCount = 0U;

          /* Disable I2C peripheral to prevent dummy data in buffer */
          __HAL_I2C_DISABLE(hi2c);

          return HAL_ERROR;
        }

        /* Clear ADDR flag */
        __HAL_I2C_CLEAR_ADDRFLAG(hi2c);

        /* Process Unlocked */
        __HAL_UNLOCK(hi2c);

        /* Note : The I2C interrupts must be enabled after unlocking current process
        to avoid the risk of I2C interrupt handle execution before current
        process unlock */
        /* Enable ERR interrupt */
        __HAL_I2C_ENABLE_IT(hi2c, I2C_IT_ERR);

        /* Enable DMA Request */
        SET_BIT(hi2c->Instance->CR2, I2C_CR2_DMAEN);

        return HAL_OK;
      }
      else
      {
        /* Update I2C state */
        hi2c->State     = HAL_I2C_STATE_READY;
        hi2c->Mode      = HAL_I2C_MODE_NONE;

        /* Update I2C error code */
        hi2c->ErrorCode |= HAL_I2C_ERROR_DMA;

        /* Process Unlocked */
        __HAL_UNLOCK(hi2c);

        return HAL_ERROR;
      }
    }
    else
    {
      /* Update I2C state */
      hi2c->State     = HAL_I2C_STATE_READY;
      hi2c->Mode      = HAL_I2C_MODE_NONE;

      /* Update I2C error code */
      hi2c->ErrorCode |= HAL_I2C_ERROR_SIZE;

      /* Process Unlocked */
      __HAL_UNLOCK(hi2c);

      return HAL_ERROR;
    }
  }
  else
  {
    return HAL_BUSY;
  }
}

/* Reads an amount of data in non-blocking mode with DMA from a specific memory address.
 * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
 *              the configuration information for the specified I2C.
 * @param  DevAddress Target device address: The device 7 bits address value
 *         in datasheet must be shifted to the left before calling the interface
 * @param  MemAddress Internal memory address
 * @param  MemAddSize Size of internal memory address
 * @param  pData Pointer to data buffer
 * @param  Size Amount of data to be read
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_I2C_Mem_Read_DMA(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size)
{
  /* Init tickstart for timeout management*/
  uint32_t tickstart = xTaskGetTickCount();
  __IO uint32_t count = 0U;
  HAL_StatusTypeDef dmaxferstatus;

  /* Check the parameters */
  assert_param(IS_I2C_MEMADD_SIZE(MemAddSize));

  if (hi2c->State == HAL_I2C_STATE_READY)
  {
    /* Wait until BUSY flag is reset */
    count = I2C_TIMEOUT_BUSY_FLAG * (SystemCoreClock / 25U / 1000U);
    do
    {
      count--;
      if (count == 0U)
      {
        hi2c->PreviousState       = I2C_STATE_NONE;
        hi2c->State               = HAL_I2C_STATE_READY;
        hi2c->Mode                = HAL_I2C_MODE_NONE;
        hi2c->ErrorCode           |= HAL_I2C_ERROR_TIMEOUT;

        /* Process Unlocked */
        __HAL_UNLOCK(hi2c);

        return HAL_ERROR;
      }
    }
    while (I2C_GetFlagStatus(hi2c->Instance, I2C_FLAG_BUSY) != RESET);

    /* Process Locked */
    __HAL_LOCK(hi2c);

    /* Check if the I2C is already enabled */
    if ((hi2c->Instance->CR1 & I2C_CR1_PE) != I2C_CR1_PE)
    {
      /* Enable I2C peripheral */
      __HAL_I2C_ENABLE(hi2c);
    }

    /* Disable Pos */
    CLEAR_BIT(hi2c->Instance->CR1, I2C_CR1_POS);

    hi2c->State     = HAL_I2C_STATE_BUSY_RX;
    hi2c->Mode      = HAL_I2C_MODE_MEM;
    hi2c->ErrorCode = HAL_I2C_ERROR_NONE;

    /* Prepare transfer parameters */
    hi2c->pBuffPtr    = pData;
    hi2c->XferCount   = Size;
    hi2c->XferSize    = hi2c->XferCount;
    hi2c->XferOptions = I2C_NO_OPTION_FRAME;

    if (hi2c->XferSize > 0U)
    {
      if (hi2c->hdmarx != NULL)
      {
        /* Set the I2C DMA transfer complete callback */
        hi2c->hdmarx->XferCpltCallback = I2C_DMAXferCplt;

        /* Set the DMA error callback */
        hi2c->hdmarx->XferErrorCallback = I2C_DMAError;

        /* Set the unused DMA callbacks to NULL */
        hi2c->hdmarx->XferHalfCpltCallback = NULL;
        hi2c->hdmarx->XferM1CpltCallback = NULL;
        hi2c->hdmarx->XferM1HalfCpltCallback = NULL;
        hi2c->hdmarx->XferAbortCallback = NULL;

        /* Enable the DMA stream */
        dmaxferstatus = HAL_DMA_Start_IT(hi2c->hdmarx, (uint32_t)&hi2c->Instance->DR, (uint32_t)hi2c->pBuffPtr, hi2c->XferSize);
      }
      else
      {
        /* Update I2C state */
        hi2c->State     = HAL_I2C_STATE_READY;
        hi2c->Mode      = HAL_I2C_MODE_NONE;

        /* Update I2C error code */
        hi2c->ErrorCode |= HAL_I2C_ERROR_DMA_PARAM;

        /* Process Unlocked */
        __HAL_UNLOCK(hi2c);

        return HAL_ERROR;
      }

      if (dmaxferstatus == HAL_OK)
      {
        /* Send Slave Address and Memory Address */
        if (I2C_RequestMemoryRead(hi2c, DevAddress, MemAddress, MemAddSize, I2C_TIMEOUT_FLAG, tickstart) != HAL_OK)
        {
          /* Abort the ongoing DMA */
          dmaxferstatus = HAL_DMA_Abort_IT(hi2c->hdmarx);

          /* Prevent unused argument(s) compilation and MISRA warning */
          UNUSED(dmaxferstatus);

          /* Set the unused I2C DMA transfer complete callback to NULL */
          hi2c->hdmarx->XferCpltCallback = NULL;

          /* Disable Acknowledge */
          CLEAR_BIT(hi2c->Instance->CR1, I2C_CR1_ACK);

          hi2c->XferSize = 0U;
          hi2c->XferCount = 0U;

          /* Disable I2C peripheral to prevent dummy data in buffer */
          __HAL_I2C_DISABLE(hi2c);

          return HAL_ERROR;
        }

        if (hi2c->XferSize == 1U)
        {
          /* Disable Acknowledge */
          CLEAR_BIT(hi2c->Instance->CR1, I2C_CR1_ACK);
        }
        else
        {
          /* Enable Last DMA bit */
          SET_BIT(hi2c->Instance->CR2, I2C_CR2_LAST);
        }

        /* Clear ADDR flag */
        __HAL_I2C_CLEAR_ADDRFLAG(hi2c);

        /* Process Unlocked */
        __HAL_UNLOCK(hi2c);

        /* Note : The I2C interrupts must be enabled after unlocking current process
        to avoid the risk of I2C interrupt handle execution before current
        process unlock */
        /* Enable ERR interrupt */
        __HAL_I2C_ENABLE_IT(hi2c, I2C_IT_ERR);

        /* Enable DMA Request */
        hi2c->Instance->CR2 |= I2C_CR2_DMAEN;
      }
      else
      {
        /* Update I2C state */
        hi2c->State     = HAL_I2C_STATE_READY;
        hi2c->Mode      = HAL_I2C_MODE_NONE;

        /* Update I2C error code */
        hi2c->ErrorCode |= HAL_I2C_ERROR_DMA;

        /* Process Unlocked */
        __HAL_UNLOCK(hi2c);

        return HAL_ERROR;
      }
    }
    else
    {
      /* Send Slave Address and Memory Address */
      if (I2C_RequestMemoryRead(hi2c, DevAddress, MemAddress, MemAddSize, I2C_TIMEOUT_FLAG, tickstart) != HAL_OK)
      {
        return HAL_ERROR;
      }

      /* Clear ADDR flag */
      __HAL_I2C_CLEAR_ADDRFLAG(hi2c);

      /* Generate Stop */
      SET_BIT(hi2c->Instance->CR1, I2C_CR1_STOP);

      hi2c->State = HAL_I2C_STATE_READY;

      /* Process Unlocked */
      __HAL_UNLOCK(hi2c);
    }

    return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }
}
