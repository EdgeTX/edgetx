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

BoardFactory::BoardFactory(const Board::Type & id, const QString & hwdefn, const bool isSupported) :
  m_board(new Boards(id, hwdefn, isSupported))
{

}

  BoardFactories::BoardFactories() :
  m_default(nullptr)
{
  registerAllBoards();
}

BoardFactories::~BoardFactories()
{
  unregisterBoardFactories();
}

Boards * BoardFactories::boardForHwDefn(const QString & hwdefn) const
{
  for (auto *registeredFactory : registeredBoardFactories) {
    auto board = registeredFactory->board();
    if (board->getHwDefn() == hwdefn) {
      if (!board->isLoaded())
        board->loadDefinitions();

      return board;
    }
  }

  return m_default;
}

Boards * BoardFactories::boardForId(const Board::Type & id) const
{
  for (auto *registeredFactory : registeredBoardFactories) {
    auto board = registeredFactory->board();
    if (board->getId() == id) {
      if (!board->isLoaded())
        board->loadDefinitions();

      return board;
    }
  }

  return m_default;
}

//  Registering firmware triggers registering the associated board
bool BoardFactories::registerBoard(const Board::Type & id, const QString & hwdefn, const bool isSupported)
{
  if (m_default || id != Board::BOARD_UNKNOWN) {
    Boards* regboard = boardForId(id);

    if (regboard->getId() == id) {
      if (regboard->getHwDefn() == hwdefn) {
        //qDebug() << "Warning - Board" << Boards::getBoardName(regboard->board()) << "already registered";
        return true;
      }
      else {
        qDebug() << "Error - Board" << regboard->getName() << "already registered with"
                 << regboard->getHwDefn() << "hwdefn!";
        return false;
      }
    }
  }

  BoardFactory *bf = new BoardFactory(id, hwdefn, isSupported);

  if (bf->board()->loadDefinitions()) {
    if (registerBoardFactory(bf)) {
      qDebug() << "Registered board:" << (id != Board::BOARD_UNKNOWN ? bf->board()->getName() : "UNKNOWN (default)");
      return true;
    }
    else
      delete bf;
  }
  else
    delete bf;

  return false;
}

void BoardFactories::registerAllBoards()
{
  QStringList filters = { "*.json" };

  QDirIterator it(QString("%1/").arg(HWDEFNSDIR), filters, QDir::Files);

  while (it.hasNext()) {
    QString path = it.next();
    //qDebug() << "found file:" << path;
    QJsonDocument *doc = new QJsonDocument();

    if (Boards::load(doc, path)) {
      QJsonObject obj = doc->object();
      // ignore intermediate definitions
      if (!Boards::getValueBool(obj, "hidden", false)) {
        QString id = Boards::getValueString(obj, "id", QFileInfo(path).baseName());
        registerBoard(id, path, Boards::getValueBool(obj, "supported", true));
      } else {
        //qDebug() << "ignoring file:" << path;
      }
    }

    delete doc;
  }
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
