#include <stdint.h>
#include "stm32_cmsis.h"
#include "system_clock.h"

#include "stm32_hal.h"

#define NAKED __attribute__((naked))
#define BOOTSTRAP __attribute__((section(".bootstrap")))

// Linker script symbols
extern uint32_t _sisr_vector;
extern uint32_t PSRAM_START;

extern "C" NAKED BOOTSTRAP
void Reset_Handler()
{
  asm inline (
    "ldr sp, =_estack \n"
  );

#if defined(BOOT)
  asm inline (
    "bl SystemClock_Config \n"
    "bl CPU_CACHE_Enable \n"
  );
#endif

  // Copy code into normal RAM
  asm inline (
    "ldr r0, =_stext    \n"
    "ldr r1, =_etext    \n"
    "ldr r2, =_text_load \n"
    "bl naked_copy      \n"
  );

#if defined(BOOT)
  asm inline (
    "bl SystemInit \n"
    "bl MPU_Init \n"
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

  // Copy code into fast RAM
  asm inline (
    "ldr r0, =_siram    \n"
    "ldr r1, =_eiram    \n"
    "ldr r2, =_stext_iram \n"
    "bl naked_copy      \n"
  );

  // Copy initialized data segment
  asm inline (
    "ldr r0, =_sdata    \n"
    "ldr r1, =_edata    \n"
    "ldr r2, =_sidata   \n"
    "bl naked_copy      \n"
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

  asm inline (
    "bl clean_dcache \n"
    // Call static constructors
    "bl __libc_init_array \n"
    "bl SystemCoreClockUpdate \n"
    // Call the application's entry point
    "bl main \n"
    "bx lr   \n"
  );
}

extern "C" BOOTSTRAP void set_vtor() {
  SCB->VTOR = (intptr_t)&_sisr_vector;
}

extern "C" NAKED BOOTSTRAP void naked_copy() {
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

extern "C" BOOTSTRAP
void CPU_CACHE_Enable()
{
  /* Enable I-Cache */
  SCB_EnableICache();

  /* Enable D-Cache */
  SCB_EnableDCache();
}

extern "C" __attribute__((used))
void MPU_Init()
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

#if defined(FIRMWARE_QSPI)
  /* Region 2: QSPI memory range, bank1 */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER2;
  MPU_InitStruct.BaseAddress = 0x70000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_8MB;
  MPU_InitStruct.SubRegionDisable = 0x0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);
#endif

#if defined(SDRAM)
  /* Region 3: SDRAM memory range */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER3;
  MPU_InitStruct.BaseAddress = (uint32_t)&PSRAM_START;
  MPU_InitStruct.Size = MPU_REGION_SIZE_32MB;
  MPU_InitStruct.SubRegionDisable = 0x0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);
#endif

  /* Region 3: SRAM1 memory range */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER4;
  MPU_InitStruct.BaseAddress = 0x30000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_32KB;
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

extern "C" void clean_dcache() {
  SCB_CleanDCache();
}
