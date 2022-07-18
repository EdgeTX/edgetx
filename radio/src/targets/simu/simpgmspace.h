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

#ifndef _SIMPGMSPACE_H_
#define _SIMPGMSPACE_H_

#ifndef __GNUC__
#include <windows.h>
#include <intrin.h>
#define sleep(x) Sleep(x)
#define strcasecmp  _stricmp
#define strncasecmp _tcsnicmp
#define chdir  _chdir
#define getcwd _getcwd
inline int __builtin_clz(unsigned x)
{
    return (int)__lzcnt(x);
}
#else
#include <unistd.h>
#define sleep(x) usleep(1000*x)
#endif

#ifdef SIMU_EXCEPTIONS
extern char * main_thread_error;
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#if defined(_MSC_VER) || !defined(__GNUC__)
#define write_backtrace(output)
#else
#include <execinfo.h>
inline void write_backtrace(char *output)
{
  void *buf[16];
  char **s;
  int n = backtrace(buf, 16);
  s = backtrace_symbols(buf, n);
  if (s) {
    for (int i=0; i<n; ++i) {
      sprintf(output+strlen(output), "%02i: %s\n", i, s[i]);
    }
  }
}
#endif
void sig(int sgn)
{
  main_thread_error = (char *)malloc(2048);
  sprintf(main_thread_error,"Signal %d caught\n", sgn);
  write_backtrace(main_thread_error);
  throw std::exception();
}
#define assert(x) do { if (!(x)) { main_thread_error = (char *)malloc(2048); sprintf(main_thread_error, "Assert failed, %s:%d: %s\n", __FILE__, __LINE__, #x); write_backtrace(main_thread_error); throw std::exception(); } } while(0)
#else
#include <assert.h>
#endif

#include <inttypes.h>
#include <stdio.h>
#include <stddef.h>
#include <errno.h>

#undef min
#undef max

#define __REV

extern GPIO_TypeDef gpioa, gpiob, gpioc, gpiod, gpioe, gpiof, gpiog, gpioh, gpioi, gpioj;
extern TIM_TypeDef tim1, tim2, tim3, tim4, tim5, tim6, tim7, tim8, tim9, tim10;
extern USART_TypeDef Usart0, Usart1, Usart2, Usart3, Usart4;
extern RCC_TypeDef rcc;
extern DMA_Stream_TypeDef dma1_stream0, dma1_stream1, dma1_stream2, dma1_stream3, dma1_stream4, dma1_stream5, dma1_stream6, dma1_stream7, dma2_stream1, dma2_stream2, dma2_stream5, dma2_stream6, dma2_stream7;
extern DMA_TypeDef dma2;
extern SysTick_Type systick;
extern ADC_Common_TypeDef adc;
extern RTC_TypeDef rtc;
#undef SysTick
#define SysTick (&systick)
#undef GPIOA
#undef GPIOB
#undef GPIOC
#undef GPIOD
#undef GPIOE
#undef GPIOF
#undef GPIOG
#undef GPIOH
#undef GPIOI
#undef GPIOJ
#define GPIOA (&gpioa)
#define GPIOB (&gpiob)
#define GPIOC (&gpioc)
#define GPIOD (&gpiod)
#define GPIOE (&gpioe)
#define GPIOF (&gpiof)
#define GPIOG (&gpiog)
#define GPIOH (&gpioh)
#define GPIOI (&gpioi)
#define GPIOJ (&gpioj)
#undef TIM1
#undef TIM2
#undef TIM3
#undef TIM4
#undef TIM5
#undef TIM6
#undef TIM7
#undef TIM8
#undef TIM9
#undef TIM10
#define TIM1 (&tim1)
#define TIM2 (&tim2)
#define TIM3 (&tim3)
#define TIM4 (&tim4)
#define TIM5 (&tim4)
#define TIM6 (&tim4)
#define TIM7 (&tim4)
#define TIM8 (&tim8)
#define TIM9 (&tim9)
#define TIM10 (&tim10)
#undef USART0
#undef USART1
#undef USART2
#undef USART3
#define USART0 (&Usart0)
#define USART1 (&Usart1)
#define USART2 (&Usart2)
#define USART3 (&Usart3)
#undef RCC
#define RCC (&rcc)
#undef DMA1_Stream0
#undef DMA1_Stream1
#undef DMA1_Stream2
#undef DMA1_Stream3
#undef DMA1_Stream4
#undef DMA1_Stream5
#undef DMA1_Stream7
#undef DMA2_Stream1
#undef DMA2_Stream2
#undef DMA2_Stream5
#undef DMA2_Stream6
#undef DMA2_Stream7
#define DMA1_Stream0 (&dma1_stream0)
#define DMA1_Stream1 (&dma1_stream1)
#define DMA1_Stream2 (&dma1_stream2)
#define DMA1_Stream3 (&dma1_stream3)
#define DMA1_Stream4 (&dma1_stream4)
#define DMA1_Stream5 (&dma1_stream5)
#define DMA1_Stream7 (&dma1_stream7)
#define DMA2_Stream1 (&dma2_stream1)
#define DMA2_Stream2 (&dma2_stream2)
#define DMA2_Stream5 (&dma2_stream5)
#define DMA2_Stream6 (&dma2_stream6)
#define DMA2_Stream7 (&dma2_stream7)
#undef DMA2
#define DMA2 (&dma2)
#undef ADC
#define ADC (&adc)
#undef RTC
#define RTC (&rtc)

