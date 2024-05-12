/**
  ******************************************************************************
  * @file    mfxstm32l152.c
  * @author  MCD Application Team
  * @brief   This file provides a set of functions needed to manage the MFXSTM32L152
  *          IO Expander devices.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2018-2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#include "mfxstm32l152.h"

/*******************************************************************************
  * Function Name : mfxstm32l152_read_reg
  * Description   : Generic Reading function. It must be fulfilled with either
  *                 I2C or SPI reading functions
  * Input         : Register Address, length of buffer
  * Output        : Data Read
  *******************************************************************************/
static int32_t mfxstm32l152_read_reg(mfxstm32l152_ctx_t *ctx, uint16_t reg, uint8_t *data, uint16_t length)
{
  return ctx->ReadReg(ctx->handle, reg, data, length);
}

/*******************************************************************************
  * Function Name : mfxstm32l152_write_reg
  * Description   : Generic Writing function. It must be fulfilled with either
  *                 I2C or SPI writing function
  * Input         : Register Address, Data to be written, length of buffer
  * Output        : None
  *******************************************************************************/
static int32_t mfxstm32l152_write_reg(mfxstm32l152_ctx_t *ctx, uint16_t reg, uint8_t *data, uint16_t length)
{
  return ctx->WriteReg(ctx->handle, reg, data, length);
}

/**
  * @brief  Internal routine
  * @param  pObj   Pointer to component object.
  * @param  RegisterAddr: Register Address
  * @param  PinPosition: Pin [0:23]
  * @param  PinValue: 0/1
  * @retval Component status
  */
static int32_t MFXSTM32L152_reg24_setPinValue(MFXSTM32L152_Object_t *pObj, uint8_t RegisterAddr, uint32_t PinPosition,
                                              uint8_t PinValue)
{
  int32_t ret = MFXSTM32L152_OK;
  uint8_t tmp;
  uint8_t pin_0_7;
  uint8_t pin_8_15;
  uint8_t pin_16_23;

  pin_0_7   = (uint8_t)(PinPosition & 0x0000ffU);
  pin_8_15  = (uint8_t)(PinPosition >> 8);
  pin_8_15  = (uint8_t)(pin_8_15 & 0x00ffU);
  pin_16_23 = (uint8_t)(PinPosition >> 16);

  if (pin_0_7 != 0U)
  {
    /* Get the current register value */
    ret += mfxstm32l152_read_reg(&pObj->Ctx, RegisterAddr, &tmp, 1);

    /* Set the selected pin direction */
    if (PinValue != 0U)
    {
      tmp |= (uint8_t)pin_0_7;
    }
    else
    {
      tmp &= ~(uint8_t)pin_0_7;
    }

    /* Set the new register value */
    ret += mfxstm32l152_write_reg(&pObj->Ctx, RegisterAddr, &tmp, 1);
  }

  if (pin_8_15 != 0U)
  {
    /* Get the current register value */
    ret += mfxstm32l152_read_reg(&pObj->Ctx, ((uint16_t)RegisterAddr + 1U), &tmp, 1);

    /* Set the selected pin direction */
    if (PinValue != 0U)
    {
      tmp |= (uint8_t)pin_8_15;
    }
    else
    {
      tmp &= ~(uint8_t)pin_8_15;
    }

    /* Set the new register value */
    ret += mfxstm32l152_write_reg(&pObj->Ctx, ((uint16_t)RegisterAddr + 1U), &tmp, 1);
  }

  if (pin_16_23 != 0U)
  {
    /* Get the current register value */
    ret += mfxstm32l152_read_reg(&pObj->Ctx, ((uint16_t)RegisterAddr + 2U), &tmp, 1);

    /* Set the selected pin direction */
    if (PinValue != 0U)
    {
      tmp |= (uint8_t)pin_16_23;
    }
    else
    {
      tmp &= ~(uint8_t)pin_16_23;
    }

    /* Set the new register value */
    ret += mfxstm32l152_write_reg(&pObj->Ctx, ((uint16_t)RegisterAddr + 2U), &tmp, 1);
  }

  if (ret != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }

  return ret;
}

/**
  * @brief  Start the IO functionality used and enable the AF for selected IO pin(s).
  * @param  pObj   Pointer to component object.
  * @param  IO_Pin IO pin
  * @retval Component status
  */
