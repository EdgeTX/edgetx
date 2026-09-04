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

#include "autofileselectbutton.h"

#include <QLabel>
#include <QLineEdit>

AutoFileSelectButton::AutoFileSelectButton(QWidget * parent, const QString & buttonText) :
  AutoPushButton(parent, buttonText),
  m_caption(""),
  m_dir(""),
  m_filter(""),
  m_displayFile(nullptr),
  m_options(QFileDialog::Options())
{
  connect(this, &QPushButton::released, [&] ()
  {
    QString file = QFileDialog::getOpenFileName(this, m_caption, m_dir, m_filter, nullptr, m_options);

    if (!file.isEmpty()) {
      m_file = file;

      if (m_displayFile) {
        QLabel *lbl = dynamic_cast<QLabel *>(m_displayFile);

        if (lbl) {
          lbl->setText(m_file);
        } else {
          QLineEdit *le = dynamic_cast<QLineEdit *>(m_displayFile);

          if (le)
            le->setText(m_file);
        }
      }

      emit fileChanged(m_file);
      runPostChanged();
    }
  });
}

AutoFileSelectButton::~AutoFileSelectButton()
{
}

void AutoFileSelectButton::setup(const QString dlgCaption, const QString dir,
                                 const QString filter, QWidget * displayFile,
                                 QFileDialog::Options options)
{
  m_caption = dlgCaption;
  m_dir = dir;
  m_filter = filter;
  m_displayFile = displayFile;
  m_options = options;
}
