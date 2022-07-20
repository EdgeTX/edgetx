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

#include "updatesdcard.h"

#include <QMessageBox>

UpdateSDCard::UpdateSDCard(QWidget * parent) :
  UpdateInterface(parent)
{
  setName("SD Card");
  setRepo(QString(GH_REPOS_EDGETX).append("/edgetx-sdcard"));

  dfltParams->data.flags = UPDFLG_Common | UPDFLG_Locked;

  UpdateParameters::AssetParams &ap = dfltParams->addAsset();
  ap.filterType = UpdateParameters::UFT_Startswith;
  ap.filter = QString("%1-").arg(fwFlavour);
  ap.maxExpected = 1;
  ap.flags = dfltParams->data.flags | UPDFLG_CopyStructure;
}

bool UpdateSDCard::flagAssets()
{
  progressMessage(tr("Flagging assets"));
  /*
  {
    "targets": [
      ["Flysky NV14", "nv14-", "nv14"],
      ["FrSky Horus X10", "x10-", "horus"],
  */

  const QString mappingfile = "sdcard.json";

  if (!downloadTextFileToBuffer(mappingfile)) {
    return false;
  }

  QJsonDocument *json = new QJsonDocument();

  if (!convertDownloadToJson(json)) {
    return false;
  }

  UpdateParameters::AssetParams ap = runParams->data.assets.at(0);

  QRegularExpression filter(UpdateParameters::buildFilterPattern(ap.filterType, ap.filter), QRegularExpression::CaseInsensitiveOption);
  bool found = false;
  QString sdimage;

  if (json->isObject()) {
    const QJsonObject &obj = json->object();

    if (obj.value("targets").isArray()) {
      const QJsonArray &t = obj.value("targets").toArray();

      foreach (const QJsonValue &v, t) {

        if (v.isArray()) {
          const QJsonArray &i = v.toArray();
          if (i.size() > 2) {
            if (i.at(1).toString().contains(filter)) {
              sdimage = i.at(2).toString();
              found = true;
              break;
            }
          }
        }
      }
    }
  }

  if (!found) {
    reportProgress(tr("Radio flavour %1 not listed in %2").arg(fwFlavour).arg(mappingfile), QtCriticalMsg);
    return false;
  }

  delete json;

  if (!getSetAssets(ap.flags, UpdateParameters::UFT_Startswith, sdimage, ap.maxExpected, ap.destSubDir, ap.copyFilterType, ap.copyFilter))
    return false;

  return true;
}
