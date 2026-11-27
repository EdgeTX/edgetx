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

#include <stdio.h>
#include "edgetx.h"
#include "io/frsky_firmware_update.h"
#include "bluetooth_driver.h"
#include "os/sleep.h"
#include "trainer.h"

#if defined(LOG_BLUETOOTH)
extern FIL g_bluetoothFile;
#endif

#if defined(PCBX9E)
#define BLUETOOTH_COMMAND_NAME         "TTM:REN-"
#define BLUETOOTH_ANSWER_NAME          "TTM:REN"
#define BLUETOOTH_COMMAND_BAUD_115200  "TTM:BPS-115200"
#else
#define BLUETOOTH_COMMAND_NAME         "AT+NAME"
#define BLUETOOTH_ANSWER_NAME          "OK+"
#define BLUETOOTH_COMMAND_BAUD_115200  "AT+BAUD115200"
#endif

#if defined(_MSC_VER)
  #define SWAP32(val)      (_byteswap_ulong(val))
#elif defined(__GNUC__ )
  #define SWAP32(val)      (__builtin_bswap32(val))
#endif

Bluetooth bluetooth;

[[maybe_unused]] static bool isRoleAck(const char * line, bool audio)
{
  if (audio) return !strncmp(line, "Mode:A2DP", 9);
  return !strncmp(line, "Central:", 8) || !strncmp(line, "Peripheral:", 11);
}

void Bluetooth::write(const uint8_t * data, uint8_t length)
{
  BLUETOOTH_TRACE_VERBOSE("BT>");
  for (int i = 0; i < length; i++) {
    BLUETOOTH_TRACE_VERBOSE(" %02X", data[i]);
  }
  BLUETOOTH_TRACE_VERBOSE(CRLF);
  bluetoothWrite(data, length);
}

static const char _bt_crlf[] = "\r\n";

void Bluetooth::writeString(const char * str)
{
  BLUETOOTH_TRACE("BT> %s" CRLF, str);
  bluetoothWrite(str, strlen(str));
  bluetoothWrite(_bt_crlf, sizeof(_bt_crlf) - 1);
}

char * Bluetooth::readline(bool error_reset)
{
  uint8_t byte;

  while (true) {
    if (!bluetoothRead(&byte)) {
#if defined(PCBX9E)
      // X9E BT module can get unresponsive
      BLUETOOTH_TRACE("NO RESPONSE FROM BT MODULE" CRLF);
#endif
      return nullptr;
    }

    BLUETOOTH_TRACE_VERBOSE("%02X ", byte);

#if 0
    if (error_reset && byte == 'R' && bufferIndex == 4 && memcmp(buffer, "ERRO", 4)) {
#if defined(PCBX9E)  // X9E enter BT reset loop if following code is implemented
      BLUETOOTH_TRACE("BT Error..." CRLF);
#else
      BLUETOOTH_TRACE("BT Reset..." CRLF);
      bufferIndex = 0;
      bluetoothDisable();
      state = BLUETOOTH_STATE_OFF;
      wakeupTime = get_tmr10ms() + 100; /* 1s */
#endif
      return nullptr;
    }
    else
#endif

    if (byte == '\n') {
      if (bufferIndex > 2 && buffer[bufferIndex-1] == '\r') {
        buffer[bufferIndex-1] = '\0';
        bufferIndex = 0;
        BLUETOOTH_TRACE("BT< %s" CRLF, buffer);
        if (error_reset && !strcmp((char *)buffer, "ERROR")) {
#if defined(PCBX9E) // X9E enter BT reset loop if following code is implemented
          BLUETOOTH_TRACE("BT Error..." CRLF);
#else
          BLUETOOTH_TRACE("BT Reset..." CRLF);
          bluetoothDisable();
          state = BLUETOOTH_STATE_OFF;
          wakeupTime = get_tmr10ms() + 100; /* 1s */
#endif
          return nullptr;
        }
        else {
          if (!memcmp(buffer, "Central:", 8))
            strcpy(localAddr, (char *) buffer + 8);
          else if (!memcmp(buffer, "Peripheral:", 11))
            strcpy(localAddr, (char *) buffer + 11);
          return (char *) buffer;
        }
      }
      else {
        bufferIndex = 0;
      }
    }
    else {
      buffer[bufferIndex++] = byte;
      bufferIndex &= (BLUETOOTH_LINE_LENGTH-1);
    }
  }
}

void Bluetooth::processTrainerFrame(const uint8_t * buffer)
{
  for (uint8_t channel=0, i=1; channel<BLUETOOTH_TRAINER_CHANNELS; channel+=2, i+=3) {
    // +-500 != 512, but close enough.
    trainerInput[channel] = buffer[i] + ((buffer[i+1] & 0xf0) << 4) - 1500;
    trainerInput[channel+1] = ((buffer[i+1] & 0x0f) << 4) + ((buffer[i+2] & 0xf0) >> 4) + ((buffer[i+2] & 0x0f) << 8) - 1500;
  }

  trainerResetTimer();
}

