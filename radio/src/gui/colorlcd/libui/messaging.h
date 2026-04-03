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

#include <functional>
#include <stdint.h>

class Messaging
{
 public:
  enum MessageId
  {
    COLOR_CHANGED = 1,
    COLOR_PREVIEW,
    MODULE_UPDATE,
    CURVE_UPDATE,
    CURVE_EDIT,
    DECORATION_UPDATE,
    QUICK_MENU_ITEM_SELECT,
    MENU_CHANGE_FILTER,
    REFRESH,
    REFRESH_OUTPUTS_WIDGET,
  };

  Messaging() {}
  ~Messaging();
  void subscribe(uint32_t id, std::function<void(uint32_t)> cb);
  void unsubscribe();

  uint32_t getUIntParam();
  bool getBoolParam();

  static void send(uint32_t id);
  static void send(uint32_t id, uint32_t data);

 protected:
  uint32_t id = 0;
  std::function<void(uint32_t)> callback = nullptr;
};
