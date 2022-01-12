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
#include "opentx_constants.h"
#include "board_common.h"
#include "hal.h"

#if !defined(LUA_EXPORT_GENERATION)
#include "stm32f4xx_sdio.h"
#include "stm32f4xx_dma2d.h"
#include "stm32f4xx_ltdc.h"
#include "stm32f4xx_fmc.h"
#endif

#include "tp_cst340.h"
//#include "hallStick_driver.h"
#include "lcd_driver.h"
#include "battery_driver.h"

#define FLASHSIZE                       0x200000
#define BOOTLOADER_SIZE                 0x20000
#define FIRMWARE_ADDRESS                0x08000000

#define MB                              *1024*1024
#define LUA_MEM_EXTRA_MAX               (2 MB)    // max allowed memory usage for Lua bitmaps (in bytes)
#define LUA_MEM_MAX                     (6 MB)    // max allowed memory usage for complete Lua  (in bytes), 0 means unlimited

// HSI is at 168Mhz (over-drive is not enabled!)
#define PERI1_FREQUENCY                 42000000
#define PERI2_FREQUENCY                 84000000
#define TIMER_MULT_APB1                 2
#define TIMER_MULT_APB2                 2

extern uint16_t sessionTimer;

#define SLAVE_MODE()                    (g_model.trainerData.mode == TRAINER_MODE_SLAVE)
#define TRAINER_CONNECTED()             (true)

PACK(typedef struct {
  uint8_t pxx2Enabled:1;
}) HardwareOptions;

extern HardwareOptions hardwareOptions;

// Board driver
void boardInit();
void boardOff();

// Timers driver
void init2MhzTimer();
void init1msTimer();

// CPU Unique ID
#define LEN_CPU_UID                     (3*8+2)
void getCPUUniqueID(char * s);

// SD driver
#define BLOCK_SIZE                      512 /* Block Size in Bytes */
#if !defined(SIMU) || defined(SIMU_DISKIO)
uint32_t sdIsHC();
uint32_t sdGetSpeed();
#define SD_IS_HC()                     (sdIsHC())
#define SD_GET_SPEED()                 (sdGetSpeed())
#define SD_GET_FREE_BLOCKNR()          (sdGetFreeSectors())
#define SD_CARD_PRESENT()              true
void sdInit();
void sdMount();
void sdDone();
#define sdPoll10ms()
uint32_t sdMounted();
#else
#define SD_IS_HC()                      (0)
#define SD_GET_SPEED()                  (0)
#define sdInit()
#define sdMount()
#define sdDone()
#define SD_CARD_PRESENT()               true
#endif

// Flash Write driver
#define FLASH_PAGESIZE 256
void unlockFlash();
void lockFlash();
void flashWrite(uint32_t * address, const uint32_t * buffer);
uint32_t isFirmwareStart(const uint8_t * buffer);
uint32_t isBootloaderStart(const uint8_t * buffer);

// SDRAM driver
void SDRAM_Init();

// Pulses driver
void EXTERNAL_MODULE_ON();
void EXTERNAL_MODULE_OFF();
#define EXTERNAL_MODULE_PWR_OFF EXTERNAL_MODULE_OFF
#define BLUETOOTH_MODULE_ON()           GPIO_ResetBits(BT_EN_GPIO, BT_EN_GPIO_PIN)
#define BLUETOOTH_MODULE_OFF()          GPIO_SetBits(BT_EN_GPIO, BT_EN_GPIO_PIN)
#define IS_EXTERNAL_MODULE_ON()         (GPIO_ReadInputDataBit(EXTMODULE_PWR_GPIO, EXTMODULE_PWR_GPIO_PIN) == Bit_SET)
#define IS_PXX2_INTERNAL_ENABLED()      (false)

void init_intmodule_heartbeat();
void check_intmodule_heartbeat();

//void extmoduleSerialStart(uint32_t baudrate, uint32_t period_half_us, bool inverted);
void extmoduleSerialStart();
void extmoduleSendNextFrame();
void extmoduleSendInvertedByte(uint8_t byte);

