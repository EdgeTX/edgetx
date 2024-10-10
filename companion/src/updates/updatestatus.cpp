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

#include "updatestatus.h"

#include <QMessageBox>

UpdateStatus::UpdateStatus(QObject * parent) :
  QObject(parent),
  m_progress(nullptr),
  m_logLevel(QtInfoMsg)
{

}

void UpdateStatus::reportProgress(QString text, QtMsgType type)
{
  if (m_logLevel == QtDebugMsg ||
     (m_logLevel == QtInfoMsg && type > QtDebugMsg) ||
     (type < QtInfoMsg && type >= m_logLevel)) {
    if (m_progress)
      m_progress->addMessage(text, type);
    else
      qDebug() << text;
  }

  if (type == QtFatalMsg)
    fatalMsg(text);
}

void UpdateStatus::progressMessage(QString text)
{
  if (m_progress)
    m_progress->setInfo(text);
  else
    qDebug() << text;
}

void UpdateStatus::fatalMsg(QString msg)
{
  QMessageBox::critical(m_progress, tr("Update Interface"), msg);
}

void UpdateStatus::setProgress(ProgressWidget * progress)
{
  m_progress = progress;
  if (m_progress) {
    disconnect(m_progress, &ProgressWidget::stopped, this, &UpdateStatus::onWidgetStopped); // to avoid duplicate entries in stack
    connect(m_progress, &ProgressWidget::stopped, this, &UpdateStatus::onWidgetStopped);
  }
}

void UpdateStatus::onWidgetStopped()
{
  emit cancelled();
}