void Bluetooth::appendTrainerByte(uint8_t data)
{
  if (bufferIndex < BLUETOOTH_LINE_LENGTH - 1) {
    buffer[bufferIndex++] = data;
    // we check for "DisConnected", but the first byte could be altered (if received in state STATE_DATA_XOR)
    if (data == '\n') {
      if (bufferIndex >= 13) {      
        if (!strncmp((char *)&buffer[bufferIndex-13], "isConnected", 11)) {
          BLUETOOTH_TRACE("BT< DisConnected" CRLF);
          state = BLUETOOTH_STATE_DISCONNECTED;
          bufferIndex = 0;
          wakeupTime += 200; // 1s
        }
      }
    }
  }
}

void Bluetooth::processTrainerByte(uint8_t data)
{
  static uint8_t dataState = STATE_DATA_IDLE;

  switch (dataState) {
    case STATE_DATA_START:
      if (data == START_STOP) {
        dataState = STATE_DATA_IN_FRAME;
        bufferIndex = 0;
      }
      else {
        appendTrainerByte(data);
      }
      break;

    case STATE_DATA_IN_FRAME:
      if (data == BYTE_STUFF) {
        dataState = STATE_DATA_XOR; // XOR next byte
      }
      else if (data == START_STOP) {
        dataState = STATE_DATA_IN_FRAME;
        bufferIndex = 0;
      }
      else {
        appendTrainerByte(data);
      }
      break;

    case STATE_DATA_XOR:
      switch (data) {
        case BYTE_STUFF ^ STUFF_MASK:
        case START_STOP ^ STUFF_MASK:
          // Expected content, save the data
          appendTrainerByte(data ^ STUFF_MASK);
          dataState = STATE_DATA_IN_FRAME;
          break;
        case START_STOP:  // Illegal situation, as we have START_STOP, try to start from the beginning
          bufferIndex = 0;
          dataState = STATE_DATA_IN_FRAME;
          break;
        default:  
          // Illegal situation, start looking for a new START_STOP byte
          dataState = STATE_DATA_START;
          break;
      }
      break;

    case STATE_DATA_IDLE:
      if (data == START_STOP) {
        bufferIndex = 0;
        dataState = STATE_DATA_START;
      }
      else {
        appendTrainerByte(data);
      }
      break;
  }

  if (bufferIndex >= BLUETOOTH_PACKET_SIZE) {
    uint8_t crc = 0x00;
    for (int i = 0; i < BLUETOOTH_PACKET_SIZE - 1; i++) {
      crc ^= buffer[i];
    }
    if (crc == buffer[BLUETOOTH_PACKET_SIZE - 1]) {
      if (buffer[0] == TRAINER_FRAME) {
        processTrainerFrame(buffer);
      }
    }
    dataState = STATE_DATA_IDLE;
  }
}

void Bluetooth::pushByte(uint8_t byte)
{
  crc ^= byte;
  if (byte == START_STOP || byte == BYTE_STUFF) {
    buffer[bufferIndex++] = BYTE_STUFF;
    byte ^= STUFF_MASK;
  }
  buffer[bufferIndex++] = byte;
}

void Bluetooth::sendTrainer()
{
  int16_t PPM_range = g_model.extendedLimits ? 640*2 : 512*2;

  int firstCh = g_model.trainerData.channelsStart;
  int lastCh = firstCh + BLUETOOTH_TRAINER_CHANNELS;

  uint8_t * cur = buffer;
  bufferIndex = 0;
  crc = 0x00;

  buffer[bufferIndex++] = START_STOP; // start byte
  pushByte(TRAINER_FRAME);
  for (int channel=firstCh; channel<lastCh; channel+=2, cur+=3) {
    uint16_t channelValue1 = PPM_CH_CENTER(channel) + limit((int16_t)-PPM_range, channelOutputs[channel], (int16_t)PPM_range) / 2;
    uint16_t channelValue2 = PPM_CH_CENTER(channel+1) + limit((int16_t)-PPM_range, channelOutputs[channel+1], (int16_t)PPM_range) / 2;
    pushByte(channelValue1 & 0x00ff);
    pushByte(((channelValue1 & 0x0f00) >> 4) + ((channelValue2 & 0x00f0) >> 4));
    pushByte(((channelValue2 & 0x000f) << 4) + ((channelValue2 & 0x0f00) >> 8));
  }
  pushByte(crc);
  buffer[bufferIndex++] = START_STOP; // end byte

  write(buffer, bufferIndex);

  // If not in verbose mode output one buffer per line
  #if defined(DEBUG_BLUETOOTH) && !defined(DEBUG_BLUETOOTH_VERBOSE)
    BLUETOOTH_TRACE_TIMESTAMP();
    for(int i=0; i < bufferIndex; i++) {
      BLUETOOTH_TRACE(" %02X", buffer[i]);
    }
    BLUETOOTH_TRACE(CRLF);
  #endif

  bufferIndex = 0;
}

void Bluetooth::forwardTelemetry(const uint8_t * packet)
{
  crc = 0x00;

  buffer[bufferIndex++] = START_STOP; // start byte
  for (uint8_t i=0; i<sizeof(SportTelemetryPacket); i++) {
    pushByte(packet[i]);
  }
  pushByte(crc);
  buffer[bufferIndex++] = START_STOP; // end byte

  if (bufferIndex >= 2*FRSKY_SPORT_PACKET_SIZE) {
    write(buffer, bufferIndex);
    bufferIndex = 0;
  }

  // If not in verbose mode output one buffer per line
  #if defined(DEBUG_BLUETOOTH) && !defined(DEBUG_BLUETOOTH_VERBOSE)
    BLUETOOTH_TRACE_TIMESTAMP();
    for(int i=0; i < bufferIndex; i++) {
      BLUETOOTH_TRACE(" %02X", buffer[i]);
    }
    BLUETOOTH_TRACE(CRLF);
  #endif
}

