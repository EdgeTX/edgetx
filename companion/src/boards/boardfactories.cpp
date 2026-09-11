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
  if (registerBoard(Board::BOARD_UNKNOWN, "", ""))
    m_default = boardForId(Board::BOARD_UNKNOWN);
}

BoardFactories::~BoardFactories()
{
  unregisterBoardFactories();
}

Boards * BoardFactories::boardForHwDefn(const QString & hwdefn) const
{
  for (auto *registeredFactory : registeredBoardFactories) {
    if (registeredFactory->board()->hwdefn() == hwdefn)
      return registeredFactory->board();
  }

  return m_default;
}

Boards * BoardFactories::boardForId(const Board::Type & id) const
{
  for (auto *registeredFactory : registeredBoardFactories) {
    if (registeredFactory->board()->id() == id)
      return registeredFactory->board();
  }

  return m_default;
}

//  Registering firmware triggers registering the associated board
bool BoardFactories::registerBoard(const Board::Type & id, const QString & hwdefn, const QString & bddefn)
{
  if (m_default || id != Board::BOARD_UNKNOWN) {
    Boards* regboard = boardForId(id);

    if (regboard->id() == id) {
      if (regboard->hwdefn() == hwdefn) {
        //qDebug() << "Warning - Board" << Boards::getBoardName(regboard->board()) << "already registered";
        return true;
      }
      else {
        qDebug() << "Error - Board" << regboard->name() << "already registered with"
                 << regboard->hwdefn() << "hwdefn!";
        return false;
      }
    }
  }

  BoardFactory *bf = new BoardFactory(id, hwdefn, bddefn);

  if (bf->board()->loadDefinitions()) {
    if (registerBoardFactory(bf)) {
      qDebug() << "Registered board:" << (id != Board::BOARD_UNKNOWN ? bf->board()->name() : "UNKNOWN (default)");
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
