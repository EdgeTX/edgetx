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
  _dbg_csr = LL_RCC_ReadReg(CSR);
#endif
  
#ifdef STM32H7
  if (LL_RCC_IsActiveFlag_IWDG1RST()) {
#else
  if (LL_RCC_IsActiveFlag_IWDGRST()) {
#endif
    _reboot_cause = ARC_Watchdog;
  } else if (LL_RCC_IsActiveFlag_SFTRST()) {
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
