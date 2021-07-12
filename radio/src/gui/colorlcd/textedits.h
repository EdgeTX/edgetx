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

#include "libopenui.h"
#include "textedit.h"
#include "storage/storage.h"

class ModelTextEdit: public TextEdit
{
  public:
    ModelTextEdit(Window * parent, const rect_t & rect, char * value, uint8_t length, LcdFlags windowFlags = 0, const char * extra_chars = nullptr):
      TextEdit(parent, rect, value, length, windowFlags, extra_chars)
    {
      setChangeHandler([](){
          storageDirty(EE_MODEL);
      });
    }
};

class RadioTextEdit: public TextEdit
{
  public:
    RadioTextEdit(Window * parent, const rect_t & rect, char * value, uint8_t length, LcdFlags windowFlags = 0, const char * extra_chars = nullptr):
      TextEdit(parent, rect, value, length, windowFlags, extra_chars)
    {
      setChangeHandler([](){
          storageDirty(EE_GENERAL);
      });
    }
};
