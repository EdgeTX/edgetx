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

#include "multi_rfprotos.h"
#include "strhelpers.h"
#include "audio.h"
#include "translations.h"
#include "pulses/modules_helpers.h"
#include "gui_common.h"
#include <algorithm>

class MultiRfProtocols
{
  static constexpr uint8_t ScanStartProto = MODULE_SUBTYPE_MULTI_CONFIG;
  static MultiRfProtocols* _instance[NUM_MODULES];
  
  struct RfProto {
    int         proto;
    std::string label;

    RfProto(int proto, const char* label)
      : proto(proto), label(label)
    {}
  };
  std::list<RfProto> protoList;

  unsigned int moduleIdx;

  enum ScanState {
    ScanStop,
    ScanBegin,
    Scanning,
    ScanInvalid,
    ScanEnd
  };

  ScanState scanState = ScanStop;
  uint8_t   scanProto = ScanStartProto;
  tmr10ms_t lastScan  = 0;

  MultiRfProtocols(unsigned int moduleIdx)
    : moduleIdx(moduleIdx)
  {
  }
  
  void scanNext()
  {
    TRACE("scan with proto=%d", scanProto);
    g_model.moduleData[moduleIdx].setMultiProtocol(scanProto);

    // record start time (timeout)
    lastScan = get_tmr10ms();
    scanState = Scanning;

    // invalidate update status (force timeout)
    MultiModuleStatus& status = getMultiModuleStatus(moduleIdx);
    status.lastUpdate = get_tmr10ms() - 500;
  }

public:
  static MultiRfProtocols* instance(unsigned int moduleIdx)
  {
    if (moduleIdx >= NUM_MODULES) return nullptr;
    if (!_instance[moduleIdx]) {
      _instance[moduleIdx] = new MultiRfProtocols(moduleIdx);
    }
    return _instance[moduleIdx];
  }

  static void removeInstances()
  {
    for (int i=0; i<NUM_MODULES; i++) {
      delete _instance[i];
      _instance[i] = nullptr;
    }
  }

  std::string getProtoLabel(unsigned int proto)
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
      auto it = std::find_if(
          protoList.begin(), protoList.end(),
          [=](const RfProto& p) { return p.proto == (const int)proto; });
      if (it != protoList.end()) {
        return it->label;
      }
    }
    return std::string();
  }

  bool isScanning() {
    return scanState != ScanStop && scanState != ScanEnd;
  }

  int getNProtos() {
    return protoList.size();
  }

  std::string getLastProtoLabel() const
  {
    auto it = protoList.rbegin();
    if (it != protoList.rend()) {
      return it->label;
    }
    return std::string();
  }
  
  void triggerScan()
  {
    if (scanState == ScanStop)
      scanState = ScanBegin;
  }

  void stateMachine(MultiProtoChoice* choice)
  {
    //TRACE("%p: <%d|%d|%d>", this, scanState, scanProto, get_tmr10ms() - lastScan);

    switch(scanState) {

    case ScanBegin:
      scanNext();
      break;

    case Scanning: {
      MultiModuleStatus& status = getMultiModuleStatus(moduleIdx);
      if (status.isValid()) {

        // TRACE("scanProto = %d; prev=%d; next=%d",
        //       scanProto,
        //       status.protocolPrev,
        //       status.protocolNext);

        // new status received
        if ((scanProto == ScanStartProto) || // special proto to detect start of list
            (status.protocolPrev == status.protocolNext)) {

          TRACE("detected start of proto list!");
          scanProto = convertMultiToOtx(status.protocolNext);
          scanNext();
          break;
        }

        if (status.protocolValid()) {
          
          TRACE("proto(%d): '%s'/%d/%d",
                scanProto,
                status.protocolName,
                convertMultiToOtx(status.protocolPrev),
                convertMultiToOtx(status.protocolNext));

          protoList.emplace_back(scanProto, status.protocolName);
          choice->addProto(scanProto, status.protocolName);

          if (status.protocolNext == scanProto) {
            TRACE("proto scan ended sucessfully!");
            scanState = ScanEnd;
          } else {
            scanProto = convertMultiToOtx(status.protocolNext);
            scanNext();
          }
        } else {
          // search led to an invalid proto: feature probably not supported
          TRACE("search led to an invalid proto!");
          scanState = ScanInvalid;
        }

      } else if (get_tmr10ms() - lastScan >= 50) {
        // Timeout = 500ms
        // MPM takes on average 110ms to reply
        TRACE("proto scan timeout!");
        scanState = ScanInvalid;
      }
    } break;

    case ScanInvalid:
      //TODO: fill with static list (sorted)
      scanState = ScanEnd;
      break;

    default:
      break;
    }
  }

  void fillList(MultiProtoChoice* choice)
  {
    for(const auto& p : protoList) {
      choice->addProto(p.proto, p.label.c_str());
    }
  }
};

