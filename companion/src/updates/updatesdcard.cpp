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
  UpdateInterface(parent, CID_SDCard, tr("SD Card"), Repo::REPO_TYPE_GITHUB,
                  QString(GH_API_REPOS_EDGETX).append("/edgetx-sdcard"), "Latest")
{
  init(); // call after UpdateInterface ctor due to virtual functions
}

void UpdateSDCard::assetSettingsInit()
{
  if (!isSettingsIndexValid())
    return;

  g.component[id()].initAllAssets();

  ComponentAssetData &cad = g.component[id()].asset[0];
  cad.desc("files");
  cad.processes(UPDFLG_Common_Asset);
  cad.flags(cad.processes() | UPDFLG_Locked | UPDFLG_CopyStructure);
  cad.filterType(UpdateParameters::UFT_Startswith);
  cad.filter("%FWFLAVOUR%-");
  cad.maxExpected(1);

  qDebug() << "Asset settings initialised";
}

bool UpdateSDCard::flagAssets()
{
  status()->progressMessage(tr("Flagging assets"));

  QJsonDocument *json = new QJsonDocument();

  const QString jsonFile("sdcard.json");

  if (!retrieveAssetsJsonFile(jsonFile, json)) {
    status()->reportProgress(tr("Unable to retrieve asset '%1' from release '%2'").arg(jsonFile).arg(repo()->releases()->name()), QtDebugMsg);
    //  assume older release where file not an asset
    if (!retrieveRepoJsonFile(jsonFile, json)) {
      status()->reportProgress(tr("Unable to retrieve file '%1' from repo '%2'").arg(jsonFile).arg(repo()->path()), QtCriticalMsg);
      delete json;
      return false;
    }
  }

  /*
  {
    "targets": [
      ["Flysky NV14", "nv14-", "c320x480"],
      ["FrSky Horus X10", "x10-", "c480x272"],
  */

  const UpdateParameters::AssetParams & ap = params()->assets.at(0);

  QRegularExpression filter(params()->buildFilterPattern(ap.filterType, ap.filter), QRegularExpression::CaseInsensitiveOption);
  bool found = false;
  QString sdimage;

  //qDebug() << *json;

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

  delete json;

  if (!found) {
    status()->reportProgress(tr("Radio flavour '%1' not listed in '%2'").arg(params()->fwFlavour).arg(jsonFile), QtCriticalMsg);
    return false;
  }

  UpdateParameters::AssetParams sd = ap;
  sd.filterType = UpdateParameters::UFT_Startswith;
  sd.filter = sdimage;

  if (!setFilteredAssets(sd))
    return false;

  return true;
}