int32_t MFXSTM32L152_IO_Start(MFXSTM32L152_Object_t *pObj, uint32_t IO_Pin)
{
  int32_t ret = MFXSTM32L152_OK;
  uint8_t mode;

  /* Get the current register value */
  if (mfxstm32l152_read_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_SYS_CTRL, &mode, 1) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }
  else
  {
    /* Set the IO Functionalities to be Enabled */
    mode |= MFXSTM32L152_GPIO_EN;

    /* Enable ALTERNATE functions */
    /* AGPIO[0..3] can be either IDD or GPIO */
    /* AGPIO[4..7] can be either TS or GPIO */
    /* if IDD or TS are enabled no matter the value this bit GPIO are not available for those pins */
    /*  however the MFX will waste some cycles to to handle these potential GPIO (pooling, etc) */
    /* so if IDD and TS are both active it is better to let ALTERNATE off (0) */
    /* if however IDD or TS are not connected then set it on gives more GPIOs availability */
    /* remind that AGPIO are less efficient then normal GPIO (They use pooling rather then EXTI */
    if (IO_Pin > 0xFFFFU)
    {
      mode |= MFXSTM32L152_ALTERNATE_GPIO_EN;
    }
    else
    {
      mode &= ~MFXSTM32L152_ALTERNATE_GPIO_EN;
    }

    /* Write the new register value */
    if (mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_SYS_CTRL, &mode, 1) != MFXSTM32L152_OK)
    {
      ret = MFXSTM32L152_ERROR;
    }
  }

  return ret;
}

/**
  * @brief  Initialize the selected IO pin direction.
  * @param  pObj   Pointer to component object.
  * @param  IO_Pin The IO pin to be configured. This parameter could be any
  *         combination of the following values:
  *   @arg  MFXSTM32L152_GPIO_PIN_x: Where x can be from 0 to 23.
  * @param  Direction could be MFXSTM32L152_GPIO_DIR_IN or MFXSTM32L152_GPIO_DIR_OUT.
  * @retval Component status
  */
int32_t MFXSTM32L152_IO_InitPin(MFXSTM32L152_Object_t *pObj, uint32_t IO_Pin, uint8_t Direction)
{
  int32_t ret = MFXSTM32L152_OK;

  if (MFXSTM32L152_reg24_setPinValue(pObj, MFXSTM32L152_REG_ADR_GPIO_DIR1, IO_Pin, Direction) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }

  return ret;
}

/**
  * @brief  Set the global interrupt Type.
  * @param  pObj   Pointer to component object.
  * @param  IO_Pin The IO pin to be configured. This parameter could be any
  *         combination of the following values:
  *   @arg  MFXSTM32L152_GPIO_PIN_x: Where x can be from 0 to 23.
  * @param  Evt: Interrupt line activity type, could be one of the following values:
  *   @arg  MFXSTM32L152_IRQ_GPI_EVT_LEVEL: Interrupt line is active in level model
  *   @arg  MFXSTM32L152_IRQ_GPI_EVT_EDGE: Interrupt line is active in edge model
  * @retval Component status
  */
int32_t MFXSTM32L152_IO_SetIrqEvtMode(MFXSTM32L152_Object_t *pObj, uint32_t IO_Pin, uint8_t Evt)
{
  int32_t ret = MFXSTM32L152_OK;

  if (MFXSTM32L152_reg24_setPinValue(pObj, MFXSTM32L152_REG_ADR_IRQ_GPI_EVT1, IO_Pin, Evt) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }

  return ret;
}

/**
  * @brief  Configure the Edge for which a transition is detectable for the
  *         selected pin.
  * @param  pObj   Pointer to component object.
  * @param  IO_Pin The IO pin to be configured. This parameter could be any
  *         combination of the following values:
  *   @arg  MFXSTM32L152_GPIO_PIN_x: Where x can be from 0 to 23.
  * @param  Evt: Interrupt line activity type, could be one of the following values:
  *   @arg  MFXSTM32L152_IRQ_GPI_TYPE_LLFE: Interrupt line is active in Low Level or Falling Edge
  *   @arg  MFXSTM32L152_IRQ_GPI_TYPE_HLRE: Interrupt line is active in High Level or Rising Edge
  * @retval Component status
  */
int32_t MFXSTM32L152_IO_SetIrqTypeMode(MFXSTM32L152_Object_t *pObj, uint32_t IO_Pin, uint8_t Type)
{
  int32_t ret = MFXSTM32L152_OK;

  if (MFXSTM32L152_reg24_setPinValue(pObj, MFXSTM32L152_REG_ADR_IRQ_GPI_TYPE1, IO_Pin, Type) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }

  return ret;
}

/**
  * @brief  When GPIO is in output mode, puts the corresponding GPO in High (1) or Low (0) level.
  * @param  pObj   Pointer to component object.
  * @param  IO_Pin The output pin to be set or reset. This parameter can be one
  *         of the following values:
  *   @arg  MFXSTM32L152_GPIO_PIN_x: where x can be from 0 to 23.
  * @param PinState: The new IO pin state.
  * @retval Component status
  */
int32_t MFXSTM32L152_IO_WritePin(MFXSTM32L152_Object_t *pObj, uint32_t IO_Pin, uint8_t PinState)
{
  int32_t ret = MFXSTM32L152_OK;

  /* Apply the bit value to the selected pin */
  if (PinState != 0U)
  {
    /* Set the SET register */
    if (MFXSTM32L152_reg24_setPinValue(pObj, MFXSTM32L152_REG_ADR_GPO_SET1, IO_Pin, 1) != MFXSTM32L152_OK)
    {
      ret = MFXSTM32L152_ERROR;
    }
  }
  else
  {
    /* Set the CLEAR register */
    if (MFXSTM32L152_reg24_setPinValue(pObj, MFXSTM32L152_REG_ADR_GPO_CLR1, IO_Pin, 1) != MFXSTM32L152_OK)
    {
      ret = MFXSTM32L152_ERROR;
    }
  }

  return ret;
}

