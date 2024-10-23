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

#include <inttypes.h>
#include "hal.h"
#include "hal/serial_port.h"
#include "hal/watchdog_driver.h"

#include "definitions.h"
#include "edgetx_constants.h"
#include "board_common.h"

#if defined(ROTARY_ENCODER_NAVIGATION)
// Rotary Encoder driver
void rotaryEncoderInit();
void rotaryEncoderCheck();
#endif

#if defined(STM32F413xx) || defined(STM32F407xG)
#define FLASHSIZE                       0x100000 // 1M
#else
#define FLASHSIZE                       0x80000  // 512k
#endif
#define FLASH_PAGESIZE                  256
#define BOOTLOADER_SIZE                 0x8000
#define FIRMWARE_ADDRESS                0x08000000
#define FIRMWARE_LEN(fsize)             (fsize - BOOTLOADER_SIZE)
#define FIRMWARE_MAX_LEN                (FLASHSIZE - BOOTLOADER_SIZE)
#define APP_START_ADDRESS               (uint32_t)(FIRMWARE_ADDRESS + BOOTLOADER_SIZE)

#define LUA_MEM_MAX                     (0)    // max allowed memory usage for complete Lua  (in bytes), 0 means unlimited

#if defined(PCBXLITE)
# define BOOTLOADER_KEYS                0x0F
#elif defined(RADIO_MT12)
# define BOOTLOADER_KEYS                0x06
#else
# define BOOTLOADER_KEYS                0x42
#endif

#if defined(RADIO_FAMILY_T20)
# define SECONDARY_BOOTLOADER_KEYS      0x1200
#endif

extern uint16_t sessionTimer;

// Board driver
void boardInit();
void boardOff();

// PCBREV driver
enum {
  // X7
  PCBREV_X7_STD = 0,
  PCBREV_X7_40 = 1,
};

// Pulses driver
#define INTERNAL_MODULE_ON()   gpio_set(INTMODULE_PWR_GPIO)
#define INTERNAL_MODULE_OFF()  gpio_clear(INTMODULE_PWR_GPIO)

#if (defined(INTERNAL_MODULE_PXX1) || defined(INTERNAL_MODULE_PXX2)) && (!defined(PCBX9LITE) || defined(PCBX9LITES))
  #define HARDWARE_INTERNAL_RAS
#endif

#define EXTERNAL_MODULE_ON()            EXTERNAL_MODULE_PWR_ON()
#define EXTERNAL_MODULE_OFF()           EXTERNAL_MODULE_PWR_OFF()

// Trainer driver
#define SLAVE_MODE()                    (g_model.trainerData.mode == TRAINER_MODE_SLAVE)

#if defined(TRAINER_DETECT_GPIO)
  // Trainer detect is a switch on the jack
  #if defined(TRAINER_DETECT_INVERTED)
    #define TRAINER_CONNECTED()           (gpio_read(TRAINER_DETECT_GPIO) ? 0 : 1)
  #else
    #define TRAINER_CONNECTED()           (gpio_read(TRAINER_DETECT_GPIO) ? 1 : 0)
  #endif
#elif defined(PCBXLITES)
  // Trainer is on the same connector than Headphones
  enum JackState
  {
    SPEAKER_ACTIVE,
    HEADPHONE_ACTIVE,
    TRAINER_ACTIVE,
  };
  extern uint8_t jackState;
  #define TRAINER_CONNECTED()           (jackState == TRAINER_ACTIVE)
#elif defined(PCBXLITE)
  // No Tainer jack on Taranis X-Lite
  #define TRAINER_CONNECTED()           false
#else
  // Trainer detect catches PPM, detection would use more CPU
  #define TRAINER_CONNECTED()           true
#endif

// POTS and SLIDERS default configuration
#if defined(RADIO_BOXER)
#define XPOS_CALIB_DEFAULT  {0x5, 0xd, 0x16, 0x1f, 0x28}
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

#if defined(ADC_GPIO_PIN_STICK_TH)
  #define SURFACE_RADIO  true
#endif

PACK(typedef struct {
  uint8_t pcbrev:2;
}) HardwareOptions;

extern HardwareOptions hardwareOptions;

// Battery driver
#if defined(PCBX9E)
  // NI-MH 9.6V
  #define BATTERY_WARN                  87 // 8.7V
  #define BATTERY_MIN                   85 // 8.5V
  #define BATTERY_MAX                   115 // 11.5V
#elif defined(PCBXLITE) || defined(RADIO_FAMILY_T20)
  // 2 x Li-Ion
  #define BATTERY_WARN                  66 // 6.6V
  #define BATTERY_MIN                   67 // 6.7V
  #define BATTERY_MAX                   83 // 8.3V
