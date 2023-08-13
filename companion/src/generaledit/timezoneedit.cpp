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

#include "timezoneedit.h"

TimezoneEdit::TimezoneEdit(QWidget * parent) :
  TimerEdit(parent)
{
  setShowSeconds(false);
  setMinimumTime(-12 * 60 * 60);
  setMaximumTime(14 * 60 * 60);
  setSingleStep(15 * 60);
  setPageStep(60 * 60);
}

void TimezoneEdit::setupFormat()
{
	QString inputRe  = "^(?<pol>-|\\+|\\s)?(?<hrs>[0-9]*[0-9]):(?<mins>00|15|30|45)";
	m_validator->setRegularExpression(QRegularExpression(inputRe));

	setInputMask("#99:99");
	setValidator(m_validator);
}
