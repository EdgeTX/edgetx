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

#include "opentx.h"
#include "multi.h"
#include "pulses/afhds3.h"
#include "mixer_scheduler.h"

#if defined(LIBOPENUI)
  #include "libopenui.h"
#endif

uint8_t telemetryStreaming = 0;
uint8_t telemetryRxBuffer[TELEMETRY_RX_PACKET_SIZE];   // Receive buffer. 9 bytes (full packet), worst case 18 bytes with byte-stuffing (+1)
uint8_t telemetryRxBufferCount = 0;

uint8_t telemetryState = TELEMETRY_INIT;

TelemetryData telemetryData;

uint8_t telemetryProtocol = 255;

#if defined(PCBSKY9X) && defined(REVX)
uint8_t serialInversion = 0;
#endif

void processTelemetryData(uint8_t data)
{
#if defined(CROSSFIRE)
  if (telemetryProtocol == PROTOCOL_TELEMETRY_CROSSFIRE) {
    processCrossfireTelemetryData(data);
    return;
  }
#endif

#if defined(GHOST)
  if (telemetryProtocol == PROTOCOL_TELEMETRY_GHOST) {
    processGhostTelemetryData(data);
    return;
  }
#endif

#if defined(MULTIMODULE)
  if (telemetryProtocol == PROTOCOL_TELEMETRY_SPEKTRUM) {
    processSpektrumTelemetryData(EXTERNAL_MODULE, data, telemetryRxBuffer, telemetryRxBufferCount);
    return;
  }
  if (telemetryProtocol == PROTOCOL_TELEMETRY_FLYSKY_IBUS) {
    processFlySkyTelemetryData(data, telemetryRxBuffer, telemetryRxBufferCount);
    return;
  }
  if (telemetryProtocol == PROTOCOL_TELEMETRY_MULTIMODULE) {
    processMultiTelemetryData(data, EXTERNAL_MODULE);
    return;
  }
#endif

#if defined(AFHDS3)
  if (telemetryProtocol == PROTOCOL_TELEMETRY_AFHDS3) {
    afhds3::processTelemetryData(EXTERNAL_MODULE, data, telemetryRxBuffer, telemetryRxBufferCount, TELEMETRY_RX_PACKET_SIZE);
    return;
  }
#endif

  processFrskyTelemetryData(data);
}

inline bool isBadAntennaDetected()
{
  if (!isRasValueValid())
    return false;

  if (telemetryData.swrInternal.isFresh() && telemetryData.swrInternal.value() > FRSKY_BAD_ANTENNA_THRESHOLD)
    return true;

  if (telemetryData.swrExternal.isFresh() && telemetryData.swrExternal.value() > FRSKY_BAD_ANTENNA_THRESHOLD)
    return true;

  return false;
}

