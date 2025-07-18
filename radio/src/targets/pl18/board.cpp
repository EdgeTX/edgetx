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
 
#include "stm32_adc.h"
#include "stm32_gpio.h"
#include "stm32_ws2812.h"
#include "stm32_spi.h"

#include "hal/adc_driver.h"
#include "hal/trainer_driver.h"
#include "hal/switch_driver.h"
#include "hal/abnormal_reboot.h"
#include "hal/watchdog_driver.h"
#include "hal/usb_driver.h"
#include "hal/gpio.h"
#include "hal/rotary_encoder.h"

#include "board.h"
#include "boards/generic_stm32/analog_inputs.h"
#include "boards/generic_stm32/module_ports.h"
#include "boards/generic_stm32/rgb_leds.h"

#include "globals.h"
#include "sdcard.h"
#include "touch.h"
#include "debug.h"

#if defined(AUDIO_SPI)
  #include "vs1053b.h"
#endif

#include "timers_driver.h"
#include "battery_driver.h"
#include "touch_driver.h"

#include "bitmapbuffer.h"
#include "colors.h"

#include <string.h>

// Common ADC driver
extern const etx_hal_adc_driver_t _adc_driver;

extern "C" void SDRAM_Init();

#if defined(SEMIHOSTING)
extern "C" void initialise_monitor_handles();
#endif

#if defined(SPI_FLASH)
extern "C" void flushFTL();
#endif

#if defined(RADIO_NV14_FAMILY)
  HardwareOptions hardwareOptions;

  static uint8_t boardGetPcbRev()
  {
    delaysInit();
    gpio_init(INTMODULE_PWR_GPIO, GPIO_IN, GPIO_PIN_SPEED_LOW);
    delay_ms(1); // delay to let the input settle, else it does not work properly

    // detect NV14 vs EL18
    if (gpio_read(INTMODULE_PWR_GPIO)) {
      // pull-up connected: EL18
      return PCBREV_EL18;
    } else {
      // pull-down connected: NV14
      return PCBREV_NV14;
    }
  }
#endif

#if defined(AUDIO_SPI)
static void audio_set_rst_pin(bool set)
{
  gpio_write(AUDIO_RST_GPIO, set);
}

static void audio_set_mute_pin(bool set)
{
#if defined(INVERTED_MUTE_PIN)
  gpio_write(AUDIO_MUTE_GPIO, !set);
#else
  gpio_write(AUDIO_MUTE_GPIO, set);
#endif
}

static void audioInit()
{
  static stm32_spi_t spi_dev = {
      .SPIx = AUDIO_SPI,
      .SCK = AUDIO_SPI_SCK_GPIO,
      .MISO = AUDIO_SPI_MISO_GPIO,
      .MOSI = AUDIO_SPI_MOSI_GPIO,
      .CS = AUDIO_CS_GPIO,
  };

  static vs1053b_t vs1053 = {
      .spi = &spi_dev,
      .XDCS = AUDIO_XDCS_GPIO,
      .DREQ = AUDIO_DREQ_GPIO,
      .set_rst_pin = audio_set_rst_pin,
      .set_mute_pin = audio_set_mute_pin,
      .mute_delay_ms = AUDIO_MUTE_DELAY,
      .unmute_delay_ms = AUDIO_UNMUTE_DELAY,
  };

  gpio_init(AUDIO_RST_GPIO, GPIO_OUT, 0);
  gpio_init(AUDIO_MUTE_GPIO, GPIO_OUT, 0);
  audio_set_mute_pin(true);

  vs1053b_init(&vs1053);
}
#endif

void delay_self(int count)
{
   for (int i = 50000; i > 0; i--)
   {
       for (; count > 0; count--);
   }
}

#if defined(LED_STRIP_GPIO)
// Common LED driver
extern const stm32_pulse_timer_t _led_timer;

void ledStripOff()
{
  for (uint8_t i = 0; i < LED_STRIP_LENGTH; i++) {
    ws2812_set_color(i, 0, 0, 0);
  }
  ws2812_update(&_led_timer);
}
#endif

