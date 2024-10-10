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

#include "updates.h"
#include "updatefactories.h"
#include "updatesdialog.h"
#include "progressdialog.h"
#include "progresswidget.h"
#include "appdata.h"
#include "helpers.h"

#include <QMessageBox>
#include <QApplication>

Updates::Updates(QWidget * parent, UpdateFactories * updateFactories) :
  QWidget(parent),
  factories(updateFactories)
{
}

Updates::~Updates()
{
}

void Updates::autoUpdates(bool interactive)
{
  if (!interactive) {
    if (g.updateCheckFreq() == AppData::UPDATE_CHECK_MANUAL)
      return;

    if (g.lastUpdateCheck().trimmed().isEmpty())
      g.lastUpdateCheck(QDateTime::currentDateTime().addDays(-60).toString(Qt::ISODate));

    QDateTime dt = QDateTime::fromString(g.lastUpdateCheck(), Qt::ISODate);

    if (g.updateCheckFreq() == AppData::UPDATE_CHECK_DAILY)
      dt = dt.addDays(1);
    else if (g.updateCheckFreq() == AppData::UPDATE_CHECK_WEEKLY)
      dt = dt.addDays(7);
    else if (g.updateCheckFreq() == AppData::UPDATE_CHECK_MONTHLY)
      dt = dt.addDays(30);

    if (dt > QDateTime::currentDateTime().toLocalTime()) {
      qDebug() << "Update next due:" << dt.toString(Qt::ISODate);
      return;
    }
  }

  g.lastUpdateCheck(QDateTime::currentDateTime().toString(Qt::ISODate));

  factories->resetAllEnvironments();

  QMap<QString, int> components;

  if (!factories->isUpdateAvailable(components)) {
    if (interactive)
      QMessageBox::information(parentWidget(), CPN_STR_APP_NAME, tr("No updates available at this time"));
    return;
  }

  QMapIterator<QString, int> it(components);

  QStringList list;

  while (it.hasNext()) {
    it.next();
    list << it.key();
  }

  if (QMessageBox::question(parentWidget(), CPN_STR_APP_NAME % ": " % tr("Checking for Updates"),
                            tr("Updates available for:\n  - %1\n\nProcess now?").arg(list.join("\n  - ")),
                            (QMessageBox::Yes | QMessageBox::No), QMessageBox::No) == QMessageBox::No) {
    return;
  }

  it.toFront();

  while (it.hasNext()) {
    it.next();
    UpdateInterface *iface = factories->instance(it.value());
    iface->setRunUpdate();
    iface->releaseUpdate();
  }

  runUpdate();
}

void Updates::manualUpdates()
{
  if (factories->sortedComponentsList(true).isEmpty()) {
    QMessageBox::warning(this, CPN_STR_APP_NAME, tr("No components have been flagged to check in Update Settings!"));
    return;
  }

  factories->resetAllEnvironments();

  UpdatesDialog *dlg = new UpdatesDialog(this, factories);
  dlg->deleteLater();

  if (dlg->exec())
    runUpdate();
}

void Updates::runUpdate()
{
  bool ok = false;
  ProgressDialog progressDialog(this, tr("Update Components"), CompanionIcon("fuses.png"), true);
  progressDialog.setProcessStarted();
  progressDialog.progress()->setInfo(tr("Starting..."));
  ok = factories->updateAll(progressDialog.progress());
  progressDialog.setProcessStopped();
  progressDialog.progress()->setInfo(tr("Finished %1").arg(ok ? tr("successfully") : tr("with errors")));
  progressDialog.progress()->setValue(progressDialog.progress()->maximum());
  progressDialog.progress()->refresh();
  QApplication::processEvents();
  progressDialog.exec();

  if (ok)
    checkRunSDSync();
}

void Updates::checkRunSDSync()
{
  if (!g.currentProfile().runSDSync())
    return;

  if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Run SD card sync now?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
    emit runSDSync();
}