void Bluetooth::receiveTrainer()
{
  uint8_t byte;

  while (true) {
    if (!bluetoothRead(&byte)) {
      return;
    }

#if defined(DEBUG_BLUETOOTH)
    static uint8_t lastb=0;
    BLUETOOTH_TRACE("%02X ", byte);
    if(byte == START_STOP && lastb != START_STOP) {
      BLUETOOTH_TRACE(CRLF);
      BLUETOOTH_TRACE_TIMESTAMP();
    }
    lastb = byte;
#endif

    processTrainerByte(byte);
  }
}

#if defined(PCBX9E)
void Bluetooth::wakeup(void)
{
#if defined(PCBX9E)
  if (g_eeGeneral.bluetoothMode == BLUETOOTH_AUDIO) {
    g_eeGeneral.bluetoothMode = BLUETOOTH_OFF;
  }
#endif
#if !defined(SIMU)
  if (!g_eeGeneral.bluetoothMode) {
    if (state != BLUETOOTH_INIT) {
      bluetoothDisable();
      state = BLUETOOTH_INIT;
    }
  }
  else {
    static tmr10ms_t waitEnd = 0;
    if (state != BLUETOOTH_STATE_IDLE) {
      if (state == BLUETOOTH_INIT) {
        bluetoothInit(BLUETOOTH_DEFAULT_BAUDRATE, true);
        char command[32];
        char * cur = strAppend(command, BLUETOOTH_COMMAND_NAME);
        uint8_t len = ZLEN(g_eeGeneral.bluetoothName);
        if (len > 0) {
          for (int i = 0; i < len; i++) {
            *cur++ = char2lower(g_eeGeneral.bluetoothName[i]);
          }
          *cur = '\0';
        }
        else {
          cur = strAppend(cur, FLAVOUR);
        }
        writeString(command);
        state = BLUETOOTH_WAIT_TTM;
        waitEnd = get_tmr10ms() + 25; // 250ms
      }
      else if (state == BLUETOOTH_WAIT_TTM) {
        if (get_tmr10ms() > waitEnd) {
          char * line = readline();
          if (strncmp(line, "OK+", 3)) {
            state = BLUETOOTH_STATE_IDLE;
          }
          else {
            bluetoothInit(BLUETOOTH_FACTORY_BAUDRATE, true);
            writeString("TTM:BPS-115200");
            state = BLUETOOTH_WAIT_BAUDRATE_CHANGE;
            waitEnd = get_tmr10ms() + 250; // 2.5s
          }
        }
      }
      else if (state == BLUETOOTH_WAIT_BAUDRATE_CHANGE) {
        if (get_tmr10ms() > waitEnd) {
          state = BLUETOOTH_INIT;
        }
      }
    }
    else if (IS_BLUETOOTH_TRAINER()){
      state = BLUETOOTH_STATE_CONNECTED;
      sendTrainer();
    }
  }
#endif
}
#else // PCBX9E
void Bluetooth::wakeup()
{
#if defined(BLUETOOTH_A2DP)
  if (audioWakeup(get_tmr10ms())) {
    return;
  }
#else
  if (g_eeGeneral.bluetoothMode == BLUETOOTH_AUDIO) {
    g_eeGeneral.bluetoothMode = BLUETOOTH_OFF;  // setting from an A2DP build
  }
#endif

  if (state != BLUETOOTH_STATE_OFF) {
    if (bluetoothIsWriting()) {
      return;
    }
  }

  tmr10ms_t now = get_tmr10ms();

#if defined(BLUETOOTH_A2DP)
  if (state == BLUETOOTH_STATE_BOOT_WAIT) {
    discardInput();
    if (now < wakeupTime) return;
    state = BLUETOOTH_STATE_BAUDRATE_INIT;
    discardInput();
  }
#endif

  if (now < wakeupTime)
    return;

  wakeupTime = now + 5; /* 50ms default */

  if (state == BLUETOOTH_STATE_FLASH_FIRMWARE) {
    return;
  }

#if defined(BLUETOOTH_A2DP)
  // The A2DP role needs a different init sequence than telemetry/trainer
  const bool wantAudio = g_eeGeneral.bluetoothMode == BLUETOOTH_AUDIO;
  if (wantAudio != audioMode) {
    const bool leavingAudio = audioMode;
    audioMode = wantAudio;
    state = leavingAudio ? (audioLink ? BLUETOOTH_STATE_AUDIO_BAUD_DOWN
                                      : BLUETOOTH_STATE_AUDIO_STOP)
                         : BLUETOOTH_STATE_OFF;
    audioNextState = BLUETOOTH_STATE_AUDIO_STOP;
  }
#endif

#if defined(BLUETOOTH_A2DP)
  if (state == BLUETOOTH_STATE_AUDIO_STOP) {
    writeString("AT+A2DPOFF");
    state = BLUETOOTH_STATE_OFF;
    wakeupTime = now + 10;
    return;
  }
#endif

#if defined(BT_PWR_GPIO)
  if (g_eeGeneral.bluetoothMode == BLUETOOTH_OFF) {
    bluetoothDisable();
    state = BLUETOOTH_STATE_OFF;
    wakeupTime = now + 10; /* 100ms */
    return;
  }
#endif

  if (g_eeGeneral.bluetoothMode == BLUETOOTH_OFF ||
      (g_eeGeneral.bluetoothMode == BLUETOOTH_TRAINER &&
       !IS_BLUETOOTH_TRAINER())) {

    if (state != BLUETOOTH_STATE_OFF) {
      bluetoothDisable();
      state = BLUETOOTH_STATE_OFF;
    }
    wakeupTime = now + 10; /* 100ms */
  }
  else if (state == BLUETOOTH_STATE_OFF) {
#if defined(BLUETOOTH_A2DP)
    audioConnectTried = false;
    if (audioMode) {
      memset(distantAddr, 0, sizeof(distantAddr));
      memcpy(distantAddr, g_eeGeneral.bluetoothAudioAddr, LEN_BLUETOOTH_AUDIO_ADDR);
    }
    bluetoothInit(BLUETOOTH_DEFAULT_BAUDRATE, true);
    state = BLUETOOTH_STATE_BOOT_WAIT;
    wakeupTime = now + 100;
#else
    bluetoothInit(BLUETOOTH_FACTORY_BAUDRATE, true);
    state = BLUETOOTH_STATE_FACTORY_BAUDRATE_INIT;
#endif
  }

  if (state == BLUETOOTH_STATE_FACTORY_BAUDRATE_INIT) {
    writeString("AT+BAUD4");
    state = BLUETOOTH_STATE_BAUDRATE_SENT;
    wakeupTime = now + 10; /* 100ms */
  }
  else if (state == BLUETOOTH_STATE_BAUDRATE_SENT) {
    bluetoothInit(BLUETOOTH_DEFAULT_BAUDRATE, true);
    state = BLUETOOTH_STATE_BAUDRATE_INIT;
    readline(false);
    wakeupTime = now + 10; /* 100ms */
  }
  else if (state == BLUETOOTH_STATE_CONNECTED) {
    if (g_eeGeneral.bluetoothMode == BLUETOOTH_TRAINER && g_model.trainerData.mode == TRAINER_MODE_MASTER_BLUETOOTH) {
      receiveTrainer();
    }
    else {
      if (g_eeGeneral.bluetoothMode == BLUETOOTH_TRAINER && g_model.trainerData.mode == TRAINER_MODE_SLAVE_BLUETOOTH) {
        sendTrainer();
        wakeupTime = now + 2; /* 20ms */
      }
      readline(); // to deal with "ERROR"
    }
  }
  else {
    char * line = readline();
    if (state == BLUETOOTH_STATE_BAUDRATE_INIT) {
    #if defined(BLUETOOTH_A2DP)
      discardInput();
    #endif
      char command[32];
      char * cur = strAppend(command, BLUETOOTH_COMMAND_NAME);
      uint8_t len = ZLEN(g_eeGeneral.bluetoothName);
      if (len > 0) {
        for (int i = 0; i < len; i++) {
          *cur++ = char2lower(g_eeGeneral.bluetoothName[i]);
        }
        *cur = '\0';
      }
      else {
        cur = strAppend(cur, FLAVOUR);
      }
      writeString(command);
      state = BLUETOOTH_STATE_NAME_SENT;
    } else if (state == BLUETOOTH_STATE_NAME_SENT && (line != nullptr) &&
               (!strncmp(line, "OK+", 3) || !strncmp(line, "Central:", 8) ||
                !strncmp(line, "Peripheral:", 11))) {
      writeString("AT+TXPW2");
      state = BLUETOOTH_STATE_POWER_SENT;
    } else if (state == BLUETOOTH_STATE_POWER_SENT &&
               (line != nullptr) &&
               (!strncmp(line, "Central:", 8) ||
                !strncmp(line, "Peripheral:", 11) ||
                !strncmp(line, "Mode:", 5))) {
#if defined(BLUETOOTH_A2DP)
      if (audioMode)
        writeString("AT+A2DPON");
      else
#endif
      if (g_eeGeneral.bluetoothMode == BLUETOOTH_TRAINER &&
          g_model.trainerData.mode == TRAINER_MODE_MASTER_BLUETOOTH)
        writeString("AT+ROLE1");
      else
        writeString("AT+ROLE0");
      state = BLUETOOTH_STATE_ROLE_SENT;
    } else if (state == BLUETOOTH_STATE_ROLE_SENT &&
               (line != nullptr) &&
#if defined(BLUETOOTH_A2DP)
               isRoleAck(line, audioMode)) {
#else
               isRoleAck(line, false)) {
#endif
      state = BLUETOOTH_STATE_IDLE;
    } else if (state == BLUETOOTH_STATE_DISCOVER_REQUESTED) {
      writeString("AT+DISC?");
      state = BLUETOOTH_STATE_DISCOVER_SENT;
    } else if (state == BLUETOOTH_STATE_DISCOVER_SENT &&
               (line != nullptr) &&
               !strcmp(line, "OK+DISCS")) {
      state = BLUETOOTH_STATE_DISCOVER_START;
    } else if (state == BLUETOOTH_STATE_DISCOVER_START &&
               (line != nullptr) &&
               !strncmp(line, "OK+DISC:", 8)) {
#if defined(BLUETOOTH_A2DP)
      if (audioMode) {
        const char * addr = &line[8];
        bool known = false;
        for (uint8_t i = 0; i < audioDevicesCount; i++) {
          known = known || !strcmp(audioDevices[i], addr);
        }
        if (strlen(addr) == LEN_BLUETOOTH_AUDIO_ADDR && !known &&
            audioDevicesCount < MAX_BLUETOOTH_DISTANT_ADDR) {
          strcpy(audioDevices[audioDevicesCount], addr);
          audioDevicesCount = audioDevicesCount + 1;
        }
      } else
#endif
      if (strlen(line) < 8 + LEN_BLUETOOTH_ADDR &&
          reusableBuffer.moduleSetup.bt.devicesCount <
              MAX_BLUETOOTH_DISTANT_ADDR) {
        strncpy(reusableBuffer.moduleSetup.bt
                    .devices[reusableBuffer.moduleSetup.bt.devicesCount],
                &line[8], LEN_BLUETOOTH_ADDR);
        ++reusableBuffer.moduleSetup.bt.devicesCount;
      }
    }
#if defined(BLUETOOTH_A2DP)
    else if (audioMode && state == BLUETOOTH_STATE_DISCOVER_START &&
             (line != nullptr) && !strcmp(line, "OK+DISCE")) {
      state = BLUETOOTH_STATE_DISCOVER_END;
    }
#endif
#if defined(PCBHORUS)
    else if (state == BLUETOOTH_STATE_DISCOVER_START &&
             (line != nullptr) && !strcmp(line, "OK+DISCE")) {
      state = BLUETOOTH_STATE_DISCOVER_END;
    }
#endif
    else if (state == BLUETOOTH_STATE_CLEAR_REQUESTED) {
#if defined(BLUETOOTH_A2DP)
      if (audioMode) {
        // drop the sink connection by restarting the A2DP role
        writeString("AT+A2DPOFF");
        state = BLUETOOTH_STATE_POWER_SENT;
      } else
#endif
      {
        char command[] = "AT+CLEAR";
        writeString(command);
        state = BLUETOOTH_STATE_IDLE;
      }
    } else if (state == BLUETOOTH_STATE_BIND_REQUESTED) {
      char command[32];
#if defined(BLUETOOTH_A2DP)
      strAppend(strAppend(command, audioMode ? "AT+A2DPCON" : "AT+CON"), distantAddr);
#else
      strAppend(strAppend(command, "AT+CON"), distantAddr);
#endif
      writeString(command);
      state = BLUETOOTH_STATE_CONNECT_SENT;
    }
#if defined(BLUETOOTH_A2DP)
    else if (audioMode && state == BLUETOOTH_STATE_CONNECT_SENT &&
             (line != nullptr) && !strncmp(line, "OK+A2DPCON", 10)) {
      audioSetStoredAddr(distantAddr);
      writeString(BLUETOOTH_AUDIO_BAUD_CMD);
      state = BLUETOOTH_STATE_AUDIO_BAUD_UP;
      wakeupTime = now + 10;
    } else if (audioMode && state == BLUETOOTH_STATE_AUDIO_BAUD_UP) {
      bluetoothInit(BLUETOOTH_AUDIO_BAUDRATE, true);
      audioRidx = audioWidx = 0;
      audioLink = true;
      state = BLUETOOTH_STATE_CONNECTED;
    } else if (audioMode && !audioConnectTried && distantAddr[0] &&
               (state == BLUETOOTH_STATE_IDLE || state == BLUETOOTH_STATE_DISCOVER_END)) {
      audioConnectTried = true;
      state = BLUETOOTH_STATE_BIND_REQUESTED;
    }
#endif
    else if ((state == BLUETOOTH_STATE_IDLE ||
                state == BLUETOOTH_STATE_DISCONNECTED ||
                state == BLUETOOTH_STATE_CONNECT_SENT) &&
               (line != nullptr) && !strncmp(line, "Connected:", 10)) {
      strcpy(distantAddr, &line[10]); // TODO quick & dirty
      state = BLUETOOTH_STATE_CONNECTED;
      if (g_model.trainerData.mode == TRAINER_MODE_SLAVE_BLUETOOTH) {
        wakeupTime += 500; // it seems a 5s delay is needed before sending the 1st frame
      }
    } else if (state == BLUETOOTH_STATE_DISCONNECTED && !line) {
      char command[32];
      strAppend(strAppend(command, "AT+CON"), distantAddr);
      writeString(command);
      wakeupTime = now + 200; /* 2s */
    }
  }
}
#endif

