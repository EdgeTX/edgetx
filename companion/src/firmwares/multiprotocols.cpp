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

#include <QObject>

#include "multiprotocols.h"
#include "radiodata.h"
#include "compounditemmodels.h"

#define CPN
#include "radio/src/MultiSubtypeDefs.h"
#undef CPN

const Multiprotocols::MultiProtocolDefinition & Multiprotocols::getProtocol(int protocol) const {
  for (const Multiprotocols::MultiProtocolDefinition & proto: protocols)
    if (proto.protocol == protocol)
      return proto;

  // Protocol not found, return the sentinel proto
  Q_ASSERT(protocols.rbegin()->protocol == MODULE_SUBTYPE_MULTI_SENTINEL);
  return *protocols.rbegin();
}

// static
QString Multiprotocols::protocolToString(int protocol)
{
  static const QStringList strings { PROTO_NAMES };

  return strings.value(protocol, CPN_STR_UNKNOWN_ITEM);
}

// static
QString Multiprotocols::subTypeToString(int protocol, unsigned int subType)
{
  if (protocol > MODULE_SUBTYPE_MULTI_LAST)
    return tr(qPrintable(QString::number(subType)));
  else
    return tr(qPrintable(multiProtocols.getProtocol(protocol).subTypeStrings.value(subType, CPN_STR_UNKNOWN_ITEM)));
}

//  static
AbstractStaticItemModel * Multiprotocols::protocolItemModel()
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName("multiprotocols.protocol");

  for (int i = 0; i <= MODULE_SUBTYPE_MULTI_LAST; i++) {
    if (isMultiProtocolSelectable(i))
      mdl->appendToItemList(protocolToString(i), i);
  }

  mdl->loadItemList();
  mdl->sort(0);
  return mdl;
}
