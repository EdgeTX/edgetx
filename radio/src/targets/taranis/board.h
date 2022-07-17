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

#include <inttypes.h>
#include "hal.h"
#include "hal/serial_port.h"
#include "watchdog_driver.h"

#include "definitions.h"
#include "opentx_constants.h"
#include "board_common.h"

#if defined(RADIO_TX12) || defined(RADIO_TX12MK2) || defined(RADIO_ZORRO)
  #define  NAVIGATION_X7_TX12
#endif

#if defined(ROTARY_ENCODER_NAVIGATION)
// Rotary Encoder driver
void rotaryEncoderInit();
void rotaryEncoderCheck();
#endif

#define FLASHSIZE                       0x80000
#define BOOTLOADER_SIZE                 0x8000
#define FIRMWARE_ADDRESS                0x08000000

#define LUA_MEM_MAX                     (0)    // max allowed memory usage for complete Lua  (in bytes), 0 means unlimited

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

// SD driver
#define BLOCK_SIZE                      512 /* Block Size in Bytes */
#if !defined(SIMU) || defined(SIMU_DISKIO)
uint32_t sdIsHC();
uint32_t sdGetSpeed();
#define SD_IS_HC()                      (sdIsHC())
#define SD_GET_SPEED()                  (sdGetSpeed())
#define SD_GET_FREE_BLOCKNR()           (sdGetFreeSectors())
#else
#define SD_IS_HC()                      (0)
#define SD_GET_SPEED()                  (0)
#endif
#define __disk_read                     disk_read
#define __disk_write                    disk_write
#if defined(SIMU)
  #if !defined(SIMU_DISKIO)
    #define sdInit()
    #define sdDone()
  #endif
  #define sdMount()
  #define SD_CARD_PRESENT()               true
#else
void sdInit();
void sdMount();
void sdDone();
void sdPoll10ms();
uint32_t sdMounted();
#define SD_CARD_PRESENT()               ((SD_GPIO_PRESENT_GPIO->IDR & SD_GPIO_PRESENT_GPIO_PIN) == 0)
#endif

// Flash Write driver
#define FLASH_PAGESIZE 256
void unlockFlash();
void lockFlash();
void flashWrite(uint32_t * address, const uint32_t * buffer);
uint32_t isFirmwareStart(const uint8_t * buffer);
uint32_t isBootloaderStart(const uint8_t * buffer);

// Pulses driver
#define INTERNAL_MODULE_ON()   GPIO_SetBits(INTMODULE_PWR_GPIO, INTMODULE_PWR_GPIO_PIN)
#define INTERNAL_MODULE_OFF()  GPIO_ResetBits(INTMODULE_PWR_GPIO, INTMODULE_PWR_GPIO_PIN)

#if (defined(INTERNAL_MODULE_PXX1) || defined(INTERNAL_MODULE_PXX2)) && (!defined(PCBX9LITE) || defined(PCBX9LITES))
  #define HARDWARE_INTERNAL_RAS
#endif

#define EXTERNAL_MODULE_ON()            EXTERNAL_MODULE_PWR_ON()
#define EXTERNAL_MODULE_OFF()           EXTERNAL_MODULE_PWR_OFF()

#if defined(RADIO_T12)
#define IS_INTERNAL_MODULE_ON()         false
#else
#define IS_INTERNAL_MODULE_ON()         (GPIO_ReadInputDataBit(INTMODULE_PWR_GPIO, INTMODULE_PWR_GPIO_PIN) == Bit_SET)
#endif

// Trainer driver
#define SLAVE_MODE()                    (g_model.trainerData.mode == TRAINER_MODE_SLAVE)

#if defined(TRAINER_DETECT_GPIO)
  // Trainer detect is a switch on the jack
  #define TRAINER_CONNECTED()           (GPIO_ReadInputDataBit(TRAINER_DETECT_GPIO, TRAINER_DETECT_GPIO_PIN) == TRAINER_DETECT_GPIO_PIN_VALUE)
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

#if defined(TRAINER_GPIO)
  void init_trainer_ppm();
  void stop_trainer_ppm();
  void init_trainer_capture();
  void stop_trainer_capture();
#else
  #define init_trainer_ppm()
  #define stop_trainer_ppm()
  #define init_trainer_capture()
  #define stop_trainer_capture()
#endif

#if defined(TRAINER_MODULE_CPPM)
  void init_trainer_module_cppm();
  void stop_trainer_module_cppm();
#endif

#if defined(TRAINER_MODULE_SBUS)
  void init_trainer_module_sbus();
  void stop_trainer_module_sbus();
  int trainerModuleSbusGetByte(uint8_t* byte);
#endif

void check_telemetry_exti();

