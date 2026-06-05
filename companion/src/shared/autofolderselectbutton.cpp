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

#include "autofolderselectbutton.h"

#include <QLabel>
#include <QLineEdit>

AutoFolderSelectButton::AutoFolderSelectButton(QWidget * parent) :
  AutoPushButton(parent),
  m_caption(tr("Select folder")),
  m_fldr(""),
  m_displayFldr(nullptr),
  m_opts(0)
{
  connect(this, &QPushButton::released, [&] ()
  {
    QString fldr = QFileDialog::getExistingDirectory(this, m_caption, m_fldr, m_opts);

    if (!fldr.isEmpty()) {
      m_fldr = fldr;

      if (m_displayFldr) {
        QLabel *lbl = dynamic_cast<QLabel *>(m_displayFldr);

        if (lbl) {
          lbl->setText(m_fldr);
        } else {
          QLineEdit *le = dynamic_cast<QLineEdit *>(m_displayFldr);

          if (le) {
            le->setText(m_fldr);
          }
        }
      }

      emit folderChanged(m_fldr);
      runPostChanged();
    }
  });
}

AutoFolderSelectButton::~AutoFolderSelectButton()
{
}

void AutoFolderSelectButton::setup(QString caption, QString fldr, QWidget * displayFldr, QFileDialog::Options opts)
{
  m_caption = caption;
  m_fldr = fldr;
  m_displayFldr = displayFldr;
  m_opts = opts;
}
