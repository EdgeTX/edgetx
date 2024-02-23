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

#include "namevalidator.h"

// characters supportd by B&W radio firmware gui editor
constexpr char BW_REGEX[]    {"[ A-Za-z0-9\\-]*"};
// characters supported by color radio firmware keyboard widget
constexpr char COLOR_REGEX[] {"[ A-Za-z0-9\\-\\_\\,\\.\"\\+\\/\\*\\=\\%\\!\\?\\#\\<\\>\\@\\$\\(\\)\\{\\}\\[\\]\\;\\:\\']*"};

NameValidator::NameValidator(Board::Type board, QObject * parent):
  QRegularExpressionValidator(parent)
{
  regexp.setPattern(Boards::getCapability(board, Board::HasColorLcd) ? COLOR_REGEX : BW_REGEX);

  if (!regexp.isValid())
    qDebug() << "Error - Pattern:" << regexp.pattern() << "Error msg:" << regexp.errorString() << "Error offset:" << regexp.patternErrorOffset();

  setRegularExpression(regexp);
}

void NameValidator::fixup(QString &input) const
{
  for (int i = 0; i < input.size(); i++) {
    QRegularExpressionMatch match = regexp.match(input.at(i));
    if (match.captured() != input.at(i))  // cannot rely on hasMatch as the regexp accepts 0 or more matches
      input.replace(i, 1, " ");           // replacement character MUST be valid for regexp to avoid possible loop condition
  }

  input = input.trimmed();
}

bool NameValidator::isValid(const QString &input) const
{
  QRegularExpressionMatch match = regexp.match(input);
  return input == match.captured();
}
