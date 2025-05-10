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

#include "multi_protolist.h"
#include "multi_rfprotos.h"
#include "strhelpers.h"
#include "audio.h"
#include "translations.h"
#include "pulses/modules_helpers.h"
#include "pulses/pulses.h"
#include "gui_common.h"

#include "edgetx.h" // reusableBuffer
#include "os/time.h"

#include <algorithm>

static timer_handle_t _protoScanTimers[NUM_MODULES] = {TIMER_INITIALIZER};

void MultiRfProtocols::timerCb(timer_handle_t* h)
{
  uint8_t moduleIdx = (uint8_t)(h - _protoScanTimers);
  auto instance = MultiRfProtocols::instance(moduleIdx);
  if (instance->scanState == ScanBegin || instance->scanState == Scanning) {
    instance->fillBuiltinProtos();
  }
}

MultiRfProtocols* MultiRfProtocols::_instance[MAX_MODULES] = {};

// MPM telemetry packet format type = MultiProtoDef (0x11)
//
// data[0]     = protocol number, 0xFF is an invalid list entry
//               (Option value too large) and nothing sent after
// data[1..n]  = protocol name null terminated
// data[n+1]   = flags
//                flags>>4 Option text number to be displayed
//                         (check multi status for description)
//                flags&0x01 failsafe supported
//                flags&0x02 Channel Map Disabled supported
// data[n+2]   = number of sub protocols
// data[n+3]   = sub protocols text length, only sent if nbr_sub != 0
// data[n+4..] = sub protocol names, only sent if nbr_sub != 0

constexpr uint8_t MULTI_INVALID_PROTO = 0xFF;
constexpr uint32_t MULTI_PROTOLIST_TIMEOUT = 100; // 100ms
constexpr uint32_t MULTI_PROTOLIST_START_TIMEOUT = 3000; // 3s

MultiRfProtocols::MultiRfProtocols(unsigned int moduleIdx) : moduleIdx(moduleIdx)
{
#if defined(SIMU)
  // prevent scanning from starting on simu
  fillBuiltinProtos();
#endif
}

bool MultiRfProtocols::RfProto::parse(const uint8_t* data, uint8_t len)
{
  const char* s = (const char*)data;

  uint8_t subProtoNr = 0;
  uint8_t subProtoLen = 0;

  // proto label string
  while (*s && len--) s++;
  if (*s || !len) return false;
  label = (const char*)data;
  s++;
  len--;

  // flags, subProtoNr, subProto label max length
  if (len < 2) return false;
  flags = (uint8_t) * (s++);
  //TRACE("flags: 0x%02X", flags);

  subProtoNr = (uint8_t) * (s++);
  len -= 2;

  if (!len) return true;
  subProtoLen = (uint8_t) * (s++);
  len--;

  if (len < subProtoNr * subProtoLen) return false;

  fillSubProtoList(s, subProtoNr, subProtoLen);
  return true;
}

void MultiRfProtocols::RfProto::fillSubProtoList(const char* str, int n, int len)
{
  char tmp[len + 1];
  subProtos.reserve(n);

  for (int i = 0; i < n; i++, str += len) {
    strncpy(tmp, str, len);
    tmp[len] = '\0';
    subProtos.emplace_back((const char*)tmp);
    //TRACE("%s: '%s'", label.c_str(), subProtos.back().c_str());
  }
}

void MultiRfProtocols::RfProto::fillSubProtoList(const char *const *str, int n)
{
  subProtos.reserve(n);

  for (int i = 0; i < n; i++) {
    subProtos.emplace_back(str[i]);
  }
}

MultiRfProtocols* MultiRfProtocols::instance(unsigned int moduleIdx)
{
  if (moduleIdx >= NUM_MODULES) return nullptr;
  if (!_instance[moduleIdx]) {
    _instance[moduleIdx] = new MultiRfProtocols(moduleIdx);
  }
  return _instance[moduleIdx];
}

void MultiRfProtocols::removeInstance(unsigned int moduleIdx)
{
  delete _instance[moduleIdx];
  _instance[moduleIdx] = nullptr;
}

int MultiRfProtocols::getIndex(unsigned int proto) const
{
  auto it = proto2idx.find(proto);
  return it != proto2idx.end() ? it->second : -1;
}

const MultiRfProtocols::RfProto* MultiRfProtocols::getProto(unsigned int proto) const
{
  int idx = getIndex(proto);
  if (idx >= 0 && (unsigned)idx < protoList.size())
    return &protoList[idx];

  return nullptr;
}

std::string MultiRfProtocols::getProtoLabel(unsigned int proto) const
{
  if (scanState != ScanEnd) {
    MultiModuleStatus& status = getMultiModuleStatus(moduleIdx);
    if (status.protocolName[0] && status.isValid()) {
      return std::string(status.protocolName);
    } else if (proto <= MODULE_SUBTYPE_MULTI_LAST) {
      char tmp[8];
      getStringAtIndex(tmp, STR_MULTI_PROTOCOLS, proto);
      return std::string(tmp);
    }
  } else {
    int idx = getIndex(proto);
    if (idx >= 0 && (unsigned)idx < protoList.size()) {
      return protoList[idx].label;
    }
  }
  return std::to_string(proto);
}

std::string MultiRfProtocols::getLastProtoLabel() const
{
  if (!protoList.empty())
    return protoList.back().label;

  return std::string();
}