// Keys driver
enum EnumKeys
{
#if defined(KEYS_GPIO_REG_SHIFT)
  KEY_SHIFT,
#endif

#if defined(KEYS_GPIO_REG_MENU)
  KEY_MENU,
#endif

  KEY_EXIT,
  KEY_ENTER,

#if defined(KEYS_GPIO_REG_DOWN)
  KEY_DOWN,
  KEY_UP,
#endif

#if defined(KEYS_GPIO_REG_RIGHT)
  KEY_RIGHT,
  KEY_LEFT,
#endif

#if defined(KEYS_GPIO_REG_PAGE)
  KEY_PAGE,
#endif

#if defined(KEYS_GPIO_REG_PAGEUP)
  KEY_PAGEUP,
#endif

#if defined(KEYS_GPIO_REG_PAGEDN)
  KEY_PAGEDN,
#endif

#if defined(KEYS_GPIO_REG_SYS)
  KEY_SYS,
#endif

#if defined(KEYS_GPIO_REG_MDL)
  KEY_MODEL,
#endif

#if defined(KEYS_GPIO_REG_TELE)
  KEY_TELE,
#endif

#if defined(KEYS_GPIO_REG_PLUS)
  KEY_PLUS,
  KEY_MINUS,
#endif

  KEY_COUNT,
  KEY_MAX = KEY_COUNT - 1,

#if defined(KEYS_GPIO_REG_BIND)
  KEY_BIND,
#endif

#if defined(ROTARY_ENCODER_NAVIGATION)
  KEY_PLUS,
  KEY_MINUS,
#endif

  TRM_BASE,
  TRM_LH_DWN = TRM_BASE,
  TRM_LH_UP,
  TRM_LV_DWN,
  TRM_LV_UP,
  TRM_RV_DWN,
  TRM_RV_UP,
  TRM_RH_DWN,
  TRM_RH_UP,
  TRM_LAST = TRM_RH_UP,

  NUM_KEYS
};

#if defined(PCBX9E) && !defined(SIMU)
  #define KEY_UP                        KEY_MINUS
  #define KEY_DOWN                      KEY_PLUS
  #define KEY_RIGHT                     KEY_PLUS
  #define KEY_LEFT                      KEY_MINUS
#elif defined(NAVIGATION_XLITE)
  #define KEY_PLUS                      KEY_RIGHT
  #define KEY_MINUS                     KEY_LEFT
#elif defined(NAVIGATION_9X)
  #define KEY_MENU                      KEY_ENTER
  #define KEY_MINUS                     KEY_DOWN
  #define KEY_PLUS                      KEY_UP
#else
  #define KEY_UP                        KEY_PLUS
  #define KEY_DOWN                      KEY_MINUS
  #define KEY_RIGHT                     KEY_MINUS
  #define KEY_LEFT                      KEY_PLUS
#endif

#if defined(KEYS_GPIO_PIN_SHIFT)
#define IS_SHIFT_KEY(index)             (index == KEY_SHIFT)
#if defined(SIMU)
#define IS_SHIFT_PRESSED()              (readKeys() & (1 << KEY_SHIFT))
#else
#define IS_SHIFT_PRESSED()              (~KEYS_GPIO_REG_SHIFT & KEYS_GPIO_PIN_SHIFT)
#endif
#else
#define IS_SHIFT_KEY(index)             (false)
#define IS_SHIFT_PRESSED()              (false)
#endif

enum EnumSwitches
{
  SW_SA,
  SW_SB,
  SW_SC,
  SW_SD,
  SW_SE,
  SW_SF,
  SW_SG,
  SW_SH,
  SW_SI,
  SW_SJ,
  SW_SK,
  SW_SL,
  SW_SM,
  SW_SO,
  SW_SP,
  SW_SQ,
  SW_SR,
};
#if defined(RADIO_TX12) || defined(RADIO_TX12MK2)
  #define IS_3POS(x)                      ((x) != SW_SA && (x) != SW_SD)
#elif defined(RADIO_ZORRO)
  #define IS_3POS(x)                      ((x) == SW_SB || (x) == SW_SC)
#else
  #define IS_3POS(x)                      ((x) != SW_SF && (x) != SW_SH)
