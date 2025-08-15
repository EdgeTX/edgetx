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

#ifndef _BURNCONFIGDIALOG_H_
#define _BURNCONFIGDIALOG_H_

#include <QtWidgets>

#define MEM_TYPE_EEPROM 1
#define MEM_TYPE_FLASH  2

#define OPR_TYPE_READ  1
#define OPR_TYPE_WRITE 2

namespace Ui {
    class burnConfigDialog;
}

class burnConfigDialog : public QDialog {
    Q_OBJECT
public:
    burnConfigDialog(QWidget *parent = 0);
    ~burnConfigDialog();

    QString getSAMBA() {return sambaLoc;}
    QString getDFU() {return dfuLoc;}
    QStringList getDFUArgs() {return dfuArgs;}
    QString getArmMCU() {return armMCU;}
    QString getSambaPort() {return sambaPort;}

private:
    Ui::burnConfigDialog *ui;

    void populateProgrammers();

    QString sambaLoc;
    QString dfuLoc;
    QString armMCU;
    QString sambaPort;
    QStringList dfuArgs;

private slots:
    void shrink();
    void on_samba_location_editingFinished();
    void on_samba_port_editingFinished();
    void on_arm_mcu_currentIndexChanged(QString );
    void on_sb_browse_clicked();
    void on_dfu_browse_clicked();
    void on_dfu_location_editingFinished();
    void on_dfuArgs_editingFinished();
    void on_advCtrChkB_toggled(bool checked);
    void getSettings();
    void putSettings();
};

#endif // _BURNCONFIGDIALOG_H_
