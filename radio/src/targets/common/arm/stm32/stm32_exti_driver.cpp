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

#include "stm32_hal.h"
#include "stm32_hal_ll.h"
#include "stm32_exti_driver.h"
#include "hal.h"

#if defined(EXTI_PR1_PR) || defined(EXTI_PR1_PR0)
# define _PR_Reg PR1
#else
# define _PR_Reg PR
#endif

#define _EXTI_HANDLERS(irq_name) _handlers_ ## irq_name

#define _PR_MASK(first, last) (((1 << (last - first + 1))-1))

#define _DEFINE_EXTI_IRQ_HANDLER(irq_name, first, last)    \
  static stm32_exti_handler_t _EXTI_HANDLERS(irq_name) [last - first + 1] = { nullptr }; \
  extern "C" void irq_name ## Handler()                                 \
  {                                                                     \
    /* Read Pending register */                                         \
    /* (shifted by start line) */                                       \
    uint32_t pr = LL_EXTI_ReadReg(_PR_Reg) >> first;                    \
    pr &=  _PR_MASK(first,last);                                        \
    while (pr) {                                                        \
      uint32_t i = POSITION_VAL(pr);                                    \
      /* Clear Pending Flag */                                          \
      LL_EXTI_ClearFlag_0_31(1 << (first + i));                         \
      pr &= ~(1 << i);                                                  \
      /* ... and trigger handler */                                     \
      stm32_exti_handler_t h_fct = _EXTI_HANDLERS(irq_name) [i];        \
      if (h_fct) h_fct();                                               \
    }                                                                   \
  }

