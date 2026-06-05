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

#include <QFileDialog>
#include <QPushButton>

class AutoFldrSelectBtn : public QPushButton, public AutoWidget
{
  Q_OBJECT

  public:
    explicit AutoFldrSelectBtn(QWidget * parent = nullptr, const QString & caption = QString(),
                           const QString & dir = QString(), QFileDialog::Options opts = QFileDialog::ShowDirsOnly);
    virtual ~AutoFldrSelectBtn() = default;

    virtual void updateValue() override {}

    signals:
    void folderChanged(QString & fldr);

  private:
    const QString &m_caption;
    QString m_dir;
    const QFileDialog::Options m_opts;
};
