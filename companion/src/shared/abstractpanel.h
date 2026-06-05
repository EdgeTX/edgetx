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

#include <QWidget>
#include <memory>

class QGridLayout;
class QString;
class WidgetBindings;

class AbstractPanel : public QWidget
{
  Q_OBJECT

  friend class GVarGroup;
  friend class AutoWidget;

  public:
    explicit AbstractPanel(QWidget *parent);
    virtual ~AbstractPanel();

  signals:
    void modified();

  public slots:
    virtual void save();
    virtual void update();

  protected:
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
    void saveAutoWidgets();

    WidgetBindings *bindings();
    void applyBindings();

  private:
    std::unique_ptr<WidgetBindings> m_bindings;
};