#if defined(BLUETOOTH_A2DP)
static constexpr uint16_t AUDIO_RING_MASK = BLUETOOTH_AUDIO_RING_SIZE - 1;

void Bluetooth::discardInput()
{
  uint8_t byte;
  while (bluetoothRead(&byte)) {}
  bufferIndex = 0;
}

bool Bluetooth::audioActive() const
{
  return audioLink && state == BLUETOOTH_STATE_CONNECTED &&
         g_eeGeneral.bluetoothMode == BLUETOOTH_AUDIO;
}

// Producer side (audio task): mono s16 LE at AUDIO_SAMPLE_RATE, no conversion of the rate
bool Bluetooth::audioPush(const void * samples, uint16_t count)
{
  if (!audioActive()) return false;

  auto in = (const audio_data_t *)samples;
  uint16_t w = audioWidx;
  const uint16_t r = audioRidx;

  for (uint16_t i = 0; i < count; i++) {
    if (((r - w - 1) & AUDIO_RING_MASK) < 2) break;
    int16_t s = (int16_t)((int32_t)in[i] - AUDIO_DATA_SILENCE);
    audioRing[w] = s & 0xFF;
    audioRing[(w + 1) & AUDIO_RING_MASK] = (s >> 8) & 0xFF;
    w = (w + 2) & AUDIO_RING_MASK;
  }

  audioWidx = w;
  return true;
}

