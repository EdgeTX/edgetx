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

class ButtonMatrix : public FormField
{
    char** lv_btnm_map = nullptr;
    uint8_t* txt_index = nullptr;
    uint8_t txt_cnt = 0;
    uint8_t btn_cnt = 0;

  public:
    ButtonMatrix(Window* parent, const rect_t& rect);
    ~ButtonMatrix();

    virtual void onPress(uint8_t btn_id) {}
    virtual bool isActive(uint8_t btn_id) { return false; }

  protected:
    void deallocate();
    void initBtnMap(uint8_t cols, uint8_t btns);
    void setText(uint8_t btn_id, const char* txt);
    void setChecked(uint8_t btn_id);
    void update();

    void onClicked() override;
};
