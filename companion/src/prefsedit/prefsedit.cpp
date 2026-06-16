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

#include "prefsedit.h"
//#include "prefs_app.h"
//#include "prefs_profile.h"
#include "prefs_simu.h"
//#include "prefs_update.h"
#include "ui_prefsedit.h"
#include "helpers.h"

/*
    PrefsPanel
*/

PrefsPanel::PrefsPanel(QWidget * parent):
  QWidget(parent),
  lock(false),
  grid(new GridLayout())
{
}

/*
    PrefsScrollArea
*/

PrefsScrollArea::PrefsScrollArea(QWidget * parent, PrefsPanel * panel):
  QScrollArea(parent),
  panel(panel)
{
  setWidgetResizable(true);
  setWidget(panel);
  panel->installEventFilter(this);
}

bool PrefsScrollArea::eventFilter(QObject * o, QEvent * e)
{
  if (o == panel && e->type() == QEvent::Resize) {
    setMinimumWidth(panel->minimumSizeHint().width() * 0.75);
  }
  return false;
}

/*
    PrefsEditDialog
*/

PrefsEditDialog::PrefsEditDialog(QWidget * parent, UpdateFactories * factories) :
  QDialog(parent),
  ui(new Ui::PrefsEdit),
  mainWinHasDirtyChild(false)
{
  ui->setupUi(this);
  setWindowIcon(CompanionIcon("apppreferences.png"));
  setAttribute(Qt::WA_DeleteOnClose);
  restoreGeometry(g.prefsEditGeo());

  //addTab(new PrefsProfilePanel(this), tr("Radio Profile"));
  //addTab(new PrefsAppPanel(this), tr("Application"));
  addTab(new PrefsSimuPanel(this), tr("Simulator"));
  //addTab(new PrefsUpdatePanel(this, factories), tr("Update"));

  ui->tabWidget->setCurrentIndex(0);
  shrink();
}

PrefsEditDialog::~PrefsEditDialog()
{
  delete ui;
}

void PrefsEditDialog::accept()
{
  bool res = true;

  for (const auto panel : panels) {
    if (!(res = panel->save())) break;
  }

  if (res)
    QDialog::accept();
}

void PrefsEditDialog::addTab(PrefsPanel * panel, QString text)
{
  panels << panel;
  QWidget * widget = new QWidget(ui->tabWidget);
  QVBoxLayout *baseLayout = new QVBoxLayout(widget);
  PrefsScrollArea * area = new PrefsScrollArea(widget, panel);
  baseLayout->addWidget(area);
  ui->tabWidget->addTab(widget, text);
  connect(panel, &PrefsPanel::modified, this, &PrefsEditDialog::onTabModified);
}

void PrefsEditDialog::closeEvent(QCloseEvent *event)
{
  g.prefsEditGeo(saveGeometry());
}

void PrefsEditDialog::onTabModified()
{
  emit modified();
}

void PrefsEditDialog::shrink()
{
  adjustSize();
}

void PrefsEditDialog::setMainWinHasDirtyChild(bool value)
{
  mainWinHasDirtyChild = value;
}
