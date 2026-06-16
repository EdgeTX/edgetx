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

#include "../storage/appdata.h"
#include "gridlayout.h"

#include <QDialog>
#include <QScrollArea>

class UpdateFactories;

class PrefsEditDialog;

class PrefsPanel : public QWidget
{
  Q_OBJECT

  public:
    friend class PrefsEditDialog;

    PrefsPanel(QWidget * parent);
    virtual ~PrefsPanel() {}

    virtual bool save() = 0;
    virtual void update() = 0;

  signals:
    void modified();

  protected:
    bool lock;
    GridLayout *grid;
};

class PrefsScrollArea : public QScrollArea
{
  Q_OBJECT

  public:
    PrefsScrollArea(QWidget * parent, PrefsPanel * panel);

  protected:
    virtual bool eventFilter(QObject * o, QEvent * e);

  private:
    PrefsPanel * panel;
};

namespace Ui {
  class PrefsEdit;
}

class PrefsEditDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit PrefsEditDialog(QWidget * parent, UpdateFactories * factories);
    virtual ~PrefsEditDialog();

  public slots:
    void accept() Q_DECL_OVERRIDE;
    void setMainWinHasDirtyChild(bool value);

  signals:
    void firmwareProfileChanged();
    void firmwareProfileAboutToChange(bool saveFiles = true);
    void modified();

  protected:
    void closeEvent(QCloseEvent *event);

  private slots:
    void onTabModified();

  private:
    Ui::PrefsEdit *ui;
    bool mainWinHasDirtyChild;
    QList<PrefsPanel *> panels;

    void addTab(PrefsPanel * panel, QString text);
    void shrink();
};