// Consumer side (mixer task)
void Bluetooth::audioPump()
{
  if (bluetoothIsWriting()) return;

  static uint8_t chunk[BLUETOOTH_AUDIO_MAX_CHUNK];
  const uint16_t w = audioWidx;
  uint16_t r = audioRidx;
  uint16_t n = 0;

  while (r != w && n < sizeof(chunk)) {
    chunk[n++] = audioRing[r];
    r = (r + 1) & AUDIO_RING_MASK;
  }
  if (n & 1) {  // never split a sample
    n--;
    r = (r - 1) & AUDIO_RING_MASK;
  }
  if (n > 0) {
    char command[32];
    snprintf(command, sizeof(command), "AT+A2DPSAMPLES%u", n / 2);
    writeString(command);
    bluetoothWrite(chunk, n);
    audioRidx = r;
  }
}

void Bluetooth::audioSetStoredAddr(const char * addr)
{
  if (strncmp(g_eeGeneral.bluetoothAudioAddr, addr, LEN_BLUETOOTH_AUDIO_ADDR) != 0) {
    memset(g_eeGeneral.bluetoothAudioAddr, 0, LEN_BLUETOOTH_AUDIO_ADDR);
    strncpy(g_eeGeneral.bluetoothAudioAddr, addr, LEN_BLUETOOTH_AUDIO_ADDR);
    storageDirty(EE_GENERAL);
  }
}

