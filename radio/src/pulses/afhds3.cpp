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

#include "afhds3.h"
#include "../debug.h"
#include "../definitions.h"
#include <cstdio>

#define FAILSAFE_HOLD 1
#define FAILSAFE_CUSTOM 2
#define MAX_RETRIES_AFHDS3 5

extern void processFlySkySensor(const uint8_t * packet, uint8_t type);

extern void extmoduleSerialStart(uint32_t baudrate, uint32_t period_half_us, bool inverted);

namespace afhds3
{

static const char* const moduleStateText[] =
{
  "Not ready",
  "HW Error",
  "Binding",
  "Connecting",
  "Connected",
  "Standby",
  "Waiting for update",
  "Updating",
  "Updating RX",
  "Updating RX failed",
  "Testing",
  "Ready",
  "HW test"
};

static const char* const powerSourceText[] =
{
  "Unknown",
  "Internal",
  "External"
};

static const COMMAND periodicRequestCommands[] =
{
  COMMAND::MODULE_STATE,
  COMMAND::MODULE_POWER_STATUS,
  COMMAND::MODULE_GET_CONFIG,
  COMMAND::VIRTUAL_FAILSAFE
};

//Address used in transmitted frames - it constrains of target address and source address
const uint8_t FrameAddress = DeviceAddress::TRANSMITTER | (DeviceAddress::MODULE << 4);

//Static collection of afhds3 object instances by module
PulsesData* AFHDS3PulsesData[EXTERNAL_MODULE + 1] = { nullptr, nullptr };

//friends function that can access telemetry parsing method
void processTelemetryData(uint8_t module, uint8_t data, uint8_t* rxBuffer, uint8_t& rxBufferCount, uint8_t maxSize)
{
  if (AFHDS3PulsesData[module]) {
    AFHDS3PulsesData[module]->processTelemetryData(data, rxBuffer, rxBufferCount, maxSize);
  }
}

void CommandFifo::clearCommandFifo()
{
  memclear(commandFifo, sizeof(commandFifo));
  setIndex = getIndex = 0;
}

void CommandFifo::enqueueACK(COMMAND command, uint8_t frameNumber)
{
  uint32_t next = nextIndex(setIndex);
  if (next != getIndex) {
    commandFifo[setIndex].command = command;
    commandFifo[setIndex].frameType = FRAME_TYPE::RESPONSE_ACK;
    commandFifo[setIndex].payload = 0;
    commandFifo[setIndex].payloadSize = 0;
    commandFifo[setIndex].frameNumber = frameNumber;
    commandFifo[setIndex].useFrameNumber = true;
    setIndex = next;
  }
}

void CommandFifo::enqueue(COMMAND command, FRAME_TYPE frameType, bool useData, uint8_t byteContent)
{
  uint32_t next = nextIndex(setIndex);
  if (next != getIndex) {
    commandFifo[setIndex].command = command;
    commandFifo[setIndex].frameType = frameType;
    commandFifo[setIndex].payload = byteContent;
    commandFifo[setIndex].payloadSize = useData ? 1 : 0;
    commandFifo[setIndex].frameNumber = 0;
    commandFifo[setIndex].useFrameNumber = false;
    setIndex = next;
  }
}

void PulsesData::getStatusString(char * buffer) const
{
  strcpy(buffer, this->state <= ModuleState::STATE_READY ? moduleStateText[this->state] : "Unknown");
}

void PulsesData::getPowerStatus(char* buffer) const
{
  strcpy(buffer, this->powerSource <= MODULE_POWER_SOURCE::EXTERNAL ? powerSourceText[this->powerSource] : "Unknown");
}

void PulsesData::processTelemetryData(uint8_t byte, uint8_t* rxBuffer, uint8_t& rxBufferCount, uint8_t maxSize)
{
  if (rxBufferCount == 0 && byte != AfhdsSpecialChars::START) {
    TRACE("AFHDS3 [SKIP] %02X", byte);
    this->esc_state = 0;
    return;
  }

  if (byte == AfhdsSpecialChars::ESC) {
    this->esc_state = rxBufferCount;
    return;
  }

  if (rxBufferCount > 1 && byte == AfhdsSpecialChars::END) {
    rxBuffer[rxBufferCount++] = byte;
    parseData(rxBuffer, rxBufferCount);
    rxBufferCount = 0;
    return;
  }

  if (this->esc_state && byte == AfhdsSpecialChars::ESC_END) {
    byte = AfhdsSpecialChars::END;
  }
  else if (esc_state && byte == AfhdsSpecialChars::ESC_ESC) {
    byte = AfhdsSpecialChars::ESC;
  }
  //reset esc index
  this->esc_state = 0;

  if (rxBufferCount >= maxSize) {
    TRACE("AFHDS3 [BUFFER OVERFLOW]");
    rxBufferCount = 0;
  }
  rxBuffer[rxBufferCount++] = byte;
}

bool PulsesData::isConnectedUnicast()
{
  return cfg.config.telemetry == TELEMETRY::TELEMETRY_ENABLED && this->state == ModuleState::STATE_SYNC_DONE;
}

bool PulsesData::isConnectedMulticast()
{
  return cfg.config.telemetry == TELEMETRY::TELEMETRY_DISABLED && this->state == ModuleState::STATE_SYNC_RUNNING;
}

void PulsesData::setupFrame()
{
  if (operationState == State::AWAITING_RESPONSE) {
    if (repeatCount++ < MAX_RETRIES_AFHDS3) {
      return; //re-send
    }
    else
    {
      TRACE("AFHDS3 [NO RESP] module state %d", this->state);
      clearFrameData();
      this->state = ModuleState::STATE_NOT_READY;
    }
  }
  else if (operationState == State::UNKNOWN) {
    this->state = ModuleState::STATE_NOT_READY;
  }
  repeatCount = 0;
  if (this->state == ModuleState::STATE_NOT_READY) {
    TRACE("AFHDS3 [GET MODULE READY]");
    putFrame(COMMAND::MODULE_READY, FRAME_TYPE::REQUEST_GET_DATA);
    return;
  }

  //check waiting commands
  if (!isEmpty()) {
    Frame f = commandFifo[getIndex];
    putFrame(f.command, f.frameType, &f.payload, f.payloadSize, f.useFrameNumber ? &f.frameNumber : &frame_index);
    getIndex = nextIndex(getIndex);
    TRACE("AFHDS3 [CMD QUEUE] cmd: %d frameType %d, useFrameNumber %d frame Number %d size %d", f.command, f.frameType, f.useFrameNumber, f.frameNumber, f.payloadSize);
    return;
  }

  //config should be loaded already
  if (syncSettings()) {
    return;
  }

  //if module is ready but not started
  if (this->state == ModuleState::STATE_READY || this->state == ModuleState::STATE_STANDBY) {
    cmdCount = 0;
    repeatCount = 0;
    requestInfoAndRun(true);
    return;
  }

  ::ModuleSettingsMode moduleMode = getModuleMode(module_index);

  if (moduleMode == ::ModuleSettingsMode::MODULE_MODE_BIND) {
    if (state != STATE_BINDING) {
      TRACE("AFHDS3 [BIND]");
      setConfigFromModel();
      putFrame(COMMAND::MODULE_SET_CONFIG, FRAME_TYPE::REQUEST_SET_EXPECT_DATA, cfg.buffer, sizeof(cfg.buffer));
      requestedModuleMode = MODULE_MODE_E::BIND;
      enqueue(COMMAND::MODULE_MODE, FRAME_TYPE::REQUEST_SET_EXPECT_DATA, true, requestedModuleMode);
      return;
    }
  }
  else if (moduleMode == ::ModuleSettingsMode::MODULE_MODE_RANGECHECK) {
    if (cfg.config.runPower != RUN_POWER::RUN_POWER_FIRST) {
      TRACE("AFHDS3 [RANGE CHECK]");
      cfg.config.runPower = RUN_POWER::RUN_POWER_FIRST;
      uint8_t data[] = { 0x13, 0x20, 0x02, cfg.config.runPower, 0 };
      TRACE("AFHDS3 SET TX POWER %d", moduleData->afhds3.runPower);
      putFrame(COMMAND::SEND_COMMAND, FRAME_TYPE::REQUEST_SET_EXPECT_DATA, data, sizeof(data));
      return;
    }
  }
  else if (moduleMode == ::ModuleSettingsMode::MODULE_MODE_NORMAL) { //exit bind
    if (state == STATE_BINDING) {
      TRACE("AFHDS3 [EXIT BIND]");
      requestedModuleMode = MODULE_MODE_E::RUN;
      putFrame(COMMAND::MODULE_MODE, FRAME_TYPE::REQUEST_SET_EXPECT_DATA, &requestedModuleMode, 1);
      return;
    }
  }

  bool isConnected = isConnectedUnicast() || isConnectedMulticast();

  if (cmdCount++ >= 150) {
    cmdCount = 0;
    if (cmdIndex >= sizeof(periodicRequestCommands)) {
      cmdIndex = 0;
    }
    COMMAND cmd = periodicRequestCommands[cmdIndex++];

    if (cmd == COMMAND::VIRTUAL_FAILSAFE) {
      if (isConnected) {
        if (isConnectedMulticast()) {
          TRACE("AFHDS ONE WAY FAILSAFE");
          uint16_t failSafe[MAX_CHANNELS + 1] = { ((MAX_CHANNELS << 8) | CHANNELS_DATA_MODE::FAIL_SAFE), 0 };
          setFailSafe((int16_t*) (&failSafe[1]));
          putFrame(COMMAND::CHANNELS_FAILSAFE_DATA, FRAME_TYPE::REQUEST_SET_NO_RESP, (uint8_t*) failSafe, MAX_CHANNELS * 2 + 2);
        }
        else {
          TRACE("AFHDS TWO WAYS FAILSAFE");
          uint8_t failSafe[3 + MAX_CHANNELS * 2] = { 0x11, 0x60, MAX_CHANNELS * 2 };
          setFailSafe((int16_t*) (failSafe + 3));
          putFrame(COMMAND::SEND_COMMAND, FRAME_TYPE::REQUEST_SET_EXPECT_DATA, failSafe, 3 + MAX_CHANNELS * 2);
        }
      }
      else {
        putFrame(COMMAND::MODULE_STATE, FRAME_TYPE::REQUEST_GET_DATA);
      }
    }
    else
    {
      putFrame(cmd, FRAME_TYPE::REQUEST_GET_DATA);
    }
  }
  else if (isConnected) {
    sendChannelsData();
  }
  else {
    //default frame - request state
    putFrame(MODULE_STATE, FRAME_TYPE::REQUEST_GET_DATA);
  }
}

void PulsesData::init(uint8_t moduleIndex, bool resetFrameCount)
{
  module_index = moduleIndex;
  AFHDS3PulsesData[module_index] = this;
  //clear local vars because it is member of union
  moduleData = &g_model.moduleData[module_index];
  operationState = State::UNKNOWN;
  state = ModuleState::STATE_NOT_READY;
  clearFrameData();
}

void PulsesData::clearFrameData()
{
  TRACE("AFHDS3 clearFrameData");
  reset();
  clearCommandFifo();
  repeatCount = 0;
  cmdCount = 0;
  cmdIndex = 0;
  this->frame_index = 1;
  this->timeout = 0;
  this->esc_state = 0;
}

void PulsesData::putBytes(uint8_t* data, int length)
{
  for (int i = 0; i < length; i++) {
    uint8_t byte = data[i];
    this->crc += byte;
    if (END == byte) {
      sendByte(ESC);
      sendByte(ESC_END);
    }
    else if (ESC == byte) {
      sendByte(ESC);
      sendByte(ESC_ESC);
    }
    else {
      sendByte(byte);
    }
  }
}

void PulsesData::putFrame(COMMAND command, FRAME_TYPE frame, uint8_t* data, uint8_t dataLength, uint8_t* frameIndex)
{
  //header
  operationState = State::SENDING_COMMAND;
  reset();
  this->crc = 0;
  sendByte(START);
  if (frameIndex == nullptr) {
    frameIndex = &this->frame_index;
  }
  uint8_t buffer[] = {FrameAddress, *frameIndex, frame, command};
  putBytes(buffer, 4);

  //payload
  if (dataLength > 0) {
    putBytes(data, dataLength);
  }
  //footer
  uint8_t crcValue = this->crc ^ 0xff;
  putBytes(&crcValue, 1);
  sendByte(END);
  *frameIndex = *frameIndex + 1;

  switch (frame) {
    case FRAME_TYPE::REQUEST_GET_DATA:
    case FRAME_TYPE::REQUEST_SET_EXPECT_ACK:
    case FRAME_TYPE::REQUEST_SET_EXPECT_DATA:
      operationState = State::AWAITING_RESPONSE;
      break;
    default:
      operationState = State::IDLE;
  }

  flush();
}

bool checkCRC(const uint8_t* data, uint8_t size)
{
  uint8_t crc = 0;
  //skip start byte
  for (uint8_t i = 1; i < size; i++) {
    crc += data[i];
  }
  return (crc ^ 0xff) == data[size];
}

bool containsData(enum FRAME_TYPE frameType)
{
  return (frameType == FRAME_TYPE::RESPONSE_DATA ||
      frameType == FRAME_TYPE::REQUEST_SET_EXPECT_DATA ||
      frameType == FRAME_TYPE::REQUEST_SET_EXPECT_ACK ||
      frameType == FRAME_TYPE::REQUEST_SET_EXPECT_DATA ||
      frameType == FRAME_TYPE::REQUEST_SET_NO_RESP);
}

void PulsesData::setState(uint8_t state)
{
  if (state == this->state) {
    return;
  }
  uint8_t oldState = this->state;
  this->state = state;
  if (oldState == ModuleState::STATE_BINDING) {
    setModuleMode(module_index, ::ModuleSettingsMode::MODULE_MODE_NORMAL);
  }
  if (state == ModuleState::STATE_NOT_READY) {
    operationState = State::UNKNOWN;
  }
}

void PulsesData::requestInfoAndRun(bool send)
{
  if (!send) {
    enqueue(COMMAND::MODULE_VERSION, FRAME_TYPE::REQUEST_GET_DATA);
  }
  enqueue(COMMAND::MODULE_POWER_STATUS, FRAME_TYPE::REQUEST_GET_DATA);
  requestedModuleMode = MODULE_MODE_E::RUN;
  enqueue(COMMAND::MODULE_MODE, FRAME_TYPE::REQUEST_SET_EXPECT_DATA, true, (uint8_t) MODULE_MODE_E::RUN);
  if (send) {
    putFrame(COMMAND::MODULE_VERSION, FRAME_TYPE::REQUEST_GET_DATA);
  }
}

void PulsesData::parseData(uint8_t* rxBuffer, uint8_t rxBufferCount)
{

  if (!checkCRC(rxBuffer, rxBufferCount - 2)) {
    TRACE("AFHDS3 [INVALID CRC]");
    return;
  }
  AfhdsFrame* responseFrame = reinterpret_cast<AfhdsFrame*>(rxBuffer);
  if (containsData((enum FRAME_TYPE) responseFrame->frameType)) {
    switch (responseFrame->command) {
      case COMMAND::MODULE_READY:
        TRACE("AFHDS3 [MODULE_READY] %02X", responseFrame->value);
        if (responseFrame->value == MODULE_STATUS_READY) {
          setState(ModuleState::STATE_READY);
          requestInfoAndRun();
        }
        else {
          setState(ModuleState::STATE_NOT_READY);
        }
        break;
      case COMMAND::MODULE_GET_CONFIG:
        std::memcpy((void*) cfg.buffer, &responseFrame->value, sizeof(cfg.buffer));
        TRACE("AFHDS3 [MODULE_GET_CONFIG] bind power %d run power %d mode %d pwm/ppm %d ibus/sbus %d", cfg.config.bindPower, cfg.config.runPower, cfg.config.telemetry, cfg.config.pulseMode, cfg.config.serialMode);
        break;
      case COMMAND::MODULE_VERSION:
        std::memcpy((void*) &version, &responseFrame->value, sizeof(version));
        TRACE("AFHDS3 [MODULE_VERSION] Product %d, HW %d, BOOT %d, FW %d", version.productNumber, version.hardwereVersion, version.bootloaderVersion, version.firmwareVersion);
        break;
      case COMMAND::MODULE_POWER_STATUS:
        powerSource = (enum MODULE_POWER_SOURCE) responseFrame->value;
        TRACE("AFHDS3 [MODULE_POWER_STATUS], %d", powerSource);
        break;
      case COMMAND::MODULE_STATE:
        TRACE("AFHDS3 [MODULE_STATE] %02X", responseFrame->value);
        setState(responseFrame->value);
        break;
      case COMMAND::MODULE_MODE:
        TRACE("AFHDS3 [MODULE_MODE] %02X", responseFrame->value);
        if (responseFrame->value != CMD_RESULT::SUCCESS) {
          setState(ModuleState::STATE_NOT_READY);
        }
        else {
          if (requestedModuleMode == MODULE_MODE_E::RUN) {
            enqueue(COMMAND::MODULE_GET_CONFIG, FRAME_TYPE::REQUEST_GET_DATA);
            enqueue(COMMAND::MODULE_STATE, FRAME_TYPE::REQUEST_GET_DATA);
          }
          requestedModuleMode = MODULE_MODE_UNKNOWN;
        }
        break;
      case COMMAND::MODULE_SET_CONFIG:
        if (responseFrame->value != CMD_RESULT::SUCCESS) {
          setState(ModuleState::STATE_NOT_READY);
        }
        TRACE("AFHDS3 [MODULE_SET_CONFIG], %02X", responseFrame->value);
        break;
      case COMMAND::TELEMETRY_DATA:
        {
        uint8_t* telemetry = &responseFrame->value;

        if (telemetry[0] == 0x22) {
          telemetry++;
          while (telemetry < rxBuffer + rxBufferCount) {
            uint8_t length = telemetry[0];
            uint8_t id = telemetry[1];
            if (id == 0xFE) {
              id = 0xF7;  //use new id because format is different
            }
            if (length == 0 || telemetry + length > rxBuffer + rxBufferCount) {
              break;
            }
            if (length == 4) {
              //one byte value fill missing byte
              uint8_t data[] = { id, telemetry[2], telemetry[3], 0 };
              ::processFlySkySensor(data, 0xAA);
            }
            if (length == 5) {
              if (id == 0xFA) {
                telemetry[1] = 0xF8; //remap to afhds3 snr
              }
              ::processFlySkySensor(telemetry + 1, 0xAA);
            }
            else if (length == 6 && id == FRM302_STATUS) {
              //convert to ibus
              uint16_t t = (uint16_t) (((int16_t) telemetry[3] * 10) + 400);
              uint8_t dataTemp[] = { ++id, telemetry[2], (uint8_t) (t & 0xFF), (uint8_t) (t >> 8) };
              ::processFlySkySensor(dataTemp, 0xAA);
              uint8_t dataVoltage[] = { ++id, telemetry[2], telemetry[4], telemetry[5] };
              ::processFlySkySensor(dataVoltage, 0xAA);
            }
            else if (length == 7) {
              ::processFlySkySensor(telemetry + 1, 0xAC);
            }
            telemetry += length;
          }
        }
      }
        break;
      case COMMAND::COMMAND_RESULT:
        {
        //AfhdsFrameData* respData = responseFrame->GetData();
        //TRACE("COMMAND RESULT %02X result %d datalen %d", respData->CommandResult.command, respData->CommandResult.result, respData->CommandResult.respLen);
        }
        break;
    }
  }

  if (responseFrame->frameType == FRAME_TYPE::REQUEST_GET_DATA || responseFrame->frameType == FRAME_TYPE::REQUEST_SET_EXPECT_DATA) {
    TRACE("Command %02X NOT IMPLEMENTED!", responseFrame->command);
  }
  else if (responseFrame->frameType == FRAME_TYPE::REQUEST_SET_EXPECT_ACK) {
    //check if such request is not queued
    if (!isEmpty()) {
      Frame f = commandFifo[getIndex];
      if (f.frameType == FRAME_TYPE::RESPONSE_ACK && f.frameNumber == responseFrame->frameNumber) {
        TRACE("ACK for frame %02X already queued", responseFrame->frameNumber);
        return;
      }
    }
    TRACE("AFHDS3 [QUEUE ACK] cmd %02X type %02X number %02X", responseFrame->command, responseFrame->frameType, responseFrame->frameNumber);
    enqueueACK((enum COMMAND) responseFrame->command, responseFrame->frameNumber);
  }
  else if (responseFrame->frameType == FRAME_TYPE::RESPONSE_DATA || responseFrame->frameType == FRAME_TYPE::RESPONSE_ACK) {
    if (operationState == State::AWAITING_RESPONSE) {
      operationState = State::IDLE;
    }
  }
}

inline bool isSbus(uint8_t mode)
{
  return (mode & 1);
}

inline bool isPWM(uint8_t mode)
{
  return !(mode & 2);
}

RUN_POWER PulsesData::getMaxRunPower() const
{
  if (powerSource == MODULE_POWER_SOURCE::EXTERNAL) {
    return RUN_POWER::PLUS_33dBm;
  }

  return RUN_POWER::PLUS_27dbm;
}

RUN_POWER PulsesData::actualRunPower() const
{
  uint8_t actualRfPower = cfg.config.runPower;
  if (getMaxRunPower() < actualRfPower) {
    actualRfPower = getMaxRunPower();
  }
  return (RUN_POWER) actualRfPower;
}

RUN_POWER PulsesData::getRunPower() const
{
  RUN_POWER targetPower = (RUN_POWER) moduleData->afhds3.runPower;
  if (getMaxRunPower() < targetPower) {
    targetPower = getMaxRunPower();
  }
  return targetPower;
}

bool PulsesData::syncSettings()
{
  RUN_POWER targetPower = getRunPower();

  /*not sure if we need to prevent them in bind mode*/
  if (getModuleMode(module_index) != ::ModuleSettingsMode::MODULE_MODE_BIND && targetPower != cfg.config.runPower) {
    cfg.config.runPower = moduleData->afhds3.runPower;
    uint8_t data[] = { 0x13, 0x20, 0x02, moduleData->afhds3.runPower, 0 };
    TRACE("AFHDS3 SET TX POWER %d", moduleData->afhds3.runPower);
    putFrame(COMMAND::SEND_COMMAND, FRAME_TYPE::REQUEST_SET_EXPECT_DATA, data, sizeof(data));
    return true;
  }

  //other settings only in 2 way mode (state must be synchronized)
  if (this->state != ModuleState::STATE_SYNC_DONE) {
    return false;
  }

  if (moduleData->afhds3.rxFreq() != cfg.config.pwmFreq) {
    cfg.config.pwmFreq = moduleData->afhds3.rxFreq();
    uint8_t data[] = { 0x17, 0x70, 0x02, (uint8_t) (moduleData->afhds3.rxFreq() & 0xFF), (uint8_t) (moduleData->afhds3.rxFreq() >> 8) };
    TRACE("AFHDS3 SET RX FREQ");
    putFrame(COMMAND::SEND_COMMAND, FRAME_TYPE::REQUEST_SET_EXPECT_DATA, data, sizeof(data));
    return true;
  }
  PULSE_MODE modelPulseMode = isPWM(moduleData->afhds3.mode)
                                  ? PULSE_MODE::PWM_MODE
                                  : PULSE_MODE::PPM_MODE;
  if (modelPulseMode != cfg.config.pulseMode) {
    cfg.config.pulseMode = modelPulseMode;
    TRACE("AFHDS3 PWM/PPM %d", modelPulseMode);
    uint8_t data[] = { 0x16, 0x70, 0x01, (uint8_t) (modelPulseMode) };
    putFrame(COMMAND::SEND_COMMAND, FRAME_TYPE::REQUEST_SET_EXPECT_DATA, data, sizeof(data));
    return true;
  }

  SERIAL_MODE modelSerialMode = isSbus(moduleData->afhds3.mode)
                                    ? SERIAL_MODE::SBUS_MODE
                                    : SERIAL_MODE::IBUS;
  if (modelSerialMode != cfg.config.serialMode) {
    cfg.config.serialMode = modelSerialMode;
    TRACE("AFHDS3 IBUS/SBUS %d", modelSerialMode);
    uint8_t data[] = { 0x18, 0x70, 0x01, (uint8_t) (modelSerialMode) };
    putFrame(COMMAND::SEND_COMMAND, FRAME_TYPE::REQUEST_SET_EXPECT_DATA, data, sizeof(data));
    return true;
  }

  if (moduleData->afhds3.failsafeTimeout != cfg.config.failSafeTimout) {
    moduleData->afhds3.failsafeTimeout = cfg.config.failSafeTimout;
    uint8_t data[] = { 0x12, 0x60, 0x02, (uint8_t) (moduleData->afhds3.failsafeTimeout & 0xFF), (uint8_t) (moduleData->afhds3.failsafeTimeout >> 8) };
    putFrame(COMMAND::SEND_COMMAND, FRAME_TYPE::REQUEST_SET_EXPECT_DATA, data, sizeof(data));
    TRACE("AFHDS3 FAILSAFE TMEOUT, %d", moduleData->afhds3.failsafeTimeout);
    return true;
  }
  return false;
}

void PulsesData::sendChannelsData()
{
  uint8_t channels_start = moduleData->channelsStart;
  uint8_t channelsCount = 8 + moduleData->channelsCount;
  uint8_t channels_last = channels_start + channelsCount;
  int16_t buffer[MAX_CHANNELS + 1] = { ((MAX_CHANNELS << 8) | CHANNELS_DATA_MODE::CHANNELS), 0 };

  for (uint8_t channel = channels_start, index = 1; channel < channels_last; channel++, index++) {
    int16_t channelValue = convert(::getChannelValue(channel));
    buffer[index] = channelValue;
  }
  putFrame(COMMAND::CHANNELS_FAILSAFE_DATA, FRAME_TYPE::REQUEST_SET_NO_RESP, (uint8_t*) buffer, sizeof(buffer));
}

void PulsesData::stop()
{
  TRACE("AFHDS3 STOP");
  requestedModuleMode = MODULE_MODE_E::STANDBY;
  putFrame(COMMAND::MODULE_MODE, FRAME_TYPE::REQUEST_SET_EXPECT_DATA, &requestedModuleMode, 1);
}

void PulsesData::setConfigFromModel()
{
  cfg.config.bindPower = moduleData->afhds3.bindPower;
  cfg.config.runPower = getRunPower();
  cfg.config.emiStandard = EMI_STANDARD::FCC;
  cfg.config.telemetry = moduleData->afhds3.telemetry; //always use bidirectional mode
  cfg.config.pwmFreq = moduleData->afhds3.rxFreq();
  cfg.config.serialMode = isSbus(moduleData->afhds3.mode)
                              ? SERIAL_MODE::SBUS_MODE
                              : SERIAL_MODE::IBUS;
  cfg.config.pulseMode = isPWM(moduleData->afhds3.mode) ? PULSE_MODE::PWM_MODE
                                                        : PULSE_MODE::PPM_MODE;
  // use max channels - because channel count can not be changed after bind
  cfg.config.channelCount = MAX_CHANNELS;
  cfg.config.failSafeTimout = moduleData->afhds3.failsafeTimeout;
  setFailSafe(cfg.config.failSafeMode);
}

inline int16_t PulsesData::convert(int channelValue)
{
  //pulseValue = limit<uint16_t>(0, 988 + ((channelValue + 1024) / 2), 0xfff);
  //988 - 750 = 238
  //238 * 20 = 4760
  //2250 - 2012 = 238
  //238 * 20 = 4760
  // 988   ---- 2012
  //-10240 ---- 10240
  //-1024  ---- 1024
  return ::limit<int16_t>(FAILSAFE_MIN, channelValue * 10, FAILSAFE_MAX);
}

uint8_t PulsesData::setFailSafe(int16_t* target)
{
  int16_t pulseValue = 0;
  uint8_t channels_start = moduleData->channelsStart;
  uint8_t channels_last = channels_start + 8 + moduleData->channelsCount;

  for (uint8_t channel = channels_start; channel < channels_last; channel++) {
    if (moduleData->failsafeMode == FAILSAFE_CUSTOM) {
      pulseValue = convert(g_model.failsafeChannels[channel]);
    }
    else if (moduleData->failsafeMode == FAILSAFE_HOLD) {
      pulseValue = FAILSAFE_KEEP_LAST;
    }
    else {
      pulseValue = convert(::getChannelValue(channel));
    }
    target[channel - channels_start] = pulseValue;
  }
  //return max channels because channel count can not be change after bind
  return (uint8_t) (MAX_CHANNELS);
}

}