// Trainer driver
void init_trainer_ppm();
void stop_trainer_ppm();
void init_trainer_capture();
void stop_trainer_capture();

// Keys driver
enum EnumKeys
{
  KEY_PGUP,
  KEY_PGDN,
  KEY_ENTER,
  KEY_MODEL,
  KEY_UP = KEY_MODEL,
  KEY_EXIT,
  KEY_DOWN = KEY_EXIT,
  KEY_TELEM,
  KEY_RIGHT = KEY_TELEM,
  KEY_RADIO,
  KEY_LEFT = KEY_RADIO,
  TRM_BASE,
  TRM_LH_DWN = TRM_BASE,
  TRM_LH_UP,
  TRM_LV_DWN,
  TRM_LV_UP,
  TRM_RV_DWN,
  TRM_RV_UP,
  TRM_RH_DWN,
  TRM_RH_UP,
  TRM_LS_DWN,
  TRM_LS_UP,
  TRM_RS_DWN,
  TRM_RS_UP,
  TRM_EX1_DWN,
  TRM_EX1_UP,
  TRM_EX2_DWN,
  TRM_EX2_UP,
  TRM_LAST = TRM_EX2_UP,
  NUM_KEYS
};

#define IS_SHIFT_KEY(index)             (false)
#define IS_SHIFT_PRESSED()              (false)
enum VirtualKeys {
  VKEY_MIN,
  VKEY_MAX,
  VKEY_INC,
  VKEY_DEC,
  VKEY_INC_LARGE,
  VKEY_DEC_LARGE,
  VKEY_DEFAULT,
};

enum LUATouchEvent {
  TOUCH_DOWN = 1,
  TOUCH_UP,
  TOUCH_SLIDE_UP,
  TOUCH_SLIDE_DOWN,
  TOUCH_SLIDE_LEFT,
  TOUCH_SLIDE_RIGHT,
};

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
  NUM_SWITCHES
};

#define STORAGE_NUM_SWITCHES  NUM_SWITCHES
#define DEFAULT_SWITCH_CONFIG (SWITCH_3POS << 14) + (SWITCH_3POS << 12) + (SWITCH_2POS << 10) + (SWITCH_3POS << 8) + (SWITCH_3POS << 6) + (SWITCH_2POS << 4) + (SWITCH_3POS << 2) + (SWITCH_2POS << 0); /* SWH ... SWA */

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
  SW_SE0,
  SW_SE1,
  SW_SE2,
  SW_SF0,
  SW_SF1,
  SW_SF2,
  SW_SG0,
  SW_SG1,
  SW_SG2,
  SW_SH0,
  SW_SH1,
  SW_SH2,
  NUM_SWITCHES_POSITIONS
};

#define STORAGE_NUM_SWITCHES_POSITIONS  (STORAGE_NUM_SWITCHES * 3)

#if !defined(NUM_FUNCTIONS_SWITCHES)
#define NUM_FUNCTIONS_SWITCHES        0
#endif

void monitorInit();
void keysInit();
uint8_t keyState(uint8_t index);
uint32_t switchState(uint8_t index);
uint32_t readKeys();
uint32_t readTrims();
#define NUM_TRIMS                       8
#define NUM_TRIMS_KEYS                  (NUM_TRIMS * 2)
#define TRIMS_PRESSED()                 (readTrims())
#define KEYS_PRESSED()                  (readKeys())
#define DBLKEYS_PRESSED_RGT_LFT(in)     (false)
#define DBLKEYS_PRESSED_UP_DWN(in)      (false)
#define DBLKEYS_PRESSED_RGT_UP(in)      (false)
#define DBLKEYS_PRESSED_LFT_DWN(in)     (false)

#define WDG_DURATION                              500 /*ms*/
void watchdogInit(unsigned int duration);
#if defined(SIMU)
  #define WAS_RESET_BY_WATCHDOG()               (false)
  #define WAS_RESET_BY_SOFTWARE()               (false)
  #define WAS_RESET_BY_WATCHDOG_OR_SOFTWARE()   (false)
  #define WDG_ENABLE(x)
  #define WDG_RESET()
