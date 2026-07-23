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

#include "autodirectoryselectbutton.h"

#include <QLabel>
#include <QLineEdit>

AutoDirectorySelectButton::AutoDirectorySelectButton(QWidget * parent, const QString & buttonText) :
  AutoPushButton(parent, buttonText),
  m_caption(""),
  m_dir(""),
  m_displayDir(nullptr),
  m_options(QFileDialog::ShowDirsOnly)
{
  connect(this, &QPushButton::released, [&] ()
  {
    QString dir = QFileDialog::getExistingDirectory(this, m_caption, m_dir, m_options);

    if (!dir.isEmpty()) {
      m_dir = dir;

      if (m_displayDir) {
        QLabel *lbl = dynamic_cast<QLabel *>(m_displayDir);

        if (lbl) {
          lbl->setText(m_dir);
        } else {
          QLineEdit *le = dynamic_cast<QLineEdit *>(m_displayDir);

          if (le) {
            le->setText(m_dir);
          }
        }
      }

      emit directoryChanged(m_dir);
      runPostChanged();
    }
  });
}

AutoDirectorySelectButton::~AutoDirectorySelectButton()
{
}

void AutoDirectorySelectButton::setup(QString dlgCaption, QString dir,
                                      QWidget * displayDir, QFileDialog::Options options)
{
  m_caption = dlgCaption;
  m_dir = dir;
  m_displayDir = displayDir;
  m_options = options;
}
