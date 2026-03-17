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

#include "cortex_m_isr.h"
#include "stm32_cmsis.h"

#define GET_VECTACTIVE() \
  ((SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) >> SCB_ICSR_VECTACTIVE_Pos)

#define HARDFAULT_HANDLING_ASM(_x) \
  __asm volatile(                  \
      "tst lr, #4 \n"              \
      "ite eq \n"                  \
      "mrseq r0, msp \n"           \
      "mrsne r0, psp \n"           \
      "b hard_fault_handler_c \n")

#define HALT_IF_DEBUGGING()                                 \
  do {                                                      \
    if (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) { \
      __asm("bkpt 1");                                      \
    }                                                       \
  } while (0)


__attribute__((optimize("O0")))
void default_isr_handler()
{
  /* Halt in debugger if connected */
  if (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) {
    uint32_t active_irq = GET_VECTACTIVE();
    if (active_irq >= NVIC_USER_IRQ_OFFSET) {
      /* External interrupt: check IRQn_Type for number */
      active_irq -= NVIC_USER_IRQ_OFFSET;
      __asm__("bkpt 1");
    } else {
      /* Cortex-M Exception */
      __asm__("bkpt 2");
    }
  }
  while (1) {}
}

typedef struct __attribute__((packed)) ContextStateFrame {
  uint32_t r0;
  uint32_t r1;
  uint32_t r2;
  uint32_t r3;
  uint32_t r12;
  uint32_t lr;
  uint32_t return_address;
  uint32_t xpsr;
} sContextStateFrame;

__attribute__((optimize("O0")))
void hard_fault_handler_c(sContextStateFrame *frame) {
  HALT_IF_DEBUGGING();
}

void HardFault_Handler(void) {
  HARDFAULT_HANDLING_ASM();
}
