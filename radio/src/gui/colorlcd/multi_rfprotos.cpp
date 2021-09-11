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

#include "io/multi_protolist.h"
#include "multi_rfprotos.h"
#include "strhelpers.h"

#include "progress.h"

// TODO: format for LCD_WIDTH > LCD_HEIGHT
constexpr rect_t RFSCAN_DIALOG_RECT = {
  50, 100, LCD_W - 100, LCD_H - 200
};

// TODO: translation
const char* RFSCAN_TITLE = "MPM: Scanning protocols...";

class RfScanDialog : public Dialog
{
  MultiRfProtocols* protos;
  uint32_t lastUpdate = 0;

  Progress* progress;
  std::function<void()> onClose;

 public:
  RfScanDialog(Window* parent, MultiRfProtocols* protos,
               std::function<void()> onClose) :
      Dialog(parent, RFSCAN_TITLE, RFSCAN_DIALOG_RECT),
      protos(protos),
      progress(new Progress(&content->form,
                            {PAGE_PADDING, PAGE_PADDING,
                             content->form.width() - 2 * PAGE_PADDING,
                             content->form.height() - 2 * PAGE_PADDING})),
      onClose(std::move(onClose))
  {
    // disable canceling dialog
    setCloseWhenClickOutside(false);
    setFocus();
  }

  void showProgress()
  {
    progress->setValue((int)(protos->getProgress() * 100.0));
  }

  // disable keys
  void onEvent(event_t) override { return; }
  
  void checkEvents() override
  {
    if (!protos->isScanning()) {
      deleteLater();
      onClose();
    } else if (RTOS_GET_MS() - lastUpdate >= 200) {
      showProgress();
      lastUpdate = RTOS_GET_MS();
    }
    
    Dialog::checkEvents();
  }
};

MultiProtoChoice::MultiProtoChoice(FormGroup* parent, const rect_t& rect,
                                   unsigned int moduleIdx,
                                   std::function<void(int)> setValue,
                                   std::function<void()> updateForm) :
    Choice(
        parent, rect, 0, 0,
        [=] { return g_model.moduleData[moduleIdx].getMultiProtocol(); },
        setValue),
    moduleIdx(moduleIdx)
{
  TRACE("MultiProtoChoice::MultiProtoChoice(%p)", this);
  protos = MultiRfProtocols::instance(moduleIdx);
  protos->triggerScan();

  if (protos->isScanning()) {
    new RfScanDialog(parent, protos, std::move(updateForm));
  } else {
    TRACE("!protos->isScanning()");
  }

  setTextHandler([=](int value) {
    return protos->getProtoLabel(value);
  });
}

void MultiProtoChoice::openMenu()
{
  menu = new Menu(this);

  if (!menuTitle.empty()) menu->setTitle(menuTitle);
  menu->setCloseHandler([=]() { setEditMode(false); });

  setEditMode(true);
  invalidate();

  protos->fillList([=](const MultiRfProtocols::RfProto& p){
      addProto(p.proto, p.label.c_str());
    });

  int idx = protos->getIndex(g_model.moduleData[moduleIdx].getMultiProtocol());
  if (idx >= 0) menu->select(idx);
}

void MultiProtoChoice::addProto(unsigned int proto, const char* protocolName)
{
  addValue(protocolName);
  menu->addLine(protocolName, [=]() { setValue(proto); });
}

