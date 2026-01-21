/*
 * Copyright (C) EdgeTX
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

#pragma once

#include "page.h"
#include "pagegroup.h"
#include "gui_common.h"

class TextViewer;

class ViewTextWindow : public Page
{
 public:
  ViewTextWindow(const std::string path, const std::string name,
                 EdgeTxIcon icon);

  FRESULT sdReadTextFileBlock(const uint32_t bufSize,
                              const uint32_t offset);

  void onCancel() override;

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "ViewTextWindow"; };
#endif

  void delayedInit() override;

 protected:
  TextViewer* textViewer = nullptr;

  void onEvent(event_t event) override;
};

class ModelNotesPage : public PageGroupItem
{
 public:
  ModelNotesPage(const PageDef& pageDef);

  void build(Window* window) override;

  void cleanup() override;

 protected:
  TextViewer* textViewer = nullptr;
};

void readModelNotes(bool fromMenu = false);
void readChecklist();
