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

#include <QWidget>
#include <QLabel>
#include <functional>
#include <vector>

class WidgetBindings
{
  public:
    WidgetBindings() = default;

    void bindVisible(QWidget *widget, std::function<bool()> pred);
    void bindEnabled(QWidget *widget, std::function<bool()> pred);
    void bindText(QLabel *label, std::function<QString()> fn);
    void applyAll();

  private:
    struct VisibleBinding { QWidget *widget; std::function<bool()> pred; };
    struct EnabledBinding { QWidget *widget; std::function<bool()> pred; };
    struct TextBinding    { QLabel  *label;  std::function<QString()> fn; };

    std::vector<VisibleBinding> m_visible;
    std::vector<EnabledBinding> m_enabled;
    std::vector<TextBinding>    m_text;
};