MultiRfProtocols* MultiRfProtocols::_instance[NUM_MODULES] = {};

class RfScanDialog : public MessageDialog
{
  MultiRfProtocols* protos;

 public:
  RfScanDialog(Window* parent, MultiRfProtocols* protos) :
    MessageDialog(parent, "Scanning protocols", ""),
    protos(protos)
  {
    setFocus();
  }

  // void deleteLater(bool detach = true, bool trash = true) override
  // {
  //   if (_deleted) return;
  //   FullScreenDialog::deleteLater(detach, trash);
  // }

  void showProgress()
  {
    // TODO
    char msg[64] = {'\0'};
    char* s = msg;

    s = strAppend(s, "Protocols: ");
    s = strAppendSigned(s, protos->getNProtos());
    s = strAppend(s, " / Found: ");
    s = strAppend(s, protos->getLastProtoLabel().c_str());
    setInfoText(msg);
    if (MainWindow::instance()->refresh()) {
      lcdRefresh();
    }
  }
};

MultiProtoChoice::MultiProtoChoice(FormGroup* parent, const rect_t& rect,
                                   unsigned int moduleIdx,
                                   std::function<void(int)> setValue) :
    Choice(
        parent, rect, 0, 0,
        [=] { return g_model.moduleData[moduleIdx].getMultiProtocol(); },
        setValue),
    moduleIdx(moduleIdx)
{
  //protoBackup = getValue();
  protos = MultiRfProtocols::instance(moduleIdx);

  setTextHandler([=](int value) {
    TRACE("textHandler(%d)", value);
    return protos->getProtoLabel(value);
  });
  TRACE("MultiProtoChoice::MultiProtoChoice(%p)", this);
}

void MultiProtoChoice::openMenu()
{
  menu = new Menu(this);

  if (!menuTitle.empty()) menu->setTitle(menuTitle);
  menu->setCloseHandler([=]() { setEditMode(false); });

  setEditMode(true);
  invalidate();

  int proto = getValue();
  protos->triggerScan();
  if (protos->isScanning()) {
    RfScanDialog* rfScan = new RfScanDialog(this, protos);
    do {
      // MPM takes on average 110ms to reply
#if defined(SIMU)
      RTOS_WAIT_MS(200);
#else
      RTOS_WAIT_MS(20);
#endif
      protos->stateMachine(this);
      rfScan->showProgress();
    } while(protos->isScanning());

    rfScan->deleteLater();
    g_model.moduleData[moduleIdx].setMultiProtocol(proto);
    getMultiModuleStatus(moduleIdx).protocolName[0] = '\0';
  } else {
    //TODO: fill with existing protos
    protos->fillList(this);
  }

  // TODO: find current element
}

void MultiProtoChoice::addProto(unsigned int proto, const char* protocolName)
{
  // proto2idx[scanProto] = vmax;
  addValue(protocolName);
  menu->addLine(protocolName, [=]() { setValue(proto); });
}

void MultiProtoChoice::removeInstances()
{
  MultiRfProtocols::removeInstances();
}