#if defined(RADIO_NB4P)
void disableVoiceChip()
{
  gpio_init(VOICE_CHIP_EN_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_clear(VOICE_CHIP_EN_GPIO);
}
#endif

void boardBLEarlyInit()
{
#if defined(RADIO_PL18U)
  pwrOn();
#endif  
  // USB charger status pins
  gpio_init(UCHARGER_GPIO, GPIO_IN, GPIO_PIN_SPEED_LOW);

#if defined(USB_SW_GPIO)
  gpio_init(USB_SW_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
#endif

#if defined(RADIO_NV14_FAMILY)
  // detect NV14 vs EL18
  hardwareOptions.pcbrev = boardGetPcbRev();
#endif
}

void boardBLPreJump()
{
  SDRAM_Init();
#if defined(RADIO_NB4P)
  LL_ADC_Disable(ADC_MAIN);
#endif
}

void boardBLInit()
{
  SDRAM_Init();
}

static void monitorInit()
{
#if defined(VBUS_MONITOR_GPIO)
  gpio_init(VBUS_MONITOR_GPIO, GPIO_IN, GPIO_PIN_SPEED_LOW);
#endif
}

void boardInit()
{
#if defined(SEMIHOSTING)
  initialise_monitor_handles();
#endif

#if !defined(SIMU)
  // enable interrupts
  __enable_irq();
#endif

#if defined(RADIO_NV14_FAMILY)
  // detect NV14 vs EL18
  hardwareOptions.pcbrev = boardGetPcbRev();
  TRACE("\n%s board started :)",
        hardwareOptions.pcbrev == PCBREV_NV14 ?
        "NV14" : "EL18");
#else
  TRACE("\nPL18 board started :)");
#endif

  delay_ms(10);
  TRACE("RCC->CSR = %08x", RCC->CSR);

  pwrInit();
  boardInitModulePorts();

#if defined(AUDIO_SPI)
  gpio_init(AUDIO_RST_GPIO, GPIO_OUT, GPIO_PIN_SPEED_MEDIUM);
  gpio_init(AUDIO_MUTE_GPIO, GPIO_OUT, GPIO_PIN_SPEED_MEDIUM);
#endif

  board_trainer_init();
  battery_charge_init();

  gimbalsDetect();  
  timersInit();
  touchPanelInit();
  usbInit();

#if defined(LED_STRIP_GPIO)
  rgbLedInit();
#endif

  uint32_t press_start = 0;
  uint32_t press_end = 0;

  if (UNEXPECTED_SHUTDOWN()) {
    pwrOn();
  } else if (isChargerActive()) {
    while (true) {
      pwrOn();
      uint32_t now = timersGetMsTick();
      if (pwrPressed()) {
        press_end = now;
        if (press_start == 0) press_start = now;
        if ((now - press_start) > POWER_ON_DELAY) {
          break;
        }
      } else if (!isChargerActive()) {
        boardOff();
      } else {
        uint32_t press_end_touch = press_end;
        if (touchPanelEventOccured()) {
          touchPanelRead();
          press_end_touch = timersGetMsTick();
        }
        press_start = 0;
        handle_battery_charge(press_end_touch);
        delay_ms(10);
        press_end = 0;
      }
    }
    battery_charge_end();
  }

  keysInit();
  switchInit();
#if defined(ROTARY_ENCODER_NAVIGATION) && !defined(USE_HATS_AS_KEYS)
  rotaryEncoderInit();
#endif
#if defined(RADIO_NB4P)
  disableVoiceChip();
#endif

  audioInit();
  monitorInit();
  adcInit(&_adc_driver);
  hapticInit();


 #if defined(RTCLOCK)
  rtcInit(); // RTC must be initialized before rambackupRestore() is called
#endif
#if defined(LED_STRIP_GPIO)
#if !defined(POWER_LED_BLUE)
  ledBlue();
#else
  ledGreen();
#endif
#endif
}

extern void rtcDisableBackupReg();

void boardOff()
{
  lcdOff();

  while (pwrPressed()) {
    WDG_RESET();
  }

  SysTick->CTRL = 0; // turn off systick

  // Shutdown the Haptic
  hapticDone();

  rtcDisableBackupReg();

#if !defined(BOOT)
#if defined(LED_STRIP_GPIO)
  ledStripOff();
#endif
  if (isChargerActive())
  {
    NVIC_SystemReset();
  }
  else
#endif
  {    
    pwrOff();
  }

  // We reach here only in forced power situations, such as hw-debugging with external power  
  // Enter STM32 stop mode / deep-sleep
  // Code snippet from ST Nucleo PWR_EnterStopMode example
#define PDMode             0x00000000U
#if defined(PWR_CR_MRUDS) && defined(PWR_CR_LPUDS) && defined(PWR_CR_FPDS)
  MODIFY_REG(PWR->CR, (PWR_CR_PDDS | PWR_CR_LPDS | PWR_CR_FPDS | PWR_CR_LPUDS | PWR_CR_MRUDS), PDMode);
#elif defined(PWR_CR_MRLVDS) && defined(PWR_CR_LPLVDS) && defined(PWR_CR_FPDS)
  MODIFY_REG(PWR->CR, (PWR_CR_PDDS | PWR_CR_LPDS | PWR_CR_FPDS | PWR_CR_LPLVDS | PWR_CR_MRLVDS), PDMode);
#else
  MODIFY_REG(PWR->CR, (PWR_CR_PDDS| PWR_CR_LPDS), PDMode);
#endif // PWR_CR_MRUDS && PWR_CR_LPUDS && PWR_CR_FPDS

  // Set SLEEPDEEP bit of Cortex System Control Register
  SET_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SLEEPDEEP_Msk));
  
  // To avoid HardFault at return address, end in an endless loop
  while (1) {

  }
}

#if !defined(RADIO_NV14_FAMILY)
int usbPlugged()
{
  static uint8_t debouncedState = 0;
  static uint8_t lastState = 0;

  uint8_t state = IS_UCHARGER_ACTIVE();

  if (state == lastState)
    debouncedState = state;
  else
    lastState = state;
  
  return debouncedState;
}
#endif
