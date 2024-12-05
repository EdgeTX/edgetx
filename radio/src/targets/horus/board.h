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

// Defines used in board_common.h
#define ROTARY_ENCODER_NAVIGATION

#define BOOTLOADER_KEYS 0x42

#include "board_common.h"
#include "hal.h"

#include "hal/serial_port.h"
#include "hal/watchdog_driver.h"

#if defined(HARDWARE_TOUCH)
#include "tp_gt911.h"
#endif


PACK(typedef struct {
  uint8_t pcbrev:2;
  uint8_t pxx2Enabled:1;
}) HardwareOptions;

extern HardwareOptions hardwareOptions;

#define FLASHSIZE                      0x200000
#define FLASH_PAGESIZE                 256
#define BOOTLOADER_SIZE                0x20000
#define FIRMWARE_ADDRESS               0x08000000
#define FIRMWARE_LEN(fsize)            (fsize - BOOTLOADER_SIZE)
#define FIRMWARE_MAX_LEN               (FLASHSIZE - BOOTLOADER_SIZE)
#define APP_START_ADDRESS              (uint32_t)(FIRMWARE_ADDRESS + BOOTLOADER_SIZE)

#define MB                             *1024*1024
#define LUA_MEM_EXTRA_MAX              (2 MB)    // max allowed memory usage for Lua bitmaps (in bytes)
#define LUA_MEM_MAX                    (6 MB)    // max allowed memory usage for complete Lua  (in bytes), 0 means unlimited


extern uint16_t sessionTimer;

#define SLAVE_MODE()                   (g_model.trainerData.mode == TRAINER_MODE_SLAVE)

// Board driver
void boardInit();
void boardOff();

// PCBREV driver
enum {
  // X12S
  PCBREV_X12S_LT13 = 0,
  PCBREV_X12S_GTE13 = 1,

  // X10
  PCBREV_X10_STD = 0,
  PCBREV_X10_EXPRESS = 3,

  //T15
  PCBREV_T15_STD = 0,
  PCBREV_T15_IPS = 1,
};

#if defined(SIMU)
  #define IS_FIRMWARE_COMPATIBLE_WITH_BOARD() true
#elif defined(PCBX10)
  #if defined(PCBREV_EXPRESS)
    #define IS_FIRMWARE_COMPATIBLE_WITH_BOARD() (hardwareOptions.pcbrev == PCBREV_X10_EXPRESS)
  #elif defined(RADIO_FAMILY_T16) || defined(RADIO_F16)
    #define IS_FIRMWARE_COMPATIBLE_WITH_BOARD() (true)
  #else
    #define IS_FIRMWARE_COMPATIBLE_WITH_BOARD() (hardwareOptions.pcbrev == PCBREV_X10_STD)
  #endif
#else
  #if PCBREV >= 13
    #define IS_FIRMWARE_COMPATIBLE_WITH_BOARD() (hardwareOptions.pcbrev == PCBREV_X12S_GTE13)
  #else
    #define IS_FIRMWARE_COMPATIBLE_WITH_BOARD() (hardwareOptions.pcbrev == PCBREV_X12S_LT13)
  #endif
#endif

#if defined(INTERNAL_MODULE_PXX1) || defined(INTERNAL_MODULE_PXX2)
  #define HARDWARE_INTERNAL_RAS
#endif

// Pulses driver
#if defined(RADIO_T18) || defined(RADIO_T16)

// TX18S Workaround (see https://github.com/EdgeTX/edgetx/issues/802)
// and also T16     (see https://github.com/EdgeTX/edgetx/issues/1239)
//   Add some delay after turning the internal module ON
//   on the T16, T18 & TX18S, as they seem to have issues
//   with power supply instability
//
#define INTERNAL_MODULE_ON()                                  \
  do {                                                        \
    gpio_set(INTMODULE_PWR_GPIO);			      \
    delay_ms(1);                                              \
  } while (0)

#else

// Just turn the modue ON for all other targets
#define INTERNAL_MODULE_ON()    gpio_set(INTMODULE_PWR_GPIO)

#endif

#define INTERNAL_MODULE_OFF()   gpio_clear(INTMODULE_PWR_GPIO)
#define EXTERNAL_MODULE_ON()    gpio_set(EXTMODULE_PWR_GPIO)
#define EXTERNAL_MODULE_OFF()   gpio_clear(EXTMODULE_PWR_GPIO)

#if !defined(PXX2)
  #define IS_PXX2_INTERNAL_ENABLED()            (false)
  #define IS_PXX1_INTERNAL_ENABLED()            (true)
#elif !defined(PXX1)
  #define IS_PXX2_INTERNAL_ENABLED()            (true)
  #define IS_PXX1_INTERNAL_ENABLED()            (false)
#else
  // TODO #define PXX2_PROBE
  // TODO #define IS_PXX2_INTERNAL_ENABLED()            (hardwareOptions.pxx2Enabled)
  #define IS_PXX2_INTERNAL_ENABLED()            (true)
  #define IS_PXX1_INTERNAL_ENABLED()            (true)
#endif

// POTS and SLIDERS default configuration
#if defined(RADIO_TX16S) || defined(RADIO_F16) || defined(RADIO_V16)
#define XPOS_CALIB_DEFAULT  {0x3, 0xc, 0x15, 0x1e, 0x26}
#endif

// Trims driver
#define NUM_TRIMS                               6
#define NUM_TRIMS_KEYS                          (NUM_TRIMS * 2)

// Battery driver
#if defined(RADIO_T15)
#define VOLTAGE_DROP 65
#endif

#if defined(PCBX10)
  // Lipo 2S
  #define BATTERY_WARN      66 // 6.6V
  #define BATTERY_MIN       67 // 6.7V
  #define BATTERY_MAX       83 // 8.3V
