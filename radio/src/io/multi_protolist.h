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

#include "dataconstants.h"
#include "opentx_types.h"

#include <functional>
#include <string>
#include <vector>
#include <map>

#if !defined(SIMU)
// Forward declare FreeRTOS timer
struct tmrTimerControl;
typedef struct tmrTimerControl * TimerHandle_t;
#endif

class MultiRfProtocols
{
  static MultiRfProtocols* _instance[NUM_MODULES];

  unsigned int moduleIdx;

  enum ScanState { ScanStop, ScanBegin, Scanning, ScanInvalid, ScanEnd };

  ScanState scanState = ScanStop;

  uint32_t scanStart = 0;
  uint32_t lastScan = 0;
  uint8_t currentProto = 0;
  uint8_t totalProtos = 0;

  MultiRfProtocols(unsigned int moduleIdx);
  void fillBuiltinProtos();

#if !defined(SIMU)
  static void timerCb(TimerHandle_t xTimer);
#endif
  
 public:

  struct RfProto {
    int proto;
    std::string label;

    uint8_t flags = 0;
    std::vector<std::string> subProtos;

    RfProto(int proto) : proto(proto) {}

    bool parse(const uint8_t* data, uint8_t len);

    // array of strings
    void fillSubProtoList(const char** str, int n);

    // fixed length strings concatenated
    void fillSubProtoList(const char* str, int n, int len);

    uint8_t getOption() const;
    const char* getOptionStr() const;
    
    bool supportsFailsafe() const { return flags & 0x01; }
    bool supportsDisableMapping() const { return flags & 0x02; }
  };

  static MultiRfProtocols* instance(unsigned int moduleIdx);
  static void removeInstance(unsigned int moduleIdx);

  int getIndex(unsigned int proto) const;
  const RfProto* getProto(unsigned int proto) const;
  std::string getProtoLabel(unsigned int proto) const;

  bool isScanning() const { return scanState != ScanStop && scanState != ScanEnd; }
  int  getNProtos() const { return protoList.size(); }
  int  getTotalProtos() const { return totalProtos; }
  float getProgress() const;

  std::string getLastProtoLabel() const;
  bool triggerScan();
  uint8_t getScanProto() { return currentProto; }

  bool scanReply(const uint8_t * packet = nullptr, uint8_t len = 0);
  void fillList(std::function<void(const RfProto&)> addProto) const;

 private:
  std::vector<RfProto> protoList;
  std::map<int,int>    proto2idx;
};
