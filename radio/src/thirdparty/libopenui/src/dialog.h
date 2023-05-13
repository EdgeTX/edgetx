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
#include "form.h"

class Dialog;

class DialogWindowContent : public ModalWindowContent
{
  friend class Dialog;

 public:
  DialogWindowContent(Dialog* parent, const rect_t& rect);

  void setTitle(const std::string& text) override;

  void deleteLater(bool detach = true, bool trash = true) override;
  void updateSize() override;

#if defined(DEBUG_WINDOWS)
  std::string getName() const override;
#endif

 public:
  FormGroup form;
};

class Dialog : public ModalWindow
{
 public:
  Dialog(Window* parent, std::string title, const rect_t& rect);

 protected:
  DialogWindowContent* content;

  void onCancel() override;
  void onEvent(event_t event) override;
};