#else
  // NI-MH 9.6V
  #define BATTERY_WARN      87 // 8.7V
  #define BATTERY_MIN       85 // 8.5V
  #define BATTERY_MAX       115 // 11.5V
#endif

// bool UNEXPECTED_SHUTDOWN();
// void SET_POWER_REASON(uint32_t value);

#if defined(__cplusplus) && !defined(SIMU)
extern "C" {
#endif

// Power driver
#define SOFT_PWR_CTRL
void pwrInit();
uint32_t pwrCheck();
void pwrOn();
void pwrOff();
void pwrResetHandler();
bool pwrPressed();
bool pwrOffPressed();
#if defined(PWR_EXTRA_SWITCH_GPIO)
  bool pwrForcePressed();
#else
  #define pwrForcePressed() false
#endif
uint32_t pwrPressedDuration();

// USB Charger
void usbChargerInit();
bool usbChargerLed();

#if defined(RADIO_V16)
  uint16_t getSixPosAnalogValue(uint16_t adcValue);
#endif

// Led driver
void ledInit();
void ledOff();
void ledRed();
void ledBlue();
#if defined(PCBX10)
  void ledGreen();
#endif

// LCD driver

void lcdSetInitalFrameBuffer(void* fbAddress);

void lcdInit();
void lcdCopy(void * dest, void * src);


#define lcdOff()              backlightEnable(0) /* just disable the backlight */

#define lcdRefreshWait(...)

// Backlight driver
#define BACKLIGHT_LEVEL_MAX     100
#define BACKLIGHT_FORCED_ON     BACKLIGHT_LEVEL_MAX + 1
#if defined(PCBX12S)
#define BACKLIGHT_LEVEL_MIN   5
#elif defined(RADIO_FAMILY_T16) || defined(RADIO_X10E)
#define BACKLIGHT_LEVEL_MIN   1
#else
#define BACKLIGHT_LEVEL_MIN   46
#endif

extern bool boardBacklightOn;
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

// Audio driver
void audioInit();
void audioConsumeCurrentBuffer();
#define audioDisableIrq()             // interrupts must stay enabled on Horus
#define audioEnableIrq()              // interrupts must stay enabled on Horus
#if defined(PCBX12S)
#define setSampleRate(freq)
#else
void setSampleRate(uint32_t frequency);
#define audioWaitReady()
#endif
void setScaledVolume(uint8_t volume);
void setVolume(uint8_t volume);
int32_t getVolume();
#define VOLUME_LEVEL_MAX               23
#define VOLUME_LEVEL_DEF               12

// Telemetry driver
#define INTMODULE_FIFO_SIZE            512
#define TELEMETRY_FIFO_SIZE            512
void telemetryPortInit(uint32_t baudrate, uint8_t mode);
void telemetryPortSetDirectionInput();
void telemetryPortSetDirectionOutput();
void sportSendByte(uint8_t byte);
void sportSendBuffer(const uint8_t * buffer, uint32_t count);
bool sportGetByte(uint8_t * byte);
void telemetryClearFifo();
extern uint32_t telemetryErrors;

// soft-serial
void telemetryPortInvertedInit(uint32_t baudrate);


// Aux serial port driver
#if defined(RADIO_TX16S) || defined(RADIO_F16)
  #define DEBUG_BAUDRATE                  400000
  #define LUA_DEFAULT_BAUDRATE            115200
#else
  #define DEBUG_BAUDRATE                  115200
  #define LUA_DEFAULT_BAUDRATE            115200
#endif

const etx_serial_port_t* auxSerialGetPort(int port_nr);

// Haptic driver
void hapticInit();
void hapticDone();
void hapticOff();
void hapticOn(uint32_t pwmPercent);

// BT driver
#define BT_TX_FIFO_SIZE    64
#define BT_RX_FIFO_SIZE    256
#define BLUETOOTH_BOOTLOADER_BAUDRATE  230400
#define BLUETOOTH_FACTORY_BAUDRATE     57600
#define BLUETOOTH_DEFAULT_BAUDRATE     115200
void bluetoothInit(uint32_t baudrate, bool enable);
void bluetoothWriteWakeup();
uint8_t bluetoothIsWriting();
void bluetoothDisable();

#if defined(RADIO_TX16S) || defined(RADIO_F16) || defined(RADIO_V16)
  #define BATTERY_DIVIDER 1495
#else
  #define BATTERY_DIVIDER 1629
#endif

#if defined(FUNCTION_SWITCHES)
#define NUM_FUNCTIONS_SWITCHES 6
#define NUM_FUNCTIONS_GROUPS   3
#define DEFAULT_FS_CONFIG                                         \
  (SWITCH_2POS << 10) + (SWITCH_2POS << 8) + (SWITCH_2POS << 6) + \
      (SWITCH_2POS << 4) + (SWITCH_2POS << 2) + (SWITCH_2POS << 0)

#define DEFAULT_FS_GROUPS                                 \
  (1 << 10) + (1 << 8) + (1 << 6) + (1 << 4) + (1 << 2) + \
      (1 << 0)  // Set all FS to group 1 to act like a 6pos

#define DEFAULT_FS_STARTUP_CONFIG                         \
  ((FS_START_PREVIOUS << 10) + (FS_START_PREVIOUS << 8) + \
   (FS_START_PREVIOUS << 6) + (FS_START_PREVIOUS << 4) +  \
   (FS_START_PREVIOUS << 2) +                             \
   (FS_START_PREVIOUS << 0))  // keep last state by default

#else
#define NUM_FUNCTIONS_SWITCHES 0
#endif
