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

#ifndef _BOARD_H_
#define _BOARD_H_

#include "definitions.h"
#include "edgetx_constants.h"

#include "board_common.h"
#include "hal.h"
#include "hal/serial_port.h"
#include "hal/watchdog_driver.h"

#define FLASHSIZE                       0x800000
#define FLASH_PAGESIZE                  256
#define BOOTLOADER_SIZE                 0x10000
#define BOOTLOADER_ADDRESS              0x08000000
#define FIRMWARE_ADDRESS                0x90000000
#define FIRMWARE_LEN(fsize)             (fsize)
#define FIRMWARE_MAX_LEN                FLASHSIZE
#define APP_START_ADDRESS               (uint32_t)(FIRMWARE_ADDRESS)

#define BOOTLOADER_KEYS                 0x42

#define MB                              *1024*1024
#define LUA_MEM_EXTRA_MAX               (2 MB)    // max allowed memory usage for Lua bitmaps (in bytes)
#define LUA_MEM_MAX                     (6 MB)    // max allowed memory usage for complete Lua  (in bytes), 0 means unlimited

extern uint16_t sessionTimer;

#define SLAVE_MODE()                    (g_model.trainerData.mode == TRAINER_MODE_SLAVE)

// Board driver
void boardInit();
void boardOff();

// CPU Unique ID
#define LEN_CPU_UID                     (3*8+2)
void getCPUUniqueID(char * s);

// Flash Write driver
#define FLASH_PAGESIZE 256
void unlockFlash();
void lockFlash();
void flashWrite(uint32_t * address, const uint32_t * buffer);
uint32_t isFirmwareStart(const uint8_t * buffer);
uint32_t isBootloaderStart(const uint8_t * buffer);

// SDRAM driver
extern "C" void SDRAM_Init();

// Pulses driver
#if !defined(SIMU)

void INTERNAL_MODULE_ON();            /*gpio_set(INTMODULE_PWR_GPIO)*/
void INTERNAL_MODULE_OFF();           /*gpio_clear(INTMODULE_PWR_GPIO);*/
void INTERNAL_MODULE_BOOTCMD(uint8_t enable);
void EXTERNAL_MODULE_ON();            /*gpio_set(EXTMODULE_PWR_GPIO)*/
void EXTERNAL_MODULE_OFF();           /*gpio_clear(EXTMODULE_PWR_GPIO)*/
#define EXTERNAL_MODULE_PWR_OFF         EXTERNAL_MODULE_OFF
#define BLUETOOTH_MODULE_ON()           gpio_clear(BLUETOOTH_ON_GPIO)
#define BLUETOOTH_MODULE_OFF()          gpio_set(BLUETOOTH_ON_GPIO)
#define IS_INTERNAL_MODULE_ON()         (false)
#define IS_EXTERNAL_MODULE_ON()         (GPIO_ReadInputDataBit(EXTMODULE_PWR_GPIO, EXTMODULE_PWR_GPIO_PIN) == Bit_SET)

#else

#define INTERNAL_MODULE_OFF()
#define INTERNAL_MODULE_ON()
#define EXTERNAL_MODULE_ON()
#define EXTERNAL_MODULE_OFF()
#define BLUETOOTH_MODULE_ON()
#define BLUETOOTH_MODULE_OFF()
#define IS_INTERNAL_MODULE_ON()         (false)
#define IS_EXTERNAL_MODULE_ON()         (false)

#endif // defined(SIMU)

#if defined(FUNCTION_SWITCHES)
#define NUM_FUNCTIONS_SWITCHES 4
#define NUM_FUNCTIONS_GROUPS   2
#define DEFAULT_FS_CONFIG                                         \
  (SWITCH_2POS << 6) + \
      (SWITCH_2POS << 4) + (SWITCH_2POS << 2) + (SWITCH_2POS << 0)

#define DEFAULT_FS_GROUPS                                 \
  (1 << 6) + (1 << 4) + (1 << 2) + \
      (1 << 0)  // Set all FS to group 1 to act like a 6pos

#define DEFAULT_FS_STARTUP_CONFIG                         \
   ((FS_START_PREVIOUS << 6) + (FS_START_PREVIOUS << 4) +  \
   (FS_START_PREVIOUS << 2) +                             \
   (FS_START_PREVIOUS << 0))  // keep last state by default

#else
#define NUM_FUNCTIONS_SWITCHES 0
#endif

#define NUM_TRIMS                       4
#define DEFAULT_STICK_DEADZONE          2

