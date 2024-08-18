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

#include "boards.h"
#include "textvalidator.h"

// characters supportd by B&W radio firmware gui editor
constexpr char NAME_VALID_PATTERN_BW[]    {"[ A-Za-z0-9\\_\\-\\,\\.]*"};
// characters supported by color radio firmware keyboard widget
constexpr char NAME_VALID_PATTERN_COLOR[] {"[ A-Za-z0-9\\-\\_\\,\\.\"\\+\\/\\*\\=\\%\\!\\?\\#\\<\\>\\@\\$\\(\\)\\{\\}\\[\\]\\;\\:\\']*"};

class NameValidator : public TextValidator
{
  Q_OBJECT

  public:
    explicit NameValidator(Board::Type board, QObject * parent = nullptr) :
       TextValidator(parent, Boards::getCapability(board, Board::HasColorLcd) ? NAME_VALID_PATTERN_COLOR : NAME_VALID_PATTERN_BW) {}
    virtual ~NameValidator() {}
};

