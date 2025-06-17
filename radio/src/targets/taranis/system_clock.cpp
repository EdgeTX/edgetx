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
#if defined(STM32H5)
#include "stm32h5xx_ll_pwr.h"
#include "stm32h5xx_ll_rcc.h"
#include "stm32h5xx_ll_system.h"
#include "stm32h5xx_ll_bus.h"

/**
  */
extern "C"
void SystemClock_Config()
{
  /* Power Configuration */
  LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE0);
  while (LL_PWR_IsActiveFlag_VOS() == 0) {
  }

  /* Enable HSE oscillator */
  LL_RCC_HSE_Enable();
  while (LL_RCC_HSE_IsReady() != 1) {
  }

  /* Set FLASH latency */
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_6);

  /* Main PLL configuration and activation */
  LL_RCC_PLL1_SetSource(LL_RCC_PLL1SOURCE_HSE);
  LL_RCC_PLL1P_Enable();
  LL_RCC_PLL1Q_Enable();
  LL_RCC_PLL1R_Enable();
  LL_RCC_PLL1FRACN_Disable();
  LL_RCC_PLL1_SetVCOInputRange(LL_RCC_PLLINPUTRANGE_2_4);
  LL_RCC_PLL1_SetVCOOutputRange(LL_RCC_PLLVCORANGE_WIDE);
  LL_RCC_PLL1_SetM(2);
  LL_RCC_PLL1_SetN(124);
  LL_RCC_PLL1_SetP(2);
  LL_RCC_PLL1_SetQ(2);
  LL_RCC_PLL1_SetR(2);
  LL_RCC_PLL1_Enable();
  while (LL_RCC_PLL1_IsReady() != 1) {
  }

  /* Set Sys & AHB & APB1 & APB2 & APB4  prescaler */
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
  LL_RCC_SetAPB3Prescaler(LL_RCC_APB3_DIV_1);

  /* Set PLL1 as System Clock Source */
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL1);
  while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL1) {
  }

  /* PLL2 configuration and activation */
  LL_RCC_PLL2_SetSource(LL_RCC_PLL2SOURCE_HSE);
  LL_RCC_PLL2P_Enable();
  LL_RCC_PLL2Q_Enable();
  LL_RCC_PLL2R_Enable();
  LL_RCC_PLL2FRACN_Disable();
  LL_RCC_PLL2_SetVCOInputRange(LL_RCC_PLLINPUTRANGE_2_4);
  LL_RCC_PLL2_SetVCOOutputRange(LL_RCC_PLLVCORANGE_WIDE);
  LL_RCC_PLL2_SetM(2);
  LL_RCC_PLL2_SetN(125);
  LL_RCC_PLL2_SetP(2);
  LL_RCC_PLL2_SetQ(2);
  LL_RCC_PLL2_SetR(4);
  LL_RCC_PLL2_Enable();
  while (LL_RCC_PLL2_IsReady() != 1) {
  }

  /* PLL2 configuration and activation */
  LL_RCC_PLL3_SetSource(LL_RCC_PLL3SOURCE_HSE);
  LL_RCC_PLL3P_Enable();
  LL_RCC_PLL3Q_Enable();
  LL_RCC_PLL3R_Enable();
  LL_RCC_PLL3FRACN_Disable();
  LL_RCC_PLL3_SetVCOInputRange(LL_RCC_PLLINPUTRANGE_4_8);
  LL_RCC_PLL3_SetVCOOutputRange(LL_RCC_PLLVCORANGE_WIDE);
  LL_RCC_PLL3_SetM(1);
  LL_RCC_PLL3_SetN(36);
  LL_RCC_PLL3_SetP(2);
  LL_RCC_PLL3_SetQ(6);
  LL_RCC_PLL3_SetR(2);
  LL_RCC_PLL3_Enable();
  while (LL_RCC_PLL3_IsReady() != 1) {
  }

  LL_RCC_SetUSBClockSource(LL_RCC_USB_CLKSOURCE_PLL3Q);


  LL_RCC_SetSPIClockSource(LL_RCC_SPI1_CLKSOURCE_PLL1Q);

  // Only required if using Async ADC clock ???
  //LL_RCC_SetADCClockSource(LL_RCC_ADC_CLKSOURCE_CLKP);

  SystemCoreClockUpdate();
}
#else
extern "C"
void SystemClock_Config()
{
}
#endif // STM32H5
