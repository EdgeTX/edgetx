// SPDX-License-Identifier: GPL-2.0-only
#include "encoder.h"
#include "board.h"
#include "hal/rotary_encoder.h"
#include "stm32_exti_driver.h"
#include "stm32_hal_ll.h"

void mambo::encoderStartPolling()
{
  const uint32_t mask = __get_PRIMASK();
  __disable_irq();
  // Sample both phases independently of the shared EXTI routing.
  stm32_exti_disable(ROTARY_ENCODER_EXTI_LINE1);
  stm32_exti_disable(ROTARY_ENCODER_EXTI_LINE2);
  LL_EXTI_ClearFlag_0_31(ROTARY_ENCODER_EXTI_LINE1 |
                        ROTARY_ENCODER_EXTI_LINE2);
  // TIM10 uses APB2, not the APB1 clock assumed by the generic initializer.
  ROTARY_ENCODER_TIMER->PSC =
      (PERI2_FREQUENCY * TIMER_MULT_APB2) / 1000000 - 1;
  rotaryEncoderStartPolling();
  __set_PRIMASK(mask);
}
