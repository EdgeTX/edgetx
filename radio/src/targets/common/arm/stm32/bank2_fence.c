/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/* DIAG_BANK2_FENCE (validation builds, TX16S/STM32F429):
 *
 * While a USB session is active, flash bank 2 (0x08100000, 1MB) must
 * never be read: USB D+ (PA12) switching noise corrupts the flash
 * read path on some units (ST errata ES0166 §2.4.6, EdgeTX#5899).
 * The firmware layout keeps everything hot in bank 1 and freezes the
 * UI (whose code lives in bank 2) during USB.
 *
 * This fence arms an MPU no-access region over bank 2 for the whole
 * USB session, so any stray access — including through function
 * pointers that the static scan (bench/check_bank1.sh) cannot see —
 * traps deterministically (MemManage) instead of corrupting reads
 * probabilistically.  The fault handler stores a magic and the
 * faulting address in RTC backup registers (they survive the reset)
 * and reboots.
 *
 * Post-mortem readout: BKP4R = 0xB2FE0000 | (CFSR & 0xFFFF),
 * BKP5R = faulting address (MMFAR if valid, else stacked PC).
 */

#if defined(DIAG_BANK2_FENCE) && !defined(BOOT)

#include "stm32_hal.h"

#define FENCE_REGION_NUMBER  7
#define FENCE_BASE           0x08100000UL
#define FENCE_SIZE_1MB       (19UL << MPU_RASR_SIZE_Pos)
#define FENCE_MAGIC          0xB2FE0000UL

void bank2FenceEnable(void)
{
  __disable_irq();
  MPU->RNR = FENCE_REGION_NUMBER;
  MPU->RBAR = FENCE_BASE;
  MPU->RASR = MPU_RASR_ENABLE_Msk | FENCE_SIZE_1MB | MPU_RASR_XN_Msk;
  /* AP = 0b000: no access, privileged included */
  MPU->CTRL |= MPU_CTRL_PRIVDEFENA_Msk | MPU_CTRL_ENABLE_Msk;
  SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk;
  __DSB();
  __ISB();
  __enable_irq();
}

void bank2FenceDisable(void)
{
  __disable_irq();
  MPU->RNR = FENCE_REGION_NUMBER;
  MPU->RASR = 0;
  MPU->RBAR = 0;
  /* MPU left enabled with PRIVDEFENA: background map = default */
  __DSB();
  __ISB();
  __enable_irq();
}

void MemManage_C(uint32_t *frame)
{
  uint32_t cfsr = SCB->CFSR;
  uint32_t addr = (cfsr & SCB_CFSR_MMARVALID_Msk) ? SCB->MMFAR : frame[6];

  /* Backup domain write access (registers survive the reset) */
  __HAL_RCC_PWR_CLK_ENABLE();
  SET_BIT(PWR->CR, PWR_CR_DBP);
  RTC->BKP4R = FENCE_MAGIC | (cfsr & 0xFFFF);
  RTC->BKP5R = addr;

  NVIC_SystemReset();
}

__attribute__((naked)) void MemManage_Handler(void)
{
  __asm volatile(
      "tst lr, #4        \n"
      "ite eq            \n"
      "mrseq r0, msp     \n"
      "mrsne r0, psp     \n"
      "b MemManage_C     \n");
}

#endif /* DIAG_BANK2_FENCE && !BOOT */