#else
  #if defined(WATCHDOG)
    #define WDG_ENABLE(x)                       watchdogInit(x)
    #define WDG_RESET()                         IWDG->KR = 0xAAAA
  #else
    #define WDG_ENABLE(x)
    #define WDG_RESET()
  #endif
  #define WAS_RESET_BY_WATCHDOG()               (RCC->CSR & (RCC_CSR_WDGRSTF | RCC_CSR_WWDGRSTF))
  #define WAS_RESET_BY_SOFTWARE()               (RCC->CSR & RCC_CSR_SFTRSTF)
  #define WAS_RESET_BY_WATCHDOG_OR_SOFTWARE()   (RCC->CSR & (RCC_CSR_WDGRSTF | RCC_CSR_WWDGRSTF | RCC_CSR_SFTRSTF))
#endif

// ADC driver
#define NUM_POTS                        3
#define NUM_XPOTS                       0 // NUM_POTS
#define NUM_SLIDERS                     2
#define NUM_PWMSTICKS                   0
#define NUM_MOUSE_ANALOGS               0
#define STORAGE_NUM_POTS                3
#define STORAGE_NUM_SLIDERS             2
#define STORAGE_NUM_MOUSE_ANALOGS       0

enum Analogs {
  STICK1,
  STICK2,
  STICK3,
  STICK4,
  POT_FIRST,
  POT1 = POT_FIRST,
  POT2,
  POT3,
  POT_LAST = POT_FIRST + NUM_POTS - 1,
  SLIDER_FIRST,
  SLIDER_FRONT_LEFT = SLIDER_FIRST,
  SLIDER_FRONT_RIGHT,
  SLIDER_LAST = SLIDER_FIRST + NUM_SLIDERS - 1,
  SWITCH_FIRST,
  SWB = SWITCH_FIRST,
  SWD,
  SWE,
  SWF,
  SWG,
  SWH,
  SUB_ANALOG_POS = SWH,
  SWITCH_END = SWH,
  TX_VOLTAGE,
  TX_VBAT,
  NUM_ANALOGS
};

#define HARDWARE_POT3

#define SLIDER1 SLIDER_FRONT_LEFT
#define SLIDER2 SLIDER_FRONT_RIGHT

#define DEFAULT_POTS_CONFIG    (POT_WITH_DETENT << 4) + (POT_WITHOUT_DETENT << 2) + (POT_WITH_DETENT << 0) // VRA and VRC pots with detent, VRB without
#define DEFAULT_SLIDERS_CONFIG (SLIDER_WITH_DETENT << 1) + (SLIDER_WITH_DETENT << 0)

enum CalibratedAnalogs {
  CALIBRATED_STICK1,
  CALIBRATED_STICK2,
  CALIBRATED_STICK3,
  CALIBRATED_STICK4,
  CALIBRATED_POT1,
  CALIBRATED_POT2,
  CALIBRATED_POT3,
  CALIBRATED_SLIDER_REAR_LEFT,
  CALIBRATED_SLIDER_REAR_RIGHT,
  CALIBRATED_SWB,
  CALIBRATED_SWD,
  CALIBRATED_SWE,
  CALIBRATED_SWF,
  CALIBRATED_SWG,
  CALIBRATED_SWH,
  NUM_CALIBRATED_ANALOGS
};

#define IS_POT(x)                     ((x)>=POT_FIRST && (x)<=POT_LAST)
#define IS_SLIDER(x)                  ((x)>=SLIDER_FIRST && (x)<=SLIDER_LAST)

extern uint16_t adcValues[NUM_ANALOGS];


#define BATTERY_WARN                  37 // 3.7V
#define BATTERY_MIN                   35 // 3.4V
#define BATTERY_MAX                   43 // 4.3V

enum EnumPowerupState
{
  BOARD_POWER_OFF = 0xCAFEDEAD,
  BOARD_POWER_ON = 0xDEADBEEF,
  BOARD_STARTED = 0xBAADF00D,
  BOARD_REBOOT = 0xC00010FF,
};


#if defined(__cplusplus)
enum PowerReason {
  SHUTDOWN_REQUEST = 0xDEADBEEF,
  SOFTRESET_REQUEST = 0xCAFEDEAD,
};

