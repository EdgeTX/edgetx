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
    m_default = board(Board::BOARD_UNKNOWN);
}

BoardFactories::~BoardFactories()
{
  unregisterBoardFactories();
}

Boards* BoardFactories::board(const Board::Type & board) const
{
  for (auto *registeredFactory : registeredBoardFactories) {
    if (registeredFactory->board()->boardType() == board)
      return registeredFactory->board();
  }

  return m_default;
}

Boards* BoardFactories::board(const QString & hwdefn) const
{
  for (auto *registeredFactory : registeredBoardFactories) {
    if (registeredFactory->board()->hwdefn() == hwdefn)
      return registeredFactory->board();
  }

  return m_default;
}

//  Registering firmware triggers registering the associated board
bool BoardFactories::registerBoard(const Board::Type & boardType, const QString & hwdefn)
{
  if (m_default || boardType != Board::BOARD_UNKNOWN) {
    Boards* regboard = board(boardType);

    if (regboard->boardType() == boardType) {
      if (regboard->hwdefn() == hwdefn) {
        //qDebug() << "Warning - Board" << Boards::getBoardName(regboard->board()) << "already registered";
        return true;
      }
      else {
        qDebug() << "Error - Board" << Boards::getBoardName(regboard->boardType()) << "already registered with"
                 << regboard->hwdefn() << "hwdefn!";
        return false;
      }
    }
  }

  BoardFactory *bf = new BoardFactory(boardType, hwdefn);

  if (bf->board()->loadDefinition()) {
    if (registerBoardFactory(bf)) {
      qDebug() << "Registered board:" << (boardType != Board::BOARD_UNKNOWN ? bf->board()->name() : "UNKNOWN (default)");
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
