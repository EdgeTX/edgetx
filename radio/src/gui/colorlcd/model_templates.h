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

#include <functional>
#include "opentx.h"

constexpr size_t LEN_INFO_TEXT = 300;
constexpr size_t LEN_PATH = sizeof(TEMPLATES_PATH) + TEXT_FILENAME_MAXLEN;
constexpr size_t LEN_BUFFER = sizeof(TEMPLATES_PATH) + 2 * TEXT_FILENAME_MAXLEN + 1;

class TemplatePage : public Page
{
  public:

  TemplatePage() : Page(ICON_MODEL_SELECT)
  { }

  void updateInfo();

#if defined(HARDWARE_KEYS)
  void onEvent(event_t event) override;
#endif

#if defined(DEBUG_WINDOWS)
  std::string getName() const { return "TemplatePage"; }
#endif

  protected:

  static char path[LEN_PATH + 1];
  char buffer[LEN_BUFFER + 1];
  char infoText[LEN_INFO_TEXT + 1] = { 0 };
  unsigned int count = 0;
  static std::function<void(void)> update;

  void paint(BitmapBuffer *dc) override;
};

class TemplateButton : public TextButton
{
  public:
  TemplateButton(FormGroup* parent, const rect_t& rect, std::string name, std::function<uint8_t(void)> pressHandler = nullptr)
  : TextButton(parent, rect, name, pressHandler)
  { }
};

class SelectTemplate : public TemplatePage
{
  public:
  SelectTemplate(TemplatePage* tp);

  protected:
  TemplatePage* templateFolderPage;
};

class SelectTemplateFolder : public TemplatePage
{
  public:
  SelectTemplateFolder(std::function<void(void)> update);
  ~SelectTemplateFolder();
};