#endif
enum EnumSwitchesPositions
{
  SW_SA0,
  SW_SA1,
  SW_SA2,
  SW_SB0,
  SW_SB1,
  SW_SB2,
  SW_SC0,
  SW_SC1,
  SW_SC2,
  SW_SD0,
  SW_SD1,
  SW_SD2,
#if defined(HARDWARE_SWITCH_E)
  SW_SE0,
  SW_SE1,
  SW_SE2,
#endif
#if defined(HARDWARE_SWITCH_F)
  SW_SF0,
  SW_SF1,
  SW_SF2,
#endif
#if defined(HARDWARE_SWITCH_G)
  SW_SG0,
  SW_SG1,
  SW_SG2,
#endif
#if defined(HARDWARE_SWITCH_H)
  SW_SH0,
  SW_SH1,
  SW_SH2,
#endif
#if defined(RADIO_X9DP2019)
  SW_SI0,
  SW_SI1,
  SW_SI2,
#endif
#if defined(PCBX7ACCESS)
  SW_SI0,
  SW_SI1,
  SW_SI2,
#elif defined(PCBX7) && !defined(RADIO_ZORRO)
  SW_SI0,
  SW_SI1,
  SW_SI2,
  SW_SJ0,
  SW_SJ1,
  SW_SJ2,
#endif
#if defined(PCBX9E)
  SW_SI0,
  SW_SI1,
  SW_SI2,
  SW_SJ0,
  SW_SJ1,
  SW_SJ2,
  SW_SK0,
  SW_SK1,
  SW_SK2,
  SW_SL0,
  SW_SL1,
  SW_SL2,
  SW_SM0,
  SW_SM1,
  SW_SM2,
  SW_SN0,
  SW_SN1,
  SW_SN2,
  SW_SO0,
  SW_SO1,
  SW_SO2,
  SW_SP0,
  SW_SP1,
  SW_SP2,
  SW_SQ0,
  SW_SQ1,
  SW_SQ2,
  SW_SR0,
  SW_SR1,
  SW_SR2,
#endif
  NUM_SWITCHES_POSITIONS
};

#if defined(PCBXLITES)
  #define NUM_SWITCHES                  6
  #define STORAGE_NUM_SWITCHES          NUM_SWITCHES
  #define DEFAULT_SWITCH_CONFIG         (SWITCH_TOGGLE << 10) + (SWITCH_TOGGLE << 8) + (SWITCH_2POS << 6) + (SWITCH_2POS << 4) + (SWITCH_3POS << 2) + (SWITCH_3POS << 0)
  #define DEFAULT_POTS_CONFIG           (POT_WITHOUT_DETENT << 2) + (POT_WITHOUT_DETENT << 0)
#elif defined(PCBXLITE)
  #define NUM_SWITCHES                  4
  #define STORAGE_NUM_SWITCHES          6
  #define DEFAULT_SWITCH_CONFIG         (SWITCH_2POS << 6) + (SWITCH_2POS << 4) + (SWITCH_3POS << 2) + (SWITCH_3POS << 0);
  #define DEFAULT_POTS_CONFIG           (POT_WITHOUT_DETENT << 2) + (POT_WITHOUT_DETENT << 0)
#elif defined(RADIO_TLITE)
  #define NUM_SWITCHES                  4
  #define STORAGE_NUM_SWITCHES          8
  #define DEFAULT_SWITCH_CONFIG         (SWITCH_2POS << 6) + (SWITCH_2POS << 4) + (SWITCH_3POS << 2) + (SWITCH_3POS << 0);
  #define DEFAULT_POTS_CONFIG           (0)
#elif defined(RADIO_TPRO)
  #define NUM_SWITCHES                  10
  #define NUM_FUNCTIONS_SWITCHES        6
  #define NUM_REGULAR_SWITCHES          (NUM_SWITCHES - NUM_FUNCTIONS_SWITCHES)
  #define STORAGE_NUM_SWITCHES          10
  #define DEFAULT_SWITCH_CONFIG         (SWITCH_TOGGLE << 6) + (SWITCH_TOGGLE << 4) + (SWITCH_3POS << 2) + (SWITCH_3POS << 0);
  #define DEFAULT_FS_CONFIG             (SWITCH_2POS << 10) + (SWITCH_2POS << 8) + (SWITCH_2POS << 6) + (SWITCH_2POS << 4) + (SWITCH_2POS << 2) + (SWITCH_2POS << 0)
  #define DEFAULT_FS_GROUPS             (1 << 10) + (1 << 8) + (1 << 6) + (1 << 4) + (1 << 2) + (1 << 0)  // Set all FS to group 1 to act like a 6pos
  #define DEFAULT_FS_STARTUP_CONFIG     ((FS_START_PREVIOUS << 10) + (FS_START_PREVIOUS << 8) + (FS_START_PREVIOUS << 6) + (FS_START_PREVIOUS << 4) + (FS_START_PREVIOUS << 2) + (FS_START_PREVIOUS << 0))  // keep last state by default
  #define DEFAULT_POTS_CONFIG           (POT_WITHOUT_DETENT << 0) + (POT_WITH_DETENT << 2); // S1 = pot without detent, S2 = pot with detent
