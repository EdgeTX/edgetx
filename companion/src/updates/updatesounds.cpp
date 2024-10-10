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

#include "updatesounds.h"
#include "chooserdialog.h"

#include <QMessageBox>
#include <QStandardItem>
#include <QItemSelectionModel>
#include <QAbstractItemView>

UpdateSounds::UpdateSounds(QWidget * parent) :
  UpdateInterface(parent, CID_Sounds, tr("Sounds"), Repo::REPO_TYPE_GITHUB,
                  QString(GH_API_REPOS_EDGETX).append("/edgetx-sdcard-sounds"), "latest", 50)
{
  init(); // call after UpdateInterface ctor due to virtual functions
  langPacks = new QStandardItemModel();
}

UpdateSounds::~UpdateSounds()
{
  delete langPacks;
}

void UpdateSounds::assetSettingsInit()
{
  if (!isSettingsIndexValid())
    return;

  g.component[id()].initAllAssets();

  ComponentAssetData &cad = g.component[id()].asset[0];
  cad.desc("sounds");
  cad.processes(UPDFLG_Common_Asset);
  cad.flags(cad.processes() | UPDFLG_CopyStructure);
  cad.filterType(UpdateParameters::UFT_Startswith);
  cad.filter("edgetx-sdcard-sounds-%LANGUAGE%");

  qDebug() << "Asset settings initialised";
}

bool UpdateSounds::flagAssets()
{
  status()->progressMessage(tr("Processing available sounds"));

  // always refresh to allow for language change in radio profile
  langPacks->clear();

  UpdateParameters::AssetParams ap = params()->assets[0];
  if (!filterAssets(ap))
    return false;

  for (int i = 0; i < repo()->assets()->count(); i++) {
    repo()->assets()->getSetId(i);
    QStandardItem * item = new QStandardItem();
    item->setText(repo()->assets()->name());
    langPacks->appendRow(item);
  }

  if (langPacks->rowCount() > 1) {
    ChooserDialog *dlg = new ChooserDialog(nullptr, tr("Choose Language Packs"), langPacks);

    bool ok = dlg->exec();

    QItemSelectionModel *selItems = dlg->selectedItems();

    if (!ok || !selItems->hasSelection()) {
      QMessageBox::warning(status()->progress(), CPN_STR_APP_NAME, tr("No language packs have been selected. Sounds update will be skipped!"));
      dlg->deleteLater();
      return true;
    }

    QModelIndexList selIndexes = selItems->selectedIndexes();

    if (!flagLanguageAsset(langPacks->data(selIndexes.at(0)).toString()))
      return false;

    dlg->deleteLater();
  }
  else if (langPacks->rowCount() == 1) {
    if (!flagLanguageAsset(langPacks->data(langPacks->index(0, 0)).toString()))
      return false;
  }

  return true;
}

bool UpdateSounds::flagLanguageAsset(QString asset)
{
  status()->progressMessage(tr("Flagging assets"));

  UpdateParameters::AssetParams ap = params()->assets[0];
  ap.filter = asset;
  ap.filterType = UpdateParameters::UFT_Exact;
  return setFilteredAssets(ap);
}
