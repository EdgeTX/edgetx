/*
 * Copyright (C) EdgeTX
 *
 * Source:
 *  https://github.com/EdgeTX/libopenui
 *
 * This file is a part of libopenui library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
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
  void update();

  void onClicked() override;
};
