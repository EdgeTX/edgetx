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

UpdateSounds::UpdateSounds(QWidget * parent) :
  UpdateInterface(parent)
{
  init(CID_Sounds, tr("Sounds"), QString(GH_REPOS_EDGETX).append("/edgetx-sdcard-sounds"));
  langPacks = new QStandardItemModel();
}

UpdateSounds::~UpdateSounds()
{
  delete langPacks;
}

void UpdateSounds::initAssetSettings()
{
  if (!isValidSettingsIndex())
    return;

  g.component[id()].initAllAssets();

  ComponentAssetData &cad = g.component[id()].asset[0];
  cad.desc("sounds");
  cad.processes(UPDFLG_Common_Asset);
  cad.flags(cad.processes() | UPDFLG_CopyStructure);
  cad.filterType(UpdateParameters::UFT_Startswith);
  cad.filter("edgetx-sdcard-sounds-%LANGUAGE%-");

  qDebug() << "Asset settings initialised";
}

bool UpdateSounds::flagAssets()
{
  progressMessage(tr("Processing available sounds"));

  QString pattern("sounds.json");
  assets->setFilterPattern(pattern);

  if (assets->count() < 0) {
    reportProgress(tr("Asset not found in release '%1' using filter pattern '%2'").arg(releases->name()).arg(pattern), QtCriticalMsg);
    return false;
  }
  else if (assets->count() > 1) {
    reportProgress(tr("%1 assets found when %2 expected in release '%3' using filter pattern '%4'")
                        .arg(assets->count()).arg(1).arg(releases->name()).arg(pattern), QtCriticalMsg);
    return false;
  }

  if (!downloadAssetToBuffer(assets->getSetId(0))) {
    return false;
  }

  QJsonDocument *json = new QJsonDocument();

  if (!convertDownloadToJson(json)) {
    return false;
  }

  /*
    {
        "language": "en-GB",
        "name": "British English Female",
        "description": "British English Female Voice (en-GB-Libby)",
        "directory": "en_gb-libby"
    },
  */

  //  always refresh to allow for language change in radio profile
  langPacks->clear();

  if (json->isArray()) {
    const QJsonArray &arr = json->array();

    foreach (const QJsonValue &v, arr) {
      if (v.isObject()) {
        const QJsonObject obj = v.toObject();
        QString langVariant;
        QString lang;

        if (!obj.value("language").isUndefined()) {
          langVariant = obj.value("language").toString();
          lang = langVariant.split("-").at(0);
        }

        if (lang == params->language) {
          QStandardItem * item = new QStandardItem();

          if (!obj.value("language").isUndefined())
            item->setData(obj.value("language").toString(), IMDR_Language);
          if (!obj.value("name").isUndefined())
            item->setData(obj.value("name").toString(), IMDR_Name);
          if (!obj.value("description").isUndefined())
            item->setData(obj.value("description").toString(), Qt::DisplayRole);
          if (!obj.value("directory").isUndefined())
            item->setData(obj.value("directory").toString(), IMDR_Directory);

          langPacks->appendRow(item);
        }
      }
    }
  }

  delete json;

  if (langPacks->rowCount() < 1) {
    reportProgress(tr("Language %1 not listed in %2").arg(params->language).arg(pattern), QtCriticalMsg);
    return false;
  }

  if (langPacks->rowCount() > 1) {
    ChooserDialog *dlg = new ChooserDialog(nullptr, tr("Choose Language Packs"), langPacks);

    bool ok = dlg->exec();

    QItemSelectionModel *selItems = dlg->selectedItems();

    if (!ok || !selItems->hasSelection()) {
      QMessageBox::warning(progress, CPN_STR_APP_NAME, tr("No language packs have been selected. Sounds update will be skipped!"));
      dlg->deleteLater();
      return true;
    }

    QModelIndexList selIndexes = selItems->selectedIndexes();

    for (int i = 0; i < selIndexes.size(); i++) {
      if (!flagLanguageAsset(langPacks->data(selIndexes.at(i), IMDR_Directory).toString()))
        return false;
    }

    dlg->deleteLater();
  }
  else if (langPacks->rowCount() == 1) {
    if (!flagLanguageAsset(langPacks->data(langPacks->index(0, 0), IMDR_Directory).toString()))
      return false;
  }

  return true;
}

bool UpdateSounds::flagLanguageAsset(QString lang)
{
  progressMessage(tr("Flagging assets"));

  params->language = lang;
  UpdateParameters::AssetParams ap = params->assets[0];
  return getSetAssets(ap);
}