void telemetryWakeup()
{
  uint8_t requiredTelemetryProtocol = modelTelemetryProtocol();
  uint8_t data;

#if defined(REVX)
  uint8_t requiredSerialInversion = g_model.moduleData[EXTERNAL_MODULE].invertedSerial;
  if (telemetryProtocol != requiredTelemetryProtocol || serialInversion != requiredSerialInversion) {
    serialInversion = requiredSerialInversion;
    telemetryInit(requiredTelemetryProtocol);
  }
#else
  if (telemetryProtocol != requiredTelemetryProtocol) {
    telemetryInit(requiredTelemetryProtocol);
  }
#endif

#if defined(INTERNAL_MODULE_PXX2) || defined(EXTMODULE_USART)
  uint8_t frame[PXX2_FRAME_MAXLENGTH];

  #if defined(INTERNAL_MODULE_PXX2)
  while (intmoduleFifo.getFrame(frame)) {
    processPXX2Frame(INTERNAL_MODULE, frame);
  }
  #endif

  #if defined(EXTMODULE_USART)
  while (isModulePXX2(EXTERNAL_MODULE) && extmoduleFifo.getFrame(frame)) {
    processPXX2Frame(EXTERNAL_MODULE, frame);
  }
  #endif
#endif

#if defined(INTERNAL_MODULE_MULTI)

  #if defined(SIMU)
  // MULTI test code dynamic protocol list
  if (isModuleMultimodule(INTERNAL_MODULE)) {
    MultiModuleStatus& status = getMultiModuleStatus(INTERNAL_MODULE);
    if (!status.isValid()) {
      status.flags = 0;
      status.protocolName[0] = '\0';
      status.protocolSubNbr = 0;
      uint8_t proto = g_model.moduleData[INTERNAL_MODULE].getMultiProtocol();
      uint8_t subtype = g_model.moduleData[INTERNAL_MODULE].subType;
      switch (proto) {
      case MODULE_SUBTYPE_MULTI_FRSKY:
        strcpy(status.protocolName, "FrSky");
        status.protocolPrev = MODULE_SUBTYPE_MULTI_FRSKY + 1;
        status.protocolNext = MODULE_SUBTYPE_MULTI_FRSKYX2 + 3;
        status.protocolSubNbr = 8;
        if (subtype != MM_RF_FRSKY_SUBTYPE_D8 && subtype != MM_RF_FRSKY_SUBTYPE_D8_CLONED) {
          status.flags |= 0x20; // failsafe supported
        }
        break;
      case MODULE_SUBTYPE_MULTI_FRSKYX2:
        strcpy(status.protocolName, "FrSkyX2");
        status.protocolSubNbr = 6;
        status.protocolPrev = MODULE_SUBTYPE_MULTI_FRSKY + 1;
        status.protocolNext = MODULE_SUBTYPE_MULTI_FRSKYL + 3;
        status.flags |= 0x20; // failsafe supported
        break;
      case MODULE_SUBTYPE_MULTI_FRSKYL:
        strcpy(status.protocolName, "FrSkyL");
        status.protocolSubNbr = 2;
        status.protocolPrev = MODULE_SUBTYPE_MULTI_FRSKYX2 + 3;
        status.protocolNext = MODULE_SUBTYPE_MULTI_FRSKYX_RX + 3;
        break;
      case MODULE_SUBTYPE_MULTI_FRSKYX_RX:
        strcpy(status.protocolName, "FrSkyRX");
        //status.protocolSubNbr = 2;
        status.protocolPrev = MODULE_SUBTYPE_MULTI_FRSKYL + 3;
        status.protocolNext = proto;
        break;
      default: // special case: invalid proto to detect beginning of list
        TRACE("invalid proto: beginning of list");
        status.protocolPrev = MODULE_SUBTYPE_MULTI_FRSKY + 1;
        status.protocolNext = MODULE_SUBTYPE_MULTI_FRSKY + 1;
        break;
      }
      if (status.protocolName[0]) {
        status.flags |= 0x04; // mark proto as valid
      }
      status.lastUpdate = get_tmr10ms();
    }
  }
  #endif

  if (intmoduleFifo.pop(data)) {
    LOG_TELEMETRY_WRITE_START();
    do {
      processMultiTelemetryData(data, INTERNAL_MODULE);
      LOG_TELEMETRY_WRITE_BYTE(data);
    } while (intmoduleFifo.pop(data));
  }
#endif

#if defined(STM32)
  if (telemetryGetByte(&data)) {
    LOG_TELEMETRY_WRITE_START();
    do {
      processTelemetryData(data);
      LOG_TELEMETRY_WRITE_BYTE(data);
    } while (telemetryGetByte(&data));
  }
#elif defined(PCBSKY9X)
  if (telemetryProtocol == PROTOCOL_TELEMETRY_FRSKY_D_SECONDARY) {
    while (telemetrySecondPortReceive(data)) {
      processTelemetryData(data);
    }
  }
  else {
    // Receive serial data here
    rxPdcUsart(processTelemetryData);
  }
#endif

  for (int i=0; i<MAX_TELEMETRY_SENSORS; i++) {
    const TelemetrySensor & sensor = g_model.telemetrySensors[i];
    if (sensor.type == TELEM_TYPE_CALCULATED) {
      telemetryItems[i].eval(sensor);
    }
  }

#if defined(VARIO)
  if (TELEMETRY_STREAMING() && !IS_FAI_ENABLED()) {
    varioWakeup();
  }
#endif

  static tmr10ms_t alarmsCheckTime = 0;
  #define SCHEDULE_NEXT_ALARMS_CHECK(seconds) alarmsCheckTime = get_tmr10ms() + (100*(seconds))
  if (int32_t(get_tmr10ms() - alarmsCheckTime) > 0) {

    SCHEDULE_NEXT_ALARMS_CHECK(1/*second*/);

    bool sensorLost = false;
    for (int i=0; i<MAX_TELEMETRY_SENSORS; i++) {
      if (isTelemetryFieldAvailable(i)) {
        TelemetryItem & item = telemetryItems[i];
        if (item.timeout == 0) {
          TelemetrySensor * sensor = & g_model.telemetrySensors[i];
          if (sensor->unit != UNIT_DATETIME) {
            item.setOld();
            sensorLost = true;
          }
        }
      }
    }

    if (sensorLost && TELEMETRY_STREAMING() && !g_model.rssiAlarms.disabled) {
      audioEvent(AU_SENSOR_LOST);
    }

#if defined(PCBFRSKY)
    if (isBadAntennaDetected()) {
      AUDIO_RAS_RED();
      POPUP_WARNING(STR_WARNING, STR_ANTENNAPROBLEM);
      SCHEDULE_NEXT_ALARMS_CHECK(10/*seconds*/);
    }
#endif

    if (!g_model.rssiAlarms.disabled) {
      if (TELEMETRY_STREAMING()) {
        if (TELEMETRY_RSSI() < g_model.rssiAlarms.getCriticalRssi() ) {
          AUDIO_RSSI_RED();
          SCHEDULE_NEXT_ALARMS_CHECK(10/*seconds*/);
        }
        else if (TELEMETRY_RSSI() < g_model.rssiAlarms.getWarningRssi() ) {
          AUDIO_RSSI_ORANGE();
          SCHEDULE_NEXT_ALARMS_CHECK(10/*seconds*/);
        }
      }

      if (TELEMETRY_STREAMING()) {
        if (telemetryState == TELEMETRY_KO) {
          AUDIO_TELEMETRY_BACK();
#if defined(CROSSFIRE)
          if (isModuleCrossfire(EXTERNAL_MODULE)) {
            moduleState[EXTERNAL_MODULE].counter = CRSF_FRAME_MODELID;
          }
#endif
        }
        telemetryState = TELEMETRY_OK;
      }
      else if (telemetryState == TELEMETRY_OK) {
        telemetryState = TELEMETRY_KO;
        if (!isModuleInBeepMode()) {
          AUDIO_TELEMETRY_LOST();
        }
      }
    }
  }
}

