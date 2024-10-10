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

BoardFactories::BoardFactories() :
  m_default(nullptr)
{
  if (registerBoard(Board::BOARD_UNKNOWN, ""))
    m_default = instance(Board::BOARD_UNKNOWN);
}

BoardFactories::~BoardFactories()
{
  unregisterBoardFactories();
}

BoardJson* BoardFactories::instance(Board::Type board) const
{
  for (auto *registeredFactory : registeredBoardFactories) {
    if (registeredFactory->instance()->board() == board)
      return registeredFactory->instance();
  }

  return m_default;
}

//  Registering firmware triggers registering the associated board
bool BoardFactories::registerBoard(Board::Type board, QString hwdefn)
{
  if (board < Board::BOARD_UNKNOWN || board >= Board::BOARD_TYPE_COUNT)
    return false;

  if (m_default || board != Board::BOARD_UNKNOWN) {
    BoardJson* regboard = instance(board);

    if (regboard->board() == board) {
      if (regboard->hwdefn() == hwdefn) {
        //qDebug() << "Warning - Board" << Boards::getBoardName(regboard->board()) << "already registered";
        return true;
      }
      else {
        qDebug() << "Error - Board" << Boards::getBoardName(regboard->board()) << "already registered with"
                 << regboard->hwdefn() << "hwdefn!";
        return false;
      }
    }
  }

  BoardFactory *bf = new BoardFactory(board, hwdefn);
  if (bf->instance()->loadDefinition()) {
    if (registerBoardFactory(bf)) {
      qDebug() << "Registered board:" << (board != Board::BOARD_UNKNOWN ? Boards::getBoardName(board) : "UNKNOWN (default)");
      return true;
    }
    else
      delete bf;
  }
  else
    delete bf;

  return false;
}

bool BoardFactories::registerBoardFactory(BoardFactory * factory)
{
  registeredBoardFactories.append(factory);
  return true;
}

void BoardFactories::unregisterBoardFactories()
{
  for (auto *registeredFactory : registeredBoardFactories)
    delete registeredFactory;
}
