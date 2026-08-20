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
#include <QJsonDocument>
#include <QJsonObject>

class JsonBase {

  Q_DECLARE_TR_FUNCTIONS(JsonBase)

  public:
    explicit JsonBase() {}
    virtual ~JsonBase() {}

    static bool load(QJsonDocument * doc, const QString & path);

    static const QVariant getValue(const QJsonObject & obj, const QString & name,
                                   const QVariant & dflt = QVariant());
    static const bool getValueBool(const QJsonObject & obj, const QString & name,
                                   const bool dflt = false);
    static const int getValueInt(const QJsonObject & obj, const QString & name,
                                 const int dflt = 0, const int max = 99, const int min = 0);
    static const QString getValueString(const QJsonObject & obj, const QString & name,
                                        const QString & dflt = QString());

    static const bool isArray(const QJsonObject & obj, const QString & name);
    static const bool isObject(const QJsonObject & obj, const QString & name);

};
