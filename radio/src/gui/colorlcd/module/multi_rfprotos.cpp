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

// TODO: format for LCD_WIDTH > LCD_HEIGHT

// TODO: translation
const char* const RFSCAN_TITLE = "MPM: Scanning protocols...";

RfScanDialog::RfScanDialog(MultiRfProtocols* protos,
                           std::function<void()> onClose) :
  ProgressDialog(RFSCAN_TITLE, onClose),
  protos(protos)
{
}

void RfScanDialog::showProgress()
{
  updateProgress((int)(protos->getProgress() * 100.0));
}
  
void RfScanDialog::checkEvents()
{
  if (!protos->isScanning()) {
    closeDialog();
  } else if (RTOS_GET_MS() - lastUpdate >= 200) {
    showProgress();
    lastUpdate = RTOS_GET_MS();
  }
  
  ProgressDialog::checkEvents();
}

