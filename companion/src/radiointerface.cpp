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

#include "radiointerface.h"
#include "appdata.h"
#include "radionotfound.h"
#include "helpers.h"
#include "process_copy.h"
#include "storage.h"
#include "progresswidget.h"

#include <QMessageBox>

bool readFirmware(const QString & filename, ProgressWidget * progress)
{
  bool result = false;

  QFile file(filename);
  if (file.exists() && !file.remove()) {
    QMessageBox::warning(NULL, CPN_STR_TTL_ERROR,
                         QCoreApplication::translate("RadioInterface", "Could not delete temporary file: %1").arg(filename));
    return false;
  }

  // TODO: read firmware to file
  qDebug() << "TODO: read firmware to file";

  if (!QFileInfo(filename).exists()) {
    result = false;
  }

  return result;
}

bool writeFirmware(const QString & filename, ProgressWidget * progress)
{
  // TODO: read firmware to file
  qDebug() << "TODO: write firmware to device";
  return true;
}

bool readSettings(const QString & filename, ProgressWidget * progress)
{
  QFile file(filename);
  if (file.exists() && !file.remove()) {
    QMessageBox::warning(NULL, CPN_STR_TTL_ERROR,
                         QCoreApplication::translate("RadioInterface", "Could not delete temporary file: %1").arg(filename));
    return false;
  }

  return readSettingsSDCard(filename, progress);
}

bool readSettingsSDCard(const QString & filename, ProgressWidget * progress, bool fromRadio)
{
  QString radioPath;

  if (fromRadio) {
    radioPath = findMassstoragePath("RADIO", true, progress);
    qDebug() << "Searching for SD card, found" << radioPath;
  }
  else {
    radioPath = g.currentProfile().sdPath();
    if (!QFile::exists(radioPath % "/RADIO"))
      radioPath.clear();
  }

  if (radioPath.isEmpty()) {
    QMessageBox::critical(progress, CPN_STR_TTL_ERROR,
                          QCoreApplication::translate("RadioInterface", "Unable to find SD card!"));
    return false;
  }

  RadioData radioData;
  Storage inputStorage(radioPath);
  if (!inputStorage.load(radioData)) {
    QString errorMsg = inputStorage.error();
    if (errorMsg.isEmpty()) {
      errorMsg = QCoreApplication::translate("RadioInterface", "Failed to read Models and Settings from") % radioPath;
    }
    QMessageBox::critical(progress, CPN_STR_TTL_ERROR, errorMsg);
    return false;
  }
  Storage outputStorage(filename);
  if (!outputStorage.write(radioData)) {
    QMessageBox::critical(progress, CPN_STR_TTL_ERROR, QCoreApplication::translate("RadioInterface", "Failed to write Models and Setting file") % " " % filename);
    return false;
  }

  return QFileInfo(filename).exists();
}

QString findMassstoragePath(const QString & filename, bool onlyPath, ProgressWidget *progress)
{
  QString foundPath;
  QString foundProbefile;
  int found = 0;

  QRegularExpression fstypeRe("^(v?fat|msdos|lifs)", QRegularExpression::CaseInsensitiveOption);  // Linux: "vfat"; macOS: "msdos" or "lifs"; Win: "FAT32"

  foreach(const QStorageInfo & si, QStorageInfo::mountedVolumes()) {
    //qDebug() << si.rootPath() << si.name() << si.device() << si.displayName() << si.fileSystemType() << si.isReady() << si.bytesTotal() << si.blockSize();
    if (!si.isReady() || si.isReadOnly() || !QString(si.fileSystemType()).contains(fstypeRe))
      continue;

    QString temppath = si.rootPath();
    QString probefile = temppath % "/" % filename;
    qDebug() << "Searching for" << probefile;

    if (QFile::exists(probefile)) {
      found++;
      foundPath = temppath;
      foundProbefile = probefile;
      qDebug() << probefile << "found";
    }
  }

  if (found == 1) {
    return onlyPath ? foundPath : foundProbefile;
  } else if (found > 1) {
    QMessageBox::critical(progress, CPN_STR_TTL_ERROR, filename % " " % QCoreApplication::translate("RadioInterface", "found in multiple locations"));
  }

  return QString();
}