void Bluetooth::audioStartScan()
{
  audioDevicesCount = 0;
  audioConnectTried = false;
  state = BLUETOOTH_STATE_DISCOVER_REQUESTED;
}

void Bluetooth::audioSelectDevice(const char * addr)
{
  memset(distantAddr, 0, sizeof(distantAddr));
  strncpy(distantAddr, addr, LEN_BLUETOOTH_AUDIO_ADDR);
  state = BLUETOOTH_STATE_BIND_REQUESTED;
}

void Bluetooth::audioForgetDevice()
{
  memset(distantAddr, 0, sizeof(distantAddr));
  audioSetStoredAddr("");
  state = BLUETOOTH_STATE_CLEAR_REQUESTED;
}

// Handles the PCM link; returns true when it consumed this wakeup
bool Bluetooth::audioWakeup(tmr10ms_t now)
{
  if (!audioLink) return false;

  if (g_eeGeneral.bluetoothMode == BLUETOOTH_AUDIO && state == BLUETOOTH_STATE_CONNECTED) {
    audioPump();
    readline(false);  // module chatter such as A2DPVOL:n
    return true;
  }

  if (state == BLUETOOTH_STATE_AUDIO_STATS_WAIT) {
    if (bluetoothIsWriting()) return true;
    discardInput();
    writeString("AT+A2DPSTATS?");
    state = BLUETOOTH_STATE_AUDIO_STATS_SENT;
    wakeupTime = now + 25;
    return true;
  }

  if (state == BLUETOOTH_STATE_AUDIO_STATS_SENT) {
    char * line = readline(false);
    if (line && !strncmp(line, "OK+A2DPSTATS:", 13)) {
      unsigned int underruns;
      unsigned int uartDropBytes;
      unsigned int uartOverflowEvents;
      unsigned int uartFifoOverflowEvents;
      unsigned int uartBufferFullEvents;
      unsigned int pcmDropBytes;
      unsigned int pcmQueueDropBytes;
      unsigned int pcmPacketTimeoutBytes;
      unsigned int resets;
      unsigned int lowWaterBytes;
      if (sscanf(line,
                 "OK+A2DPSTATS:underruns=%u,uart_drop_bytes=%u,uart_overflow_events=%u,uart_fifo_overflow_events=%u,uart_buffer_full_events=%u,pcm_drop_bytes=%u,pcm_queue_drop_bytes=%u,pcm_packet_timeout_bytes=%u,resets=%u,low_water_bytes=%u",
                 &underruns, &uartDropBytes, &uartOverflowEvents,
                 &uartFifoOverflowEvents, &uartBufferFullEvents, &pcmDropBytes,
                 &pcmQueueDropBytes, &pcmPacketTimeoutBytes, &resets,
                 &lowWaterBytes) == 10) {
        audioUnderruns = underruns;
        audioUartDropBytes = uartDropBytes;
        audioUartOverflowEvents = uartOverflowEvents;
        audioUartFifoOverflowEvents = uartFifoOverflowEvents;
        audioUartBufferFullEvents = uartBufferFullEvents;
        audioPcmDropBytes = pcmDropBytes;
        audioPcmQueueDropBytes = pcmQueueDropBytes;
        audioPcmPacketTimeoutBytes = pcmPacketTimeoutBytes;
        audioResets = resets;
        audioLowWaterBytes = lowWaterBytes;
        audioStatsValid = true;
        BLUETOOTH_TRACE("BT A2DP stats: %s" CRLF, line);
      }
    }

    if ((line && !strncmp(line, "OK+A2DPSTATS:", 13)) || now >= wakeupTime) {
      discardInput();
      writeString("AT+BAUD4");
      state = BLUETOOTH_STATE_AUDIO_BAUD_DOWN;
      wakeupTime = now + 10;
    }
    return true;
  }

  // Leave PCM mode first: the module only understands AT commands at the default rate
  if (state == BLUETOOTH_STATE_AUDIO_BAUD_DOWN) {
    if (now >= wakeupTime && !bluetoothIsWriting()) {
      bluetoothInit(BLUETOOTH_DEFAULT_BAUDRATE, true);
      audioLink = false;
      audioConnectTried = false;
      state = audioNextState;
      wakeupTime = now + 10;
    }
    return true;
  }

  if (bluetoothIsWriting()) return true;

  audioNextState = BLUETOOTH_STATE_AUDIO_STOP;
  audioStatsValid = false;
  state = BLUETOOTH_STATE_AUDIO_STATS_WAIT;
  wakeupTime = now;
  return true;
}
#endif // BLUETOOTH_A2DP

