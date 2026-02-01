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

#include "autosourcecb.h"
#include "autosourcedsb.h"

#include <QCheckBox>
#include <QStackedLayout>

class AutoSourceListNum : public QWidget, public AutoSource{

  Q_OBJECT

  public:
    explicit AutoSourceListNum(QWidget * parent = nullptr);
    virtual ~AutoSourceListNum();

    //virtual QSize sizeHint() const;

    virtual void updateValue() override;

    void setField(RawSource * field, CompoundItemModelFactory * itemModels, int filter,
                  bool isAvailable = true,
                  RawSource dflt = RawSource(),
                  QString typeLabel = "",
                  int min = -1024, int max = 1024,
                  int precision = 0,
                  QString prefix = "", QString suffix = "",
                  GenericPanel * panel = nullptr);

    void setField(RawSource * field, AbstractItemModel * itemModel, int filter,
                  bool isAvailable = true,
                  RawSource dflt = RawSource(),
                  QString typeLabel = "",
                  int min = -1024, int max = 1024,
                  int precision = 0,
                  QString prefix = "", QString suffix = "",
                  GenericPanel * panel = nullptr);

  signals:
    void resized();

  protected slots:
    void typeChanged(int state);
    void setVisible(bool state);

  private:
    QCheckBox *m_chkType;
    AutoSourceCB *m_sourceCB;
    AutoSourceDSB *m_sourceDSB;
    QStackedLayout *m_stack;
    void shrink();
};