#define BATTERY_WARN                  74 // 7.4V
#define BATTERY_MIN                   70 // 6.8V
#define BATTERY_MAX                   86 // 8.6V
#define BATTERY_DIVIDER               2323

#if defined(__cplusplus) && !defined(SIMU)
extern "C" {
#endif

// Power driver
#define SOFT_PWR_CTRL
#define POWER_ON_STEP                     200 // 200ms
#define POWER_ON_DELAY    (POWER_ON_STEP * 5)
#define INTER_PRESS_TIMEOUT              5000 // 5s
#define FIRST_PRESS_TIMEOUT             10000 // 10s
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

#define IS_UCHARGER_ACTIVE()              gpio_read(UCHARGER_GPIO) ? 1 : 0
#define IS_UCHARGER_CHARGE_END_ACTIVE()   gpio_read(UCHARGER_CHARGE_END_GPIO) ? 0 : 1
#define ENABLE_UCHARGER()                 bsp_output_set(BSP_CHARGE_EN);
#define DISABLE_UCHARGER()                bsp_output_clear(BSP_CHARGE_EN)  

// Audio driver
void audioInit();
void audioConsumeCurrentBuffer();
void setSampleRate(uint32_t frequency);
#define audioWaitReady()
#define audioDisableIrq()             // interrupts must stay enabled on Horus
#define audioEnableIrq()              // interrupts must stay enabled on Horus
#define setSampleRate(freq)
void setScaledVolume(uint8_t volume);
void setVolume(uint8_t volume);
int32_t getVolume();
#define VOLUME_LEVEL_MAX               23
#define VOLUME_LEVEL_DEF               12

// Telemetry driver
#define INTMODULE_FIFO_SIZE            512
#define TELEMETRY_FIFO_SIZE            512

// Haptic driver
void hapticInit();
void hapticDone();
void hapticOff();
void hapticOn(uint32_t pwmPercent);

// Second serial port driver
//#define AUX_SERIAL
#define DEBUG_BAUDRATE                  115200
#define LUA_DEFAULT_BAUDRATE            115200

extern uint8_t currentTrainerMode;
void checkTrainerSettings();

// Touch panel driver
bool touchPanelEventOccured();
struct TouchState touchPanelRead();
struct TouchState getInternalTouchState();

enum rgb_state_e {
  RGB_STATE_NONE,
  RGB_STATE_BREATH,
  RGB_STATE_CHARGE,
  RGB_STATE_ON,
  RGB_STATE_OFF,
  RGB_STATE_POWER_ON,
  RGB_STATE_POWER_OFF,
  RGB_STATE_BAT_DIS,
};

enum rgb_color_e {
  RGB_COLOR_NONE   = 0,
  RGB_COLOR_RED    = (1 << 0),  // 00000001
  RGB_COLOR_GREEN  = (1 << 1),  // 00000010
  RGB_COLOR_BLUE   = (1 << 2),  // 00000100
  RGB_COLOR_YELLOW = RGB_COLOR_RED | RGB_COLOR_GREEN,
  RGB_COLOR_PURPLE = RGB_COLOR_RED | RGB_COLOR_BLUE,
  RGB_COLOR_WHITE  = RGB_COLOR_RED | RGB_COLOR_GREEN | RGB_COLOR_BLUE,
};

enum rgb_group_e {
  RGB_GROUP_MASK_NONE = 0,
  RGB_GROUP_MASK_FUNC_1 = (1 << 0),
  RGB_GROUP_MASK_FUNC_2 = (1 << 1),
  RGB_GROUP_MASK_FUNC_3 = (1 << 2),
  RGB_GROUP_MASK_FUNC_4 = (1 << 3),
  RGB_GROUP_MASK_POWER = (1 << 4),
  RGB_GROUP_MASK_AROUND_L = (1 << 5),
  RGB_GROUP_MASK_AROUND_R = (1 << 6),
  RGB_GROUP_MASK_ALL = 0x7f, // 0111 1111
};

enum rgb_power_step_e {
  RGB_STEP_POWER_AROUND,
  RGB_STEP_FUNC1,
  RGB_STEP_FUNC2,
  RGB_STEP_FUNC3,
  RGB_STEP_FUNC4,
};

void rgbChargeInit(void);
void ledLoop(void);
void ledSetColor(uint8_t color);
void ledSetState(uint8_t state);
void ledSetGroup(uint8_t group);
void setLedGroupColor(uint8_t index, uint8_t color, uint8_t brightness);

#endif // _BOARD_H_