/**
  * @brief  Return the state of the selected IO pin(s).
  * @param  pObj   Pointer to component object.
  * @param  IO_Pin The output pin to read its state. This parameter can be one
  *         of the following values:
  *   @arg  MFXSTM32L152_GPIO_PIN_x: where x can be from 0 to 23.
  * @retval IO pin(s) state.
  */
int32_t MFXSTM32L152_IO_ReadPin(MFXSTM32L152_Object_t *pObj, uint32_t IO_Pin)
{
  uint8_t tmpreg[3];
  uint32_t tmp;

  if (mfxstm32l152_read_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_GPIO_STATE1, &tmpreg[0], 1) != MFXSTM32L152_OK)
  {
    return MFXSTM32L152_ERROR;
  }
  if (mfxstm32l152_read_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_GPIO_STATE2, &tmpreg[1], 1) != MFXSTM32L152_OK)
  {
    return MFXSTM32L152_ERROR;
  }
  if (mfxstm32l152_read_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_GPIO_STATE3, &tmpreg[2], 1) != MFXSTM32L152_OK)
  {
    return MFXSTM32L152_ERROR;
  }
  tmp = ((uint32_t)tmpreg[0] + ((uint32_t)tmpreg[1] << 8) + ((uint32_t)tmpreg[2] << 16));
  tmp &= IO_Pin;

  return (int32_t)(tmp);
}

/**
  * @brief  Enable the interrupt mode for the selected IT source
  * @param  pObj   Pointer to component object.
  * @param Source: The interrupt source to be configured, could be:
  *   @arg  MFXSTM32L152_IRQ_GPIO: IO interrupt
  *   @arg  MFXSTM32L152_IRQ_IDD : IDD interrupt
  *   @arg  MFXSTM32L152_IRQ_ERROR : Error interrupt
  *   @arg  MFXSTM32L152_IRQ_TS_DET : Touch Screen Controller Touch Detected interrupt
  *   @arg  MFXSTM32L152_IRQ_TS_NE : Touch Screen FIFO Not Empty
  *   @arg  MFXSTM32L152_IRQ_TS_TH : Touch Screen FIFO threshold triggered
  *   @arg  MFXSTM32L152_IRQ_TS_FULL : Touch Screen FIFO Full
  *   @arg  MFXSTM32L152_IRQ_TS_OVF : Touch Screen FIFO Overflow
  * @retval Component status
  */
int32_t MFXSTM32L152_EnableITSource(MFXSTM32L152_Object_t *pObj, uint8_t Source)
{
  int32_t ret = MFXSTM32L152_OK;
  uint8_t tmp;

  /* Get the current value of the INT_EN register */
  if (mfxstm32l152_read_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IRQ_SRC_EN, &tmp, 1) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }
  else
  {
    /* Set the interrupts to be Enabled */
    tmp |= Source;

    /* Set the register */
    if (mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IRQ_SRC_EN, &tmp, 1) != MFXSTM32L152_OK)
    {
      ret = MFXSTM32L152_ERROR;
    }
  }

  return ret;
}

/**
  * @brief  Disable the interrupt mode for the selected IT source
  * @param  pObj   Pointer to component object.
  * @param  Source: The interrupt source to be configured, could be:
  *   @arg  MFXSTM32L152_IRQ_GPIO: IO interrupt
  *   @arg  MFXSTM32L152_IRQ_IDD : IDD interrupt
  *   @arg  MFXSTM32L152_IRQ_ERROR : Error interrupt
  *   @arg  MFXSTM32L152_IRQ_TS_DET : Touch Screen Controller Touch Detected interrupt
  *   @arg  MFXSTM32L152_IRQ_TS_NE : Touch Screen FIFO Not Empty
  *   @arg  MFXSTM32L152_IRQ_TS_TH : Touch Screen FIFO threshold triggered
  *   @arg  MFXSTM32L152_IRQ_TS_FULL : Touch Screen FIFO Full
  *   @arg  MFXSTM32L152_IRQ_TS_OVF : Touch Screen FIFO Overflow
  * @retval Component status
  */
int32_t MFXSTM32L152_DisableITSource(MFXSTM32L152_Object_t *pObj, uint8_t Source)
{
  int32_t ret = MFXSTM32L152_OK;
  uint8_t tmp;

  /* Get the current value of the INT_EN register */
  if (mfxstm32l152_read_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IRQ_SRC_EN, &tmp, 1) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }
  else
  {
    /* Set the interrupts to be Enabled */
    tmp &= ~Source;

    /* Set the register */
    if (mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IRQ_SRC_EN, &tmp, 1) != MFXSTM32L152_OK)
    {
      ret = MFXSTM32L152_ERROR;
    }
  }

  return ret;
}

/**
  * @brief  Enable the global IO interrupt source.
  * @param  pObj   Pointer to component object.
  * @retval Component status
  */
