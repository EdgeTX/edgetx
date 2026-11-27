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

#include "hal/abnormal_reboot.h"

#include "stm32_hal_ll.h"

#define __REBOOT_DATA __attribute__((section(".rebootdata"), aligned(4)))

// This variable is define at a fixed memory location
// and is used in bootloader and firmware to pass
// commands across reboot.
uint32_t _reboot_cmd __REBOOT_DATA;

constexpr uint32_t _SOFTRESET_REQUEST = 0xCAFEDEAD;
constexpr uint32_t _SHUTDOWN_REQUEST = 0xDEADBEEF;

constexpr uint32_t _REBOOT_CAUSE_DEFAULT = 0xFFFFFFFF;

static uint32_t _reboot_cause = _REBOOT_CAUSE_DEFAULT;

#if defined(DEBUG)
uint32_t _dbg_csr = 0xFFFFFFFF;
#endif

void _init_reboot_cause()
{
  if (_reboot_cause != _REBOOT_CAUSE_DEFAULT) return;

#if defined(DEBUG)
#if defined(STM32H5)
  _dbg_csr = LL_RCC_ReadReg(RSR);
#else
  _dbg_csr = LL_RCC_ReadReg(CSR);
#endif
#endif
  
#ifdef STM32H7
  if (LL_RCC_IsActiveFlag_IWDG1RST()) {
#else
  if (LL_RCC_IsActiveFlag_IWDGRST()) {
#endif
    _reboot_cause = ARC_Watchdog;
  } else if (LL_RCC_IsActiveFlag_SFTRST() ||
             _reboot_cmd == REBOOT_CMD_POWER_ON) {
    _reboot_cause = ARC_Software;
  } else {
    _reboot_cause = ARC_None;
  }

#if !defined(BOOT)
  LL_RCC_ClearResetFlags();
#endif
}

void abnormalRebootEnableDetection()
{
  _init_reboot_cause();
  _reboot_cmd = 0;
}

void abnormalRebootRequestSoftReset()
{
  _reboot_cmd = _SOFTRESET_REQUEST;
}


uint32_t abnormalRebootGetCause()
{
  _init_reboot_cause();
  return _reboot_cause;
}

uint32_t abnormalRebootGetCmd()
{
  return _reboot_cmd;
}

void abnormalRebootResetCmd()
{
  _reboot_cmd = 0;
}

void abnormalRebootSetCmd(uint32_t cmd)
{
  _reboot_cmd = cmd;
}

#if defined(BOOTLOADER_DFU)

#if !defined(STM32F4)
  #error "abnormalRebootQuiesce(): peripheral reset only defined for STM32F4"
#endif

// GPIOA..GPIOK reset bits
#define AHB1_GPIO_RESET_MASK 0x7FFUL

void abnormalRebootQuiesce()
{
  __disable_irq();
  __set_BASEPRI(0);

  SysTick->CTRL = 0;
  SCB->ICSR = SCB_ICSR_PENDSTCLR_Msk | SCB_ICSR_PENDSVCLR_Msk;
  for (unsigned i = 0; i < 8; i++) {
    NVIC->ICER[i] = 0xFFFFFFFF;
    NVIC->ICPR[i] = 0xFFFFFFFF;
  }

  // also disconnects USB (OTG reset releases the D+ pull-up)
  RCC->AHB1RSTR = ~AHB1_GPIO_RESET_MASK;
  RCC->AHB1RSTR = 0;
  RCC->AHB2RSTR = 0xFFFFFFFF;
  RCC->AHB2RSTR = 0;
  RCC->AHB3RSTR = 0xFFFFFFFF;
  RCC->AHB3RSTR = 0;
  RCC->APB1RSTR = 0xFFFFFFFF;
  RCC->APB1RSTR = 0;
  RCC->APB2RSTR = 0xFFFFFFFF;
  RCC->APB2RSTR = 0;

  // Peripheral clocks off too, as after a reset: some init code depends on
  // it (e.g. the F4 ADC prescaler write is dropped while the ADC clock is
  // off, so leaving it on changes the ADC timing). CCM holds the stack and
  // the reboot command word; GPIO clocks off would not change the pins.
  RCC->AHB1ENR &= AHB1_GPIO_RESET_MASK | RCC_AHB1ENR_CCMDATARAMEN;
  RCC->AHB2ENR = 0;
  RCC->AHB3ENR = 0;
  RCC->APB1ENR = 0;
  RCC->APB2ENR = 0;
  (void)RCC->APB2ENR;
}

void abnormalRebootRequestPowerOn()
{
  LL_RCC_ClearResetFlags();
  _reboot_cmd = REBOOT_CMD_POWER_ON;
}

bool abnormalRebootStartedFromSystemMemory()
{
  // reads as 0 while the SYSCFG clock is off
  RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
  (void)RCC->APB2ENR;
  return (SYSCFG->MEMRMP & SYSCFG_MEMRMP_MEM_MODE) == SYSCFG_MEMRMP_MEM_MODE_0;
}

void abnormalRebootResetClocks()
{
  RCC->CR |= RCC_CR_HSION;
  while (!(RCC->CR & RCC_CR_HSIRDY));

  RCC->CFGR = 0;
  while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI);

  RCC->CR &= ~(RCC_CR_PLLON | RCC_CR_PLLI2SON | RCC_CR_PLLSAION);
  RCC->CR &= ~(RCC_CR_HSEON | RCC_CR_CSSON);
  RCC->PLLCFGR = 0x24003010;  // reset value
  RCC->CIR = 0;
}

void abnormalRebootJump(uint32_t address)
{
  const uint32_t* vectors = (const uint32_t*)address;
  uint32_t sp = vectors[0];
  uint32_t pc = vectors[1];

  // no lazy FP state left behind
  FPU->FPCCR &= ~FPU_FPCCR_LSPACT_Msk;

  // switching to MSP invalidates the current (task) stack: registers only
  asm volatile(
      "msr control, %0 \n"
      "isb             \n"
      "msr msp, %1     \n"
      "bx %2           \n"
      :
      : "r"(0), "r"(sp), "r"(pc)
      : "memory");
  __builtin_unreachable();
}

#endif // BOOTLOADER_DFU
