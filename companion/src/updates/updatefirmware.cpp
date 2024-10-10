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
#include "flashfirmwaredialog.h"

UpdateFirmware::UpdateFirmware(QWidget * parent) :
  UpdateInterface(parent, CID_Firmware, tr("Firmware"), Repo::REPO_TYPE_GITHUB,
                  QString(GH_API_REPOS_EDGETX).append("/edgetx"), "nightly")
{
  init(); // call after UpdateInterface ctor due to virtual functions
}

void UpdateFirmware::assetSettingsInit()
{
  if (!isSettingsIndexValid())
    return;

  g.component[id()].initAllAssets();

  ComponentAssetData &cad = g.component[id()].asset[0];

  cad.desc("binary");
  cad.processes(UPDFLG_Common_Asset | UPDFLG_AsyncInstall);
  cad.flags(cad.processes() | UPDFLG_CopyFiles | UPDFLG_Locked);
  cad.filterType(UpdateParameters::UFT_Startswith);
  cad.filter("edgetx-firmware");
  cad.maxExpected(1);
  cad.destSubDir("FIRMWARE");
  cad.copyFilterType(UpdateParameters::UFT_Pattern);
  cad.copyFilter("^%FWFLAVOUR%-.*\\.bin$");

  qDebug() << "Asset settings initialised";
}

int UpdateFirmware::asyncInstall()
{
  status()->reportProgress(tr("Write firmware to radio: %1").arg(g.currentProfile().burnFirmware() ? tr("true") : tr("false")), QtDebugMsg);

  if (!g.currentProfile().burnFirmware())
    return true;

  status()->progressMessage(tr("Install"));

  repo()->assets()->setFilterFlags(UPDFLG_AsyncInstall);
  status()->reportProgress(tr("Asset filter applied: %1 Assets found: %2").arg(updateFlagsToString(UPDFLG_AsyncInstall)).arg(repo()->assets()->count()), QtDebugMsg);

  if (repo()->assets()->count() < 1)
    return true;

  if (repo()->assets()->count() != params()->assets.at(0).maxExpected) {
    status()->reportProgress(tr("Expected %1 asset for install but %2 found").arg(params()->assets.at(0).maxExpected).arg(repo()->assets()->count()), QtCriticalMsg);
    return false;
  }

  QString destPath = updateDir();

  if (!repo()->assets()->subDirectory().isEmpty())
    destPath.append("/" % repo()->assets()->subDirectory());

  const UpdateParameters::AssetParams &ap = params()->assets.at(0);

  QString pattern(params()->buildFilterPattern(ap.copyFilterType, ap.copyFilter));
  QRegularExpression filter(pattern, QRegularExpression::CaseInsensitiveOption);

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
    status()->reportProgress(tr("Firmware not found in %1 using filter %2").arg(destPath).arg(filter.pattern()), QtCriticalMsg);
    return false;
  }

  g.currentProfile().fwName(destPath);

  int ret = QMessageBox::question(status()->progress(), CPN_STR_APP_NAME, tr("Write the updated firmware to the radio now ?"), QMessageBox::Yes | QMessageBox::No);
  if (ret == QMessageBox::Yes) {
    FlashFirmwareDialog *dlg = new FlashFirmwareDialog(this);
    dlg->exec();
    dlg->deleteLater();
  }

  return true;
}
