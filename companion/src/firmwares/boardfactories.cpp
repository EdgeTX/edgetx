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

#include "boardfactories.h"

BoardFactories* gBoardFactories = nullptr;

BoardFactories::BoardFactories()
{
  registerBoardFactories();
}

BoardFactories::~BoardFactories()
{
  unregisterBoardFactories();
}

bool BoardFactories::registerBoardFactory(BoardFactory * factory)
{
  for (auto *registeredFactory : registeredBoardFactories) {
    if (registeredFactory->instance()->board() == factory->instance()->board()) {
      qDebug() << "Error - Duplicate board factory:" << Boards::getBoardName(factory->instance()->board());
      return false;
    }
  }

  registeredBoardFactories.append(factory);
  return true;
}

void BoardFactories::registerBoardFactories()
{
  QStringList regList;

  for (int i = Board::BOARD_UNKNOWN; i < Board::Type::BOARD_TYPE_COUNT; i++) {
    BoardFactory *bf = new BoardFactory((Board::Type)i);
    if (bf->instance()->loadDefinition()) {
      if (registerBoardFactory(bf))
        regList.append(Boards::getBoardName((Board::Type)i));
      else
        delete bf;
    }
    else
      delete bf;
  }

  m_default = instance(Board::BOARD_UNKNOWN);

//  qDebug() << "Registered board factories:" << regList;
}

void BoardFactories::unregisterBoardFactories()
{
  for (auto *registeredFactory : registeredBoardFactories)
    delete registeredFactory;
}

BoardJson* BoardFactories::instance(Board::Type board) const
{
  for (auto *registeredFactory : registeredBoardFactories) {
    if (registeredFactory->instance()->board() == board)
      return registeredFactory->instance();
  }

  return m_default;
}
