// SPDX-License-Identifier: GPL-2.0-only
#include "board.h"
#include "hal/key_driver.h"
#include "hal/rotary_encoder.h"
#include "stm32_exti_driver.h"
#include "targets/mambo/encoder.h"
#include <cstdio>
#include <cstdlib>
#include <initializer_list>

#define CHECK(x) do { if (!(x)) { std::fprintf(stderr, "line %d: %s\n", __LINE__, #x); std::exit(1); } } while (0)
GPIO_TypeDef encoderGpio;
TIM_TypeDef encoderTimer;
ExtiRegisters exti;
uint32_t interruptMask = 0, extiPort[16]{};
bool irqEnabled[3]{};
static stm32_exti_handler_t handlers[16]{};
static uint32_t pressedKeys = 0;
uint32_t readKeys() { return pressedKeys; }
void stm32_gpio_enable_clock(GPIO_TypeDef*) {}
void stm32_timer_enable_clock(TIM_TypeDef*) {}
void stm32_exti_enable(uint32_t line, uint8_t, stm32_exti_handler_t handler)
{
  const unsigned bit = __builtin_ctz(line);
  handlers[bit] = handler;
  exti.IMR |= line;
  NVIC_EnableIRQ(bit < 10 ? EXTI9_5_IRQn : EXTI15_10_IRQn);
}
void stm32_exti_disable(uint32_t line)
{
  const unsigned bit = __builtin_ctz(line);
  handlers[bit] = nullptr;
  exti.IMR &= ~line;
  bool any = false;
  for (unsigned i = bit < 10 ? 5 : 10; i <= (bit < 10 ? 9u : 15u); ++i)
    any = any || handlers[i];
  if (!any) NVIC_DisableIRQ(bit < 10 ? EXTI9_5_IRQn : EXTI15_10_IRQn);
}
extern "C" void TIM1_UP_TIM10_IRQHandler();
static void sample(unsigned pins)
{
  encoderGpio.IDR = ((pins & 1) << 8) | ((pins & 2) << 9);
  CHECK(encoderTimer.CR1 & TIM_CR1_CEN);
  encoderTimer.SR |= TIM_SR_UIF;
  TIM1_UP_TIM10_IRQHandler();
  CHECK(!(encoderTimer.SR & TIM_SR_UIF));
}
static void turn(bool reverse, unsigned detents)
{
  for (unsigned i = 0; i < detents; i += 2) {
    const unsigned a = reverse ? 2 : 1, b = reverse ? 1 : 2;
    for (unsigned state : {a, 0u, a, a, 3u, b, 3u, b, 0u}) sample(state);
  }
}
static void otherExti() {}

int main()
{
  rotaryEncoderInit();
  CHECK(handlers[8] && handlers[10]);
  handlers[8](); sample(0);
  CHECK(!(encoderTimer.CR1 & TIM_CR1_CEN));
  rotaryEncoderInit();
  stm32_exti_enable(1u << 14, 3, otherExti);
  exti.PR = LL_EXTI_LINE_8 | LL_EXTI_LINE_10 | (1u << 14);
  extiPort[10] = 4;  // Another peripheral may own this shared route.
  mambo::encoderStartPolling();
  CHECK(interruptMask == 0 && !handlers[8] && !handlers[10]);
  CHECK(exti.IMR == (1u << 14) && exti.PR == (1u << 14));
  CHECK(encoderTimer.PSC == 167 && encoderTimer.ARR == 499);
  CHECK(irqEnabled[TIM1_UP_TIM10_IRQn] && irqEnabled[EXTI15_10_IRQn]);
  const auto before = rotaryEncoderGetValue();
  turn(false, 20);
  CHECK(rotaryEncoderGetValue() == before - 20);
  turn(true, 20);
  CHECK(rotaryEncoderGetValue() == before);
  sample(0);
  CHECK(rotaryEncoderGetValue() == before);
  pressedKeys = 1u << KEY_ENTER;
  turn(false, 4);
  CHECK(rotaryEncoderGetValue() == before);
  pressedKeys = 0;
  turn(false, 4);
  CHECK(rotaryEncoderGetValue() == before - 4);
  CHECK(extiPort[10] == 4);

  interruptMask = 1;
  mambo::encoderStartPolling();
  CHECK(interruptMask == 1);
  interruptMask = 0;
  encoderGpio.IDR = LL_GPIO_PIN_8;  // Start between detents.
  rotaryEncoderInit();
  mambo::encoderStartPolling();
  const auto offDetent = rotaryEncoderGetValue();
  sample(1); sample(3);
  CHECK(rotaryEncoderGetValue() == offDetent);
  turn(false, 4);
  CHECK(encoderTimer.CR1 & TIM_CR1_CEN);
  std::puts("Standalone Mambo polling: timer clock, EXTI ownership, mask, bounce, directions, ENTER and startup passed.");
}