int32_t MFXSTM32L152_IO_EnableIT(MFXSTM32L152_Object_t *pObj)
{
  int32_t ret = MFXSTM32L152_OK;

  /* Enable global IO IT source */
  if (MFXSTM32L152_EnableITSource(pObj, MFXSTM32L152_IRQ_GPIO) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }

  return ret;
}

/**
  * @brief  Disable the global IO interrupt source.
  * @param  pObj   Pointer to component object.
  * @retval Component status
  */
int32_t MFXSTM32L152_IO_DisableIT(MFXSTM32L152_Object_t *pObj)
{
  int32_t ret = MFXSTM32L152_OK;

  /* Disable global IO IT source */
  if (MFXSTM32L152_DisableITSource(pObj, MFXSTM32L152_IRQ_GPIO) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }

  return ret;
}

/**
  * @brief  Enable interrupt mode for the selected IO pin(s).
  * @param  pObj   Pointer to component object.
  * @param  IO_Pin The IO interrupt to be enabled. This parameter could be any
  *         combination of the following values:
  *   @arg  MFXSTM32L152_GPIO_PIN_x: where x can be from 0 to 23.
  * @retval Component status
  */
int32_t MFXSTM32L152_IO_EnablePinIT(MFXSTM32L152_Object_t *pObj, uint32_t IO_Pin)
{
  int32_t ret = MFXSTM32L152_OK;

  if (MFXSTM32L152_reg24_setPinValue(pObj, MFXSTM32L152_REG_ADR_IRQ_GPI_SRC1, IO_Pin, 1) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }

  return ret;
}

/**
  * @brief  Disable interrupt mode for the selected IO pin(s).
  * @param  pObj   Pointer to component object.
  * @param  IO_Pin The IO interrupt to be disabled. This parameter could be any
  *         combination of the following values:
  *   @arg  MFXSTM32L152_GPIO_PIN_x: where x can be from 0 to 23.
  * @retval Component status
  */
int32_t MFXSTM32L152_IO_DisablePinIT(MFXSTM32L152_Object_t *pObj, uint32_t IO_Pin)
{
  int32_t ret = MFXSTM32L152_OK;

  if (MFXSTM32L152_reg24_setPinValue(pObj, MFXSTM32L152_REG_ADR_IRQ_GPI_SRC1, IO_Pin, 0) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }

  return ret;
}


/**
  * @brief  Check the status of the selected IO interrupt pending bit
  * @param  pObj   Pointer to component object.
  * @param  IO_Pin The IO interrupt to be checked could be:
  *   @arg  MFXSTM32L152_GPIO_PIN_x Where x can be from 0 to 23.
  * @retval Status of the checked IO pin(s).
  */
int32_t MFXSTM32L152_IO_ITStatus(MFXSTM32L152_Object_t *pObj, uint32_t IO_Pin)
{
  /* Get the Interrupt status */
  uint8_t tmpreg[3];
  uint32_t tmp;

  if (mfxstm32l152_read_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IRQ_GPI_PENDING1, &tmpreg[0], 1) != MFXSTM32L152_OK)
  {
    return MFXSTM32L152_ERROR;
  }

  if (mfxstm32l152_read_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IRQ_GPI_PENDING2, &tmpreg[1], 1) != MFXSTM32L152_OK)
  {
    return MFXSTM32L152_ERROR;
  }

  if (mfxstm32l152_read_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IRQ_GPI_PENDING3, &tmpreg[2], 1) != MFXSTM32L152_OK)
  {
    return MFXSTM32L152_ERROR;
  }

  tmp = (uint32_t)tmpreg[0] + ((uint32_t)tmpreg[1] << 8) + ((uint32_t)tmpreg[2] << 16);
  tmp &= IO_Pin;

  return (int32_t)tmp;
}

/**
  * @brief  Clear the selected IO interrupt pending bit(s). Also clears automatically MFXSTM32L152_REG_ADR_IRQ_PENDING
  * @param  pObj   Pointer to component object.
  * @param  IO_Pin the IO interrupt to be cleared, could be:
  *   @arg  MFXSTM32L152_GPIO_PIN_x: Where x can be from 0 to 23.
  * @retval Component status
  */
int32_t MFXSTM32L152_IO_ClearIT(MFXSTM32L152_Object_t *pObj, uint32_t IO_Pin)
{
  /* Clear the IO IT pending bit(s) by acknowledging */
  /* it cleans automatically also the Global IRQ_GPIO */
  /* normally this function is called under interrupt */
  uint8_t pin_0_7;
  uint8_t pin_8_15;
  uint8_t pin_16_23;

  pin_0_7   = (uint8_t)(IO_Pin & 0x0000ffU);
  pin_8_15  = (uint8_t)(IO_Pin >> 8);
  pin_8_15  = (uint8_t)(pin_8_15 & 0x00ffU);
  pin_16_23 = (uint8_t)(IO_Pin >> 16);

  if (pin_0_7 != 0U)
  {
    if (mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IRQ_GPI_ACK1, &pin_0_7, 1) != MFXSTM32L152_OK)
    {
      return MFXSTM32L152_ERROR;
    }
  }
  if (pin_8_15 != 0U)
  {
    if (mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IRQ_GPI_ACK2, &pin_8_15, 1) != MFXSTM32L152_OK)
    {
      return MFXSTM32L152_ERROR;
    }
  }
  if (pin_16_23 != 0U)
  {
    if (mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IRQ_GPI_ACK3, &pin_16_23, 1) != MFXSTM32L152_OK)
    {
      return MFXSTM32L152_ERROR;
    }
  }

  return MFXSTM32L152_OK;
}


