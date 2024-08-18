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

#include "logicalswitchdata.h"

#include "radiodata.h"
#include "radiodataconversionstate.h"
#include "rawsource.h"
#include "rawswitch.h"

bool LogicalSwitchData::isEmpty() const
{
  return (func == 0);
}

CSFunctionFamily LogicalSwitchData::getFunctionFamily() const
{
  if (func == LS_FN_EDGE)
    return LS_FAMILY_EDGE;
  else if (func == LS_FN_TIMER)
    return LS_FAMILY_TIMER;
  else if (func == LS_FN_STICKY)
    return LS_FAMILY_STICKY;
  else if (func < LS_FN_AND || func > LS_FN_ELESS)
    return LS_FAMILY_VOFS;
  else if (func < LS_FN_EQUAL)
    return LS_FAMILY_VBOOL;
  else
    return LS_FAMILY_VCOMP;
}

unsigned int LogicalSwitchData::getRangeFlags() const
{
  int f = 0;

  if (func == LS_FN_DPOS || func == LS_FN_DAPOS)
    f |= RANGE_DELTA_FUNCTION;

  if (func == LS_FN_DAPOS || func == LS_FN_APOS || func == LS_FN_ANEG)
    f |= RANGE_ABS_FUNCTION;

  return f;
}

QString LogicalSwitchData::funcToString() const
{
  switch (func) {
    case LS_FN_OFF:
      return tr("---");
    case LS_FN_VPOS:
      return tr("a>x");
    case LS_FN_VNEG:
      return tr("a<x");
    case LS_FN_APOS:
      return tr("|a|>x");
    case LS_FN_ANEG:
      return tr("|a|<x");
    case LS_FN_AND:
      return tr("AND");
    case LS_FN_OR:
      return tr("OR");
    case LS_FN_XOR:
      return tr("XOR");
    case LS_FN_EQUAL:
      return tr("a=b");
    case LS_FN_NEQUAL:
      return tr("a!=b");
    case LS_FN_GREATER:
      return tr("a>b");
    case LS_FN_LESS:
      return tr("a<b");
    case LS_FN_EGREATER:
      return tr("a>=b");
    case LS_FN_ELESS:
      return tr("a<=b");
    case LS_FN_DPOS:
      return tr("d>=x");
    case LS_FN_DAPOS:
      return tr("|d|>=x");
    case LS_FN_VEQUAL:
      return tr("a=x");
    case LS_FN_VALMOSTEQUAL:
      return tr("a~x");
    case LS_FN_TIMER:
      return tr("Timer");
    case LS_FN_STICKY:
      return tr("Sticky");
    case LS_FN_EDGE:
      return tr("Edge");
    default:
      return tr("Unknown");
  }
}

QString LogicalSwitchData::nameToString(int index) const
{
  return RadioData::getElementName(tr("L"), index + 1, NULL, true);
}

void LogicalSwitchData::convert(RadioDataConversionState & cstate)
{
  cstate.setComponent(tr("LSW"), 7);
  cstate.setSubComp(RawSwitch(SWITCH_TYPE_VIRTUAL, cstate.subCompIdx + 1).toString(cstate.fromType, cstate.fromGS(), cstate.fromModel()));
  CSFunctionFamily family = getFunctionFamily();
  switch(family) {
    case LS_FAMILY_VOFS:
      val1 = RawSource(val1).convert(cstate.withComponentField("V1")).toValue();
      break;
    case LS_FAMILY_STICKY:
    case LS_FAMILY_VBOOL:
      val1 = RawSwitch(val1).convert(cstate.withComponentField("V1")).toValue();
      val2 = RawSwitch(val2).convert(cstate.withComponentField("V2")).toValue();
      break;
    case LS_FAMILY_EDGE:
      val1 = RawSwitch(val1).convert(cstate.withComponentField("V1")).toValue();
      break;
    case LS_FAMILY_VCOMP:
      val1 = RawSource(val1).convert(cstate.withComponentField("V1")).toValue();
      val2 = RawSource(val2).convert(cstate.withComponentField("V2")).toValue();
      break;
    default:
      break;
  }

  andsw = RawSwitch(andsw).convert(cstate.withComponentField("AND")).toValue();
}