#define _SET_EXTI_IRQ_HANDLER(irq_name, first, last, line_pos, cb)      \
  if (line_pos >= first && line_pos <= last) {                          \
    _EXTI_HANDLERS(irq_name)[line_pos - first] = cb;                    \
    NVIC_SetPriority(irq_name ## n, irq_name ## _Priority);             \
    NVIC_EnableIRQ(irq_name ## n);                                      \
    return;                                                             \
  }

#define _CLEAR_EXTI_IRQ_HANDLER(irq_name, first, last, line_pos)        \
  if (line_pos >= first && line_pos <= last) {                          \
    _EXTI_HANDLERS(irq_name)[line_pos - first] = nullptr;               \
    if (!_has_handler(_EXTI_HANDLERS(irq_name), last - first + 1)) {    \
      NVIC_DisableIRQ(irq_name ## n);                                   \
    }                                                                   \
    return;                                                             \
  }

#if defined(USE_EXTI1_IRQ)
  _DEFINE_EXTI_IRQ_HANDLER(EXTI1_IRQ, 1, 1);
#endif

#if defined(USE_EXTI2_IRQ)
  _DEFINE_EXTI_IRQ_HANDLER(EXTI2_IRQ, 2, 2);
#endif

#if defined(USE_EXTI3_IRQ)
  _DEFINE_EXTI_IRQ_HANDLER(EXTI3_IRQ, 3, 3);
#endif

#if defined(USE_EXTI4_IRQ)
  _DEFINE_EXTI_IRQ_HANDLER(EXTI4_IRQ, 4, 4);
#endif

#if defined(STM32H7RS)

#if defined(USE_EXTI5_IRQ)
  _DEFINE_EXTI_IRQ_HANDLER(EXTI5_IRQ, 5, 5);
#endif

#if defined(USE_EXTI6_IRQ)
  _DEFINE_EXTI_IRQ_HANDLER(EXTI6_IRQ, 6, 6);
#endif

#if defined(USE_EXTI7_IRQ)
  _DEFINE_EXTI_IRQ_HANDLER(EXTI7_IRQ, 7, 7);
#endif

#if defined(USE_EXTI8_IRQ)
  _DEFINE_EXTI_IRQ_HANDLER(EXTI8_IRQ, 8, 8);
#endif

#if defined(USE_EXTI9_IRQ)
  _DEFINE_EXTI_IRQ_HANDLER(EXTI9_IRQ, 9, 9);
#endif

#if defined(USE_EXTI10_IRQ)
  _DEFINE_EXTI_IRQ_HANDLER(EXTI10_IRQ, 10, 10);
#endif

#if defined(USE_EXTI11_IRQ)
  _DEFINE_EXTI_IRQ_HANDLER(EXTI11_IRQ, 11, 11);
#endif

#if defined(USE_EXTI12_IRQ)
  _DEFINE_EXTI_IRQ_HANDLER(EXTI12_IRQ, 12, 12);
#endif

#if defined(USE_EXTI13_IRQ)
  _DEFINE_EXTI_IRQ_HANDLER(EXTI13_IRQ, 13, 13);
#endif

#if defined(USE_EXTI14_IRQ)
  _DEFINE_EXTI_IRQ_HANDLER(EXTI14_IRQ, 14, 14);
#endif

#if defined(USE_EXTI15_IRQ)
  _DEFINE_EXTI_IRQ_HANDLER(EXTI15_IRQ, 15, 15);
#endif

#else // STM32H7RS

#if defined(USE_EXTI9_5_IRQ)
  _DEFINE_EXTI_IRQ_HANDLER(EXTI9_5_IRQ, 5, 9);
#endif

#if defined(USE_EXTI15_10_IRQ)
  _DEFINE_EXTI_IRQ_HANDLER(EXTI15_10_IRQ, 10, 15);
#endif

#endif // !STM32H7RS

static bool _has_handler(stm32_exti_handler_t* _handlers, uint8_t len)
{
  for (uint8_t i = 0; i < len; i++)
    if (_handlers[i]) return true;
  return false;
}

void stm32_exti_enable(uint32_t line, uint8_t trigger, stm32_exti_handler_t cb)
{
  if (line > LL_EXTI_LINE_15) return;

#if defined(LL_APB2_GRP1_PERIPH_EXTI)
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_EXTI);
#elif defined(LL_CKGA_PERIPH_EXTI)
  LL_CKGA_Enable(LL_CKGA_PERIPH_EXTI);
#endif


  LL_EXTI_InitTypeDef EXTI_init;
  LL_EXTI_StructInit(&EXTI_init);

  EXTI_init.Line_0_31 = line;
  EXTI_init.Mode = LL_EXTI_MODE_IT;
  EXTI_init.Trigger = trigger;
  EXTI_init.LineCommand = ENABLE;

  LL_EXTI_Init(&EXTI_init);
  
  uint32_t line_pos = POSITION_VAL(line);

#if defined(USE_EXTI1_IRQ)
  _SET_EXTI_IRQ_HANDLER(EXTI1_IRQ, 1, 1, line_pos, cb);
#endif

#if defined(USE_EXTI2_IRQ)
  _SET_EXTI_IRQ_HANDLER(EXTI2_IRQ, 2, 2, line_pos, cb);
#endif

#if defined(USE_EXTI3_IRQ)
  _SET_EXTI_IRQ_HANDLER(EXTI3_IRQ, 3, 3, line_pos, cb);
#endif

#if defined(USE_EXTI4_IRQ)
  _SET_EXTI_IRQ_HANDLER(EXTI4_IRQ, 4, 4, line_pos, cb);
#endif

#if defined(USE_EXTI9_5_IRQ)
  _SET_EXTI_IRQ_HANDLER(EXTI9_5_IRQ, 5, 9, line_pos, cb);
#endif

#if defined(USE_EXTI15_10_IRQ)
  _SET_EXTI_IRQ_HANDLER(EXTI15_10_IRQ, 10, 15, line_pos, cb);
#endif
}

void stm32_exti_disable(uint32_t line)
{
  uint32_t line_pos = POSITION_VAL(line);

#if defined(USE_EXTI1_IRQ)
  _CLEAR_EXTI_IRQ_HANDLER(EXTI1_IRQ, 1, 1, line_pos);
#endif

#if defined(USE_EXTI2_IRQ)
  _CLEAR_EXTI_IRQ_HANDLER(EXTI2_IRQ, 2, 2, line_pos);
#endif

#if defined(USE_EXTI3_IRQ)
  _CLEAR_EXTI_IRQ_HANDLER(EXTI3_IRQ, 3, 3, line_pos);
#endif

#if defined(USE_EXTI4_IRQ)
  _CLEAR_EXTI_IRQ_HANDLER(EXTI4_IRQ, 4, 4, line_pos);
#endif

#if defined(USE_EXTI9_5_IRQ)
  _CLEAR_EXTI_IRQ_HANDLER(EXTI9_5_IRQ, 5, 9, line_pos);
#endif

#if defined(USE_EXTI15_10_IRQ)
  _CLEAR_EXTI_IRQ_HANDLER(EXTI15_10_IRQ, 10, 15, line_pos);
#endif

  LL_EXTI_DisableIT_0_31(line);
}