/**
  * @brief  Enable the AF for aGPIO.
  * @param  pObj   Pointer to component object.
  * @retval Component status
  */
int32_t MFXSTM32L152_IO_EnableAF(MFXSTM32L152_Object_t *pObj)
{
  int32_t ret = MFXSTM32L152_OK;
  uint8_t mode;

  /* Get the current register value */
  if (mfxstm32l152_read_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_SYS_CTRL, &mode, 1) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }
  else
  {
    /* Enable ALTERNATE functions */
    /* AGPIO[0..3] can be either IDD or GPIO */
    /* AGPIO[4..7] can be either TS or GPIO */
    /* if IDD or TS are enabled no matter the value this bit GPIO are not available for those pins */
    /*  however the MFX will waste some cycles to to handle these potential GPIO (pooling, etc) */
    /* so if IDD and TS are both active it is better to let ALTERNATE disabled (0) */
    /* if however IDD or TS are not connected then set it on gives more GPIOs availability */
    /* remind that AGPIO are less efficient then normal GPIO (they use pooling rather then EXTI) */
    mode |= MFXSTM32L152_ALTERNATE_GPIO_EN;

    /* Write the new register value */
    if (mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_SYS_CTRL, &mode, 1) != MFXSTM32L152_OK)
    {
      ret = MFXSTM32L152_ERROR;
    }
  }

  return ret;
}

/**
  * @brief  Disable the AF for aGPIO.
  * @param  pObj   Pointer to component object.
  * @retval Component status
  */
int32_t MFXSTM32L152_IO_DisableAF(MFXSTM32L152_Object_t *pObj)
{
  int32_t ret = MFXSTM32L152_OK;
  uint8_t mode;

  /* Get the current register value */
  if (mfxstm32l152_read_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_SYS_CTRL, &mode, 1) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }
  else
  {
    /* Enable ALTERNATE functions */
    /* AGPIO[0..3] can be either IDD or GPIO */
    /* AGPIO[4..7] can be either TS or GPIO */
    /* if IDD or TS are enabled no matter the value this bit GPIO are not available for those pins */
    /*  however the MFX will waste some cycles to to handle these potential GPIO (pooling, etc) */
    /* so if IDD and TS are both active it is better to let ALTERNATE disabled (0) */
    /* if however IDD or TS are not connected then set it on gives more GPIOs availability */
    /* remind that AGPIO are less efficient then normal GPIO (they use pooling rather then EXTI) */
    mode &= ~MFXSTM32L152_ALTERNATE_GPIO_EN;

    /* Write the new register value */
    if (mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_SYS_CTRL, &mode, 1) != MFXSTM32L152_OK)
    {
      ret = MFXSTM32L152_ERROR;
    }
  }

  return ret;
}

/**
  * @brief  Initializes the IO peripheral according to the specified parameters in the MFXSTM32L152_IO_Init_t.
  * @param  pObj   Pointer to component object.
  * @param  IoInit Pointer to a BSP_IO_Init_t structure that contains
  *         the configuration information for the specified IO pin.
  * @retval Component status
  */
