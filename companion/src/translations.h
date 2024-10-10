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

#ifndef TRANSLATIONS_H
#define TRANSLATIONS_H

#include <QString>

// Where to find translation files?
// First any path in environment variable OPENTX_APP_TRANSLATIONS_PATH is used if it exists.

// Default location for .qm files when NOT compiled into application (this is searched after OPENTX_APP_TRANSLATIONS_PATH).
#ifndef APP_TRANSLATIONS_FILE_PATH
  #define APP_TRANSLATIONS_FILE_PATH        QCoreApplication::applicationDirPath() % "/translations"
#endif
\
// Default location for .qm files when compiled into application as a resource (this is searched after APP_TRANSLATIONS_FILE_PATH) .
#ifndef APP_TRANSLATIONS_RESOURCE_PATH
  #define APP_TRANSLATIONS_RESOURCE_PATH    ":/translations"
#endif

class QLocale;
class QTranslator;

class Translations
{
  public:
    static QStringList const getTranslationPaths();
    static QStringList const getAvailableTranslations();
    static QStringList const getAvailableLanguages();

    static void installTranslators();

  protected:
    static bool tryLoadTranslator(QTranslator * t, const QLocale & locale, const QString & baseFile, const QStringList & paths);
};

#endif // TRANSLATIONS_H
