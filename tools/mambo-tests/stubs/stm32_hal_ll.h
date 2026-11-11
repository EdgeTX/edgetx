// SPDX-License-Identifier: GPL-2.0-only
#pragma once
#include <cstdint>

struct GPIO_TypeDef { uint32_t IDR = 0; };
struct TIM_TypeDef {
  uint32_t CR1 = 0, ARR = 0, PSC = 0, CCER = 0, CCMR1 = 0;
  uint32_t EGR = 0, DIER = 0, SR = 0;
};
struct ExtiRegisters { uint32_t IMR = 0, PR = 0; };
struct LL_GPIO_InitTypeDef { uint32_t Mode = 0, Pull = 0, Pin = 0; };
enum IRQn_Type { EXTI9_5_IRQn, EXTI15_10_IRQn, TIM1_UP_TIM10_IRQn };
extern GPIO_TypeDef encoderGpio;
extern TIM_TypeDef encoderTimer;
extern ExtiRegisters exti;
extern uint32_t interruptMask, extiPort[16];
extern bool irqEnabled[3];
#define GPIOA (&encoderGpio)
#define TIM10 (&encoderTimer)
#define EXTI (&exti)
#define LL_GPIO_PIN_8 (1u << 8)
#define LL_GPIO_PIN_10 (1u << 10)
#define LL_EXTI_LINE_8 (1u << 8)
#define LL_EXTI_LINE_10 (1u << 10)
#define LL_GPIO_MODE_INPUT 0u
#define LL_GPIO_PULL_UP 1u
#define LL_EXTI_TRIGGER_RISING_FALLING 3u
#define LL_APB2_GRP1_PERIPH_SYSCFG 1u
#define LL_SYSCFG_EXTI_PORTA 0u
#define LL_SYSCFG_EXTI_LINE8 8u
#define LL_SYSCFG_EXTI_LINE10 10u
#define TIM_CR1_CEN 1u
#define TIM_CR1_URS 4u
#define TIM_DIER_UIE 1u
#define TIM_SR_UIF 1u
#define TIM_EGR_UG 1u
inline uint32_t __get_PRIMASK() { return interruptMask; }
inline void __disable_irq() { interruptMask = 1; }
inline void __set_PRIMASK(uint32_t value) { interruptMask = value; }
inline void NVIC_EnableIRQ(IRQn_Type irq) { irqEnabled[irq] = true; }
inline void NVIC_DisableIRQ(IRQn_Type irq) { irqEnabled[irq] = false; }
inline void NVIC_SetPriority(IRQn_Type, unsigned) {}
inline void LL_GPIO_StructInit(LL_GPIO_InitTypeDef* init) { *init = {}; }
inline void LL_GPIO_Init(GPIO_TypeDef*, LL_GPIO_InitTypeDef*) {}
inline void LL_APB2_GRP1_EnableClock(unsigned) {}
inline void LL_SYSCFG_SetEXTISource(unsigned port, unsigned line) { extiPort[line] = port; }
inline void LL_EXTI_ClearFlag_0_31(uint32_t lines) { EXTI->PR &= ~lines; }
