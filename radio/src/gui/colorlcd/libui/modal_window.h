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

#include "window.h"

class ModalWindow : public Window
{
 public:
  explicit ModalWindow(bool closeWhenClickOutside = false);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "ModalWindow"; }
#endif

  void setCloseWhenClickOutside(bool value = true)
  {
    closeWhenClickOutside = value;
  }

  void onClicked() override;
  void deleteLater(bool detach = true, bool trash = true) override;

 protected:
  bool closeWhenClickOutside;
};
