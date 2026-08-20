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

#include <string>

class Json {

  Q_DECLARE_TR_FUNCTIONS(Json)

  public:
    explicit Json() {}
    virtual ~Json() {}

    static bool load(QJsonDocument * doc, const QString & path);

    static const bool exists(const QJsonObject & obj, const QString & key);

    static const QVariant value(const QJsonObject & obj, const QString & key,
                                const bool manditory = true, const QVariant & dflt = QVariant());

    static const bool valueBool(const QJsonObject::const_iterator & it,
                                const bool dflt = false);
    static const bool valueBool(const QJsonObject & obj, const QString & key,
                                const bool dflt = false);

    static const int valueInt(const QJsonObject::const_iterator & it,
                              const int dflt = 0, const int max = 999999, const int min = 0);
    static const int valueInt(const QJsonObject & obj, const QString & key,
                              const int dflt = 0, const int max = 999999, const int min = 0);

    static const std::string valueStdString(const QJsonObject::const_iterator & it,
                                            const std::string & dflt = std::string());
    static const std::string valueStdString(const QJsonObject & obj, const QString & key,
                                            const std::string & dflt = std::string());

    static const QString valueString(const QJsonObject::const_iterator & it,
                                     const QString & dflt = QString());
    static const QString valueString(const QJsonObject & obj, const QString & key,
                                     const QString & dflt = QString());

    static const bool isArray(const QJsonObject & obj, const QString & key);
    static const bool isObject(const QJsonObject & obj, const QString & key);
};
