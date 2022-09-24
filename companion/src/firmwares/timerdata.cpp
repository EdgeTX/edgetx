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

#include "timerdata.h"
#include "radiodataconversionstate.h"
#include "compounditemmodels.h"

void TimerData::convert(RadioDataConversionState & cstate)
{
  cstate.setComponent(tr("TMR"), 1);
  cstate.setSubComp(tr("Timer %1").arg(cstate.subCompIdx + 1));
  swtch.convert(cstate);
}

void TimerData::clear()
{
  memset(reinterpret_cast<void *>(this), 0, sizeof(TimerData));
  swtch = RawSwitch(SWITCH_TYPE_NONE);
}

bool TimerData::isEmpty() const
{
  return (swtch == RawSwitch(SWITCH_TYPE_NONE, 0) && mode == TIMERMODE_OFF &&
          name[0] == '\0' && minuteBeep == 0 &&
          countdownBeep == COUNTDOWNBEEP_SILENT && val == 0 &&
          persistent == 0 /*&& pvalue == 0*/);
}

bool TimerData::isModeOff()
{
  return swtch == RawSwitch(SWITCH_TYPE_TIMER_MODE, 0);
}

QString TimerData::nameToString(int index) const
{
  return DataHelpers::getElementName(tr("TMR", "as in Timer"), index + 1, name);
}

QString TimerData::countdownBeepToString() const
{
  return countdownBeepToString(countdownBeep);
}

QString TimerData::countdownStartToString() const
{
  if (countdownBeep == COUNTDOWNBEEP_SILENT)
    return "";
  else
    return countdownStartToString(countdownStart);
}

QString TimerData::persistentToString(const bool verbose) const
{
  return persistentToString(persistent, verbose);
}

QString TimerData::pvalueToString() const
{
  return pvalueToString(pvalue);
}

QString TimerData::valToString() const
{
  return valToString(val);
}

QString TimerData::modeToString() const
{
  return modeToString(mode);
}

void TimerData::countdownBeepChanged()
{
  if (countdownBeep == COUNTDOWNBEEP_SILENT)
    countdownStart = 0;
}

void TimerData::modeChanged()
{
  if (mode != TIMERMODE_START)
    swtch = RawSwitch(SWITCH_TYPE_NONE);
}

//  static
QString TimerData::countdownBeepToString(const int value)
{
  switch(value) {
    case COUNTDOWNBEEP_SILENT:
      return tr("Silent");
    case COUNTDOWNBEEP_BEEPS:
      return tr("Beeps");
    case COUNTDOWNBEEP_VOICE:
      return tr("Voice");
    case COUNTDOWNBEEP_HAPTIC:
      return tr("Haptic");
    default:
      return CPN_STR_UNKNOWN_ITEM;
  }
}

//  static
QString TimerData::countdownStartToString(const int value)
{
  switch(value) {
    case COUNTDOWNSTART_5:
      return tr("5s");
    case COUNTDOWNSTART_10:
      return tr("10s");
    case COUNTDOWNSTART_20:
      return tr("20s");
    case COUNTDOWNSTART_30:
      return tr("30s");
    default:
      return CPN_STR_UNKNOWN_ITEM;
  }
}

//  static
QString TimerData::persistentToString(const int value, const bool verbose)
{
  switch(value) {
    case PERSISTENT_NOT:
      return verbose ? tr("Not persistent") : tr("NOT");
    case PERSISTENT_FLIGHT:
      return verbose ? tr("Persistent (flight)") : tr("Flight");
    case PERSISTENT_MANUALRESET:
      return verbose ? tr("Persistent (manual reset)") : tr("Manual reset");
    default:
      return CPN_STR_UNKNOWN_ITEM;
  }
}

//  static
QString TimerData::pvalueToString(const int value)
{
  return DataHelpers::timeToString(value, TIMESTR_MASK_HRSMINS | TIMESTR_MASK_ZEROHRS);
}

//  static
QString TimerData::valToString(const int value)
{
  return DataHelpers::timeToString(value, TIMESTR_MASK_HRSMINS | TIMESTR_MASK_ZEROHRS);
}

//  static
QString TimerData::modeToString(const int value)
{
  switch(value) {
    case TIMERMODE_OFF:
      return tr("OFF");
    case TIMERMODE_ON:
      return tr("ON");
    case TIMERMODE_START:
      return tr("Start");
    case TIMERMODE_THR:
      return tr("THs");
    case TIMERMODE_THR_REL:
      return tr("TH%");
    case TIMERMODE_THR_START:
      return tr("THt");
    default:
      return CPN_STR_UNKNOWN_ITEM;
  }
}

QString TimerData::showElapsedToString(const unsigned int value)
{
  return (value) ? tr("Show Elapsed") : tr("Show Remaining");
}

QString TimerData::showElapsedToString() const
{
  return showElapsedToString(showElapsed);
}

//  static
AbstractStaticItemModel * TimerData::countdownBeepItemModel()
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName(AIM_TIMER_COUNTDOWNBEEP);

  for (int i = 0; i < COUNTDOWNBEEP_COUNT; i++) {
    mdl->appendToItemList(countdownBeepToString(i), i);
  }

  mdl->loadItemList();
  return mdl;
}

//  static
AbstractStaticItemModel * TimerData::countdownStartItemModel()
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName(AIM_TIMER_COUNTDOWNSTART);

  for (int i = COUNTDOWNSTART_FIRST; i <= COUNTDOWNSTART_LAST; i++) {
    mdl->appendToItemList(countdownStartToString(i), i);
  }

  mdl->loadItemList();
  return mdl;
}

//  static
AbstractStaticItemModel * TimerData::persistentItemModel()
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName(AIM_TIMER_PERSISTENT);

  for (int i = 0; i < PERSISTENT_COUNT; i++) {
    mdl->appendToItemList(persistentToString(i), i);
  }

  mdl->loadItemList();
  return mdl;
}

//  static
AbstractStaticItemModel * TimerData::modeItemModel()
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName(AIM_TIMER_MODE);

  for (int i = 0; i < TIMERMODE_COUNT; i++) {
    mdl->appendToItemList(modeToString(i), i);
  }

  mdl->loadItemList();
  return mdl;
}

AbstractStaticItemModel * TimerData::showElapsedItemModel()
{
  AbstractStaticItemModel *mdl = new AbstractStaticItemModel();
  mdl->setName(AIM_TIMER_SHOWELAPSED);

  for (unsigned int i = 0; i < TIMER_SHOW_COUNT; i++) {
    mdl->appendToItemList(showElapsedToString(i), i);
  }

  mdl->loadItemList();
  return mdl;
}