#elif defined(RADIO_T8) || defined(RADIO_TLITE) || defined(RADIO_LR3PRO)
  // 1S Li-ion /  Lipo, LDO for 3.3V
  #define BATTERY_WARN                  35 // 3.5V
  #define BATTERY_MIN                   34 // 3.4V
  #define BATTERY_MAX                   42 // 4.2V
#elif defined(RADIO_COMMANDO8)
  #define BATTERY_WARN                  32 // 3.5V
  #define BATTERY_MIN                   30 // 3.0V
  #define BATTERY_MAX                   42 // 4.2V
#else
  // NI-MH 7.2V
  #define BATTERY_WARN                  65 // 6.5V
  #define BATTERY_MIN                   60 // 6.0V
  #define BATTERY_MAX                   80 // 8.0V
#endif

#if defined(PCBXLITE)
  #define BATT_SCALE                    131
#elif defined(PCBX7)
  #define BATT_SCALE                    123
#elif defined(PCBX9LITE)
  #define BATT_SCALE                    117
#elif defined(RADIO_X9DP2019)
  #define BATT_SCALE                    117
#else
  #define BATT_SCALE                    150
#endif

#if defined(__cplusplus) && !defined(SIMU)
extern "C" {
#endif

// Power driver
#define SOFT_PWR_CTRL
#if defined(PWR_BUTTON_PRESS) && !defined(RADIO_COMMANDO8)
#  define STARTUP_ANIMATION
#endif

void pwrInit();
uint32_t pwrCheck();
void pwrOn();
void pwrOff();
bool pwrPressed();
bool pwrOffPressed();
void pwrResetHandler();
#define pwrForcePressed()   false

// Backlight driver
#if defined(OLED_SCREEN)
#define BACKLIGHT_DISABLE()             lcdSetRefVolt(0)
#define BACKLIGHT_FORCED_ON             255
#else
#define BACKLIGHT_DISABLE()             backlightDisable()
#define BACKLIGHT_FORCED_ON             101
#endif


void backlightInit();
void backlightDisable();
void backlightFullOn();
uint8_t isBacklightEnabled();

#if defined(PCBX9E) || defined(PCBX9DP)
  void backlightEnable(uint8_t level, uint8_t color);
  #define BACKLIGHT_ENABLE() \
    backlightEnable(currentBacklightBright, g_eeGeneral.backlightColor)
#elif defined(OLED_SCREEN)
  #define BACKLIGHT_ENABLE() lcdSetRefVolt(currentBacklightBright)
#else
  void backlightEnable(uint8_t level);
  #define BACKLIGHT_ENABLE() backlightEnable(currentBacklightBright)
#endif

#if defined(__cplusplus) && !defined(SIMU)
}
#endif

// Debug driver
void debugPutc(const char c);

// Audio driver
void audioInit() ;
void audioEnd() ;
void dacStart();
void dacStop();
void setSampleRate(uint32_t frequency);
#define VOLUME_LEVEL_MAX  23
#define VOLUME_LEVEL_DEF  12
#if !defined(SOFTWARE_VOLUME)
void setScaledVolume(uint8_t volume);
void setVolume(uint8_t volume);
int32_t getVolume();
#endif
#if defined(AUDIO_SPEAKER_ENABLE_GPIO)
void initSpeakerEnable();
void enableSpeaker();
void disableSpeaker();
#else
static inline void initSpeakerEnable() { }
static inline void enableSpeaker() { }
static inline void disableSpeaker() { }
#endif
#if defined(HEADPHONE_TRAINER_SWITCH_GPIO)
void initHeadphoneTrainerSwitch();
void enableHeadphone();
void enableTrainer();
#else
static inline void initHeadphoneTrainerSwitch() { }
static inline void enableHeadphone() { }
static inline void enableTrainer() { }
#endif
#if defined(JACK_DETECT_GPIO)
void initJackDetect();
bool isJackPlugged();
#endif
void audioConsumeCurrentBuffer();
#define audioDisableIrq()               __disable_irq()
#define audioEnableIrq()                __enable_irq()

// Haptic driver
void hapticInit();
void hapticOff();
#if defined(HAPTIC_PWM)
  void hapticOn(uint32_t pwmPercent);
#else
  void hapticOn();
#endif

#define DEBUG_BAUDRATE                  115200
#define LUA_DEFAULT_BAUDRATE            115200

const etx_serial_port_t* auxSerialGetPort(int port_nr);

// USB Charger
#if defined(USB_CHARGER)
void usbChargerInit();
bool usbChargerLed();
#endif

#if defined(RADIO_V14) || defined(RADIO_V12)
  uint16_t getSixPosAnalogValue(uint16_t adcValue);
#endif

// LED driver
void ledInit();
void ledOff();
void ledRed();
void ledGreen();
void ledBlue();

