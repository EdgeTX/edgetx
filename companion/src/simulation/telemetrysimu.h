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

#ifndef _TELEMETRYSIMU_H_
#define _TELEMETRYSIMU_H_

#include <QCloseEvent>
#include <QWidget>
#include <QTimer>
#include <QDateTime>
#include <QtCore/qmath.h>
#include <QFileDialog>
#include <QScrollArea>

#include "simulatorinterface.h"
#include "telemetryprovider.h"

static double const SPEEDS[] = { 0.2, 0.4, 0.6, 0.8, 1, 2, 3, 4, 5 };
template<class t> t LIMIT(t mi, t x, t ma) { return std::min(std::max(mi, x), ma); }

namespace Ui {
  class TelemetrySimulator;
}

class TelemetrySimulator : public QWidget
{
  Q_OBJECT

  public:

    explicit TelemetrySimulator(QWidget * parent, SimulatorInterface * simulator);
    virtual ~TelemetrySimulator();

  signals:
    void internalTelemetryDataChanged(const quint8 protocol, const QByteArray data);
    void externalTelemetryDataChanged(const quint8 protocol, const QByteArray data);

  protected slots:

    void hideEvent(QHideEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void startTelemetry();
    void stopTelemetry();
    void onSimulatorStarted();
    void onSimulatorStopped();
    void onSimulateToggled(bool isChecked);
    void onLogTimerEvent();
    void onLoadLogFile();
    void onPlay();
    void onRewind();
    void onStepForward();
    void onStepBack();
    void onStop();
    void onPositionIndicatorChanged(int value);
    void onReplayRateChanged(int value);
    void onInternalTelemetrySelectorChanged(const QString &text);
    void onExternalTelemetrySelectorChanged(const QString &text);
    void onInternalTelemetryProviderDataChanged(const quint8 protocol, const QByteArray data);
    void onExternalTelemetryProviderDataChanged(const quint8 protocol, const QByteArray data);
    void generateTelemetryFrame();
    TelemetryProvider * newTelemetryProviderFromDropdownChoice(const QString &text, QScrollArea * parent);

  protected:

    Ui::TelemetrySimulator * ui;
    QTimer timer;
    QTimer logTimer;
    SimulatorInterface *simulator;
    bool m_simuStarted;
    bool m_logReplayEnable;
    TelemetryProvider * internalProvider;
    TelemetryProvider * externalProvider;

  // protected classes follow

    class LogPlaybackController
    {
      public:
        LogPlaybackController(Ui::TelemetrySimulator * ui);
        bool isReady();
        void loadLogFile();
        void play();
        void stop();
        void rewind();
        void stepForward(bool focusOnStop = false);
        void stepBack();
        void updatePositionLabel(int32_t percentage);
        void setUiDataValues();
        double logFrequency; // in seconds
        bool logFileGpsCordsInDecimalFormat;

      private:
        enum CONVERT_TYPE {
          RXBT_V,
          RSSI,
          RAS,
          A1,
          A2,
          A3,
          A4,
          T1_DEGC,
          T1_DEGF,
          T2_DEGC,
          T2_DEGF,
          RPM,
          FUEL,
          VSPD_MS,
          VSPD_FS,
          ALT_FEET,
          ALT_METERS,
          FASV,
          FASC,
          CELS_GRE,
          ASPD_KTS,
          ASPD_KMH,
          ASPD_MPH,
          GALT_FEET,
          GALT_METERS,
          GSPD_KNTS,
          GSPD_KMH,
          GSPD_MPH,
          GHDG_DEG,
          GDATE,
          G_LATLON,
          ACCX,
          ACCY,
          ACCZ,
          FUEL_QTY,
        };

        struct DATA_TO_FUNC_XREF {
          CONVERT_TYPE functionIndex;
          int32_t dataIndex;
        };

        double convertFeetToMeters(QString input);
        double convertFahrenheitToCelsius(QString input);
        QString convertGPSDate(QString input);
        QString convertGPS(QString input);
        void addColumnHash(QString key, CONVERT_TYPE functionIndex);
        double convertDegMin(QString input);
        QDateTime parseTransmittterTimestamp(QString row);
        void calcLogFrequency();
        void checkGpsFormat();

        QMap<QString, CONVERT_TYPE> colToFuncMap; // contains all 'known' column headings and how they are to be processed
        Ui::TelemetrySimulator * ui;
        QStringList csvRecords; // contents of the log file (one string per line);
        QStringList columnNames;
        QList<DATA_TO_FUNC_XREF> supportedCols;
        int32_t recordIndex;
        bool stepping;
    };  // LogPlaybackController

    LogPlaybackController *logPlayback;
};  // TelemetrySimulator

#endif // _TELEMETRYSIMU_H_