extern uint8_t * eeprom;
extern void startPdcUsartReceive() ;
extern uint32_t txPdcUsart( uint8_t *buffer, uint32_t size );
extern uint32_t txPdcPending();
extern void rxPdcUsart( void (*pChProcess)(uint8_t x) );

#define ISR(x, ...)  void x()

#if !defined(_MSC_VER) && defined(__GNUC__)
#define asm(...)
#endif

extern uint32_t Master_frequency;
#define NVIC_EnableIRQ(x)
#define NVIC_DisableIRQ(x)
#define NVIC_SetPriority(...)
#define NVIC_SystemReset() exit(0)
#define __disable_irq()
#define __enable_irq()

extern uint8_t simu_start_mode;
extern char * main_thread_error;

#define OPENTX_START_DEFAULT_ARGS  simu_start_mode

inline void getADC() { }

uint64_t simuTimerMicros(void);
uint8_t simuSleep(uint32_t ms);  // returns true if thread shutdown requested

void simuSetKey(uint8_t key, bool state);
void simuSetTrim(uint8_t trim, bool state);
void simuSetSwitch(uint8_t swtch, int8_t state);

#if defined(__cplusplus)
void simuInit();
void simuStart(bool tests = true, const char * sdPath = nullptr, const char * settingsPath = nullptr);
void simuStop();
bool simuIsRunning();
void startEepromThread(const char * filename = "eeprom.bin");
void stopEepromThread();

#if defined(SIMU_AUDIO)
  void startAudioThread(int volumeGain = 10);
  void stopAudioThread(void);
#else
  #define startAudioThread(dummy)
  #define stopAudioThread()
#endif
#endif

void simuMain();

#define UART_Stop(...)
#define UART3_Stop(...)
#define USART_GetITStatus(...)         0
#define USART_ClearFlag(...)

#define TIM_DeInit(...)
#define TIM_SetCompare2(...)
#define TIM_ClearFlag(...)
#define TIM_Cmd(...)
#define TIM_ITConfig(...)
#define GPIO_SetBits(GPIOx, pin) GPIOx->BSRRL |= pin
#define GPIO_ResetBits(GPIOx, pin) GPIOx->BSRRL &= ~pin
#define GPIO_ReadInputDataBit(GPIOx, pin) (GPIOx->BSRRL & pin)
#define RCC_AHB1PeriphClockCmd(...)
#define RCC_APB2PeriphClockCmd(...)

// inline void delay_01us(uint32_t dummy) { }

#define configure_pins(...)

#if (defined(SDCARD) || (defined(SPI_FLASH) && !defined(LITTLEFS))) && !defined(SKIP_FATFS_DECLARATION) && !defined(SIMU_DISKIO)
  #define SIMU_USE_FAT_EMU
#endif

#if defined(SIMU_DISKIO)
  uint32_t sdMounted();
  #define sdPoll10ms()
  void sdInit(void);
  void sdDone(void);
#else
  #define sdPoll10ms()
  uint32_t sdMounted(void);
  #define sdMounted()      (true)
#endif

#if defined(SIMU_USE_FAT_EMU)
  void simuFatfsSetPaths(const char * sdPath, const char * settingsPath);
#else
  #define simuFatfsSetPaths(...)
#endif

#if defined(TRACE_SIMPGMSPACE)
  #undef TRACE_SIMPGMSPACE
  #define TRACE_SIMPGMSPACE   TRACE
#else
  #define TRACE_SIMPGMSPACE(...)
#endif

#if defined(HARDWARE_TOUCH)
  extern struct TouchState simTouchState;
  extern bool simTouchOccured;
#endif

#endif // _SIMPGMSPACE_H_
