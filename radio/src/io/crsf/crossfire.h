/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#ifndef _IO_CROSSFIRE_H_
#define _IO_CROSSFIRE_H_

#include "fifo.h"

typedef enum {
  DEVICE_INTERNAL = 0,
  USB_HID,
  CRSF_SHARED_FIFO,
  CRSF_ESP,
  LAST_CRSF_PORT
} _CRSF_PORT_NAME;

typedef enum {
  DIO_IRQ_TRAMPOLINE = 0,
  NOTIFICATION_TIMER_IRQ_TRAMPOLINE,
  DEBUG_UART_IRQ_TRAMPOLINE,
  RTOS_WAIT_SEM_TRAMPOILINE,
  RTOS_CLEAR_SEM_TRAMPOILINE,
  WIFI_UART_IRQ_TRAMPOLINE,
  TRAMPOLINE_INDEX_COUNT
} TRAMPOLINE_INDEX;

typedef enum {
  XF_TASK_SEM = 0,
  CRSF_SD_TASK_SEM,
  BOOTLOADER_ICON_WAIT_SEM,
  TASK_SEM_COUNT,
  TASK_SEM_MAX = 10
} TASK_SEM_INDEX;

typedef enum {
  CRSF_FLAG_SHOW_BOOTLOADER_ICON = 0,
  CRSF_FLAG_EEPROM_SAVE,
  CRSF_FLAG_BOOTUP,
  CRSF_FLAG_POWER_OFF,
  CRSF_FLAG_RF_OFF,
  CRSF_FLAG_XF_UPDATE_REQUIRED,
  CRSF_FLAG_COUNT,
  CRSF_FLAG_MAX = 32,
} CRSF_FLAG_INDEX;

#define get_task_sem(i)     crossfireSharedData.taskSem[i]
#define getCrsfFlag(i)      (crossfireSharedData.crsfFlag & ( 1UL << i ) ? true : false )
#define setCrsfFlag(i)      (crossfireSharedData.crsfFlag |= ( 1UL << i ))
#define clearCrsfFlag(i)    (crossfireSharedData.crsfFlag &= ~( 1UL << i ))

typedef enum {
  BKREG_PREPARE_FWUPDATE = 0x0,
  BKREG_HW_ID,
  BKREG_SKIP_BOARD_OFF,
  BKREG_SERIAL_NO,
  BKREG_HW_ID_RADIO,
  BKREG_HW_ID_XF,
  BKREG_SERIAL_NO_RADIO,
  BKREG_SERIAL_NO_XF,
  BKREG_STATUS_FLAG,
  BKREG_DEFAULT_WORD,
  BKREG_TEMP_BUFFER_ADDR,
  BKREG_INDEX_COUNT
} BKREG_INDEX;

typedef enum {
  DEVICE_RESTART_WITHOUT_WARN_FLAG = 0x0,
  STORAGE_ERASE_STATUS,
  CRSF_SET_MODEL_ID_PENDING,
  BKREG_STATUS_FLAG_COUNT,
} BKREG_STATUS_FLAG_INDEX;

#if defined(SIMU)
  #define readBackupReg(...)                        0
  #define writeBackupReg()
  #define bkregGetStatusFlag(...)                   0
  #define bkregSetStatusFlag(...)
  #define bkregClrStatusFlag(...)
  #define boardSetSkipWarning()
  #define crossfirePowerOff()
  #define crossfireTurnOffRf(...)
  #define crossfireTurnOnRf()
#else
  uint32_t readBackupReg(uint8_t index);
  void writeBackupReg(uint8_t index, uint32_t data);
  uint8_t bkregGetStatusFlag(uint32_t flag);
  void bkregSetStatusFlag(uint32_t flag);
  void bkregClrStatusFlag(uint32_t flag);
  void boardSetSkipWarning();
  void crossfirePowerOff();
  void crossfireTurnOffRf(bool ask);
  void crossfireTurnOnRf();
  bool isCrossfireRfOn();
#endif

#define RTOS_API_VERSION                0x103
#define TELEMETRY_BUFFER_SIZE           128
#define CROSSFIRE_FIFO_SIZE             256
#define CROSSFIRE_CHANNELS_COUNT        16

struct CrossfireSharedData {
  uint32_t rtosApiVersion;
  uint32_t *trampoline;
  uint32_t *taskSem[TASK_SEM_MAX];
  volatile uint32_t crsfFlag;
  uint32_t reserved1[16];
  Fifo<uint8_t, CROSSFIRE_FIFO_SIZE> crsf_tx;   //from XF to OpenTX
  Fifo<uint8_t, CROSSFIRE_FIFO_SIZE> crsf_rx;   //from OpenTX to XF
  uint32_t reserved2[64];
  int16_t sticks[NUM_STICKS];
  uint8_t stick_state:5;
  uint8_t gim_select:2;
  uint8_t mixer_schedule:1;
  int32_t channels[CROSSFIRE_CHANNELS_COUNT];
};

#ifdef LIBCRSF_ENABLE_OPENTX_RELATED
//#define CRSF_ENABLE_SD_DEBUG
#ifdef CRSF_ENABLE_SD_DEBUG
#define CRSF_SD_PRINTF  TRACE_NOCRLF
#else
#define CRSF_SD_PRINTF(...)
#endif

extern bool set_model_id_needed;
extern uint8_t currentCrsfModelId;
void crsfRemoteRelatedHandler(uint8_t * p_arr);
#endif

typedef struct CrossfireSharedData CrossfireSharedData_t;
#define crossfireSharedData (*((CrossfireSharedData_t*)SHARED_MEMORY_ADDRESS))

#define isMixerTaskScheduled()    (crossfireSharedData.mixer_schedule)
#define clearMixerTaskSchedule()  {crossfireSharedData.mixer_schedule = 0;}
void crsfInit(void);
void crsfSharedFifoHandler(void);
void crsfSetModelID(void);
void crsfGetModelID(void);
uint32_t crsfGetHWID(void);
void updateIntCrossfireChannels(void);
void crsfToSharedFIFO(uint8_t * p_arr);
void crsfThisDevice(uint8_t * p_arr);

#endif // _CROSSFIRE_H_