uint8_t Bluetooth::bootloaderChecksum(uint8_t command, const uint8_t * data, uint8_t size)
{
  uint8_t sum = command;
  for (uint8_t i = 0; i < size; i++) {
    sum += data[i];
  }
  return sum;
}

uint8_t Bluetooth::read(uint8_t * data, uint8_t size, uint32_t timeout)
{
  watchdogSuspend(timeout / 10);

  uint8_t len = 0;
  while (len < size) {
    uint32_t elapsed = 0;
    uint8_t byte;
    while (!bluetoothRead(&byte)) {
      if (elapsed++ >= timeout) {
        return len;
      }
      sleep_ms(1);
    }
    data[len++] = byte;
  }
  return len;
}

#define BLUETOOTH_ACK   0xCC
#define BLUETOOTH_NACK  0x33

const char * Bluetooth::bootloaderWaitCommandResponse(uint32_t timeout)
{
  uint8_t response[2];
  if (read(response, sizeof(response), timeout) != sizeof(response)) {
    return "Bluetooth timeout";
  }

  if (response[0] != 0x00) {
    return "Bluetooth error";
  }

  if (response[1] == BLUETOOTH_ACK || response[1] == BLUETOOTH_NACK) {
    return nullptr;
  }

  return "Bluetooth error";
}

const char * Bluetooth::bootloaderWaitResponseData(uint8_t *data, uint8_t size)
{
  uint8_t header[2];
  if (read(header, 2) != 2) {
    return "Bluetooth timeout";
  }

  uint8_t len = header[0] - 2;
  uint8_t checksum = header[1];

  if (len > size) {
    return "Bluetooth error";
  }

  if (read(data, len) != len) {
    return "Bluetooth timeout";
  }

  if (bootloaderChecksum(0, data, len) != checksum) {
    return "Bluetooth CRC error";
  }

  return nullptr;
}

const char * Bluetooth::bootloaderSetAutoBaud()
{
  uint8_t packet[2] = {
    0x55,
    0x55
  };
  write(packet, sizeof(packet));
  return bootloaderWaitCommandResponse();
}

void Bluetooth::bootloaderSendCommand(uint8_t command, const void *data, uint8_t size)
{
  uint8_t packet[3] = {
    uint8_t(3 + size),
    bootloaderChecksum(command, (uint8_t *) data, size),
    command
  };
  write(packet, sizeof(packet));
  if (size > 0) {
    write((uint8_t *)data, size);
  }
}

void Bluetooth::bootloaderSendCommandResponse(uint8_t response)
{
  uint8_t packet[2] = {
    0x00,
    response
  };
  write(packet, sizeof(packet));
}

enum {
  CMD_DOWNLOAD = 0x21,
  CMD_GET_STATUS = 0x23,
  CMD_SEND_DATA = 0x24,
  CMD_SECTOR_ERASE = 0x26,
  CMD_GET_CHIP_ID = 0x28,
};

enum {
  CMD_RET_SUCCESS = 0x40,
};

constexpr uint32_t CC26XX_BOOTLOADER_SIZE = 0x00001000;
constexpr uint32_t CC26XX_FIRMWARE_BASE = CC26XX_BOOTLOADER_SIZE;

constexpr uint32_t CC26XX_PAGE_ERASE_SIZE = 0x1000;
constexpr uint32_t CC26XX_MAX_BYTES_PER_TRANSFER = 252;

const char * Bluetooth::bootloaderSendData(const uint8_t * data, uint8_t size)
{
  bootloaderSendCommand(CMD_SEND_DATA, data, size);
  return bootloaderWaitCommandResponse();
}

const char * Bluetooth::bootloaderReadStatus(uint8_t &status)
{
  bootloaderSendCommand(CMD_GET_STATUS);
  const char * result = bootloaderWaitCommandResponse();
  if (result)
    return result;
  result = bootloaderWaitResponseData(&status, 1);
  bootloaderSendCommandResponse(result == nullptr ? BLUETOOTH_ACK : BLUETOOTH_NACK);
  return result;
}

