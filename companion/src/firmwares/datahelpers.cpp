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

#include "datahelpers.h"

QString DataHelpers::boolToString(const bool value, const BoolFormat format)
{
  static const char *strings[] = {
      QT_TRANSLATE_NOOP("DataHelpers", "Disabled"),
      QT_TRANSLATE_NOOP("DataHelpers", "Enabled"),
      QT_TRANSLATE_NOOP("DataHelpers", "OFF"),
      QT_TRANSLATE_NOOP("DataHelpers", "ON"),
      QT_TRANSLATE_NOOP("DataHelpers", "False"),
      QT_TRANSLATE_NOOP("DataHelpers", "True"),
      QT_TRANSLATE_NOOP("DataHelpers", "N"),
      QT_TRANSLATE_NOOP("DataHelpers", "Y"),
      QT_TRANSLATE_NOOP("DataHelpers", "No"),
      QT_TRANSLATE_NOOP("DataHelpers", "Yes")
  };

  return QCoreApplication::translate("DataHelpers", strings[format * 2 + value]);
}

QString DataHelpers::getElementName(const QString & prefix, const unsigned int index, const char * name, const bool padding)
{
  QString result = prefix;
  result.append(padding ? QString("%1").arg(index, 2, 10, QChar('0')) : QString("%1").arg(index));
  if (name && QString(name).trimmed().length() > 0)
    result.append(":" + QString(name).trimmed());

  return result;
}

QString DataHelpers::timeToString(const int value, const unsigned int mask)
{
  bool negative = value < 0 ? true : false;
  int val = abs(value);

  QString result = QString("%1").arg(negative ? "-" : ((mask & TIMESTR_MASK_PADSIGN) ? " " : ""));

  if (mask & TIMESTR_MASK_HRSMINS) {
    int hours = val / 3600;
    if (hours > 0 || (mask & TIMESTR_MASK_ZEROHRS)) {
      val -= hours * 3600;
      result.append(QString("%1:").arg(hours, 2, 10, QLatin1Char('0')));
    }
  }

  int minutes = val / 60;
  int seconds = val % 60;
  result.append(QString("%1:%2").arg(minutes, 2, 10, QLatin1Char('0')).arg(seconds, 2, 10, QLatin1Char('0')));
  return result;
}

int DataHelpers::getStringTagMappingIndex(const StringTagMappingTable& lut, const char * tag)
{
  auto it =
    find_if(lut.begin(), lut.end(), [=](const StringTagMapping& elmt) {
      if (elmt.tag == tag) return true;
      return false;
    });

  if (it != lut.end()) {
    return it - lut.begin();
  }

  return -1;
}

std::string DataHelpers::getStringTagMappingTag(const StringTagMappingTable& lut, unsigned int index)
{
  if (index < lut.size())
    return lut[index].tag;

  return std::string();
}

std::string DataHelpers::getStringNameMappingTag(const StringTagMappingTable& lut, const char * name)
{
  auto it =
    find_if(lut.begin(), lut.end(), [=](const StringTagMapping& elmt) {
      if (elmt.name == name) return true;
      return false;
    });

  if (it != lut.end()) {
    return it->tag;
  }

  return std::string();
}

std::string DataHelpers::getStringTagMappingName(const StringTagMappingTable& lut, const char * tag)
{
  auto it =
    find_if(lut.begin(), lut.end(), [=](const StringTagMapping& elmt) {
      if (elmt.tag == tag) return true;
      return false;
    });

  if (it != lut.end()) {
    return it->name;
  }

  return std::string();
}

QString DataHelpers::getCompositeName(const QString defaultName, const QString customName, const bool prefixCustom)
{
  QString result;

  if (customName.trimmed().isEmpty() || prefixCustom)
    result = defaultName;

  if (!customName.trimmed().isEmpty()) {
    if (prefixCustom)
      result.append(":");
    result.append(customName.trimmed());
  }

  return result;
}
