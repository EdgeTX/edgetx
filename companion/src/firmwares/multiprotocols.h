/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#ifndef OPENTX_MULTI_H
#define OPENTX_MULTI_H

#include <vector>
#include <QtCore>
#include "moduledata.h"

#define MM_RF_CUSTOM_SELECTED 0xff

class Multiprotocols
{
    Q_DECLARE_TR_FUNCTIONS(Multiprotocols)

  public:

    struct radio_mm_definition {
      int protocol;
      unsigned int maxSubtype;
      bool hasFailsafe;
      QStringList protocols;
      QString optionsstr;
    };

    struct MultiProtocolDefinition {
      const int protocol;
      const bool hasFailsafe;
      const QStringList subTypeStrings;
      const QString optionsstr;

      unsigned int numSubTypes() const
      {
        return protocol > MODULE_SUBTYPE_MPM_LAST ? 8 : (unsigned int) subTypeStrings.length();
      }

      int getOptionMin() const;

      int getOptionMax() const;

      MultiProtocolDefinition(const radio_mm_definition &rd) :
        protocol(rd.protocol),
        hasFailsafe(rd.hasFailsafe),
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
    static QString protocolToString(int protocol, bool custom = false);
    static QString subTypeToString(int protocol, unsigned subType);

  private:

    std::vector<MultiProtocolDefinition> protocols;

};

extern const Multiprotocols multiProtocols;

#endif //OPENTX_MULTI_H