int32_t MFXSTM32L152_IO_Init(MFXSTM32L152_Object_t *pObj, MFXSTM32L152_IO_Init_t *IoInit)
{
  int32_t ret = MFXSTM32L152_OK;

  /* IT enable/disable */
  switch (IoInit->Mode)
  {
    case MFXSTM32L152_GPIO_MODE_OFF:
    case MFXSTM32L152_GPIO_MODE_ANALOG:
    case MFXSTM32L152_GPIO_MODE_INPUT:
    case MFXSTM32L152_GPIO_MODE_OUTPUT_OD:
    case MFXSTM32L152_GPIO_MODE_OUTPUT_PP:
      ret += MFXSTM32L152_IO_DisablePinIT(pObj, IoInit->Pin); /* first disable IT */
      break;

    case MFXSTM32L152_GPIO_MODE_IT_RISING_EDGE:
    case MFXSTM32L152_GPIO_MODE_IT_FALLING_EDGE:
    case MFXSTM32L152_GPIO_MODE_IT_LOW_LEVEL:
    case MFXSTM32L152_GPIO_MODE_IT_HIGH_LEVEL:
      ret += MFXSTM32L152_IO_EnableIT(pObj); /* first enable IT */
      break;
    default:
      break;
  }

  /* Set direction IN/OUT */
  if ((IoInit->Mode == MFXSTM32L152_GPIO_MODE_OUTPUT_PP) || (IoInit->Mode == MFXSTM32L152_GPIO_MODE_OUTPUT_OD))
  {
    ret += MFXSTM32L152_IO_InitPin(pObj, IoInit->Pin, MFXSTM32L152_GPIO_DIR_OUT);
  }
  else
  {
    ret += MFXSTM32L152_IO_InitPin(pObj, IoInit->Pin, MFXSTM32L152_GPIO_DIR_IN);
  }

  /* Set Push-Pull type */
  switch (IoInit->Pull)
  {
    case MFXSTM32L152_GPIO_NOPULL:
      ret += MFXSTM32L152_reg24_setPinValue(pObj, MFXSTM32L152_REG_ADR_GPIO_TYPE1, IoInit->Pin,
                                            MFXSTM32L152_GPI_WITHOUT_PULL_RESISTOR);
      break;
    case MFXSTM32L152_GPIO_PULLUP:
    case MFXSTM32L152_GPIO_PULLDOWN:
      ret += MFXSTM32L152_reg24_setPinValue(pObj, MFXSTM32L152_REG_ADR_GPIO_TYPE1, IoInit->Pin,
                                            MFXSTM32L152_GPI_WITH_PULL_RESISTOR);
      break;
    default:
      break;
  }

  if (IoInit->Mode == MFXSTM32L152_GPIO_MODE_OUTPUT_PP)
  {
    ret += MFXSTM32L152_reg24_setPinValue(pObj, MFXSTM32L152_REG_ADR_GPIO_TYPE1, IoInit->Pin,
                                          MFXSTM32L152_GPO_PUSH_PULL);
  }

  if (IoInit->Mode == MFXSTM32L152_GPIO_MODE_OUTPUT_OD)
  {
    ret += MFXSTM32L152_reg24_setPinValue(pObj, MFXSTM32L152_REG_ADR_GPIO_TYPE1, IoInit->Pin,
                                          MFXSTM32L152_GPO_OPEN_DRAIN);
  }

  /* Set Pullup-Pulldown */
  switch (IoInit->Pull)
  {
    case MFXSTM32L152_GPIO_NOPULL:
      if ((IoInit->Mode == MFXSTM32L152_GPIO_MODE_INPUT) || (IoInit->Mode == MFXSTM32L152_GPIO_MODE_ANALOG))
      {
        ret += MFXSTM32L152_reg24_setPinValue(pObj, MFXSTM32L152_REG_ADR_GPIO_PUPD1, IoInit->Pin,
                                              MFXSTM32L152_GPIO_PULL_DOWN);
      }
      else
      {
        ret += MFXSTM32L152_reg24_setPinValue(pObj, MFXSTM32L152_REG_ADR_GPIO_PUPD1, IoInit->Pin,
                                              MFXSTM32L152_GPIO_PULL_UP);
      }
      break;
    case MFXSTM32L152_GPIO_PULLUP:
      ret += MFXSTM32L152_reg24_setPinValue(pObj, MFXSTM32L152_REG_ADR_GPIO_PUPD1, IoInit->Pin,
                                            MFXSTM32L152_GPIO_PULL_UP);
      break;
    case MFXSTM32L152_GPIO_PULLDOWN:
      ret += MFXSTM32L152_reg24_setPinValue(pObj, MFXSTM32L152_REG_ADR_GPIO_PUPD1, IoInit->Pin,
                                            MFXSTM32L152_GPIO_PULL_DOWN);
      break;
    default:
      break;
  }

  /* Set Irq event and type mode */
  switch (IoInit->Mode)
  {
    case MFXSTM32L152_GPIO_MODE_IT_RISING_EDGE:
      ret += MFXSTM32L152_IO_SetIrqEvtMode(pObj, IoInit->Pin, MFXSTM32L152_IRQ_GPI_EVT_EDGE);
      ret += MFXSTM32L152_IO_SetIrqTypeMode(pObj, IoInit->Pin, MFXSTM32L152_IRQ_GPI_TYPE_HLRE);
      ret += MFXSTM32L152_IO_EnablePinIT(pObj, IoInit->Pin);  /* last to do: enable IT */
      break;
    case MFXSTM32L152_GPIO_MODE_IT_FALLING_EDGE:
      ret += MFXSTM32L152_IO_SetIrqEvtMode(pObj, IoInit->Pin, MFXSTM32L152_IRQ_GPI_EVT_EDGE);
      ret += MFXSTM32L152_IO_SetIrqTypeMode(pObj, IoInit->Pin, MFXSTM32L152_IRQ_GPI_TYPE_LLFE);
      ret += MFXSTM32L152_IO_EnablePinIT(pObj, IoInit->Pin);  /* last to do: enable IT */
      break;
    case MFXSTM32L152_GPIO_MODE_IT_HIGH_LEVEL:
      ret += MFXSTM32L152_IO_SetIrqEvtMode(pObj, IoInit->Pin, MFXSTM32L152_IRQ_GPI_EVT_LEVEL);
      ret += MFXSTM32L152_IO_SetIrqTypeMode(pObj, IoInit->Pin, MFXSTM32L152_IRQ_GPI_TYPE_HLRE);
      ret += MFXSTM32L152_IO_EnablePinIT(pObj, IoInit->Pin);  /* last to do: enable IT */
      break;
    case MFXSTM32L152_GPIO_MODE_IT_LOW_LEVEL:
      ret += MFXSTM32L152_IO_SetIrqEvtMode(pObj, IoInit->Pin, MFXSTM32L152_IRQ_GPI_EVT_LEVEL);
      ret += MFXSTM32L152_IO_SetIrqTypeMode(pObj, IoInit->Pin, MFXSTM32L152_IRQ_GPI_TYPE_LLFE);
      ret += MFXSTM32L152_IO_EnablePinIT(pObj, IoInit->Pin);  /* last to do: enable IT */
      break;
    default:
      break;
  }

  if (ret != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }

  return ret;
}

