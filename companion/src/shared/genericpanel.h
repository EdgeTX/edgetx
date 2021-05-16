/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#include <QWidget>

class ModelData;
class GeneralSettings;
class Firmware;
class QGridLayout;
class QString;

class GenericPanel : public QWidget
{
  Q_OBJECT

  friend class GVarGroup;
  friend class AutoWidget;

  public:
    GenericPanel(QWidget *parent, ModelData * model, GeneralSettings & generalSettings, Firmware * firmware);
    virtual ~GenericPanel();

  signals:
    void modified();

  public slots:
    virtual void update();

  protected:
    ModelData * model;
    GeneralSettings & generalSettings;
    Firmware * firmware;
    bool lock;
    void addLabel(QGridLayout * gridLayout, const QString &text, int col, bool mimimize=false);
    void addEmptyLabel(QGridLayout * gridLayout, int col);
    void addHSpring(QGridLayout *, int col, int row);
    void addVSpring(QGridLayout *, int col, int row);
    void addDoubleSpring(QGridLayout *, int col, int row);
    virtual bool eventFilter(QObject *obj, QEvent *event);
    void setFocusFilter(QWidget * w);
    void disableMouseScrolling();
    void updateAutoWidgets();
};
