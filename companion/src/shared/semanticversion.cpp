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

#include "semanticversion.h"
#include "helpers.h"

SemanticVersion::SemanticVersion(const QString vers)
{
  fromString(vers);
}

bool SemanticVersion::fromString(QString vers)
{
  if (!isValid(vers))
    return false;

  vers = vers.trimmed();

  if (vers.toLower().startsWith("v"))
    vers = vers.mid(1);

  QStringList strl = vers.split(".");
  version.major = strl.at(0).toInt();
  version.minor = strl.at(1).toInt();

  if (strl.count() > 2) {
    if (!strl.at(2).contains("-")) {
      version.patch = strl.at(2).toInt();
    } else {
      QStringList ptch = strl.at(2).toLower().split("-");
      version.patch = ptch.at(0).toInt();

      int offset = 0;
      QString relType;

      for (int i = 0; i < ptch.at(1).size(); i++) {
        QString c(ptch.at(1).mid(i, 1));
        if (c >= "0" && c <= "9") {
          break;
        } else if (c == ".") {
          offset++;
          break;
        }

        offset++;
        relType.append(c);
      }

      version.preReleaseType = preReleaseTypeToInt(relType);

      if (version.preReleaseType > -1 && offset < ptch.at(1).size())
        version.preReleaseNumber = ptch.at(1).mid(offset).toInt();
      else
        version.preReleaseType = PR_NONE;
    }
  }

  //qDebug() << "vers:" << vers << "toString:" << toString() << "toInt:" << toInt();

  return true;
}

SemanticVersion& SemanticVersion::operator=(const SemanticVersion& rhs)
{
  version.major = rhs.version.major;
  version.minor = rhs.version.minor;
  version.patch = rhs.version.patch;
  version.preReleaseType = rhs.version.preReleaseType;
  version.preReleaseNumber = rhs.version.preReleaseNumber;
  return *this;
}

bool SemanticVersion::isValid(const QString vers)
{
  QString v(vers.trimmed());

  if (v.isEmpty())
    return false;

  if (v.toLower().startsWith("v"))
    v = v.mid(1);

#if 0
  //  Keep for testing full standard
  //  Note: regexp adapted for Qt ie extra escaping
  QRegularExpression rx1("^(0|[1-9]\\d*)\\.(0|[1-9]\\d*)\\.(0|[1-9]\\d*)(?:-((?:0|[1-9]\\d*|\\d*[a-zA-Z-][0-9a-zA-Z-]*)(?:\\.(?:0|[1-9]\\d*|\\d*[a-zA-Z-][0-9a-zA-Z-]*))*))?(?:\\+([0-9a-zA-Z-]+(?:\\.[0-9a-zA-Z-]+)*))?$");

  //  Qt only test as not all patterns supported and can change in later releases
  if (!rx1.isValid()) {
    qDebug() << "Full standard is an invalid Qt regular expression";
    return false;
  }

  if (!rx1.match(v).hasMatch()) {
    qDebug() << vers << "is not a valid Semantic Version - ";
    return false;
  }
#endif // 0

  //  we only support a subset of the standard alpha, beta, rc with period optional and number optional
  //  format: major.minor.patch[-[alpha|beta|rc][.|][n|]]

  QRegularExpression rx2("^(0|[1-9]\\d*)\\.(0|[1-9]\\d*)\\.(0|[1-9]\\d*)(?:-(alpha|beta|rc)\\.?(0|[1-9]\\d*)?)*$");

  //  Qt only test as not all patterns supported and can change in later releases
  if (!rx2.isValid()) {
    qDebug() << "Standard subset is an invalid Qt regular expression";
    return false;
  }

  if (!rx2.match(v.toLower()).hasMatch()) {
    //qDebug() << vers << "is not a valid Semantic Version subset - ";
    return false;
  }

  return isValid();
}

bool SemanticVersion::isValid()
{
  //  range checks to support 32 bit OS when components compounded
  if (version.major < 0 || version.major > 255 || version.minor < 0 || version.minor > 255 || version.patch < 0 || version.patch > 255 ||
      version.preReleaseType < 0 || version.preReleaseType > PR_NONE || version.preReleaseNumber < 0 || version.preReleaseNumber > 15) {
    qDebug() << "Cannot convert to supported Semantec Version";
    version = SemanticVersion().version;
    return false;
  }

  return true;
}

QString SemanticVersion::toString() const
{
  QString ret(QString("%1.%2.%3").arg(version.major).arg(version.minor).arg(version.patch));

  if (version.preReleaseType != PR_NONE) {
    ret = QString("%1-%2").arg(ret).arg(preReleaseTypeToString());
    if (version.preReleaseNumber > 0)
      ret = QString("%1.%2").arg(ret).arg(version.preReleaseNumber);
  }

  return ret;
}

bool SemanticVersion::isEmpty(const QString vers)
{
  fromString(vers);
  return isEmpty();
}

bool SemanticVersion::isEmpty()
{
  if (toInt() == SemanticVersion().toInt() )
    return true;
  else
    return false;
}

bool SemanticVersion::isPreRelease(const QString vers)
{
  fromString(vers);
  return isPreRelease();
}

bool SemanticVersion::isPreRelease()
{
  if (version.preReleaseType != PR_NONE)
    return true;
  else
    return false;
}

int SemanticVersion::compare(const SemanticVersion& other)
{
  if (version.major != other.version.major) {
    return version.major - other.version.major;
  }

  if (version.minor != other.version.minor) {
    return version.minor - other.version.minor;
  }

  if (version.patch != other.version.patch) {
    return version.patch - other.version.patch;
  }

  if (version.preReleaseType != other.version.preReleaseType) {
    return version.preReleaseType - other.version.preReleaseType;
  }

  if (version.preReleaseNumber != other.version.preReleaseNumber) {
    return version.preReleaseNumber - other.version.preReleaseNumber;
  }

  return 0;
}

unsigned int SemanticVersion::toInt() const
{
  //  limit to 32 bits for OS backward compatibility
  unsigned int val = 0;
  Helpers::setBitmappedValue(val, version.major, 0, 8, 24);
  Helpers::setBitmappedValue(val, version.minor, 0, 8, 16);
  Helpers::setBitmappedValue(val, version.patch, 0, 8, 8);
  Helpers::setBitmappedValue(val, version.preReleaseType, 0, 4, 4);
  Helpers::setBitmappedValue(val, version.preReleaseNumber, 0, 4);
  return val;
}

bool SemanticVersion::fromInt(const unsigned int val)
{
  //  assumption val was generated by toInt() but validate anyway
  version.major = Helpers::getBitmappedValue(val, 0, 8, 24);
  version.minor = Helpers::getBitmappedValue(val, 0, 8, 16);
  version.patch = Helpers::getBitmappedValue(val, 0, 8, 8);
  version.preReleaseType = Helpers::getBitmappedValue(val, 0, 4, 4);
  version.preReleaseNumber = Helpers::getBitmappedValue(val, 0, 4);
  return isValid();
}
