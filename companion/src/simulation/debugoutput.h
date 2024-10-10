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

#include "simulator.h"
#include "simulatorinterface.h"

#include <QMutex>
#include <QTimer>
#include <QValidator>
#include <QWidget>

// NOTE : The buffer sizes need to be large enough to handle the flood of data when X12/X10 simulator starts up with TRACE_SIMPGMSPACE=1 (> 40K!).
// These are maximum sizes, not necessarily allocated sizes.
#ifndef DEBUG_OUTPUT_WIDGET_OUT_BUFF_SIZE
  // This buffer holds received and processed data until it can be printed to our console.
  #define DEBUG_OUTPUT_WIDGET_OUT_BUFF_SIZE    (50 * 1024)  // [bytes]
#endif
#ifndef DEBUG_OUTPUT_WIDGET_INP_BUFF_SIZE
  // This buffer is active if line filter is enabled and holds received data until it can be filtered and placed in output buffer.
  #define DEBUG_OUTPUT_WIDGET_INP_BUFF_SIZE    (50 * 1024)  // [bytes]
#endif

namespace Ui {
  class DebugOutput;
}

class QAbstractButton;
class QMessageLogContext;
class FilteredTextBuffer;

using namespace Simulator;

class DebugOutput : public QWidget
{
  Q_OBJECT

  public:
    explicit DebugOutput(QWidget * parent, SimulatorInterface * simulator);
    virtual ~DebugOutput();

    static QRegularExpression makeRegEx(const QString & input, bool * isExlusive = NULL);

  signals:
    void filterExprChanged(const QRegularExpression & expr);
    void filterEnabledChanged(const bool enabled);
    void filterExclusiveChanged(const bool exlusive);
    void filterChanged(bool enable, bool exclusive, const QRegularExpression & expr);
    void tracebackDeviceChange(QIODevice * device);

  protected slots:
    void saveState();
    void restoreState();
    void processBytesReceived();
    void onDataBufferOverflow(const qint64 len);
    void onFilterStateChanged();
    void onFilterTextChanged(const QString &);
    void onFilterToggled(bool enable);
    void on_bufferSize_editingFinished();
    void on_actionWordWrap_toggled(bool checked);
    void on_actionClearScr_triggered();
    void on_actionShowFilterHelp_triggered();

  protected:
    Ui::DebugOutput * ui;
    SimulatorInterface * m_simulator;
    FilteredTextBuffer * m_dataBufferDevice;
    int m_radioProfileId;
    bool m_filterEnable;
    bool m_filterExclude;

    const static quint16 m_savedViewStateVersion;
};

class DebugOutputFilterValidator : public QValidator
{
  public:
    DebugOutputFilterValidator(QObject *parent = Q_NULLPTR) : QValidator(parent) { }
    virtual State validate(QString & input, int &) const;
};

class DeleteComboBoxItemEventFilter : public QObject
{
  Q_OBJECT
  public:
    DeleteComboBoxItemEventFilter(QObject *parent = Q_NULLPTR) : QObject(parent) { }
  protected:
    bool eventFilter(QObject *obj, QEvent *event);
};