/**
  * @brief  Set the global interrupt Polarity of IRQ_OUT_PIN.
  * @param  pObj   Pointer to component object.
  * @param  Polarity: the IT mode polarity, could be one of the following values:
  *   @arg  MFXSTM32L152_OUT_PIN_POLARITY_LOW: Interrupt output line is active Low edge
  *   @arg  MFXSTM32L152_OUT_PIN_POLARITY_HIGH: Interrupt line output is active High edge
  * @retval Component status
  */
int32_t MFXSTM32L152_SetIrqOutPinPolarity(MFXSTM32L152_Object_t *pObj, uint8_t Polarity)
{
  int32_t ret = MFXSTM32L152_OK;
  uint8_t tmp;

  /* Get the current register value */
  if (mfxstm32l152_read_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_MFX_IRQ_OUT, &tmp, 1) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }
  else
  {
    /* Mask the polarity bits */
    tmp &= ~(uint8_t)0x02;

    /* Modify the Interrupt Output line configuration */
    tmp |= Polarity;

    /* Set the new register value */
    if (mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_MFX_IRQ_OUT, &tmp, 1) != MFXSTM32L152_OK)
    {
      ret = MFXSTM32L152_ERROR;
    }
  }

  return ret;
}

/**
  * @brief  Set the global interrupt Type of IRQ_OUT_PIN.
  * @param  pObj   Pointer to component object.
  * @param  Type: Interrupt line activity type, could be one of the following values:
  *   @arg  MFXSTM32L152_OUT_PIN_TYPE_OPENDRAIN: Open Drain output Interrupt line
  *   @arg  MFXSTM32L152_OUT_PIN_TYPE_PUSHPULL: Push Pull output Interrupt line
  * @retval Component status
  */
int32_t MFXSTM32L152_SetIrqOutPinType(MFXSTM32L152_Object_t *pObj, uint8_t Type)
{
  int32_t ret = MFXSTM32L152_OK;
  uint8_t tmp;

  /* Get the current register value */
  if (mfxstm32l152_read_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_MFX_IRQ_OUT, &tmp, 1) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }
  else
  {
    /* Mask the type bits */
    tmp &= ~(uint8_t)0x01;

    /* Modify the Interrupt Output line configuration */
    tmp |= Type;

    /* Set the new register value */
    if (mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_MFX_IRQ_OUT, &tmp, 1) != MFXSTM32L152_OK)
    {
      ret = MFXSTM32L152_ERROR;
    }
  }

  return ret;
}

/**
  * @brief  Initialize the mfxstm32l152 and configure the needed hardware resources
  * @param  pObj   Pointer to component object.
  * @retval Component status
  */
int32_t MFXSTM32L152_Init(MFXSTM32L152_Object_t *pObj)
{
  int32_t ret = MFXSTM32L152_OK;

  if (pObj->IsInitialized == 0U)
  {
    /* Initialize IO BUS layer */
    pObj->IO.Init();

    if (MFXSTM32L152_SetIrqOutPinPolarity(pObj, MFXSTM32L152_OUT_PIN_POLARITY_HIGH) != MFXSTM32L152_OK)
    {
      ret = MFXSTM32L152_ERROR;
    }
    else if (MFXSTM32L152_SetIrqOutPinType(pObj, MFXSTM32L152_OUT_PIN_TYPE_PUSHPULL) != MFXSTM32L152_OK)
    {
      ret = MFXSTM32L152_ERROR;
    }
    else
    {
      pObj->IsInitialized = 1U;
    }
  }

  return ret;
}

/**
  * @brief  DeInitialize the mfxstm32l152 and unconfigure the needed hardware resources
  * @param  pObj   Pointer to component object.
  * @retval Component status
  */
int32_t MFXSTM32L152_DeInit(MFXSTM32L152_Object_t *pObj)
{
  if (pObj->IsInitialized == 1U)
  {
    /* De-Initialize IO BUS layer */
    pObj->IO.DeInit();
    pObj->IsInitialized = 0U;
  }
  return MFXSTM32L152_OK;
}

/**
  * @brief  Reset the mfxstm32l152 by Software.
  * @param  pObj   Pointer to component object.
  * @retval Component status
  */