#elif defined(RADIO_FAMILY_JUMPER_T12)
  #define NUM_SWITCHES                  8
  #define STORAGE_NUM_SWITCHES          NUM_SWITCHES
  #define DEFAULT_SWITCH_CONFIG         (SWITCH_2POS << 10) + (SWITCH_2POS << 8) + (SWITCH_3POS << 6) + (SWITCH_3POS << 4) + (SWITCH_3POS << 2) + (SWITCH_3POS << 0)
  #define DEFAULT_POTS_CONFIG           (POT_WITHOUT_DETENT << 0) + (POT_WITHOUT_DETENT << 2); // S1 = pot without detent, S2 = pot with detent
#elif defined(RADIO_TX12)
  #define NUM_SWITCHES                  8
  #define STORAGE_NUM_SWITCHES          NUM_SWITCHES
  #define DEFAULT_SWITCH_CONFIG         (SWITCH_3POS << 10) + (SWITCH_3POS << 8) + (SWITCH_TOGGLE << 6) + (SWITCH_3POS << 4) + (SWITCH_3POS << 2) + (SWITCH_TOGGLE << 0)
  #define DEFAULT_POTS_CONFIG           (POT_WITH_DETENT << 0) + (POT_WITH_DETENT << 2);
#elif defined(RADIO_TX12MK2)
  #define NUM_SWITCHES                  6
  #define STORAGE_NUM_SWITCHES          8
  #define DEFAULT_SWITCH_CONFIG         (SWITCH_3POS << 10) + (SWITCH_3POS << 8) + (SWITCH_TOGGLE << 6) + (SWITCH_3POS << 4) + (SWITCH_3POS << 2) + (SWITCH_TOGGLE << 0)
  #define DEFAULT_POTS_CONFIG           (POT_WITH_DETENT << 0) + (POT_WITH_DETENT << 2);
#elif defined(RADIO_ZORRO)
  #define NUM_SWITCHES                  8
  #define STORAGE_NUM_SWITCHES          NUM_SWITCHES
  #define DEFAULT_SWITCH_CONFIG         (SWITCH_TOGGLE << 14) + (SWITCH_TOGGLE << 12) + (SWITCH_2POS << 10) + (SWITCH_2POS << 8) + (SWITCH_TOGGLE << 6) + (SWITCH_3POS << 4) + (SWITCH_3POS << 2) + (SWITCH_TOGGLE << 0)
  #define DEFAULT_POTS_CONFIG           (POT_WITHOUT_DETENT << 0) + (POT_WITHOUT_DETENT << 2);
#elif defined(RADIO_T8)
  #define NUM_SWITCHES                  4
  #define STORAGE_NUM_SWITCHES          8
  #define DEFAULT_SWITCH_CONFIG         (SWITCH_2POS << 6) + (SWITCH_3POS << 4) + (SWITCH_3POS << 2) + (SWITCH_2POS << 0);
  #define DEFAULT_POTS_CONFIG           (0)
#elif defined(PCBX7ACCESS)
  #define NUM_SWITCHES                  7
  #define STORAGE_NUM_SWITCHES          8
  #define DEFAULT_SWITCH_CONFIG         (SWITCH_TOGGLE << 10) + (SWITCH_2POS << 8) + (SWITCH_3POS << 6) + (SWITCH_3POS << 4) + (SWITCH_3POS << 2) + (SWITCH_3POS << 0)
  #define DEFAULT_POTS_CONFIG           (POT_WITHOUT_DETENT << 0) + (POT_WITH_DETENT << 2); // S1 = pot without detent, S2 = pot with detent
#elif defined(PCBX7)
  #define NUM_SWITCHES                  8
  #define STORAGE_NUM_SWITCHES          NUM_SWITCHES
  #define DEFAULT_SWITCH_CONFIG         (SWITCH_TOGGLE << 10) + (SWITCH_2POS << 8) + (SWITCH_3POS << 6) + (SWITCH_3POS << 4) + (SWITCH_3POS << 2) + (SWITCH_3POS << 0)
  #define DEFAULT_POTS_CONFIG           (POT_WITHOUT_DETENT << 0) + (POT_WITH_DETENT << 2); // S1 = pot without detent, S2 = pot with detent
#elif defined(PCBX9LITES)
  #define NUM_SWITCHES                  7
  #define STORAGE_NUM_SWITCHES          NUM_SWITCHES
  #define DEFAULT_SWITCH_CONFIG         (SWITCH_TOGGLE << 12) + (SWITCH_TOGGLE << 10) + (SWITCH_TOGGLE << 8) + (SWITCH_2POS << 6) + (SWITCH_3POS << 4) + (SWITCH_3POS << 2) + (SWITCH_3POS << 0);
  #define DEFAULT_POTS_CONFIG           (POT_WITH_DETENT << 0); // S1 = pot with detent
#elif defined(PCBX9LITE)
  #define NUM_SWITCHES                  5
  #define STORAGE_NUM_SWITCHES          NUM_SWITCHES
  #define DEFAULT_SWITCH_CONFIG         (SWITCH_TOGGLE << 8) + (SWITCH_2POS << 6) + (SWITCH_3POS << 4) + (SWITCH_3POS << 2) + (SWITCH_3POS << 0);
  #define DEFAULT_POTS_CONFIG           (POT_WITH_DETENT << 0); // S1 = pot with detent
