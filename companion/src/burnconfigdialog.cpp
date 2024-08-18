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

#include "burnconfigdialog.h"
#include "ui_burnconfigdialog.h"
#include "eeprominterface.h"
#include "helpers.h"
#include "appdata.h"
#include "progressdialog.h"
#include "process_flash.h"

#if !defined _MSC_VER && defined __GNUC__
#include <unistd.h>
#endif

burnConfigDialog::burnConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::burnConfigDialog)
{
    ui->setupUi(this);
    setWindowIcon(CompanionIcon("configure.png"));

    getSettings();
    Board::Type board = getCurrentBoard();
    if (IS_STM32(board)) {
      setWindowTitle(tr("DFU-UTIL Configuration"));
      ui->label_sb1->hide();
      ui->label_sb3->hide();
      ui->samba_location->hide();
      ui->samba_port->hide();
      ui->sb_browse->hide();
    }
    else {
      setWindowTitle(tr("SAM-BA Configuration"));
      ui->label_dfu1->hide();
      ui->dfu_location->hide();
      ui->dfu_browse->hide();
    }
    ui->label_sb2->hide();
    ui->arm_mcu->hide();
    ui->label_dfu2->hide();
    ui->dfuArgs->hide();

    QTimer::singleShot(0, this, SLOT(shrink()));
    connect(this, SIGNAL(accepted()), this, SLOT(putSettings()));
}

burnConfigDialog::~burnConfigDialog()
{
    delete ui;
}


void burnConfigDialog::getSettings()
{
    sambaLoc = g.sambaLocation();
    dfuLoc =   g.dfuLocation();

#if defined WIN32 || !defined __GNUC__
    if ( sambaLoc.isEmpty())
      sambaLoc = QFileInfo("sam-ba.exe").absoluteFilePath();
    if ( dfuLoc.isEmpty())
      dfuLoc =  QFileInfo("dfu-util.exe").absoluteFilePath();
#elif defined __APPLE__
    if ( sambaLoc.isEmpty())
      sambaLoc = "/usr/local/bin/sam-ba";
    if ( dfuLoc.isEmpty())
      dfuLoc = QFileInfo(QApplication::applicationDirPath() + "/../Resources/dfu-util").absoluteFilePath();
#elif defined __FreeBSD__
    if (dfuLoc.isEmpty())
      dfuLoc = "/usr/local/bin/dfu-util";
#else
    if ( sambaLoc.isEmpty())
      sambaLoc = "/usr/bin/sam-ba";
    if ( dfuLoc.isEmpty())
      dfuLoc =  QFileInfo("/usr/bin/dfu-util").absoluteFilePath();
#endif

    dfuArgs = g.dfuArguments().split(" ", Qt::SkipEmptyParts);
    armMCU = g.armMcu();
    sambaPort = g.sambaPort();

    ui->samba_location->setText(getSAMBA());
    ui->samba_port->setText(getSambaPort());

    ui->dfu_location->setText(getDFU());
    ui->dfuArgs->setText(getDFUArgs().join(" "));

    int idx4 = ui->arm_mcu->findText(getArmMCU());
    if(idx4>=0) ui->arm_mcu->setCurrentIndex(idx4);
    QFile file;
}

void burnConfigDialog::putSettings()
{
    g.sambaLocation( sambaLoc );
    g.sambaPort( sambaPort );
    g.armMcu( armMCU );
    g.dfuLocation( dfuLoc );
    g.dfuArguments( dfuArgs.join(" ") );
}

void burnConfigDialog::on_samba_location_editingFinished()
{
    sambaLoc = ui->samba_location->text();
}

void burnConfigDialog::on_samba_port_editingFinished()
{
    sambaPort = ui->samba_port->text();
}

void burnConfigDialog::on_arm_mcu_currentIndexChanged(QString )
{
    armMCU = ui->arm_mcu->currentText();
}

void burnConfigDialog::on_sb_browse_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select Location"),ui->samba_location->text());
    if(!fileName.isEmpty())
    {
        ui->samba_location->setText(fileName);
        sambaLoc = fileName;
    }
}

void burnConfigDialog::on_dfu_browse_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select Location"),ui->dfu_location->text());

    if(!fileName.isEmpty())
    {
        ui->dfu_location->setText(fileName);
        dfuLoc = fileName;
    }
}

void burnConfigDialog::on_dfu_location_editingFinished()
{
    dfuLoc = ui->dfu_location->text();
}

void burnConfigDialog::on_dfuArgs_editingFinished()
{
    dfuArgs = ui->dfuArgs->text().split(" ", Qt::SkipEmptyParts);
}

void burnConfigDialog::on_advCtrChkB_toggled(bool checked)
{
  Board::Type board = getCurrentBoard();
  if (checked) {
    if (IS_STM32(board)) {
      ui->label_dfu2->show();
      ui->dfuArgs->show();
    }
    else {
      ui->label_sb2->show();
      ui->arm_mcu->show();
    }
  }
  else {
    if (IS_STM32(board)) {
      ui->label_dfu2->hide();
      ui->dfuArgs->hide();
    }
    else {
      ui->label_sb2->hide();
      ui->arm_mcu->hide();
    }
  }
  QTimer::singleShot(0, this, SLOT(shrink()));
}


void burnConfigDialog::shrink()
{
  resize(0, 0);
}



