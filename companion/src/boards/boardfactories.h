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

#include "boards.h"

class BoardFactory
{
  public:
    explicit BoardFactory(const Board::Type & id, const QString & hwdefn, const bool isSupported = true);

    virtual ~BoardFactory() {}

    Boards * board() const { return m_board; }

  private:
    Boards *m_board;
};

class BoardFactories
{
  public:
    explicit BoardFactories();
    virtual ~BoardFactories();

    Boards * boardForId(const Board::Type & id) const;
    Boards * boardForHwDefn(const QString & hwdefn) const;

    void registerAllBoards();
    bool registerBoard(const Board::Type & board, const QString & hwdefn, const bool isSupported = true);
    bool registerBoardFactory(BoardFactory * factory);
    void unregisterBoardFactories();

  private:
    QList<BoardFactory *> registeredBoardFactories;

    Boards *m_default;
};

extern BoardFactories* gBoardFactories;
