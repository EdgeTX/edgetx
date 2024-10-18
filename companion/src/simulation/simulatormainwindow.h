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
#include "hostserialconnector.h"

#include <QDockWidget>
#include <QFile>
#include <QMainWindow>
#include <QThread>

class DebugOutput;
class RadioData;
class RadioOutputsWidget;
class SimulatorWidget;
class SimulatorInterface;
class TrainerSimulator;
class TelemetrySimulator;

class QKeySequence;

namespace Ui {
class SimulatorMainWindow;
}

using namespace Simulator;

class SimulatorMainWindow : public QMainWindow
{
    Q_OBJECT

  public:
    explicit SimulatorMainWindow(QWidget * parent, const QString & simulatorId = "", quint8 flags=0, Qt::WindowFlags wflags = Qt::WindowFlags());
    ~SimulatorMainWindow();

    int getExitStatus(QString * msg = Q_NULLPTR);
    bool setRadioData(RadioData * radioData);
    bool useTempDataPath(bool deleteOnClose = true);
    bool setOptions(SimulatorOptions & options, bool withSave = true);
    virtual QMenu * createPopupMenu();

  public slots:
    virtual void show();
    void start();
    void showMenuBar(bool show);
    void showRadioFixedSize(Qt::Orientation orientation, bool fixed);
    void showRadioFixedWidth(bool fixed);
    void showRadioFixedHeight(bool fixed);
    void showRadioDocked(bool dock);

  signals:
    void simulatorStart();
    void simulatorRestart();

  protected slots:
    virtual void closeEvent(QCloseEvent *);
    virtual void changeEvent(QEvent *e);
    void restoreUiState();
    void saveUiState();
    void toggleMenuBar(bool show);
    void setRadioSizePolicy(int fixType);
    void toggleRadioDocked(bool dock);
    void openJoystickDialog(bool);
    void openSerialPortsDialog(bool);
    void showHelp(bool show);

  protected:
    void createDockWidgets();
    void addTool(QDockWidget * widget, Qt::DockWidgetArea area, QIcon icon = QIcon(), QKeySequence shortcut = QKeySequence());

    SimulatorInterface  * m_simulator;
    HostSerialConnector * hostSerialConnector;

    Ui::SimulatorMainWindow * ui;
    SimulatorWidget * m_simulatorWidget;
    DebugOutput * m_consoleWidget;
    RadioOutputsWidget * m_outputsWidget;

    QDockWidget * m_simulatorDockWidget;
    QDockWidget * m_consoleDockWidget;
    QDockWidget * m_telemetryDockWidget;
    QDockWidget * m_trainerDockWidget;
    QDockWidget * m_outputsDockWidget;

    QThread simuThread;
    QFile m_simuLogFile;
    QVector<keymapHelp_t> m_keymapHelp;
    QString m_simulatorId;
    QString m_exitStatusMsg;
    int m_exitStatusCode;
    int m_radioProfileId;
    int m_radioSizeConstraint;
    bool m_firstShow;
    bool m_showRadioDocked;
    bool m_showMenubar;

    const static quint16 m_savedUiStateVersion;
};