#elif defined(PCBX9E)
  #define NUM_SWITCHES                  18 // yes, it's perfect like that !
  #define STORAGE_NUM_SWITCHES          NUM_SWITCHES
  #define DEFAULT_SWITCH_CONFIG         (SWITCH_TOGGLE << 14) + (SWITCH_3POS << 12) + (SWITCH_2POS << 10) + (SWITCH_3POS << 8) + (SWITCH_3POS << 6) + (SWITCH_3POS << 4) + (SWITCH_3POS << 2) + (SWITCH_3POS << 0)
  #define DEFAULT_POTS_CONFIG           (POT_WITH_DETENT << 0) + (POT_WITH_DETENT << 2); // S1 = pot without detent, S2 = pot with detent
  #define DEFAULT_SLIDERS_CONFIG        (SLIDER_WITH_DETENT << 3) + (SLIDER_WITH_DETENT << 2) + (SLIDER_WITH_DETENT << 1) + (SLIDER_WITH_DETENT << 0)
#elif defined(RADIO_X9DP2019)
  #define NUM_SWITCHES                  9
  #define STORAGE_NUM_SWITCHES          NUM_SWITCHES
  #define DEFAULT_SWITCH_CONFIG         (SWITCH_TOGGLE << 16) + (SWITCH_TOGGLE << 14) + (SWITCH_3POS << 12) + (SWITCH_2POS << 10) + (SWITCH_3POS << 8) + (SWITCH_3POS << 6) + (SWITCH_3POS << 4) + (SWITCH_3POS << 2) + (SWITCH_3POS << 0);
  #define DEFAULT_POTS_CONFIG           (POT_WITH_DETENT << 0) + (POT_WITH_DETENT << 2); // S1 = pot without detent, S2 = pot with detent
  #define DEFAULT_SLIDERS_CONFIG        (SLIDER_WITH_DETENT << 1) + (SLIDER_WITH_DETENT << 0)
#elif defined(PCBX9D) || defined(PCBX9DP)
  #define NUM_SWITCHES                  8
  #define STORAGE_NUM_SWITCHES          9
  #define DEFAULT_SWITCH_CONFIG         (SWITCH_TOGGLE << 14) + (SWITCH_3POS << 12) + (SWITCH_2POS << 10) + (SWITCH_3POS << 8) + (SWITCH_3POS << 6) + (SWITCH_3POS << 4) + (SWITCH_3POS << 2) + (SWITCH_3POS << 0)
  #define DEFAULT_POTS_CONFIG           (POT_WITH_DETENT << 0) + (POT_WITH_DETENT << 2); // S1 = pot without detent, S2 = pot with detent
  #define DEFAULT_SLIDERS_CONFIG        (SLIDER_WITH_DETENT << 1) + (SLIDER_WITH_DETENT << 0)
#endif

#if !defined(NUM_FUNCTIONS_SWITCHES)
  #define NUM_FUNCTIONS_SWITCHES        0
#endif

#define STORAGE_NUM_SWITCHES_POSITIONS  (STORAGE_NUM_SWITCHES * 3)

void keysInit();
uint32_t switchState(uint8_t index);
uint32_t readKeys();
uint32_t readTrims();
#if defined(FUNCTION_SWITCHES)
extern uint8_t fsPreviousState;
void evalFunctionSwitches();
void setFSStartupPosition();
uint8_t getFSLogicalState(uint8_t index);
uint8_t getFSPhysicalState(uint8_t index);
#endif

#define TRIMS_PRESSED()                 (readTrims())
#define KEYS_PRESSED()                  (readKeys())

// ADC driver
enum Analogs {
  STICK1,
  STICK2,
  STICK3,
  STICK4,
  POT_FIRST,
  POT1 = POT_FIRST,
#if defined(PCBX9LITE)
  POT_LAST = POT1,
#elif defined(PCBXLITE) || defined(PCBX7)
  POT2,
  POT_LAST = POT2,
#elif defined(PCBX9E)
  POT2,
  POT3,
  POT4,
  POT_LAST = POT4,
  SLIDER1,
  SLIDER2,
  SLIDER3,
  SLIDER4,
#else
  POT2,
  POT3,
  POT_LAST = POT3,
  SLIDER1,
  SLIDER2,
#endif
  TX_VOLTAGE,
  TX_RTC_VOLTAGE,
  NUM_ANALOGS
};

#define HAS_TX_RTC_VOLTAGE

#if defined(PCBX9LITE)
  #define NUM_POTS                      1
  #define NUM_SLIDERS                   0
  #define STORAGE_NUM_POTS              1
  #define STORAGE_NUM_SLIDERS           0
