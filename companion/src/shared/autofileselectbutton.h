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

#include "autopushbutton.h"

#include <QFileDialog>

class AutoFileSelectButton : public AutoPushButton
{
  Q_OBJECT

  public:
    explicit AutoFileSelectButton(QWidget * parent = nullptr, const QString & buttonText = tr("Select..."));
    virtual ~AutoFileSelectButton();

    virtual void updateValue() override {}

    void setup(const QString dlgCaption = QString(), const QString dir = QString(),
               const QString filter = QString(), QWidget * displayFile = nullptr,
               QFileDialog::Options options = QFileDialog::Options());

  signals:
    void fileChanged(QString file);

  private:
    QString m_caption;
    QString m_dir;
    QString m_filter;
    QWidget *m_displayFile;
    QFileDialog::Options m_options;
    QString m_file;
};
