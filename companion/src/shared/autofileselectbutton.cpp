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

AutoFileSelectButton::AutoFileSelectButton(QWidget * parent, const QString & text) :
  AutoPushButton(parent),
  m_caption(tr("Select existing file")),
  m_path(""),
  m_displayPath(nullptr)
{
  setText(text);

  connect(this, &QPushButton::released, [&] ()
  {
    QString path = QFileDialog::getOpenFileName(this, m_caption, m_path);

    if (!path.isEmpty()) {
      m_path = path;

      if (m_displayPath) {
        QLabel *lbl = dynamic_cast<QLabel *>(m_displayPath);

        if (lbl) {
          lbl->setText(m_path);
        } else {
          QLineEdit *le = dynamic_cast<QLineEdit *>(m_displayPath);

          if (le) {
            le->setText(m_path);
          }
        }
      }

      emit folderChanged(m_path);
      runPostChanged();
    }
  });
}

AutoFileSelectButton::~AutoFileSelectButton()
{
}

void AutoFileSelectButton::setup(QString caption, QString path, QWidget * displayPath)
{
  m_caption = caption;
  m_path = path;
  m_displayPath = displayPath;
}
