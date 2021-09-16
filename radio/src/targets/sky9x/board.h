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

#ifndef _BOARD_SKY9X_H_
#define _BOARD_SKY9X_H_

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include "board_lowlevel.h"
#include "audio_driver.h"
#include "../opentx_constants.h"

#define CPU_FREQ                       36000000

extern uint16_t ResetReason;

#define BOOTLOADER_SIZE                0x8000
#define FIRMWARE_SIZE                  (256*1024)
#define FIRMWARE_ADDRESS               0x00400000

// Board driver
void boardInit();
void boardOff();

// Rotary Encoder driver
void rotaryEncoderInit();
void rotaryEncoderEnd();

#if !defined(REVX) && !defined(PCBAR9X)
  #define ROTARY_ENCODER_NAVIGATION
  #define REA_DOWN()                   (!(PIOB->PIO_PDSR & 0x40))
#else
  #define REA_DOWN()                   (0)
#endif

// Keys driver
#define NUM_SWITCHES                   7
#define STORAGE_NUM_SWITCHES           NUM_SWITCHES
#define NUM_SWITCHES_POSITIONS         9

#if !defined(NUM_FUNCTIONS_SWITCHES)
#define NUM_FUNCTIONS_SWITCHES          0
#endif

enum EnumKeys
{
  KEY_MENU,
  KEY_ENTER=KEY_MENU,
  KEY_EXIT,
  KEY_DOWN,
  KEY_MINUS = KEY_DOWN,
  KEY_UP,
  KEY_PLUS = KEY_UP,
  KEY_RIGHT,
  KEY_LEFT,

