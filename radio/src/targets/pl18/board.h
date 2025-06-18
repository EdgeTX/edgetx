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

#pragma once

#include "definitions.h"
#include "edgetx_constants.h"

#include "board_common.h"
#include "hal.h"
#include "hal/serial_port.h"
#include "hal/watchdog_driver.h"

#if defined(ADC_GPIO_PIN_STICK_TH)
#define SURFACE_RADIO  true
#endif

#define FLASHSIZE                       0x200000
#define FLASH_PAGESIZE                  256
#define BOOTLOADER_SIZE                 0x20000
#define FIRMWARE_ADDRESS                0x08000000
#define FIRMWARE_LEN(fsize)             (fsize - BOOTLOADER_SIZE)
#define FIRMWARE_MAX_LEN                (FLASHSIZE - BOOTLOADER_SIZE)
#define APP_START_ADDRESS               (uint32_t)(FIRMWARE_ADDRESS + BOOTLOADER_SIZE)

#define MB                              *1024*1024
#define LUA_MEM_EXTRA_MAX               (2 MB)    // max allowed memory usage for Lua bitmaps (in bytes)
#define LUA_MEM_MAX                     (6 MB)    // max allowed memory usage for complete Lua  (in bytes), 0 means unlimited

#define BOOTLOADER_KEYS 0x42

extern uint16_t sessionTimer;

#define SLAVE_MODE()                    (g_model.trainerData.mode == TRAINER_MODE_SLAVE)

// Board driver
void boardInit();
void boardOff();

// CPU Unique ID
#define LEN_CPU_UID                     (3*8+2)
void getCPUUniqueID(char * s);

#if defined(RADIO_NV14_FAMILY)
  enum {
    PCBREV_NV14 = 0,
    PCBREV_EL18 = 1,
  };
  
  #define HAS_HARDWARE_OPTIONS

  typedef struct {
    uint8_t pcbrev;
  } HardwareOptions;

  extern HardwareOptions hardwareOptions;
#endif

// SDRAM driver
extern "C" void SDRAM_Init();

// Pulses driver
#if !defined(SIMU)

#if defined(RADIO_NV14_FAMILY)
  #define INTERNAL_MODULE_OFF()                 \
  do {                                          \
    if (hardwareOptions.pcbrev == PCBREV_NV14)  \
      gpio_set(INTMODULE_PWR_GPIO);			    \
    else                                        \
      gpio_clear(INTMODULE_PWR_GPIO);           \
  } while (0)

  #define INTERNAL_MODULE_ON()                  \
  do {                                          \
    if (hardwareOptions.pcbrev == PCBREV_NV14)  \
      gpio_clear(INTMODULE_PWR_GPIO);           \
    else                                        \
      gpio_set(INTMODULE_PWR_GPIO);             \
  } while (0)
#elif defined(RADIO_NB4P) || defined(RADIO_PL18U)
  #define INTERNAL_MODULE_ON()            gpio_clear(INTMODULE_PWR_GPIO)
  #define INTERNAL_MODULE_OFF()           gpio_set(INTMODULE_PWR_GPIO);
#else
  #define INTERNAL_MODULE_ON()            gpio_set(INTMODULE_PWR_GPIO)
  #define INTERNAL_MODULE_OFF()           gpio_clear(INTMODULE_PWR_GPIO)
#endif

#define EXTERNAL_MODULE_ON()            gpio_set(EXTMODULE_PWR_GPIO)
#define EXTERNAL_MODULE_OFF()           gpio_clear(EXTMODULE_PWR_GPIO)
#define EXTERNAL_MODULE_PWR_OFF         EXTERNAL_MODULE_OFF
#define BLUETOOTH_MODULE_ON()           gpio_clear(BLUETOOTH_ON_GPIO)
#define BLUETOOTH_MODULE_OFF()          gpio_set(BLUETOOTH_ON_GPIO)
//#define IS_INTERNAL_MODULE_ON()         (false)
//#define IS_EXTERNAL_MODULE_ON()         (gpio_read(EXTMODULE_PWR_GPIO) ? 1 : 0)

#else // defined(SIMU)

#define INTERNAL_MODULE_ON()
#define INTERNAL_MODULE_OFF()
#define EXTERNAL_MODULE_ON()
#define EXTERNAL_MODULE_OFF()
#define BLUETOOTH_MODULE_ON()
#define BLUETOOTH_MODULE_OFF()
//#define IS_INTERNAL_MODULE_ON()         (false)
//#define IS_EXTERNAL_MODULE_ON()         (false)

#endif // defined(SIMU)

#if !defined(NUM_FUNCTIONS_SWITCHES)
#define NUM_FUNCTIONS_SWITCHES        0
#endif

#define NUM_TRIMS                       8
#define DEFAULT_STICK_DEADZONE          2

#if defined(RADIO_NV14_FAMILY)
  #define BATTERY_WARN                  36 // 3.6V
  #define BATTERY_MIN                   35 // 3.5V
  #define BATTERY_MAX                   42 // 4.2V
