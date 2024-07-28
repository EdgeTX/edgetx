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

#include <vector>
#include <QtCore>
#include "moduledata.h"

class AbstractStaticItemModel;

class Multiprotocols
{
    Q_DECLARE_TR_FUNCTIONS(Multiprotocols)

  public:

    struct radio_mm_definition {
      int protocol;
      unsigned int maxSubtype;
      bool hasFailsafe;
      bool disableChannelMap;
      QStringList protocols;
      QString optionsstr;
    };

    struct MultiProtocolDefinition {
      const int protocol;
      const bool hasFailsafe;
      const bool disableChannelMap;
      const QStringList subTypeStrings;
      const QString optionsstr;

      unsigned int numSubTypes() const
      {
        return protocol > MODULE_SUBTYPE_MULTI_LAST ? 8 : (unsigned int) subTypeStrings.length();
      }

      MultiProtocolDefinition(const radio_mm_definition &rd) :
        protocol(rd.protocol),
        hasFailsafe(rd.hasFailsafe),
        disableChannelMap(rd.disableChannelMap),
        subTypeStrings(rd.protocols),
        optionsstr(rd.optionsstr)
      {
        Q_ASSERT(rd.maxSubtype + 1 == (unsigned int) rd.protocols.length());
      }
    };

    Multiprotocols(std::initializer_list<radio_mm_definition> l)
    {
      for (radio_mm_definition rd: l)
        protocols.push_back(MultiProtocolDefinition(rd));
    }

    const MultiProtocolDefinition &getProtocol(int protocol) const;
    static QString protocolToString(int protocol);
    static QString subTypeToString(int protocol, unsigned int subType);
    static AbstractStaticItemModel * protocolItemModel();

  private:

    std::vector<MultiProtocolDefinition> protocols;

};

extern const Multiprotocols multiProtocols;
