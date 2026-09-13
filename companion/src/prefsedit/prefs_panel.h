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

#include "appdata.h"
#include "abstractpanel.h"
#include "eeprominterface.h"
#include "gridlayout.h"

#include <QScrollArea>

class PrefsEditDialog;

// use to help Qt formatting
constexpr int PATH_MIN_WIDTH            {250};
constexpr QSizePolicy PATH_SIZE_POLICY  {QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed)};

class PrefsPanel : public AbstractPanel
{
  Q_OBJECT

  public:
    friend class PrefsEditDialog;

    explicit PrefsPanel(QWidget * parent, Firmware * firmware, Board::Type & board, Profile & profile) ;
    virtual ~PrefsPanel();

    virtual void save() override {};

  signals:
    void radioChanged(Firmware * firmware);

  protected slots:
    virtual void onRadioChanged(Firmware * firmware, bool deferUpdate = false);

  protected:
    Firmware *firmware;
    Board::Type board;
    Profile &profile;
    int row;
    int col;

    AutoLineEdit * newPathWidget(QWidget * parent);
    void setPathWidget(AutoLineEdit * wgt);

};

class PrefsScrollArea : public QScrollArea
{
  Q_OBJECT

  public:
    explicit PrefsScrollArea(QWidget * parent, PrefsPanel * panel);

  protected:
    virtual bool eventFilter(QObject * o, QEvent * e);

  private:
    PrefsPanel * panel;
};