void telemetryInterrupt10ms()
{
  if (telemetryStreaming > 0) {
    bool tick160ms = (telemetryStreaming & 0x0F) == 0;
    for (int i=0; i<MAX_TELEMETRY_SENSORS; i++) {
      const TelemetrySensor & sensor = g_model.telemetrySensors[i];
      if (sensor.type == TELEM_TYPE_CALCULATED) {
        telemetryItems[i].per10ms(sensor);
      }
      if (tick160ms && telemetryItems[i].timeout > 0) {
        telemetryItems[i].timeout--;
      }
    }
    telemetryStreaming--;
  }
  else {
#if !defined(SIMU)
    telemetryData.rssi.reset();
#endif
    for (auto & telemetryItem: telemetryItems) {
      if (telemetryItem.isAvailable()) {
        telemetryItem.setOld();
      }
    }
  }
}

void telemetryReset()
{
  telemetryData.clear();

  for (auto & telemetryItem : telemetryItems) {
    telemetryItem.clear();
  }

  telemetryStreaming = 0; // reset counter only if valid telemetry packets are being detected

  telemetryState = TELEMETRY_INIT;
}

// we don't reset the telemetry here as we would also reset the consumption after model load
void telemetryInit(uint8_t protocol)
{
  telemetryProtocol = protocol;

  if (protocol == PROTOCOL_TELEMETRY_FRSKY_D) {
    telemetryPortInit(FRSKY_D_BAUDRATE, TELEMETRY_SERIAL_DEFAULT);
  }

#if defined(MULTIMODULE)
  else if (protocol == PROTOCOL_TELEMETRY_MULTIMODULE || protocol == PROTOCOL_TELEMETRY_FLYSKY_IBUS) {
    // The DIY Multi module always speaks 100000 baud regardless of the telemetry protocol in use
    telemetryPortInit(MULTIMODULE_BAUDRATE, TELEMETRY_SERIAL_8E2);
#if defined(LUA)
    outputTelemetryBuffer.reset();
#endif
    telemetryPortSetDirectionInput();
  }
  else if (protocol == PROTOCOL_TELEMETRY_SPEKTRUM) {
    // Spektrum's own small race RX (SPM4648) uses 125000 8N1, use the same since there is no real standard
    telemetryPortInit(125000, TELEMETRY_SERIAL_DEFAULT);
  }
#endif

#if defined(CROSSFIRE)
  else if (protocol == PROTOCOL_TELEMETRY_CROSSFIRE) {
    telemetryPortInit(CROSSFIRE_BAUDRATE, TELEMETRY_SERIAL_DEFAULT);
#if defined(LUA)
    outputTelemetryBuffer.reset();
#endif
    telemetryPortSetDirectionOutput();
  }
#endif

#if defined(GHOST)
  else if (protocol == PROTOCOL_TELEMETRY_GHOST) {
    telemetryPortInit(GHOST_BAUDRATE, TELEMETRY_SERIAL_DEFAULT);
#if defined(LUA)
    outputTelemetryBuffer.reset();
#endif
    telemetryPortSetDirectionOutput();
  }
#endif

#if defined(AUX_SERIAL) || defined(PCBSKY9X)
  else if (protocol == PROTOCOL_TELEMETRY_FRSKY_D_SECONDARY) {
    telemetryPortInit(0, TELEMETRY_SERIAL_DEFAULT);
    auxSerialTelemetryInit(PROTOCOL_TELEMETRY_FRSKY_D_SECONDARY);
  }
#endif

#if defined(AFHDS3)
  else if (protocol == PROTOCOL_TELEMETRY_AFHDS3) {
    telemetryPortInvertedInit(AFHDS3_BAUDRATE);
    telemetryPortSetDirectionInput();
  }
#endif

  else {
    telemetryPortInit(FRSKY_SPORT_BAUDRATE, TELEMETRY_SERIAL_WITHOUT_DMA);
#if defined(LUA)
    outputTelemetryBuffer.reset();
#endif
  }

#if defined(REVX) && !defined(SIMU)
  if (serialInversion) {
    setMFP();
  }
  else {
    clearMFP();
  }
#endif
}