const char * Bluetooth::bootloaderCheckStatus()
{
  uint8_t status;
  const char * result = bootloaderReadStatus(status);
  if (result)
    return result;
  if (status != CMD_RET_SUCCESS)
    return "Wrong status";
  return nullptr;
}

const char * Bluetooth::bootloaderEraseFlash(uint32_t start, uint32_t size)
{
  uint32_t address = start;
  uint32_t end = start + size;
  while (address < end) {
    uint32_t addressBigEndian = SWAP32(address);
    bootloaderSendCommand(CMD_SECTOR_ERASE, &addressBigEndian, sizeof(addressBigEndian));
    const char * result = bootloaderWaitCommandResponse();
    if (result)
      return result;
    result = bootloaderCheckStatus();
    if (result)
      return result;
    address += CC26XX_PAGE_ERASE_SIZE;
  }
  return nullptr;
}

const char * Bluetooth::bootloaderStartWriteFlash(uint32_t start, uint32_t size)
{
  uint32_t cmdArgs[2] = {
    SWAP32(start),
    SWAP32(size),
  };
  bootloaderSendCommand(CMD_DOWNLOAD, cmdArgs, sizeof(cmdArgs));
  const char * result = bootloaderWaitCommandResponse();
  if (result)
    return result;
  result = bootloaderCheckStatus();
  if (result)
    return result;
  return result;
}

const char * Bluetooth::bootloaderWriteFlash(const uint8_t * data, uint32_t size)
{
  while (size > 0) {
    uint32_t len = min<uint32_t>(size, CC26XX_MAX_BYTES_PER_TRANSFER);
    const char * result = bootloaderSendData(data, len);
    if (result)
      return result;
    result = bootloaderCheckStatus();
    if (result)
      return result;
    data += len;
    size -= len;
  }
  return nullptr;
}

const char * Bluetooth::doFlashFirmware(const char * filename, ProgressHandler progressHandler)
{
  const char * result;
  FIL file;
  uint8_t buffer[CC26XX_MAX_BYTES_PER_TRANSFER * 4];
  UINT count;

  // Dummy command
  bootloaderSendCommand(0);
  result = bootloaderWaitCommandResponse(0);
  if (result)
    result = bootloaderSetAutoBaud();
  if (result)
    return result;

  // Get chip ID
  bootloaderSendCommand(CMD_GET_CHIP_ID);
  result = bootloaderWaitCommandResponse();
  if (result)
    return result;
  uint8_t id[4];
  result = bootloaderWaitResponseData(id, 4);
  bootloaderSendCommandResponse(result == nullptr ? BLUETOOTH_ACK : BLUETOOTH_NACK);

  if (f_open(&file, filename, FA_READ) != FR_OK) {
    return "Error opening file";
  }

  FrSkyFirmwareInformation * information = (FrSkyFirmwareInformation *)buffer;
  if (f_read(&file, buffer, sizeof(FrSkyFirmwareInformation), &count) != FR_OK || count != sizeof(FrSkyFirmwareInformation)) {
    f_close(&file);
    return "Format error";
  }

  progressHandler(getBasename(filename), STR_FLASH_ERASE, 0, 0);

  result = bootloaderEraseFlash(CC26XX_FIRMWARE_BASE, information->size);
  if (result) {
    f_close(&file);
    return result;
  }

  uint32_t size = information->size;
  progressHandler(getBasename(filename), STR_FLASH_WRITE, 0, size);

  result = bootloaderStartWriteFlash(CC26XX_FIRMWARE_BASE, size);
  if (result)
    return result;

  uint32_t done = 0;
  while (1) {
    progressHandler(getBasename(filename), STR_FLASH_WRITE, done, size);
    if (f_read(&file, buffer, min<uint32_t>(sizeof(buffer), size - done), &count) != FR_OK) {
      f_close(&file);
      return "Error reading file";
    }
    result = bootloaderWriteFlash(buffer, count);
    if (result)
      return result;
    done += count;
    if (done >= size) {
      f_close(&file);
      return nullptr;
    }
  }
}

const char * Bluetooth::flashFirmware(const char * filename, ProgressHandler progressHandler)
{
  progressHandler(getBasename(filename), STR_MODULE_RESET, 0, 0);

  state = BLUETOOTH_STATE_FLASH_FIRMWARE;

  pulsesStop();

  bluetoothInit(BLUETOOTH_BOOTLOADER_BAUDRATE, true); // normal mode
  watchdogSuspend(500 /*5s*/);
  sleep_ms(1000);

  bluetoothInit(BLUETOOTH_BOOTLOADER_BAUDRATE, false); // bootloader mode
  watchdogSuspend(500 /*5s*/);
  sleep_ms(1000);

  const char * result = doFlashFirmware(filename, progressHandler);

  AUDIO_PLAY(AU_SPECIAL_SOUND_BEEP1 );
  BACKLIGHT_ENABLE();

  if (result) {
    POPUP_WARNING(STR_FIRMWARE_UPDATE_ERROR, result);
  }
  else {
    POPUP_INFORMATION(STR_FIRMWARE_UPDATE_SUCCESS);
  }

  progressHandler(getBasename(filename), STR_MODULE_RESET, 0, 0);

  /* wait 1s off */
  watchdogSuspend(500 /*5s*/);
  sleep_ms(1000);

  state = BLUETOOTH_STATE_OFF;
  pulsesStart();

  return result;
}