#elif defined(RADIO_T8) || defined(RADIO_TLITE)
  #define NUM_POTS                      0
  #define NUM_SLIDERS                   0
  #define STORAGE_NUM_POTS              2
  #define STORAGE_NUM_SLIDERS           0
#elif defined(PCBXLITE) || defined(PCBX7)
  #define NUM_POTS                      2
  #define NUM_SLIDERS                   0
  #define STORAGE_NUM_POTS              2
  #define STORAGE_NUM_SLIDERS           0
#elif defined(PCBX9E)
  #define NUM_POTS                      4
  #define NUM_SLIDERS                   4
  #define STORAGE_NUM_POTS              4
  #define STORAGE_NUM_SLIDERS           4
#elif defined(PCBX9DP)
  #define NUM_POTS                      3
  #define NUM_SLIDERS                   2
  #define STORAGE_NUM_POTS              3
  #define STORAGE_NUM_SLIDERS           2
#else
  #define NUM_POTS                      3 // TODO X9D has only 2 pots
  #define NUM_SLIDERS                   2
  #define STORAGE_NUM_POTS              3
  #define STORAGE_NUM_SLIDERS           2
#endif

#define NUM_XPOTS                       STORAGE_NUM_POTS
#define NUM_TRIMS                       4
#define NUM_MOUSE_ANALOGS               0
#define STORAGE_NUM_MOUSE_ANALOGS       0

#if defined(PCBXLITE)
  #define NUM_TRIMS_KEYS                4
#else
  #define NUM_TRIMS_KEYS                (NUM_TRIMS * 2)
#endif

#if defined(STICKS_PWM)
  #define NUM_PWMSTICKS                 4
  #define STICKS_PWM_ENABLED()          (!hardwareOptions.sticksPwmDisabled)
  void sticksPwmInit();
  void sticksPwmRead(uint16_t * values);
  extern volatile uint32_t pwm_interrupt_count; // TODO => reusable buffer (boot section)
#else
  #define STICKS_PWM_ENABLED()          false
#endif

PACK(typedef struct {
  uint8_t pcbrev:2;
  uint8_t sticksPwmDisabled:1;
  uint8_t pxx2Enabled:1;
}) HardwareOptions;

extern HardwareOptions hardwareOptions;

#if !defined(PXX2)
  #define IS_PXX2_INTERNAL_ENABLED()            (false)
  #define IS_PXX1_INTERNAL_ENABLED()            (true)
#elif !defined(PXX1) || defined(PCBXLITES) || defined(PCBX9LITE)
  #define IS_PXX2_INTERNAL_ENABLED()            (true)
  #define IS_PXX1_INTERNAL_ENABLED()            (false)
#elif defined(INTERNAL_MODULE_PXX1)
  #define IS_PXX2_INTERNAL_ENABLED()            (false)
  #define IS_PXX1_INTERNAL_ENABLED()            (true)
#else
  // TODO #define PXX2_PROBE
  // TODO #define IS_PXX2_INTERNAL_ENABLED()            (hardwareOptions.pxx2Enabled)
  #define IS_PXX2_INTERNAL_ENABLED()            (true)
  #define IS_PXX1_INTERNAL_ENABLED()            (true)
#endif

enum CalibratedAnalogs {
  CALIBRATED_STICK1,
  CALIBRATED_STICK2,
  CALIBRATED_STICK3,
  CALIBRATED_STICK4,
  CALIBRATED_POT_FIRST,
  CALIBRATED_POT_LAST = CALIBRATED_POT_FIRST + NUM_POTS - 1,
  CALIBRATED_SLIDER_FIRST,
  CALIBRATED_SLIDER_LAST = CALIBRATED_SLIDER_FIRST + NUM_SLIDERS - 1,
  NUM_CALIBRATED_ANALOGS
};

#if defined(PCBX9D)
  #define IS_POT(x)                     ((x)>=POT_FIRST && (x)<=POT2) // POT3 is only defined in software
#else
  #define IS_POT(x)                     ((x)>=POT_FIRST && (x)<=POT_LAST)
#endif

#define IS_SLIDER(x)                    ((x)>POT_LAST && (x)<TX_VOLTAGE)

extern uint16_t adcValues[NUM_ANALOGS];

// Battery driver
#if defined(PCBX9E)
  // NI-MH 9.6V
  #define BATTERY_WARN                  87 // 8.7V
  #define BATTERY_MIN                   85 // 8.5V
  #define BATTERY_MAX                   115 // 11.5V
#elif defined(PCBXLITE)
  // 2 x Li-Ion
  #define BATTERY_WARN                  66 // 6.6V
  #define BATTERY_MIN                   67 // 6.7V
  #define BATTERY_MAX                   83 // 8.3V
