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
#include "helpers_json.h"

BoardFactory::BoardFactory(const QString & id) :
  m_board(new Board(id))
{
}

BoardFactories::BoardFactories() :
  m_default(nullptr)
{
  registerAllBoards();
}

BoardFactories::~BoardFactories()
{
  //qDebug() << "Unregister all board factories";
  for (auto *registeredFactory : registeredBoardFactories)
    delete registeredFactory;
}

Board * BoardFactories::getBoard(const QString & id, bool forceLoad) const
{
  for (auto *registeredFactory : registeredBoardFactories) {
    auto board = registeredFactory->board();

    if (board->getId() == id) {
      if (!board->isLoaded() && forceLoad)
        board->loadDefinition();

      return board;
    }
  }

  qDebug() << "Error: board id:" << id << "not registered";
  return m_default;
}

bool BoardFactories::isAvailable(const QString & id) const
{
  for (auto *registeredFactory : registeredBoardFactories) {
    auto board = registeredFactory->board();
    if (board->getId() == id) {
      return true;
    }
  }

  return false;
}

bool BoardFactories::registerBoard(const QString & id)
{
  if (isAvailable(id)) {
    auto regboard = getBoard(id);
    qDebug() << "Error - Board id:" << id << "name:" << regboard->getName()
             << "already registered";
    return false;
  }

  BoardFactory *factory = new BoardFactory(id);

  registeredBoardFactories.append(factory);
  //qDebug() << "Registered board:" << (id != Board::BOARD_UNKNOWN ? factory->board()->getName() : "UNKNOWN (default)");
  return true;
}

void BoardFactories::registerAllBoards()
{
  QStringList registered;
  QStringList filters = { "*.json" };

  QDirIterator it(QString("%1/").arg(BDDEFNSDIR), filters, QDir::Files);

  while (it.hasNext()) {
    QString path = it.next();
    //qDebug() << "found file:" << path;
    QJsonDocument *doc = new QJsonDocument();

    if (Json::load(doc, path)) {
      QJsonObject obj = doc->object();
      // ignore intermediate definitions
      if (!Json::value(obj, "hidden", false, false).toBool()) {
        if (registerBoard(QFileInfo(path).baseName()))
          registered.append(getBoard(QFileInfo(path).baseName(), false)->getName());
      } else {
        //qDebug() << "Ignoring file:" << path;
      }
    }

    delete doc;
  }

  qDebug() << "Registered boards:" << registered;
}