constexpr uint32_t POWER_REASON_SIGNATURE = 0x0178746F;

inline bool UNEXPECTED_SHUTDOWN()
{
#if defined(SIMU) || defined(NO_UNEXPECTED_SHUTDOWN)
  return false;
#else
  if (WAS_RESET_BY_WATCHDOG())
    return true;
  else if (WAS_RESET_BY_SOFTWARE())
    return RTC->BKP0R != SOFTRESET_REQUEST;
  else
    return RTC->BKP1R == POWER_REASON_SIGNATURE && RTC->BKP0R != SHUTDOWN_REQUEST;
#endif
}

inline void SET_POWER_REASON(uint32_t value)
{
  RTC->BKP0R = value;
  RTC->BKP1R = POWER_REASON_SIGNATURE;
}
#endif

#if defined(__cplusplus) && !defined(SIMU)
extern "C" {
#endif

// Power driver
#define SOFT_PWR_CTRL
#define POWER_ON_DELAY               10 // 1s
void pwrInit();
void extModuleInit();
uint32_t pwrCheck();
uint32_t lowPowerCheck();

void pwrOn();
void pwrSoftReboot();
void pwrOff();
void pwrResetHandler();
bool pwrPressed();
#if defined(PWR_EXTRA_SWITCH_GPIO)
  bool pwrForcePressed();
#else
  #define pwrForcePressed() false
#endif
uint32_t pwrPressedDuration();;
  
#define AUX_SERIAL_POWER_ON()
#define AUX_SERIAL_POWER_OFF()

// LCD driver
#define LCD_W                           320 /* TODO! should be 480 */
#define LCD_H                           480 /* TODO! should be 320 */
#define LCD_DEPTH                       16
#define LCD_CONTRAST_DEFAULT            20
void lcdInit();
void lcdRefresh();
void lcdCopy(void * dest, void * src);
void DMAFillRect(uint16_t * dest, uint16_t destw, uint16_t desth, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void DMACopyBitmap(uint16_t * dest, uint16_t destw, uint16_t desth, uint16_t x, uint16_t y, const uint16_t * src, uint16_t srcw, uint16_t srch, uint16_t srcx, uint16_t srcy, uint16_t w, uint16_t h);
void DMACopyAlphaBitmap(uint16_t * dest, uint16_t destw, uint16_t desth, uint16_t x, uint16_t y, const uint16_t * src, uint16_t srcw, uint16_t srch, uint16_t srcx, uint16_t srcy, uint16_t w, uint16_t h);
void DMABitmapConvert(uint16_t * dest, const uint8_t * src, uint16_t w, uint16_t h, uint32_t format);
void lcdStoreBackupBuffer();
int lcdRestoreBackupBuffer();
void lcdSetContrast();
void lcdOff();
void lcdOn();
#define lcdSetRefVolt(...)
#define lcdRefreshWait(...)

// Backlight driver
void backlightInit();
#if defined(SIMU) || !defined(__cplusplus)
#define backlightEnable(...)
#define isBacklightEnabled() (true)
#else
void backlightEnable(uint8_t dutyCycle = 0);
bool isBacklightEnabled();
#endif

#define BACKLIGHT_LEVEL_MAX             100
#define BACKLIGHT_FORCED_ON             BACKLIGHT_LEVEL_MAX + 1
#define BACKLIGHT_LEVEL_MIN             1

#define BACKLIGHT_ENABLE()              backlightEnable(globalData.unexpectedShutdown ? BACKLIGHT_LEVEL_MAX : BACKLIGHT_LEVEL_MAX - currentBacklightBright)
#define BACKLIGHT_DISABLE()             backlightEnable(globalData.unexpectedShutdown ? BACKLIGHT_LEVEL_MAX : ((g_eeGeneral.blOffBright == BACKLIGHT_LEVEL_MIN) && (g_eeGeneral.backlightMode != e_backlight_mode_off)) ? 0 : g_eeGeneral.blOffBright)


#if !defined(SIMU)
void usbJoystickUpdate();
#endif
#define USB_NAME                        "FlySky PL18"
#define USB_MANUFACTURER                'F', 'l', 'y', 'S', 'k', 'y', ' ', ' '  /* 8 bytes */
#define USB_PRODUCT                     'P', 'L', '1', '8', ' ', ' ', ' ', ' '  /* 8 Bytes */

#if defined(__cplusplus) && !defined(SIMU)
}
#endif

// Audio driver
void audioInit();
void audioConsumeCurrentBuffer();
void audioSpiWriteBuffer(const uint8_t * buffer, uint32_t size);
void audioSpiSetSpeed(uint8_t speed);
uint8_t audioHardReset();
uint8_t audioSoftReset();
void audioSendRiffHeader();
void audioOn();
void audioOff();
bool isAudioReady();
bool audioChipReset();

#define SPI_SPEED_2                    0
#define SPI_SPEED_4                    1
#define SPI_SPEED_8                    2
#define SPI_SPEED_16                   3
#define SPI_SPEED_32                   4
#define SPI_SPEED_64                   5
#define SPI_SPEED_128                  6
#define SPI_SPEED_256                  7

#define audioDisableIrq()             // interrupts must stay enabled on Horus
#define audioEnableIrq()              // interrupts must stay enabled on Horus
#if defined(PCBNV14)
#define setSampleRate(freq)
#else
void setSampleRate(uint32_t frequency);
#endif
void setScaledVolume(uint8_t volume);
void setVolume(uint8_t volume);
int32_t getVolume();
#define VOLUME_LEVEL_MAX               23
#define VOLUME_LEVEL_DEF               12

// Telemetry driver
#define TELEMETRY_FIFO_SIZE             512
void telemetryPortInit(uint32_t baudrate, uint8_t mode);
void telemetryPortSetDirectionOutput();
void telemetryPortSetDirectionInput();
void sportSendBuffer(const uint8_t * buffer, uint32_t count);
bool telemetryGetByte(uint8_t * byte);
void telemetryClearFifo();
void sportSendByte(uint8_t byte);
extern uint32_t telemetryErrors;

// soft-serial
void telemetryPortInvertedInit(uint32_t baudrate);

// Sport update driver
#define SPORT_UPDATE_POWER_ON()
#define SPORT_UPDATE_POWER_OFF()
#define SPORT_UPDATE_POWER_INIT()
#define IS_SPORT_UPDATE_POWER_ON()     (false)

// Haptic driver
void hapticInit();
void hapticDone();
void hapticOff();
void hapticOn(uint32_t pwmPercent);

// Second serial port driver
//#define AUX_SERIAL
#define DEBUG_BAUDRATE                  115200
#define LUA_DEFAULT_BAUDRATE            115200
extern uint8_t auxSerialMode;
#if defined __cplusplus
void auxSerialSetup(unsigned int baudrate, bool dma, uint16_t length = USART_WordLength_8b, uint16_t parity = USART_Parity_No, uint16_t stop = USART_StopBits_1);
#endif
void auxSerialInit(unsigned int mode, unsigned int protocol);
void auxSerialPutc(char c);
#define auxSerialTelemetryInit(protocol) auxSerialInit(UART_MODE_TELEMETRY, protocol)
void auxSerialSbusInit();
void auxSerialStop();
#if defined(AUX_SERIAL_PWR_GPIO)
#define AUX_SERIAL_POWER_ON()            auxSerialPowerOn()
#define AUX_SERIAL__POWER_OFF()          auxSerialPowerOff()
#else
#define AUX_SERIAL_POWER_ON()
#define AUX_SERIAL__POWER_OFF()
#endif
#define USART_FLAG_ERRORS               (USART_FLAG_ORE | USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE)

extern uint8_t currentTrainerMode;
void checkTrainerSettings();

#if defined(__cplusplus)
#include "fifo.h"
#include "dmafifo.h"
extern DMAFifo<512> telemetryFifo;
typedef Fifo<uint8_t, 32> AuxSerialRxFifo;
extern AuxSerialRxFifo auxSerialRxFifo;
#endif

// Touch panel driver
bool touchPanelEventOccured();
struct TouchState touchPanelRead();

#endif // _BOARD_H_
