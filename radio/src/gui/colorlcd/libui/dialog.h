/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   libopenui - https://github.com/opentx/libopenui
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

#include "form.h"
#include "modal_window.h"

class StaticText;
class DynamicText;
class Progress;

#define DIALOG_DEFAULT_WIDTH ((coord_t)(LCD_W * 0.8))
#define DIALOG_DEFAULT_HEIGHT ((coord_t)(LCD_H * 0.8))

//-----------------------------------------------------------------------------

class BaseDialog : public ModalWindow
{
 public:
  BaseDialog(const char* title, bool closeIfClickedOutside,
             lv_coord_t width = DIALOG_DEFAULT_WIDTH,
             lv_coord_t maxHeight = DIALOG_DEFAULT_HEIGHT,
             bool flexLayout = true);

  void setTitle(const char* title);

 protected:
  Window* form = nullptr;
  StaticText* header = nullptr;

  void onCancel() override { deleteLater(); }
  void onEvent(event_t event) override {}
};

//-----------------------------------------------------------------------------

class ProgressDialog : public BaseDialog
{
 public:
  ProgressDialog(const char* title, std::function<void()> onClose);

  void updateProgress(int percentage);
  void setTitle(std::string title);
  void closeDialog();

 protected:
  uint32_t lastUpdate = 0;
  Progress* progress;

  std::function<void()> onClose;

  // disable keys
  void onEvent(event_t) override {}
};

//-----------------------------------------------------------------------------

class MessageDialog : public BaseDialog
{
 public:
  MessageDialog(const char* title, const char* message,
                const char* info = nullptr, LcdFlags messageFlags = CENTERED,
                LcdFlags infoFlags = CENTERED);

 protected:
  StaticText* messageWidget;
  StaticText* infoWidget;

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "MessageDialog"; }
#endif

  void onClicked() override;
};

//-----------------------------------------------------------------------------

class DynamicMessageDialog : public BaseDialog
{
 public:
  DynamicMessageDialog(const char* title,
                       std::function<std::string()> textHandler,
                       const char* message = "",
                       const int lineHeight = EdgeTxStyles::PAGE_LINE_HEIGHT,
                       LcdColorIndex color = COLOR_THEME_PRIMARY1_INDEX, LcdFlags textFlags = CENTERED);
  // Attn.: FONT(XXL) is not supported by DynamicMessageDialog

 protected:
  StaticText* messageWidget;
  DynamicText* infoWidget;

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "DynamicMessageDialog"; }
#endif

  void onClicked() override;
};

//-----------------------------------------------------------------------------

class ConfirmDialog : public BaseDialog
{
 public:
  ConfirmDialog(const char* title, const char* message,
                std::function<void(void)> confirmHandler,
                std::function<void(void)> cancelHandler = nullptr);

 protected:
  std::function<void(void)> confirmHandler;
  std::function<void(void)> cancelHandler;

  void onCancel() override;
};

//-----------------------------------------------------------------------------

class LabelDialog : public ModalWindow
{
 public:
  LabelDialog(const char *label, int length, const char* title,
              std::function<void(std::string)> _saveHandler = nullptr);

  static constexpr int MAX_LABEL_LENGTH = 255;

  void onCancel() override { deleteLater(); }

 protected:
  std::function<void(std::string)> saveHandler;
  char label[MAX_LABEL_LENGTH + 1];
};