#else
  // LiHV battery thresholds
  #define BATTERY_WARN                  37 // 3.7V
  #define BATTERY_MIN                   35 // 3.4V
  #define BATTERY_MAX                   43 // 4.3V
#endif

#if defined(RADIO_NV14_FAMILY)
  #define BATTERY_DIVIDER               3102 // = 2047 * 510k / (510k + 510k) * 10 / 3.3V
#elif defined(RADIO_NB4P)
  #define BATTERY_DIVIDER               3102 // = 2047 * 10k / (10k + 10k) * 10 / 3.3V
#else
  #define BATTERY_DIVIDER               962  // = 2047 * 22k / (120k + 22k) * 10 / 3.3V
#endif

#if defined(__cplusplus) && !defined(SIMU)
extern "C" {
#endif

// Power driver
#define SOFT_PWR_CTRL
#define POWER_ON_DELAY               100 // ms
void pwrInit();
void extModuleInit();
uint32_t pwrCheck();
uint32_t lowPowerCheck();

void pwrOn();
void pwrSoftReboot();
void pwrOff();
void pwrResetHandler();
bool pwrPressed();
bool pwrOffPressed();
#if defined(PWR_EXTRA_SWITCH_GPIO)
  bool pwrForcePressed();
#else
  #define pwrForcePressed() false
#endif
uint32_t pwrPressedDuration();;
  
const etx_serial_port_t* auxSerialGetPort(int port_nr);
#define AUX_SERIAL_POWER_ON()
#define AUX_SERIAL_POWER_OFF()

// LED driver
void ledInit();
void ledOff();
void ledRed();
void ledBlue();
void ledGreen();

// LCD driver
void lcdSetInitalFrameBuffer(void* fbAddress);
void lcdInit();
void lcdCopy(void * dest, void * src);

void lcdOff();
void lcdOn();

#define lcdRefreshWait(...)

// Backlight driver
#define BACKLIGHT_LEVEL_MAX             100
#define BACKLIGHT_FORCED_ON             BACKLIGHT_LEVEL_MAX + 1
#define BACKLIGHT_LEVEL_MIN             1

extern bool boardBacklightOn;
void backlightLowInit( void );
void backlightInit();
void backlightEnable(uint8_t dutyCycle);
void backlightFullOn();
bool isBacklightEnabled();

#define BACKLIGHT_ENABLE()                                         \
  {                                                                \
    boardBacklightOn = true;                                       \
    backlightEnable(BACKLIGHT_LEVEL_MAX - currentBacklightBright); \
  }

#define BACKLIGHT_DISABLE()                                               \
  {                                                                       \
    boardBacklightOn = false;                                             \
    backlightEnable(((g_eeGeneral.blOffBright == BACKLIGHT_LEVEL_MIN) &&  \
                     (g_eeGeneral.backlightMode != e_backlight_mode_off)) \
                        ? 0                                               \
                        : g_eeGeneral.blOffBright);                       \
  }

#if defined(__cplusplus) && !defined(SIMU)
}
#endif

#if defined(RADIO_NB4P) || defined(RADIO_NV14_FAMILY)
  #define IS_UCHARGER_ACTIVE()              gpio_read(UCHARGER_GPIO) ? (gpio_read(UCHARGER_CHARGE_END_GPIO) ? 0 : 1) : 1  
#else
  #define IS_UCHARGER_ACTIVE()              gpio_read(UCHARGER_GPIO) ? 1 : 0
#endif

#if defined(RADIO_NB4P) || defined(RADIO_NV14_FAMILY)
  #define IS_UCHARGER_CHARGE_END_ACTIVE()   gpio_read(UCHARGER_CHARGE_END_GPIO) ? 0 : 1
#else
  #define IS_UCHARGER_CHARGE_END_ACTIVE()   gpio_read(UCHARGER_CHARGE_END_GPIO) ? 1 : 0
#endif

#if defined(UCHARGER_EN_GPIO)
  #if defined(RADIO_NV14_FAMILY)
    #define ENABLE_UCHARGER()               gpio_clear(UCHARGER_EN_GPIO)
    #define DISABLE_UCHARGER()              gpio_set(UCHARGER_EN_GPIO)
  #else
    #define ENABLE_UCHARGER()               gpio_set(UCHARGER_EN_GPIO)
    #define DISABLE_UCHARGER()              gpio_clear(UCHARGER_EN_GPIO)
  #endif
#else
  #define ENABLE_UCHARGER()
  #define DISABLE_UCHARGER()
#endif

#if !defined(AUDIO_SPI)
  // DAC Audio driver
  void audioInit();
#endif

// Telemetry driver
#define INTMODULE_FIFO_SIZE            512
#define TELEMETRY_FIFO_SIZE            512

// Haptic driver
void hapticInit();
void hapticDone();
void hapticOff();
void hapticOn(uint32_t pwmPercent);

// Second serial port driver
#define DEBUG_BAUDRATE                  115200
#define LUA_DEFAULT_BAUDRATE            115200

// Touch panel driver
bool touchPanelEventOccured();
struct TouchState touchPanelRead();
struct TouchState getInternalTouchState();