#if defined(LOG_TELEMETRY) && !defined(SIMU)
extern FIL g_telemetryFile;
void logTelemetryWriteStart()
{
  static tmr10ms_t lastTime = 0;
  tmr10ms_t newTime = get_tmr10ms();
  if (lastTime != newTime) {
    struct gtm utm;
    gettime(&utm);
    f_printf(&g_telemetryFile, "\r\n%4d-%02d-%02d,%02d:%02d:%02d.%02d0:", utm.tm_year+TM_YEAR_BASE, utm.tm_mon+1, utm.tm_mday, utm.tm_hour, utm.tm_min, utm.tm_sec, g_ms100);
    lastTime = newTime;
  }
}

void logTelemetryWriteByte(uint8_t data)
{
  f_printf(&g_telemetryFile, " %02X", data);
}
#endif

OutputTelemetryBuffer outputTelemetryBuffer __DMA;

#if defined(LUA)
Fifo<uint8_t, LUA_TELEMETRY_INPUT_FIFO_SIZE> * luaInputTelemetryFifo = NULL;
#endif

#if defined(HARDWARE_INTERNAL_MODULE)
static ModuleSyncStatus moduleSyncStatus[NUM_MODULES];

ModuleSyncStatus &getModuleSyncStatus(uint8_t moduleIdx)
{
  return moduleSyncStatus[moduleIdx];
}
#else
static ModuleSyncStatus moduleSyncStatus;

ModuleSyncStatus &getModuleSyncStatus(uint8_t moduleIdx)
{
  return moduleSyncStatus;
}
#endif

ModuleSyncStatus::ModuleSyncStatus()
{
  memset(this, 0, sizeof(ModuleSyncStatus));
}

void ModuleSyncStatus::update(uint16_t newRefreshRate, int16_t newInputLag)
{
  if (!newRefreshRate)
    return;
  
  if (newRefreshRate < MIN_REFRESH_RATE)
    newRefreshRate = newRefreshRate * (MIN_REFRESH_RATE / (newRefreshRate + 1));
  else if (newRefreshRate > MAX_REFRESH_RATE)
    newRefreshRate = MAX_REFRESH_RATE;

  refreshRate = newRefreshRate;
  inputLag    = newInputLag;
  currentLag  = newInputLag;
  lastUpdate  = get_tmr10ms();

  TRACE("[SYNC] update rate = %dus; lag = %dus",refreshRate,currentLag);
}

uint16_t ModuleSyncStatus::getAdjustedRefreshRate()
{
  int16_t lag = currentLag;
  int32_t newRefreshRate = refreshRate;

  if (lag == 0) {
    return refreshRate;
  }
  
  newRefreshRate += lag;
  
  if (newRefreshRate < MIN_REFRESH_RATE) {
      newRefreshRate = MIN_REFRESH_RATE;
  }
  else if (newRefreshRate > MAX_REFRESH_RATE) {
    newRefreshRate = MAX_REFRESH_RATE;
  }

  currentLag -= newRefreshRate - refreshRate;
  TRACE("[SYNC] mod rate = %dus; lag = %dus",newRefreshRate,currentLag);
  
  return (uint16_t)newRefreshRate;
}

void ModuleSyncStatus::getRefreshString(char * statusText)
{
  if (!isValid()) {
    return;
  }

  char * tmp = statusText;
#if defined(DEBUG)
  *tmp++ = 'L';
  tmp = strAppendSigned(tmp, inputLag, 5);
  tmp = strAppend(tmp, "R");
  tmp = strAppendUnsigned(tmp, refreshRate, 5);
#else
  tmp = strAppend(tmp, "Sync ");
  tmp = strAppendUnsigned(tmp, refreshRate);
#endif
  tmp = strAppend(tmp, "us");
}
