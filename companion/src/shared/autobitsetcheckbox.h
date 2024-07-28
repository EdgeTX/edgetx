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

#include "autowidget.h"

#include <QCheckBox>

class AutoBitsetCheckBox: public QCheckBox, public AutoWidget
{
    Q_OBJECT

  public:
    explicit AutoBitsetCheckBox(QWidget * parent = nullptr);
    explicit AutoBitsetCheckBox(const QString &text, QWidget * parent = nullptr);

    // int field constructors
    explicit AutoBitsetCheckBox(int & field, int bitmask, const QString & text = QString(), QWidget * parent = nullptr);
    explicit AutoBitsetCheckBox(int & field, int bitmask, bool invert, const QString & text = QString(), QWidget * parent = nullptr);
    explicit AutoBitsetCheckBox(int & field, int bitmask, int toggleMask, const QString & text = QString(), QWidget * parent = nullptr);

    // unsigned field constructors
    explicit AutoBitsetCheckBox(unsigned & field, int bitmask, const QString & text = QString(), QWidget * parent = nullptr);
    explicit AutoBitsetCheckBox(unsigned & field, int bitmask, bool invert, const QString & text = QString(), QWidget * parent = nullptr);
    explicit AutoBitsetCheckBox(unsigned & field, int bitmask, int toggleMask, const QString & text = QString(), QWidget * parent = nullptr);

    virtual ~AutoBitsetCheckBox();

    virtual void updateValue() override;

    void setField(int & field, int bitmask = 0, bool invert = false, int toggleMask = 0);
    void setField(unsigned & field, int bitmask = 0, bool invert = false, int toggleMask = 0);
    void setBitmask(int bitmask);
    void setToggleMask(int toggleMask);
    void setInverted(bool invert);

  signals:
    void currentDataChanged(bool checked);

  protected slots:
    void onToggled(bool checked);

  private:
    int *m_field = nullptr;
    int m_bitmask = 0;
    int m_toggleMask = 0;
    bool m_invert = false;

    void init();
    void initField(int bitmask, bool invert, int toggleMask);
};
