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

//  Based on Semantic Versioning 2.0.0 refer https://semver.org/

#pragma once

#include <QString>
#include <QStringList>

class SemanticVersion
{
  public:
    explicit SemanticVersion(const QString vers);
    explicit SemanticVersion() {}
    ~SemanticVersion() {}

    bool isValid(const QString vers);
    bool isValid();
    bool fromString(const QString vers);
    QString toString() const;
    unsigned int toInt() const;
    bool fromInt(const unsigned int val);
    bool isEmpty(const QString vers);
    bool isEmpty();
    bool isPreRelease(const QString vers);
    bool isPreRelease();

    SemanticVersion& operator=(const SemanticVersion& rhs);

    bool operator==(const SemanticVersion& rhs) {
      return compare(rhs) == 0;
    }

    bool operator!=(const SemanticVersion& rhs) {
      return compare(rhs) != 0;
    }

    bool operator>(const SemanticVersion& rhs) {
      return compare(rhs) > 0;
    }

    bool operator>=(const SemanticVersion& rhs) {
      return compare(rhs) >= 0;
    }

    bool operator<(const SemanticVersion& rhs) {
      return compare(rhs) < 0;
    }

    bool operator<=(const SemanticVersion& rhs) {
      return compare(rhs) <= 0;
    }

  private:
    enum PreReleaseTypes {
      PR_ALPHA = 0,
      PR_BETA,
      PR_RC,
      PR_NONE
    };

    const QStringList PreReleaseTypesStringList = { "alpha", "beta", "rc"};

    struct Version {
      int major            = 0;
      int minor            = 0;
      int patch            = 0;
      int preReleaseType   = PR_NONE;
      int preReleaseNumber = 0;
    };

    Version version;

    int compare(const SemanticVersion& other);
    inline QString preReleaseTypeToString() const { return PreReleaseTypesStringList.value(version.preReleaseType, ""); }
    inline int preReleaseTypeToInt(QString preRelType) const { return PreReleaseTypesStringList.indexOf(preRelType); }

};