  KEY_COUNT,
  KEY_MAX = KEY_COUNT - 1,

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

#define IS_SHIFT_KEY(index)             (false)
#define IS_SHIFT_PRESSED()              (false)

enum EnumSwitches
{
  SW_ID0,
  SW_ID1,
  SW_ID2,
  SW_THR,
  SW_RUD,
  SW_ELE,
  SW_AIL,
  SW_GEA,
  SW_TRN,
};
#define IS_3POS(sw)                    ((sw) == 0)
#define IS_TOGGLE(sw)                  ((sw) == SWSRC_TRN)

#define KEYS_GPIO_REG_MENU           PIOB->PIO_PDSR
#define KEYS_GPIO_REG_EXIT           PIOC->PIO_PDSR
#define KEYS_GPIO_REG_UP             PIOC->PIO_PDSR
#define KEYS_GPIO_REG_DOWN           PIOC->PIO_PDSR
#define KEYS_GPIO_REG_RIGHT          PIOC->PIO_PDSR
#define KEYS_GPIO_REG_LEFT           PIOC->PIO_PDSR
#define KEYS_GPIO_PIN_MENU           0x00000020
#define KEYS_GPIO_PIN_EXIT           0x01000000
#define KEYS_GPIO_PIN_UP             0x00000002
#define KEYS_GPIO_PIN_DOWN           0x00000020
#define KEYS_GPIO_PIN_RIGHT          0x00000010
#define KEYS_GPIO_PIN_LEFT           0x00000008

#if defined(REVX)
  #define TRIMS_GPIO_REG_LHL           PIOB->PIO_PDSR
  #define TRIMS_GPIO_REG_LVD           PIOA->PIO_PDSR
  #define TRIMS_GPIO_REG_RVU           PIOC->PIO_PDSR
  #define TRIMS_GPIO_REG_RHL           PIOA->PIO_PDSR
  #define TRIMS_GPIO_REG_LHR           PIOA->PIO_PDSR
  #define TRIMS_GPIO_REG_LVU           PIOC->PIO_PDSR
  #define TRIMS_GPIO_REG_RVD           PIOA->PIO_PDSR
  #define TRIMS_GPIO_REG_RHR           PIOC->PIO_PDSR
#else
  #define TRIMS_GPIO_REG_LHL           PIOA->PIO_PDSR
  #define TRIMS_GPIO_REG_LVD           PIOA->PIO_PDSR
  #define TRIMS_GPIO_REG_RVU           PIOA->PIO_PDSR
  #define TRIMS_GPIO_REG_RHL           PIOA->PIO_PDSR
  #define TRIMS_GPIO_REG_LHR           PIOB->PIO_PDSR
  #define TRIMS_GPIO_REG_LVU           PIOC->PIO_PDSR
  #define TRIMS_GPIO_REG_RVD           PIOC->PIO_PDSR
  #define TRIMS_GPIO_REG_RHR           PIOC->PIO_PDSR
#endif

#if defined(REVX)
  #define TRIMS_GPIO_PIN_LHL           0x00000010
  #define TRIMS_GPIO_PIN_LVD           0x01000000
  #define TRIMS_GPIO_PIN_RVU           0x00000400
  #define TRIMS_GPIO_PIN_RHL           0x00000001
  #define TRIMS_GPIO_PIN_LHR           0x00800000
  #define TRIMS_GPIO_PIN_LVU           0x10000000
  #define TRIMS_GPIO_PIN_RVD           0x00000002
  #define TRIMS_GPIO_PIN_RHR           0x00000200
#else
  #define TRIMS_GPIO_PIN_LHL           0x00800000
  #define TRIMS_GPIO_PIN_LVD           0x01000000
  #define TRIMS_GPIO_PIN_RVU           0x00000002
  #define TRIMS_GPIO_PIN_RHL           0x00000001
  #define TRIMS_GPIO_PIN_LHR           0x00000010
  #define TRIMS_GPIO_PIN_LVU           0x10000000
  #define TRIMS_GPIO_PIN_RVD           0x00000400
  #define TRIMS_GPIO_PIN_RHR           0x00000200
#endif

// LCD driver
#define LCD_W                          128
#define LCD_H                          64
#define LCD_DEPTH                      1
#define LCD_CONTRAST_MIN               10
#define LCD_CONTRAST_MAX               45
#define LCD_CONTRAST_DEFAULT           25
void lcdInit();
void lcdRefresh();
#define lcdRefreshWait()
void lcdSetRefVolt(uint8_t val);
void lcdSetContrast();

// USB driver
void usbMassStorage();

#if !defined(SIMU)
void usbJoystickUpdate();
#endif

#define PIN_ENABLE                     0x001
#define PIN_PERIPHERAL                 0x000
#define PIN_INPUT                      0x002
#define PIN_OUTPUT                     0x000
#define PIN_PULLUP                     0x004
#define PIN_NO_PULLUP                  0x000
#define PIN_PULLDOWN                   0x008
#define PIN_NO_PULLDOWN                0x000
#define PIN_PERI_MASK_L                0x010
#define PIN_PERI_MASK_H                0x020
#define PIN_PER_A                      0x000
#define PIN_PER_B                      0x010
#define PIN_PER_C                      0x020
#define PIN_PER_D                      0x030
#define PIN_PORT_MASK                  0x0C0
#define PIN_PORTA                      0x000
#define PIN_PORTB                      0x040
#define PIN_PORTC                      0x080
#define PIN_LOW                        0x000
#define PIN_HIGH                       0x100

// Telemetry port
#define SECOND_USART                   USART0
#define SECOND_ID                      ID_USART0
#define SECOND_PINS                    { PINS_USART0 }

void configure_pins( uint32_t pins, uint16_t config );
uint16_t getCurrent();

extern uint16_t Current_analogue;
extern uint16_t Current_max;
extern uint32_t Current_accumulator;
extern uint32_t Current_used;
extern uint16_t sessionTimer;
void calcConsumption();

// Trainer driver
#define SLAVE_MODE()                   (pwrCheck() == e_power_trainer)
void init_trainer_capture();
void stop_trainer_capture();
#define TRAINER_DETECT_GPIO             PIOA
#define TRAINER_DETECT_GPIO_PIN         PIO_PA8
#define TRAINER_CONNECTED()            (TRAINER_DETECT_GPIO->PIO_PDSR & TRAINER_DETECT_GPIO_PIN)

// Write Flash driver
#define FLASH_PAGESIZE                 256
void flashWrite(uint32_t * address, const uint32_t * buffer);

// Keys driver
uint32_t switchState(uint8_t index);
uint32_t readKeys();
uint32_t readTrims();
#define NUM_TRIMS                      4
#define NUM_TRIMS_KEYS                 (NUM_TRIMS * 2)
#define TRIMS_PRESSED()                readTrims()
#define KEYS_PRESSED()                 readKeys()

// Pulses driver
void extmoduleSerialStart(uint32_t baudrate, uint32_t period_half_us, bool inverted);
void extmoduleSendNextFrame();
void module_output_active();
inline void EXTERNAL_MODULE_ON()
{
  module_output_active();
}

// SD driver
#if defined(SIMU)
#define sdInit()
#define sdDone()
#else
#define sdPoll10ms()
#if defined(__cplusplus)
extern "C" {
#endif
  void init_SDcard();
  void sdInit();
  void sdDone();
  uint32_t sd_card_ready();
  uint32_t sdMounted();
#if defined(__cplusplus)
}
#endif
#endif

// WDT driver
#if !defined(WATCHDOG) || defined(SIMU)
  #define WDG_ENABLE(x)
  #define WDG_RESET()
  #define IS_RESET_REASON_WATCHDOG()   false
#else
  #define WDG_ENABLE(x)                WDT->WDT_MR = 0x3FFF207F
  #define WDG_RESET()                  WDT->WDT_CR = 0xA5000001
  #define IS_RESET_REASON_WATCHDOG()   ((ResetReason & RSTC_SR_RSTTYP) == (2 << 8))
#endif

// Backlight driver
#define backlightEnable()              (PWM->PWM_CH_NUM[0].PWM_CDTY = currentBacklightBright)
#define backlightDisable()             (PWM->PWM_CH_NUM[0].PWM_CDTY = 100)
#define isBacklightEnabled()           (PWM->PWM_CH_NUM[0].PWM_CDTY != 100)
#define BACKLIGHT_ENABLE()             backlightEnable()
#define BACKLIGHT_DISABLE()            backlightDisable()
#define BACKLIGHT_FORCED_ON            101

// ADC driver
#define NUM_POTS                       3
#define STORAGE_NUM_POTS               3
#define NUM_SLIDERS                    0
#define STORAGE_NUM_SLIDERS            0
#define NUM_XPOTS                      0
#define NUM_MOUSE_ANALOGS              0
#define STORAGE_NUM_MOUSE_ANALOGS      0
enum Analogs {
  STICK1,
  STICK2,
  STICK3,
  STICK4,
  POT_FIRST,
  POT1 = POT_FIRST,
  POT2,
  POT3,
  POT_LAST = POT3,
  TX_VOLTAGE,
  TX_CURRENT,
  NUM_ANALOGS
};
enum CalibratedAnalogs {
  CALIBRATED_STICK1,
  CALIBRATED_STICK2,
  CALIBRATED_STICK3,
  CALIBRATED_STICK4,
  CALIBRATED_POT_FIRST,
  CALIBRATED_POT1 = CALIBRATED_POT_FIRST,
  CALIBRATED_POT2,
  CALIBRATED_POT3,
  CALIBRATED_POT_LAST = CALIBRATED_POT3,
  NUM_CALIBRATED_ANALOGS
};
#define IS_POT(x)                      ((x)>=POT_FIRST && (x)<=POT_LAST)
#define IS_SLIDER(x)                   false
#define STICKS_PWM_ENABLED()           false
void adcInit();
void adcRead();
uint16_t getAnalogValue(uint8_t index);
void setSticksGain(uint8_t gains);
inline void enableVBatBridge()
{
}
inline void disableVBatBridge()
{
}

// Battery driver
uint16_t getBatteryVoltage();          // returns current battery voltage in 10mV steps
#define BATTERY_MIN                    90  // 9V
#define BATTERY_MAX                    120 // 12V
#define BATTERY_WARN                   90  // 9V

// Buzzer driver
void buzzerSound(uint8_t duration);
void buzzerHeartbeat();
#define BUZZER_HEARTBEAT               buzzerHeartbeat

// i2c driver
void i2cCheck();

// Coproc driver
void coprocInit();
void coprocWriteData(uint8_t *data, uint32_t size);
#if defined(__cplusplus)
void coprocReadData(bool onlytemp=false);
#endif
extern int8_t volumeRequired;

#if defined(COPROCESSOR)
typedef struct  {
  uint8_t read;
  int8_t valid;
  int8_t temp;
  int8_t maxtemp;
} CoprocData;

extern CoprocData coprocData;
#endif

// Haptic driver
#define HAPTIC_PWM
void hapticOff();
void hapticOn(uint32_t pwmPercent);

// BlueTooth driver
#if defined(BLUETOOTH)
void btInit();
void bluetoothTask(void * pdata);
void btPushByte(uint8_t data);
#endif

// Power driver
#define TRAINER_PWR
#define SOFT_PWR_CTRL
void pwrInit();
void pwrOff();
void pwrOn();
uint32_t pwrCheck();
bool pwrPressed();
#define UNEXPECTED_SHUTDOWN()          (g_eeGeneral.unexpectedShutdown)
#define pwrForcePressed()              (false)

// EEPROM driver
#define EEPROM_SIZE           (4*1024*1024/8)
#define EEPROM_BLOCK_SIZE     (4*1024)
void eepromInit();
uint8_t eepromReadStatus();
uint8_t eepromIsTransferComplete();
void eepromBlockErase(uint32_t address);
void eepromStartRead(uint8_t * buffer, size_t address, size_t size);
void eepromStartWrite(uint8_t * buffer, size_t address, size_t size);

// Debug driver
void debugPutc(const char c);

// Telemetry driver
void telemetryPortInit(uint32_t baudrate, uint8_t mode);
void telemetryPortSetDirectionOutput();
void telemetryPortSetDirectionInput();
void telemetryTransmitBuffer(const uint8_t * buffer, uint32_t size);
void rxPdcUsart( void (*pChProcess)(uint8_t x) );
void sportSendBuffer(const uint8_t * buffer, uint32_t size);

// Second UART driver
void auxSerialTelemetryInit(unsigned int protocol);
void auxSerialPutc(const unsigned char c);
#if defined(__cplusplus)
bool telemetrySecondPortReceive(uint8_t & data);
#endif

extern const uint8_t BootCode[];

// Pulses driver
#define HARDWARE_EXTRA_MODULE

#endif // _BOARD_SKY9X_H_