// LCD driver
#if defined(PCBX9D) || defined(PCBX9DP) || defined(PCBX9E)
#define LCD_W                           212
#define LCD_H                           64
#define LCD_DEPTH                       4
#define LCD_CONTRAST_MIN                0
#define LCD_CONTRAST_MAX                45
#define LCD_CONTRAST_DEFAULT            25
#else
#define LCD_W                           128
#define LCD_H                           64
#define LCD_DEPTH                       1
#define IS_LCD_RESET_NEEDED()           true
#if defined(OLED_SCREEN)
#define LCD_CONTRAST_MIN                2
#define LCD_CONTRAST_MAX                254
#else
#define LCD_CONTRAST_MIN                10
#define LCD_CONTRAST_MAX                30
#endif

#if defined(RADIO_MT12)
#define LCD_BRIGHTNESS_DEFAULT          50
#elif defined(RADIO_T12MAX)
#define LCD_BRIGHTNESS_DEFAULT          30
#endif

#if defined(OLED_SCREEN)
  #define LCD_CONTRAST_DEFAULT          254 // full brightness
#elif defined(RADIO_TX12) || defined(RADIO_TX12MK2) || defined(RADIO_BOXER) || defined(RADIO_MT12)
  #define LCD_CONTRAST_DEFAULT          20
#elif defined(RADIO_TPRO) || defined(RADIO_FAMILY_JUMPER_T12) || defined(RADIO_TPRO) || defined(RADIO_COMMANDO8) || defined(RADIO_T12MAX)
  #define LCD_CONTRAST_DEFAULT          25
#else
  #define LCD_CONTRAST_DEFAULT          15
#endif
#if defined(OLED_SCREEN)
  // add offset 2px because driver (SH1106) of the 1.3 OLED is for a 132 display
  #define LCD_W_OFFSET                  0x02
#endif
#endif

#if defined(PCBX9D) || defined(PCBX9E) || (defined(PCBX9DP) && PCBREV < 2019)
#define IS_LCD_RESET_NEEDED()           (!WAS_RESET_BY_WATCHDOG_OR_SOFTWARE())
#else
#define IS_LCD_RESET_NEEDED()           true
#endif

void lcdInit();
void lcdInitFinish();
void lcdOff();

// TODO lcdRefreshWait() stub in simpgmspace
#if defined(LCD_DMA) && !defined(SIMU)
void lcdRefreshWait();
#else
#define lcdRefreshWait()
#endif
#if defined(PCBX9D) || defined(SIMU) || !defined(__cplusplus)
void lcdRefresh();
#else
void lcdRefresh(bool wait=true); // TODO uint8_t wait to simplify this
#endif
void lcdSetRefVolt(unsigned char val);
#if LCD_W == 128
void lcdSetInvert(bool invert);
#endif
#ifdef __cplusplus
void lcdSetContrast(bool useDefault = false);
#endif
void lcdFlushed();

// Top LCD driver
#if defined(TOPLCD_GPIO)
void toplcdInit();
void toplcdOff();
void toplcdRefreshStart();
void toplcdRefreshEnd();
void setTopFirstTimer(int32_t value);
void setTopSecondTimer(uint32_t value);
void setTopRssi(uint32_t rssi);
void setTopBatteryState(int state, uint8_t blinking);
void setTopBatteryValue(uint32_t volts);
#endif

#if defined(CROSSFIRE)
#define TELEMETRY_FIFO_SIZE             128
#else
#define TELEMETRY_FIFO_SIZE             64
#endif

#define INTMODULE_FIFO_SIZE            128

#if defined(MANUFACTURER_RADIOMASTER) || defined(MANUFACTURER_JUMPER)
// --- MOSFET ---- R2 --- MCU
//                     |__ R1 --- GND
//
#define VBAT_DIV_R1       160 // kOhms
#define VBAT_DIV_R2       499 // kOhms
#if defined(MANUFACTURER_JUMPER)
#define VBAT_MOSFET_DROP   50 // * 10mV
#else
#define VBAT_MOSFET_DROP   25 // * 10mV
#endif
#else //--- MOSFET ---- R2 --- MCU
#if defined (RADIO_T8) || defined(RADIO_COMMANDO8)
  #define BATTERY_DIVIDER 50000
#elif defined (RADIO_LR3PRO)
  #define BATTERY_DIVIDER 39500
#else
  #define BATTERY_DIVIDER 26214
#endif 
#define VOLTAGE_DROP 20
#endif //--- MOSFET ---- R2 --- MCU

#if defined(RADIO_FAMILY_T20)
#define NUM_TRIMS                               8
#else
#define NUM_TRIMS                               4
#endif

#define NUM_TRIMS_KEYS                          (NUM_TRIMS * 2)