#elif defined(RADIO_T8) || defined(RADIO_TLITE)
  // 1S Li-ion /  Lipo, LDO for 3.3V
  #define BATTERY_WARN                  35 // 3.5V
  #define BATTERY_MIN                   34 // 3.4V
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
void pwrInit();
uint32_t pwrCheck();
void pwrOn();
void pwrOff();
bool pwrPressed();
#if defined(PWR_BUTTON_PRESS)
#define STARTUP_ANIMATION
uint32_t pwrPressedDuration();
#endif
void pwrResetHandler();
#define pwrForcePressed()   false

#if defined(SIMU)
#define UNEXPECTED_SHUTDOWN()           false
#else
#define UNEXPECTED_SHUTDOWN()           (WAS_RESET_BY_WATCHDOG() || g_eeGeneral.unexpectedShutdown)
#endif

// Backlight driver
#define BACKLIGHT_DISABLE()             backlightDisable()
#define BACKLIGHT_FORCED_ON             101

void backlightInit();
void backlightDisable();
void backlightFullOn();
uint8_t isBacklightEnabled();

#if defined(PCBX9E) || defined(PCBX9DP)
  void backlightEnable(uint8_t level, uint8_t color);
  #define BACKLIGHT_ENABLE() \
    backlightEnable(currentBacklightBright, g_eeGeneral.backlightColor)
#else
  void backlightEnable(uint8_t level);
  #define BACKLIGHT_ENABLE() backlightEnable(currentBacklightBright)
#endif

#if !defined(SIMU)
  void usbJoystickUpdate();
#endif
#if defined(RADIO_TX12) || defined(RADIO_TX12MK2)
  #define USB_NAME                     "Radiomaster TX12"
  #define USB_MANUFACTURER             'R', 'M', '_', 'T', 'X', ' ', ' ', ' '  /* 8 bytes */
  #define USB_PRODUCT                  'R', 'M', ' ', 'T', 'X', '1', '2', ' '  /* 8 Bytes */
#elif defined(RADIO_ZORRO)
  #define USB_NAME                     "Radiomaster Zorro"
  #define USB_MANUFACTURER             'R', 'M', '_', 'T', 'X', ' ', ' ', ' '  /* 8 bytes */
  #define USB_PRODUCT                  'R', 'M', ' ', 'Z', 'O', 'R', 'R', 'O'  /* 8 Bytes */
#elif defined(RADIO_T8)
  #define USB_NAME                     "Radiomaster T8"
  #define USB_MANUFACTURER             'R', 'M', '_', 'T', 'X', ' ', ' ', ' '  /* 8 bytes */
  #define USB_PRODUCT                  'R', 'M', ' ', 'T', '8', ' ', ' ', ' '  /* 8 Bytes */
#elif defined(RADIO_TLITE)
  #define USB_NAME                     "Jumper TLite"
  #define USB_MANUFACTURER             'J', 'U', 'M', 'P', 'E', 'R', ' ', ' '  /* 8 bytes */
  #define USB_PRODUCT                  'T', '-', 'L', 'I', 'T', 'E', ' ', ' '  /* 8 Bytes */
#elif defined(RADIO_TPRO)
  #define USB_NAME                     "Jumper TPro"
  #define USB_MANUFACTURER             'J', 'U', 'M', 'P', 'E', 'R', ' ', ' '  /* 8 bytes */
  #define USB_PRODUCT                  'T', '-', 'P', 'R', 'O', ' ', ' ', ' '  /* 8 Bytes */
#else
  #define USB_NAME                     "FrSky Taranis"
  #define USB_MANUFACTURER             'F', 'r', 'S', 'k', 'y', ' ', ' ', ' '  /* 8 bytes */
  #define USB_PRODUCT                  'T', 'a', 'r', 'a', 'n', 'i', 's', ' '  /* 8 Bytes */
#endif

#if defined(__cplusplus) && !defined(SIMU)
}
#endif

// Debug driver
void debugPutc(const char c);

// Telemetry driver
void telemetryPortInit(uint32_t baudrate, uint8_t mode);
void telemetryPortSetDirectionInput();
void telemetryPortSetDirectionOutput();
void sportSendByte(uint8_t byte);
void sportSendByteLoop(uint8_t byte);
void sportStopSendByteLoop();
void sportSendBuffer(const uint8_t * buffer, uint32_t count);
bool sportGetByte(uint8_t * byte);
void telemetryClearFifo();
extern uint32_t telemetryErrors;

// soft-serial
void telemetryPortInvertedInit(uint32_t baudrate);

// PCBREV driver
#if defined(PCBX7ACCESS)
  #define HAS_SPORT_UPDATE_CONNECTOR()  true
#elif defined(PCBX7)
  #define IS_PCBREV_40()                (hardwareOptions.pcbrev == PCBREV_X7_40)
  #define HAS_SPORT_UPDATE_CONNECTOR()  IS_PCBREV_40()
