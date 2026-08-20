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

#include "board.h"

class BoardFactories;

class BoardFactory
{
  protected:
    // only allow BoardFactories to instantiate
    friend BoardFactories;

    explicit BoardFactory(const QString & id);
    virtual ~BoardFactory() {}

    Board * board() const { return m_board; }

  private:
    Board *m_board;
};

class BoardFactories
{
  public:
    // Delete copy constructor and assignment operators to prevent duplicates
    BoardFactories(const BoardFactories&) = delete;
    BoardFactories& operator=(const BoardFactories&) = delete;
    BoardFactories(BoardFactories&&) = delete;
    BoardFactories& operator=(BoardFactories&&) = delete;

    // Public static method acting as the single access point
    static BoardFactories& getInstance() {
        // Guaranteed to be destroyed and instantiated thread-safely on first use
        static BoardFactories instance;
        return instance;
    }

    Board * getBoard(const QString & id, bool forceLoad = true) const;
    bool isAvailable(const QString & id) const;

  private:
    explicit BoardFactories();
    virtual ~BoardFactories();

    Board *m_default;

    QList<BoardFactory *> registeredBoardFactories;

    void registerAllBoards();
    bool registerBoard(const QString & id);
};
