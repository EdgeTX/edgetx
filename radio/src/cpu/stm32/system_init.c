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

#include <stdint.h>

#include "stm32_cmsis.h"
#include "stm32_hal.h"


#if ((defined (__ICACHE_PRESENT) && (__ICACHE_PRESENT == 1U)) || \
     (defined (__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)))
#define USE_CACHE
#endif

#if (defined (USE_CACHE) && defined (__MPU_PRESENT) && (__MPU_PRESENT == 1U))
#define REQUIRE_MPU_CONFIG
#endif

#if defined(ITCM_BASE) /* H7RS */ || defined(D1_ITCMRAM_BASE) /* H7 */
#define LOAD_FAST_CODE_RAM
#endif

#define NAKED __attribute__((naked))
#define BOOTSTRAP __attribute__((section(".bootstrap")))

// Linker script symbols
extern uint32_t _sisr_vector;

NAKED BOOTSTRAP void Reset_Handler()
{
  asm inline (
    "ldr sp, =_estack \n"
  );

#if defined(BOOT)
  asm inline (
    "bl SystemInit \n"
    "bl SystemClock_Config \n"
  );

#if defined(USE_CACHE)
  asm inline (
    "bl CPU_CACHE_Enable \n"
  );
#endif // USE_CACHE
#endif // BOOT

  // Copy code into RAM
  asm inline (
    "ldr r0, =_stext    \n"
    "ldr r1, =_etext    \n"
    "ldr r2, =_text_load \n"
    "bl naked_copy      \n"
  );

#if defined(BOOT) && defined(REQUIRE_MPU_CONFIG)
  asm inline (
    "bl MPU_Config \n"
  );
#endif
  
  // Copy / setup ISR vector
  asm inline (
    "ldr r0, =_sisr_vector \n"
    "ldr r1, =_eisr_vector \n"
    "ldr r2, =_isr_load \n"
    "bl naked_copy      \n"
    "bl set_vtor        \n"
  );

#if defined(LOAD_FAST_CODE_RAM)
  // Copy code into fast RAM
  asm inline (
    "ldr r0, =_siram    \n"
    "ldr r1, =_eiram    \n"
    "ldr r2, =_stext_iram \n"
    "bl naked_copy      \n"
  );
#endif

  // Copy initialized data segment
  asm inline (
    "ldr r0, =_sdata    \n"
    "ldr r1, =_edata    \n"
    "ldr r2, =_sidata   \n"
    "bl naked_copy      \n"
  );

  // Execute early hooks
  asm inline (
    "bl init_hooks \n"
  );
 
  // Zero fill bss segment
  asm inline (
    "ldr r2, =_sbss    \n"
    "ldr r4, =_ebss    \n"
    "movs r3, 0        \n"
    "b LoopFillZerobss \n"

  "FillZerobss:        \n"
    "str  r3, [r2]     \n"
    "adds r2, r2, 4    \n"

  "LoopFillZerobss:    \n"
    "cmp r2, r4        \n"
    "bcc FillZerobss   \n"
  );

#if defined(USE_CACHE)
  asm inline (
    "bl clean_dcache \n"
  );
#endif

  asm inline (
    // Call static constructors
    "bl __libc_init_array \n"
    // Update global clock speed variable
    "bl SystemCoreClockUpdate \n"
    // Call the application's entry point
    "bl main \n"
    "bx lr   \n"
  );
}

BOOTSTRAP void set_vtor() { SCB->VTOR = (intptr_t)&_sisr_vector; }

NAKED BOOTSTRAP void naked_copy()
{
  // r0: destination start
  // r1: destination end
  // r2: source start
  asm inline (
    "cmp r0, r2         \n"
    "beq SkipCopy       \n"
    "movs r3, 0         \n"
    "b LoopCopyInit     \n"

  "CopyInit:            \n"
    "ldr r4, [r2, r3]   \n"
    "str r4, [r0, r3]   \n"
    "adds r3, r3, #4    \n"

  "LoopCopyInit:        \n"
    "adds r4, r0, r3    \n"
    "cmp r4, r1         \n"
    "bcc CopyInit       \n"

  "SkipCopy:            \n"
    "bx lr              \n"
  );
}

void init_hooks()
{
  extern uint32_t __init_hook_array_start;
  extern uint32_t __init_hook_array_end;

  typedef void (*hook_fct_t)();

  for (uint32_t hook = (uint32_t)&__init_hook_array_start;
       hook != (uint32_t)&__init_hook_array_end; hook += sizeof(void*)) {
    (*(hook_fct_t*)hook)();
  }
}

#if defined(USE_CACHE)
// SCB_CleanDCache() is "forced inline"
void clean_dcache() { SCB_CleanDCache(); }

#if defined(BOOT)
BOOTSTRAP void CPU_CACHE_Enable()
{
  /* Enable I-Cache */
  SCB_EnableICache();

  /* Enable D-Cache */
  SCB_EnableDCache();
}
#endif // BOOT
#endif // USE_CACHE

#if defined(BOOT) && defined(REQUIRE_MPU_CONFIG)
// Linker script symbols
extern uint32_t _dram_addr;
extern uint32_t EXTRAM_START;
extern uint32_t EXTRAM_SIZE;
extern uint32_t NORFLASH_START;
extern uint32_t NORFLASH_SIZE;

__STATIC_FORCEINLINE uint32_t mpu_region_size(uint32_t size)
{
  return 32 - __CLZ(size) - 1;
}

void MPU_Config()
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /*
   * Region 0: overwrite default setting to prevent speculative
   *           read to unavailable memories.
   * Range: 0x60000000 to 0xE0000000
   *
   * Set as strongly ordered, code execution disabled
   */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Region 2: QSPI memory range, bank1 */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER2;
  MPU_InitStruct.BaseAddress = (uint32_t)&NORFLASH_START;
  MPU_InitStruct.Size = mpu_region_size((uint32_t)&NORFLASH_SIZE);
  MPU_InitStruct.SubRegionDisable = 0x0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Region 3: external RAM memory range */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER3;
  MPU_InitStruct.BaseAddress = (uint32_t)&EXTRAM_START;
  MPU_InitStruct.Size = mpu_region_size((uint32_t)&EXTRAM_SIZE);
  MPU_InitStruct.SubRegionDisable = 0x0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Region 4: dedicated DMA buffers (cache disabled) */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER4;
  MPU_InitStruct.BaseAddress = (uint32_t)&_dram_addr;
  MPU_InitStruct.Size = MPU_REGION_SIZE_64KB; // FIXME
  MPU_InitStruct.SubRegionDisable = 0x0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);


  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

  /* Enable bus fault exception */
  SCB->SHCSR |= SCB_SHCSR_BUSFAULTENA_Msk;
}
#endif // REQUIRE_MPU_CONFIG