int32_t MFXSTM32L152_Reset(MFXSTM32L152_Object_t *pObj)
{
  int32_t ret = MFXSTM32L152_OK;
  uint8_t tmp = MFXSTM32L152_SWRST;

  /* Soft Reset */
  if (mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_SYS_CTRL, &tmp, 1) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }

  return ret;
}

/**
  * @brief  Read the MFXSTM32L152 IO Expander device ID.
  * @param  pObj   Pointer to component object.
  * @retval The Device ID (two bytes).
  */
int32_t MFXSTM32L152_ReadID(MFXSTM32L152_Object_t *pObj, uint32_t *Id)
{
  int32_t ret = MFXSTM32L152_OK;
  uint8_t id;

  /* Initialize IO BUS layer */
  pObj->IO.Init();

  if (mfxstm32l152_read_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_ID, &id, 1) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }
  else
  {
    /* Store the device ID value */
    *Id = id;
  }

  return ret;
}

/* IO driver structure initialization */
MFXSTM32L152_IO_Mode_t MFXSTM32L152_IO_Driver =
{
  MFXSTM32L152_IO_Init,
  MFXSTM32L152_DeInit,
  MFXSTM32L152_ReadID,
  MFXSTM32L152_Reset,
  MFXSTM32L152_IO_Start,
  MFXSTM32L152_IO_WritePin,
  MFXSTM32L152_IO_ReadPin,
  MFXSTM32L152_IO_EnableIT,
  MFXSTM32L152_IO_DisableIT,
  MFXSTM32L152_IO_ITStatus,
  MFXSTM32L152_IO_ClearIT,
};

/**
  * @brief  Wrap MFXSTM32L152 read function to Bus IO function
  * @param  handle Component object handle
  * @param  Reg    The target register address to write
  * @param  pData  The target register value to be written
  * @param  Length buffer size to be written
  * @retval error status
  */
static int32_t MFXSTM32L152_ReadRegWrap(void *handle, uint16_t Reg, uint8_t *pData, uint16_t Length)
{
  MFXSTM32L152_Object_t *pObj = (MFXSTM32L152_Object_t *)handle;

  return pObj->IO.ReadReg(pObj->IO.Address, Reg, pData, Length);
}

/**
  * @brief  Wrap MFXSTM32L152 write function to Bus IO function
  * @param  handle  Component object handle
  * @param  Reg     The target register address to write
  * @param  pData  The target register value to be written
  * @param  Length buffer size to be written
  * @retval error status
  */
static int32_t MFXSTM32L152_WriteRegWrap(void *handle, uint16_t Reg, uint8_t *pData, uint16_t Length)
{
  MFXSTM32L152_Object_t *pObj = (MFXSTM32L152_Object_t *)handle;

  return pObj->IO.WriteReg(pObj->IO.Address, Reg, pData, Length);
}

/**
  * @brief  Register Bus Io to component
  * @param  Component object pointer
  * @retval Component status
  */
int32_t MFXSTM32L152_RegisterBusIO(MFXSTM32L152_Object_t *pObj, MFXSTM32L152_IO_t *pIO)
{
  int32_t ret;

  if (pObj == NULL)
  {
    ret = MFXSTM32L152_ERROR;
  }
  else
  {
    pObj->IO.Init      = pIO->Init;
    pObj->IO.DeInit    = pIO->DeInit;
    pObj->IO.Address   = pIO->Address;
    pObj->IO.WriteReg  = pIO->WriteReg;
    pObj->IO.ReadReg   = pIO->ReadReg;
    pObj->IO.GetTick   = pIO->GetTick;

    pObj->Ctx.ReadReg  = MFXSTM32L152_ReadRegWrap;
    pObj->Ctx.WriteReg = MFXSTM32L152_WriteRegWrap;
    pObj->Ctx.handle   = pObj;

    if (pObj->IO.Init != NULL)
    {
      ret = pObj->IO.Init();
    }
    else
    {
      ret = MFXSTM32L152_ERROR;
    }
  }
  return ret;
}

int32_t MFXSTM32L152_Probe(MFXSTM32L152_IO_t *pIOCtx,
                           MFXSTM32L152_Object_t *pObj,
                           uint8_t *i2c_address, uint32_t i2c_addr_len)
{
  for (uint32_t i = 0U; i < i2c_addr_len; i++) {
    uint32_t mfxstm32l152_id;
    pIOCtx->Address = (uint16_t)i2c_address[i];

    if ((MFXSTM32L152_RegisterBusIO(pObj, pIOCtx) != MFXSTM32L152_OK) ||
        (MFXSTM32L152_ReadID(pObj, &mfxstm32l152_id) != MFXSTM32L152_OK)) {
      return MFXSTM32L152_ERROR;
    }

    if ((mfxstm32l152_id != MFXSTM32L152_ID) &&
        (mfxstm32l152_id != MFXSTM32L152_ID_2)) {
      continue;
    }
      
    return MFXSTM32L152_Init(pObj);
  }

  return MFXSTM32L152_ERROR;
}
