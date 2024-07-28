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

#include "adjustmentreference.h"
#include "modeldata.h"

AdjustmentReference::AdjustmentReference(int value)
{
  int val = value;
  if (abs(val) > ADJUST_REF_GVAR_BASE && abs(val) <= ADJUST_REF_GVAR_BASE + CPN_MAX_GVARS) {
    type = ADJUST_REF_GVAR;
    val = val >= 0 ? val - ADJUST_REF_GVAR_BASE : val + ADJUST_REF_GVAR_BASE;
  }
  else
    this->type = ADJUST_REF_VALUE;

  this->value = val;
}

AdjustmentReference::AdjustmentReference(AdjustRefType type, int value)
{
  this->type = type;
  if (isValid(value))
    this->value = value;
  else
    clear();
}

bool AdjustmentReference::isValid(const int value) const
{
  switch (type) {
    case ADJUST_REF_VALUE:
      return abs(value) < ADJUST_REF_GVAR_BASE;

    case ADJUST_REF_GVAR:
      return abs(value) > 0 && abs(value) <= CPN_MAX_GVARS;
  }
  return false;
}

QString AdjustmentReference::toString(const ModelData * model, const bool sign) const
{
  QString ret;

  switch(type) {
    case ADJUST_REF_GVAR:
      ret = RawSource(SOURCE_TYPE_GVAR, abs(value)).toString(model);
      if (value < 0)
        ret.prepend("-");
      else if (sign)
        ret.prepend("+");
      break;
    default:
      ret = "%1%";
      if (sign && value > 0)
        ret.prepend("+");
      ret = ret.arg(value);
      break;
  }
  return ret;
}

const bool AdjustmentReference::isAvailable() const
{
  return true;
}
