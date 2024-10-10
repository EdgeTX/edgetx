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

class AutoBitMappedCheckBox : public QCheckBox, public AutoWidget
{
  Q_OBJECT

  public:
    explicit AutoBitMappedCheckBox(QWidget * parent = nullptr);
    virtual ~AutoBitMappedCheckBox();

    virtual void updateValue() override;

    void setField(int & field, GenericPanel * panel = nullptr, bool invert = false);
    void setField(unsigned int & field, GenericPanel * panel = nullptr, bool invert = false);
    void setInvert(bool invert);
    void setBits(const unsigned int numBits = 1, const unsigned int offsetBits = 0, const unsigned int index = 0,
                 const unsigned int indexBits = 1);

  signals:
    void currentDataChanged(bool value);

  protected slots:
    void onToggled(bool checked);

  private:
    int *m_field;
    bool m_invert;
    unsigned int m_index;
    unsigned int m_indexBits;
    unsigned int m_bits;
    unsigned int m_offsetBits;

    unsigned int shiftbits();
    unsigned int bitmask();
};
