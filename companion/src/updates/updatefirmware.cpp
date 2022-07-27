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

#include "updatefirmware.h"
#include "appdata.h"
#include "flashfirmwaredialog.h"

UpdateFirmware::UpdateFirmware(QWidget * parent) :
  UpdateInterface(parent)
{
  setName("Firmware");
  setRepo(QString(GH_REPOS_EDGETX).append("/edgetx"));
  setReleasesNightlyName("nightly");

  dfltParams->data.flags = UPDFLG_Common | UPDFLG_Locked | UPDFLG_AsyncInstall;

  UpdateParameters::AssetParams &ap = dfltParams->addAsset();
  ap.filterType = UpdateParameters::UFT_Startswith;
  ap.filter = "edgetx-firmware";
  ap.maxExpected = 1;
  ap.flags = dfltParams->data.flags | UPDFLG_CopyFiles;
  ap.destSubDir = "FIRMWARE";
  ap.copyFilterType = UpdateParameters::UFT_Expression;
  ap.copyFilter = QString("^%FWFLAVOUR%-.*\\.bin$");
}

bool UpdateFirmware::asyncInstall()
{
  //reportProgress(tr("Write firmware to radio: %1").arg(g.currentProfile().burnFirmware() ? tr("true") : tr("false")), QtDebugMsg);

  if (!g.currentProfile().burnFirmware())
    return true;

  progressMessage(tr("Install"));

  assets->setFilterFlags(UPDFLG_AsyncInstall);
  //reportProgress(tr("Asset filter applied: %1 Assets found: %2").arg(updateFlagsToString(UPDFLG_AsyncInstall)).arg(assets->count()), QtDebugMsg);

  if (assets->count() != 1) {
    reportProgress(tr("Expected 1 asset for install but none found"), QtCriticalMsg);
    return false;
  }

  QString destPath = updateDir;

  if (!assets->subDirectory().isEmpty())
    destPath.append("/" % assets->subDirectory());

  QRegularExpression filter(runParams->data.assets.at(0).copyFilter, QRegularExpression::CaseInsensitiveOption);

  QDirIterator it(destPath);

  bool found = false;

  while (it.hasNext())
  {
    QFileInfo file(it.next());

    if ((!filter.pattern().isEmpty()) && (!file.fileName().contains(filter)))
      continue;

    destPath.append("/" % file.fileName());
    found = true;
    break;
  }

  if (!found) {
    reportProgress(tr("Firmware not found in %1 using filter %2").arg(destPath).arg(filter.pattern()), QtCriticalMsg);
    return false;
  }

  g.currentProfile().fwName(destPath);

  int ret = QMessageBox::question(this, CPN_STR_APP_NAME, tr("Write the updated firmware to the radio now ?"), QMessageBox::Yes | QMessageBox::No);
  if (ret == QMessageBox::Yes) {
    FlashFirmwareDialog *dlg = new FlashFirmwareDialog(this);
    dlg->exec();
    dlg->deleteLater();
  }

  return true;
}
