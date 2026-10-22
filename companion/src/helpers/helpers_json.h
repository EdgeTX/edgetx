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

#define JSON_KEY_VAL(jsonit)   qInfo() << "key:" << jsonit.key() << "value:" << jsonit.value()
#define JSON_NO_RULE(jsonit)   qWarning() << "Warning: No rule to process - key:" << jsonit.key() << "value:" << jsonit.value()
#define JSON_NOT_ARR(jsonit) qCritical() << "Error: not an array" << *jsonit
#define JSON_NOT_OBJ(jsonit)   qCritical() << "Error: not an object" << *jsonit

#define JSON_ARR_INIT(jsonit, tmparray, action) \
  if (!jsonit->isArray()) { \
    JSON_NOT_ARR(jsonit); \
    action; \
  } \
  const QJsonArray &tmparray = jsonit->toArray();

#define JSON_OBJ_INIT(jsonit, tmpobj, action) \
  if (!jsonit->isObject()) { \
    JSON_NOT_OBJ(jsonit); \
    action; \
  } \
  const QJsonObject &tmpobj = jsonit->toObject();

#define JSON_ARR_INIT_C(jsonit, tmparray) JSON_ARR_INIT(jsonit, tmparray, continue)
#define JSON_ARR_INIT_R(jsonit, tmparray) JSON_ARR_INIT(jsonit, tmparray, return)
#define JSON_OBJ_INIT_C(jsonit, tmpobj) JSON_OBJ_INIT(jsonit, tmpobj, continue)
#define JSON_OBJ_INIT_R(jsonit, tmpobj) JSON_OBJ_INIT(jsonit, tmpobj, return)

class Json {

  Q_DECLARE_TR_FUNCTIONS(Json)

  public:

    struct range_t {
      int min  {0};
      int max  {0};
      int dflt {0};

      range_t() = default;
    };

    struct sizes_t {
      int cnt  {0};
      int name {0};

      sizes_t() = default;
    };

    explicit Json() {}
    virtual ~Json() {}

    static bool load(QJsonDocument * doc, const QString & path);

    static const bool exists(const QJsonObject & obj, const QString & key);

    static void loadRange(QJsonObject::const_iterator & it, range_t & range);

    static void loadSizes(QJsonObject::const_iterator & it, sizes_t & sizes,
                          const int cntMax = 99999, const int nameMax = 100,
                          const int cntMin = 0, const int nameMin = 0);

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
