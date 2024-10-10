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

#pragma once

#include <QtCore>

class UpdateParameters : public QObject
{
    Q_OBJECT

  public:
    enum UpdateFilterType {
      UFT_None,
      UFT_Exact,
      UFT_Startswith,
      UFT_Endswith,
      UFT_Contains,
      UFT_Pattern,
    };

    static QStringList updateFilterTypeList();

    static QString updateFilterTypeToString(UpdateFilterType uft);

    struct AssetParams {
      int processes;
      int flags;
      UpdateFilterType filterType;
      QString filter;
      int maxExpected;
      QString destSubDir;
      UpdateFilterType copyFilterType;
      QString copyFilter;
    };

    int flags;
    int logLevel;
    QString fwFlavour;
    QString language;
    int releaseChannel;
    QString releaseCurrent;
    QString releaseUpdate;
    QString downloadDir;
    bool decompressDirUseDwnld;
    QString decompressDir;
    QString sdDir;
    bool updateDirUseSD;
    QString updateDir;
    QVector<AssetParams> assets;

    UpdateParameters(QObject * parent);
    virtual ~UpdateParameters() {}

    AssetParams & addAsset();
    QString buildFilterPattern(const UpdateFilterType filterType, const QString & filter);
};
