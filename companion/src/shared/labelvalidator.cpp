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

#include "labelvalidator.h"

#include <QDebug>

// Note: this regexp does not include all the characters permitted for colour names
constexpr char REGEX[] {"[ A-Za-z0-9\\_\\,\\.\\+\\/\\*\\=\\%\\!\\?\\#\\<\\>\\@\\$\\(\\)\\{\\}\\[\\]\\;]*"};

LabelValidator::LabelValidator(QObject * parent):
  QRegularExpressionValidator(parent)
{
  regexp.setPattern(REGEX);

  if (!regexp.isValid())
    qDebug() << "Error - Pattern:" << regexp.pattern() << "Error msg:" << regexp.errorString() << "Error offset:" << regexp.patternErrorOffset();

  setRegularExpression(regexp);
}

void LabelValidator::fixup(QString &input) const
{
  for (int i = 0; i < input.size(); i++) {
    QRegularExpressionMatch match = regexp.match(input.at(i));
//    qDebug() << input.at(i) << match.hasMatch() << match.captured();
    if (match.captured() != input.at(i))  // cannot rely on hasMatch as the regexp accepts 0 or more matches
      input.replace(i, 1, " ");           // replacement character MUST be valid for regexp to avoid possible loop condition
  }
}

void LabelValidator::fixup(char *input) const
{
  QString in(input);
  int len = in.size();
  fixup(in);
  in.truncate(len);
  strcpy(input, in.toLatin1().data());
}

bool LabelValidator::isValid(const QString &input) const
{
  QRegularExpressionMatch match = regexp.match(input);
  // qDebug() << input << match.hasMatch() << match.captured();
  return input == match.captured();
}

bool LabelValidator::isValid(const char *input) const
{
  return isValid(QString(input));
}
