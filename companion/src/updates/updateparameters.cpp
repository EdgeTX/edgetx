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

#include "updateparameters.h"
#include "constants.h"

UpdateParameters::UpdateParameters(QObject * parent) :
  QObject(parent)
{
  logLevel = 0;
  flags = 0;
  decompressDirUseDwnld = true;
  updateDirUseSD =  true;
}

UpdateParameters::AssetParams & UpdateParameters::addAsset()
{
  AssetParams ap;
  ap.processes = 0;
  ap.flags = 0;
  ap.filterType = UFT_None;
  ap.maxExpected = 0;
  ap.copyFilterType = UFT_None;
  assets.append(ap);

  return assets[assets.size() - 1];
}

QString UpdateParameters::buildFilterPattern(const UpdateFilterType filterType, const QString & filter)
{
  QString pattern(filter);

  pattern.replace("%FWFLAVOUR%", fwFlavour);
  pattern.replace("%LANGUAGE%", language);
  QString rel = releaseUpdate;
  rel.replace(".", "\\.");
  pattern.replace("%RELEASE%", rel);

  switch ((int)filterType) {
    case UFT_Exact:
      pattern.prepend("^");
      pattern.append("$");
      break;
    case UFT_Startswith:
      pattern.prepend("^");
      break;
    case UFT_Endswith:
      pattern.append("$");
      break;
    case UFT_Contains:
    case UFT_Pattern:
      break;
    default:
      pattern = "";
  }

  return pattern;
}

//  static
QStringList UpdateParameters::updateFilterTypeList()
{
  return { tr("None"), tr("Exact"), tr("Startswith"), tr("Endswith"), tr("Contains"), tr("Pattern") };
}

//  static
QString UpdateParameters::updateFilterTypeToString(UpdateFilterType uft)
{
  return updateFilterTypeList().value(uft, CPN_STR_UNKNOWN_ITEM);
}
