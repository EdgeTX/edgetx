/*
 * Copyright (C) OpenTX
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

#include "select_fab_button.h"

class SelectFabCarousel : public FormGroup
{
public:
    explicit SelectFabCarousel(Window* parent);

    // Maximum visible buttons (-> width of the carousel)
    void setMaxButtons(uint8_t max);

    // Add a new button to the carousel
    void addButton(uint8_t icon, const char* title,
                   std::function<uint8_t(void)> pressHandler);
    
protected:
    uint8_t maxButtons = 0;
    uint8_t buttons = 0;
};
