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

#include "widget.h"

class LuaWidgetFactory : public WidgetFactory
{
  friend class LuaWidget;

 public:
  LuaWidgetFactory(const char* name, ZoneOption* widgetOptions, int optionDefinitionsReference,
                   int createFunction, int updateFunction, int refreshFunction,
                   int backgroundFunction, int translateFunction, bool lvgllayout,
                   const char* filename);
  ~LuaWidgetFactory();

  Widget* create(Window* parent, const rect_t& rect,
                 Widget::PersistentData* persistentData,
                 bool init = true) const override;

  bool isLuaWidgetFactory() const override { return true; }

  bool useLvglLayout() const { return lvglLayout; }

  static ZoneOption* parseOptionDefinitions(int reference);
  const void parseOptionDefaults() const override;

 protected:
  void translateOptions(ZoneOption * options);

  int optionDefinitionsReference;
  int createFunction;
  int updateFunction;
  int refreshFunction;
  int backgroundFunction;
  int translateFunction;
  bool lvglLayout;
  std::string path;
};
