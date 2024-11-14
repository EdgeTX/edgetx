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

#include "modal_window.h"

class Menu;
class MenuWindowContent;
class MenuToolbar;

class Menu : public ModalWindow
{
  friend class MenuBody;

 public:
  explicit Menu(bool multiple = false);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "Menu"; }
#endif

  void setCancelHandler(std::function<void()> handler);
  void setWaitHandler(std::function<void()> handler);
  void setLongPressHandler(std::function<void()> handler);

  void setToolbar(MenuToolbar *window);

  void setTitle(std::string text);

  void addLine(const uint8_t *icon_mask, const std::string &text,
               std::function<void()> onPress,
               std::function<bool()> isChecked = nullptr);

  void addLine(const std::string &text, std::function<void()> onPress,
               std::function<bool()> isChecked = nullptr)
  {
    addLine(nullptr, text, onPress, isChecked);
  }

  void addLineBuffered(const uint8_t *icon_mask, const std::string &text,
                       std::function<void()> onPress,
                       std::function<bool()> isChecked = nullptr);

  void addLineBuffered(const std::string &text, std::function<void()> onPress,
                       std::function<bool()> isChecked = nullptr)
  {
    addLineBuffered(nullptr, text, onPress, isChecked);
  }

  void updateLines();

  void removeLines();

  unsigned count() const;

  int selection() const;

  void select(int index);

  void onEvent(event_t event) override;
  void onCancel() override;

  void checkEvents() override
  {
    ModalWindow::checkEvents();
    if (waitHandler) {
      waitHandler();
    }
  }

  void handleLongPress();

 protected:
  bool multiple;
  MenuWindowContent *content;
  MenuToolbar *toolbar = nullptr;
  std::function<void()> waitHandler;
  std::function<void()> cancelHandler;
  std::function<void()> longPressHandler;

  void updatePosition();
};
