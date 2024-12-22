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

#include "stm32h7rsxx_ll_pwr.h"
#include "stm32h7rsxx_ll_rcc.h"
#include "stm32h7rsxx_ll_system.h"
#include "stm32h7rsxx_ll_bus.h"

#define BOOTSTRAP __attribute__((section(".bootstrap")))

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL1 (HSE BYPASS)
  *            SYSCLK(Hz)                     = 400000000 (CPU Clock)
  *            HCLK(Hz)                       = 200000000 (Cortex-M4 CPU, Bus matrix Clocks)
  *            AHB Prescaler                  = 2
  *            D1 APB3 Prescaler              = 2 (APB3 Clock  100MHz)
  *            D2 APB1 Prescaler              = 2 (APB1 Clock  100MHz)
  *            D2 APB2 Prescaler              = 2 (APB2 Clock  100MHz)
  *            D3 APB4 Prescaler              = 2 (APB4 Clock  100MHz)
  *            HSE Frequency(Hz)              = 25000000
  *            PLL_M                          = 5
  *            PLL_N                          = 160
  *            PLL_P                          = 2
  *            PLL_Q                          = 4
  *            PLL_R                          = 2
  *            VDD(V)                         = 3.3
  *            Flash Latency(WS)              = 4
  * @param  None
  * @retval None
  */
extern "C" BOOTSTRAP
void SystemClock_Config()
{
  /* Power Configuration */
  LL_PWR_ConfigSupply(LL_PWR_DIRECT_SMPS_SUPPLY);
  while (LL_PWR_IsActiveFlag_ACTVOSRDY() == 0) {
  }
  LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE0);
  while (LL_PWR_IsActiveFlag_VOSRDY() == 0) {
  }

  /* Enable HSE oscillator */
  LL_RCC_HSE_Enable();
  while (LL_RCC_HSE_IsReady() != 1) {
  }

  /* Set FLASH latency */
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_6);

  /* Main PLL configuration and activation */
  LL_RCC_PLL_SetSource(LL_RCC_PLLSOURCE_HSE);
  LL_RCC_PLL1P_Enable();
  // LL_RCC_PLL1Q_Enable();
  // LL_RCC_PLL1R_Enable();
  LL_RCC_PLL1FRACN_Disable();
  LL_RCC_PLL1_SetVCOInputRange(LL_RCC_PLLINPUTRANGE_2_4);
  LL_RCC_PLL1_SetVCOOutputRange(LL_RCC_PLLVCORANGE_WIDE);
  LL_RCC_PLL1_SetM(2);
  LL_RCC_PLL1_SetN(50);
  LL_RCC_PLL1_SetP(1);
  // LL_RCC_PLL1_SetQ(2);
  // LL_RCC_PLL1_SetR(2);
  // LL_RCC_PLL1_SetS(2);
  LL_RCC_PLL1_Enable();
  while (LL_RCC_PLL1_IsReady() != 1) {
  }

  /* Set Sys & AHB & APB1 & APB2 & APB4  prescaler */
  LL_RCC_SetSysPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAHBPrescaler(LL_RCC_AHB_DIV_2);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_2);
  LL_RCC_SetAPB4Prescaler(LL_RCC_APB4_DIV_2);

  /* Set PLL1 as System Clock Source */
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL1);
  while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL1) {
  }

  /* PLL2 configuration and activation */
  // LL_RCC_PLL2P_Enable();
  // LL_RCC_PLL2Q_Enable();
  // LL_RCC_PLL2R_Enable();
  LL_RCC_PLL2S_Enable();
  LL_RCC_PLL2FRACN_Disable();
  LL_RCC_PLL2_SetVCOInputRange(LL_RCC_PLLINPUTRANGE_2_4);
  LL_RCC_PLL2_SetVCOOutputRange(LL_RCC_PLLVCORANGE_WIDE);
  LL_RCC_PLL2_SetM(2);
  LL_RCC_PLL2_SetN(50);
  // LL_RCC_PLL2_SetP(1);
  // LL_RCC_PLL2_SetQ(2);
  // LL_RCC_PLL2_SetR(2);
  LL_RCC_PLL2_SetS(3);
  LL_RCC_PLL2_Enable();
  while (LL_RCC_PLL2_IsReady() != 1) {
  }

  /* Enable SRAM1, SRAM2 & SRAM3 */
  // LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_D2SRAM1 |
  //                          LL_AHB2_GRP1_PERIPH_D2SRAM2 |
  //                          LL_AHB2_GRP1_PERIPH_D2SRAM3);

  /* Set periph clock sources */
  // LL_RCC_SetFMCClockSource(LL_RCC_FMC_CLKSOURCE_HCLK);
  // LL_RCC_SetSPIClockSource(LL_RCC_SPI123_CLKSOURCE_PLL3P);

#if defined(USE_USB_HS)
  LL_RCC_SetUSBPHYCClockSource(LL_RCC_USBPHYC_CLKSOURCE_HSE);
  LL_RCC_SetUSBREFClockSource(LL_RCC_USBREF_CLKSOURCE_24M);
#else
  LL_RCC_HSI48_Enable();
  while (LL_RCC_HSI48_IsReady() != 1) {
  }
  LL_RCC_SetOTGFSClockSource(LL_RCC_OTGFS_CLKSOURCE_HSI48);
#endif

  LL_RCC_SetXSPIClockSource(LL_RCC_XSPI1_CLKSOURCE_PLL2S);
  LL_RCC_SetXSPIClockSource(LL_RCC_XSPI2_CLKSOURCE_PLL2S);

  // Only required if using Async ADC clock ???
  LL_RCC_SetADCClockSource(LL_RCC_ADC_CLKSOURCE_CLKP);
}