float MultiRfProtocols::getProgress() const
{
  constexpr float WAIT_TIME_RATIO = 0.7; // wait time accounts for 70%
  constexpr float PROTOS_RATIO = 1.0 - WAIT_TIME_RATIO;
  
  if (scanState == ScanStop)  return 0.0;
  if (scanState == ScanBegin) {
    float t = (float)(time_get_ms() - scanStart) / (float)MULTI_PROTOLIST_START_TIMEOUT;
    return t * WAIT_TIME_RATIO;
  }

  return WAIT_TIME_RATIO + ((float)getNProtos() / (float)totalProtos) * PROTOS_RATIO;
}

void MultiRfProtocols::fillList(std::function<void(const RfProto&)> addProto) const
{
  for (const auto& p : protoList) {
    addProto(p);
  }
}

bool MultiRfProtocols::triggerScan()
{
  if (scanState == ScanStop
      && moduleState[moduleIdx].mode == MODULE_MODE_NORMAL) {

    proto2idx.clear();
    protoList.clear();
    scanState = ScanBegin;
    currentProto = MULTI_INVALID_PROTO;
    moduleState[moduleIdx].mode = MODULE_MODE_GET_HARDWARE_INFO;
    scanStart = lastScan = time_get_ms();

    auto scanTimer = &_protoScanTimers[moduleIdx];
    if (!timer_is_created(scanTimer)) {
      timer_create(scanTimer, MultiRfProtocols::timerCb, "mpm",
                   MULTI_PROTOLIST_START_TIMEOUT, false);
      timer_start(scanTimer);
    } else {
      timer_set_period(scanTimer, MULTI_PROTOLIST_START_TIMEOUT);
    }

    return true;
  }

  return false;
}

bool MultiRfProtocols::scanReply(const uint8_t* packet, uint8_t len)
{
  switch (scanState) {
    case ScanBegin:
    case Scanning:
      if (packet && len) {
        uint8_t replyProtoId = *packet;
        len--;
        packet++;

        // new status received
        if (replyProtoId != MULTI_INVALID_PROTO) {
          if (currentProto == MULTI_INVALID_PROTO) {
            //TRACE("# of protos: %d", totalProtos);
            totalProtos = replyProtoId;
            scanState = Scanning;
            protoList.reserve(totalProtos);
          } else {
            //TRACE("Proto = %d; label = '%s'", replyProtoId,
            //      (const char*)packet);

            int proto = replyProtoId - 1;
            if (isMultiProtocolSelectable(proto)) {
              RfProto rfProto(proto);
              if (rfProto.parse(packet, len)) {
                proto2idx[proto] = protoList.size();
                protoList.emplace_back(rfProto);
              } else {
                TRACE("Error parsing proto [%d]", proto);
              }
            } else {
              // do not count excluded protocols
              totalProtos--;
            }
          }

          currentProto++;
          lastScan = time_get_ms();

          auto scanTimer = &_protoScanTimers[moduleIdx];
          timer_set_period(scanTimer, MULTI_PROTOLIST_TIMEOUT);

          return true;

        } else {
          scanState = ScanEnd;
          setModuleMode(moduleIdx, MODULE_MODE_NORMAL);
          auto scanTimer = &_protoScanTimers[moduleIdx];
          timer_stop(scanTimer);
          break;
        }
      } else {
        uint32_t timeout = MULTI_PROTOLIST_TIMEOUT;

        if (scanState == ScanBegin) {
          // Timeout = 3s solely because of the very slow start time of the
          // MPM...
          timeout = MULTI_PROTOLIST_START_TIMEOUT;
        }

        if (time_get_ms() - lastScan >= timeout) {
          TRACE("proto scan timeout!");
          scanState = ScanInvalid;
        }
      }
      break;

    case ScanInvalid: {
      fillBuiltinProtos();
    } break;

    default:
      break;
  }

  return false;
}

void MultiRfProtocols::fillBuiltinProtos()
{
  const mm_protocol_definition* pdef =
      getMultiProtocolDefinition(MODULE_SUBTYPE_MULTI_FIRST);

  // build the list of static protos
  protoList.clear();
  protoList.reserve(MODULE_SUBTYPE_MULTI_LAST - MODULE_SUBTYPE_MULTI_FIRST + 1);
  for (; pdef->protocol != MODULE_SUBTYPE_MULTI_SENTINEL; pdef++) {
    RfProto rfProto(pdef->protocol);

    if (pdef->protocol == MM_RF_CUSTOM_SELECTED) break;  // skip custom proto

    char tmp[8];
    rfProto.label = getStringAtIndex(tmp, STR_MULTI_PROTOCOLS, pdef->protocol);
    rfProto.flags =
        (pdef->failsafe ? 0x01 : 0) | (pdef->disable_ch_mapping ? 0x02 : 0);

    if (pdef->optionsstr != nullptr) {
      for (uint8_t i = 0; i < getMaxMultiOptions(); i++) {
        if (pdef->optionsstr == mm_options_strings::options[i]) {
          rfProto.flags |= i << 4;
          break;
        }
      }
    }

    if (pdef->subTypeString) {
      rfProto.fillSubProtoList(pdef->subTypeString, pdef->maxSubtype + 1);
    }

    protoList.emplace_back(rfProto);
  }

  // sort it by label
  std::sort(
      protoList.begin(), protoList.end(),
      [](const RfProto& a, const RfProto& b) { return a.label < b.label; });

  // index the sorted list
  proto2idx.clear();
  for (unsigned int i = 0; i < protoList.size(); i++)
    proto2idx[protoList[i].proto] = i;

  scanState = ScanEnd;
  setModuleMode(moduleIdx, MODULE_MODE_NORMAL);
}
