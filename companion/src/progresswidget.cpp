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

#include "progresswidget.h"
#include "ui_progresswidget.h"
#include "appdata.h"

#include <QApplication>
#include <QDebug>
#include <QTimer>
#include <QThread>
#include <QScrollBar>

ProgressWidget::ProgressWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::ProgressWidget),
  m_forceOpen(false),
  m_hasDetails(false)
{
  ui->setupUi(this);
  ui->info->hide();
  ui->checkBox->hide();
  ui->textEdit->hide();

  ui->checkBox->setChecked(g.outputDisplayDetails());
  connect(ui->checkBox, &QCheckBox::toggled, this, &ProgressWidget::onShowDetailsToggled);

  QFont newFont(ui->textEdit->font());
  newFont.setFamily("Courier");
#ifdef __APPLE__
  newFont.setPointSize(13);
  ui->textEdit->setAttribute(Qt::WA_MacNormalSize);
#elif defined WIN32 || !defined __GNUC__
  newFont.setPointSize(9);
#endif
  ui->textEdit->setFont(newFont);

}

ProgressWidget::~ProgressWidget()
{
  delete ui;
}

void ProgressWidget::stop()
{
  emit stopped();
}

void ProgressWidget::clearDetails() const
{
  ui->textEdit->clear();
}

void ProgressWidget::forceOpen()
{
  m_forceOpen = true;
  ui->checkBox->hide();
  toggleDetails();
}

void ProgressWidget::setInfo(const QString &text)
{
  ui->info->show();
  ui->info->setText(text);
}

void ProgressWidget::setMaximum(int value)
{
  ui->progressBar->setMaximum(value);
}

int ProgressWidget::maximum()
{
  return ui->progressBar->maximum();
}

void ProgressWidget::setValue(int value)
{
  ui->progressBar->setValue(value);
}

void ProgressWidget::addText(const QString &text, const bool richText)
{
  if (!m_hasDetails) {
    m_hasDetails = true;
    if (!m_forceOpen)
      toggleDetails();
  }

  QTextCursor cursor(ui->textEdit->textCursor());

  // is the scrollbar at the end?
  bool atEnd = (ui->textEdit->verticalScrollBar()->value() == ui->textEdit->verticalScrollBar()->maximum());

  cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor, 1);
  if (richText)
    cursor.insertHtml(text);
  else
    cursor.insertText(text);

  if (atEnd) {
    ui->textEdit->verticalScrollBar()->triggerAction(QAbstractSlider::SliderToMaximum);
  }
}

void ProgressWidget::addHtml(const QString & text)
{
  addText(text, true);
}

void ProgressWidget::addMessage(const QString & text, const int & type, bool richText)
{
  QString color;
  switch (type) {
    case QtDebugMsg:
      color = "dimgrey";  // not important messages, may be filtered out
      break;
    case QtWarningMsg:    // use warning level as emphasis
      color = "orangered";
      break;
    case QtCriticalMsg:   // use critical as a warning
      color = "darkred";
      break;
    case QtFatalMsg:      // fatal for hard errors
      color = "red";
      break;
    case QtInfoMsg:       // default plain text
    default:
      break;
  }
  if (color.isEmpty()) {
    if (richText)
      addHtml(text % "<br>");
    else
      addText(text % "\n");
  }
  else {
    addHtml(QString("<font color=%1>").arg(color) % text % "</font><br>");
  }
}

QString ProgressWidget::getText() const
{
  return ui->textEdit->toPlainText();
}

bool ProgressWidget::isEmpty() const
{
  return getText().isEmpty();
}

bool ProgressWidget::detailsVisible() const
{
  return ui->textEdit->isVisible();
}

void ProgressWidget::onShowDetailsToggled(bool checked)
{
  g.outputDisplayDetails(checked);
  toggleDetails();
}

void ProgressWidget::toggleDetails()
{
  bool showDetails = m_forceOpen || (m_hasDetails && g.outputDisplayDetails());
  if (!m_forceOpen)
    ui->checkBox->show();
  if ((!showDetails && ui->textEdit->isVisible()) || (showDetails && !ui->textEdit->isVisible())) {
    ui->textEdit->setMinimumHeight(showDetails ? 300 : 0);
    ui->textEdit->setVisible(showDetails);
    emit detailsToggled();
  }
}

void ProgressWidget::setProgressColor(const QColor &color)
{
  ui->progressBar->setStyleSheet(QString("QProgressBar  {text-align: center;} QProgressBar::chunk { background-color: %1; text-align:center;}:").arg(color.name()));
}

#define HLINE_SEPARATOR "================================================================================="

void ProgressWidget::addSeparator()
{
  addText("\n" HLINE_SEPARATOR "\n");
}

void ProgressWidget::lock(bool lock)
{
  emit locked(lock);
}

void ProgressWidget::forceKeepOpen(bool value)
{
  emit keepOpen(value);
}

void ProgressWidget::refresh()
{
  ui->info->update();
  ui->progressBar->update();
  ui->textEdit->update();
}