#elif defined(SPORT_UPDATE_PWR_GPIO)
  #define HAS_SPORT_UPDATE_CONNECTOR()  true
#else
  #define HAS_SPORT_UPDATE_CONNECTOR()  false
#endif

// Sport update driver
#if defined(SPORT_UPDATE_PWR_GPIO)
void sportUpdateInit();
void sportUpdatePowerOn();
void sportUpdatePowerOff();
void sportUpdatePowerInit();
#define SPORT_UPDATE_POWER_ON()         sportUpdatePowerOn()
#define SPORT_UPDATE_POWER_OFF()        sportUpdatePowerOff()
#define SPORT_UPDATE_POWER_INIT()       sportUpdatePowerInit()
#define IS_SPORT_UPDATE_POWER_ON()      (GPIO_ReadInputDataBit(SPORT_UPDATE_PWR_GPIO, SPORT_UPDATE_PWR_GPIO_PIN) == Bit_SET)
#else
#define sportUpdateInit()
#define SPORT_UPDATE_POWER_ON()
#define SPORT_UPDATE_POWER_OFF()
#define SPORT_UPDATE_POWER_INIT()
#define IS_SPORT_UPDATE_POWER_ON()      (false)
#endif

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

// BT driver
#define BLUETOOTH_BOOTLOADER_BAUDRATE   230400
#define BLUETOOTH_DEFAULT_BAUDRATE      115200
#if defined(PCBX9E)
#define BLUETOOTH_FACTORY_BAUDRATE      9600
#else
#define BLUETOOTH_FACTORY_BAUDRATE      57600
#endif
#define BT_TX_FIFO_SIZE    64
#define BT_RX_FIFO_SIZE    256
void bluetoothInit(uint32_t baudrate, bool enable);
void bluetoothWriteWakeup();
uint8_t bluetoothIsWriting();
void bluetoothDisable();
#if defined(PCBX9LITES) || defined(PCBX7ACCESS)
  #define IS_BLUETOOTH_CHIP_PRESENT()     (true)
#elif defined(PCBX9LITE)
  #define IS_BLUETOOTH_CHIP_PRESENT()     (false)
#elif defined(BLUETOOTH_PROBE) && !defined(SIMU)
  extern volatile uint8_t btChipPresent;
  #define IS_BLUETOOTH_CHIP_PRESENT()     (btChipPresent)
#else
  #define IS_BLUETOOTH_CHIP_PRESENT()     (true)
#endif

// USB Charger
#if defined(USB_CHARGER)
void usbChargerInit();
bool usbChargerLed();
#endif

// LED driver
void ledInit();
void ledOff();
void ledRed();
void ledGreen();
void ledBlue();
#if defined(FUNCTION_SWITCHES)
void fsLedOff(uint8_t);
void fsLedOn(uint8_t);
#endif

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
#define LCD_CONTRAST_MIN                10
#define LCD_CONTRAST_MAX                30
#if defined(RADIO_TX12) || defined(RADIO_TX12MK2)
  #define LCD_CONTRAST_DEFAULT          20
#elif defined(RADIO_TPRO) || defined(RADIO_FAMILY_JUMPER_T12) || defined(RADIO_TPRO)
  #define LCD_CONTRAST_DEFAULT          25
#else
  #define LCD_CONTRAST_DEFAULT          15
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

// TODO lcdRefreshWait() stub in simpgmspace and remove LCD_DUAL_BUFFER
#if defined(LCD_DMA) && !defined(LCD_DUAL_BUFFER) && !defined(SIMU)
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

#define USART_FLAG_ERRORS (USART_FLAG_ORE | USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE)

#if defined(__cplusplus)
#include "fifo.h"
#include "dmafifo.h"

#if defined(CROSSFIRE)
#define TELEMETRY_FIFO_SIZE             128
#else
#define TELEMETRY_FIFO_SIZE             64
#endif

extern Fifo<uint8_t, TELEMETRY_FIFO_SIZE> telemetryFifo;
#endif

#define INTMODULE_FIFO_SIZE            128

#if defined (RADIO_TX12)
  #define BATTERY_DIVIDER 22830
#elif defined (RADIO_T8)
  #define BATTERY_DIVIDER 50000
#elif defined (RADIO_ZORRO) || defined(RADIO_TX12MK2)
  #define BATTERY_DIVIDER 23711 // = 2047*128*BATT_SCALE/(100*(VREF*(160+499)/160))
#else
  #define BATTERY_DIVIDER 26214
#endif 

#if defined(RADIO_ZORRO) || defined(RADIO_TX12MK2)
  #define VOLTAGE_DROP 45
#else
  #define VOLTAGE_DROP 20
#endif

#endif // _BOARD_H_
