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

#pragma once

#include "page.h"
#include "telemetry/crsf_device_config.h"

class CrsfCommandDialog;

// ExpressLRS bind page: bind phrase / UID (ELRS >= 4.1) and classic bind
class RadioCrsfBindPage : public Page
{
 public:
  RadioCrsfBindPage();

 protected:
  uint8_t method = 0;     // 0 = bind phrase, 1 = classic
  uint8_t applyMode = 0;  // 0 = TX only, 1 = RX then TX
  bool awaitingRefresh = false;
  bool loopWasActive = false;
  Messaging forceCloseMsg;
  class TextButton* bindBtn = nullptr;
  char phrase[CRSF_BIND_PHRASE_MAXLEN + 1] = "";
  std::string statusMsg;

  Window* phraseLine = nullptr;
  Window* applyLine = nullptr;
  Window* setLine = nullptr;
  Window* uidLine = nullptr;
  Window* classicTxLine = nullptr;
  Window* classicRxLine = nullptr;

  void updateVisibility();
  void checkEvents() override;
  void onCancel() override;
#if defined(HARDWARE_KEYS)
  void doKeyShortcut(event_t event) override;
#endif
};

class RadioCrsfConfigPage : public Page
{
 public:
  explicit RadioCrsfConfigPage(uint8_t moduleIdx);
  ~RadioCrsfConfigPage() override;

 protected:
  // buffers for editing device-side strings (TBS); enough for the few
  // editable strings a folder realistically holds
  static constexpr uint8_t MAX_STR_EDITS = 4;

  uint8_t moduleIdx;
  uint8_t folder = 0;
  uint8_t strEditCount = 0;
  std::vector<uint8_t> folderStack;
  uint16_t lastGen = 0xFFFF;
  std::string lastStatus;
  Messaging forceCloseMsg;
  CrsfCommandDialog* cmdDialog = nullptr;
  bool critDialogOpen = false;
  bool needsRebuild = false;
  char strEditBuf[MAX_STR_EDITS][CRSF_BIND_PHRASE_MAXLEN + 1];

  void rebuild();
  void updateHeader();
  void checkEvents() override;
  void onCancel() override;
#if defined(HARDWARE_KEYS)
  void doKeyShortcut(event_t event) override;
#endif
};
