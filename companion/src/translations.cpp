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

#include "translations.h"
#include "appdata.h"

#include <QCoreApplication> 
#include <QDir>
#include <QLibraryInfo>
#include <QTranslator>

// List of available Companion translations
// todo: make dynamic eg. from directory listing
// todo: make locale name country-agnostic unless translation is specifically for a country (like zh_CN)
QStringList const Translations::getAvailableTranslations()
{
  static QStringList locales;

  if (!locales.size()) {
    locales << "cs_CZ"
            << "da_DA"
            << "de_DE"
            << "en"
            << "es_ES"
            << "fi_FI"
            << "fr_FR"
            << "he_IL"
            << "it_IT"
            << "nl_NL"
            << "pl_PL"
            << "pt_PT"
            << "ru_RU"
            << "sv_SE"
            << "zh_CN"
            << "zh_TW"
            << "ja_JP" ;
  }
  return locales;
}

QStringList const Translations::getAvailableLanguages()
{
  static QStringList languages;
  if (!languages.size()) {
    foreach (const QString & loc, getAvailableTranslations())
      languages.append(loc.left(2));
  }
  return languages;
}

QStringList const Translations::getTranslationPaths()
{
  // Look for translation files in the following locations, in order of priority
  QStringList paths;

  // Prefer path set in environment variable, makes it possible to test/replace translations w/out rebuilding.
  if (qEnvironmentVariableIsSet("OPENTX_APP_TRANSLATIONS_PATH") && QDir(qgetenv("OPENTX_APP_TRANSLATIONS_PATH").constData()).exists()) {
    paths << qgetenv("OPENTX_APP_TRANSLATIONS_PATH").constData();
  }
  // Try application subfolder first, also eg. to test/replace translations quickly.
  paths << APP_TRANSLATIONS_FILE_PATH;
  // Then the resource file
  paths << APP_TRANSLATIONS_RESOURCE_PATH;
  // Finally the system folder (more likely for Qt translations than Companion ones)
  paths << QLibraryInfo::location(QLibraryInfo::TranslationsPath);

  return paths;
}

void Translations::installTranslators()
{
  Q_INIT_RESOURCE(translations);

  static QList<QTranslator *> appTranslators;

  // Determine the locale

  QLocale locale;  // defaults to system locale
  if (!g.locale().isEmpty()) {
    locale = QLocale(g.locale());  // reverts to "C" locale if invalid
    if (locale.language() == QLocale::C) {
      // reset
      locale = QLocale::system();
      g.locale("");
    }
  }
  qDebug() << "Locale name:" << locale.name() << "language:" << locale.nativeLanguageName() << "country:" << locale.nativeCountryName();

  // Remove any existing translators, this lets us re-translate w/out restart.
  foreach (QTranslator * t, appTranslators) {
    if (t) {
      QCoreApplication::removeTranslator(t);
      delete t;
    }
  }
  appTranslators.clear();

  /* Multiple translation files can be installed.
   * Translations are searched for in the reverse order from which they were installed (last added is searched first).
   * The search stops as soon as a translation containing a matching string is found.
   */

  // Look for translation files in several locations
  QStringList tryPaths = getTranslationPaths();

  // First try to install Qt translations for common GUI elements.

  QStringList qtFiles = QStringList() << "qt";
  // After Qt5.3 some translation files are broken up into modules. We only need "qtbase" for now.
#if (QT_VERSION >= QT_VERSION_CHECK(5, 3, 0))
  qtFiles << "qtbase";
#endif

  foreach (const QString & qtfile, qtFiles) {
    QTranslator * translator = new QTranslator(qApp);
    if (tryLoadTranslator(translator, locale, qtfile, tryPaths)) {
      appTranslators.append(translator);
    }
    else {
      delete translator;
      //qDebug() << "Could not find Qt translations for:" << locale.name() << "In file:" << qtfile << "Using paths:" << tryPaths;
    }
  }

  // Now try to install our custom translations if we support the current locale/language.
  if (!getAvailableTranslations().contains(locale.name()) && !getAvailableLanguages().contains(locale.name().left(2)))
    return;

  QTranslator * translator = new QTranslator(qApp);
  if (tryLoadTranslator(translator, locale, "companion", tryPaths)) {
    appTranslators.append(translator);
  }
  else {
    delete translator;
    qWarning() << "Could not find Companion translations for:" << locale.name() << "Using paths:" << tryPaths;
  }
}

bool Translations::tryLoadTranslator(QTranslator * t, const QLocale & locale, const QString & baseFile, const QStringList & paths)
{
  foreach (const QString & path, paths) {
    if (t->load(locale, baseFile, "_", path)) {
      if (QCoreApplication::installTranslator(t)) {
        qDebug() << "Installed translation file" << baseFile << "from" << path << "for" << locale.name();
        return true;
      }
      else {
        qWarning() << "Error installing translation file" << baseFile << "for:" << locale.name() << "from:" << path;
        return false;
      }
    }
  }
  return false;
}
