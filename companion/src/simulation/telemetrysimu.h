/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#include "simulatorinterface.h"

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

  public slots:
    bool generateSportPacket(uint8_t * packet, uint8_t dataId, uint8_t prim, uint16_t appId, uint32_t data);

  signals:
    void telemetryDataChanged(const QByteArray data);

  protected slots:

    void hideEvent(QHideEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void startTelemetry();
    void stopTelemetry();
    void onSimulatorStarted();
    void onSimulatorStopped();
    void setupDataFields();
    void onSimulateToggled(bool isChecked);
    void onLogTimerEvent();
    void onGpsRunLoop();
    void onLoadLogFile();
    void onPlay();
    void onRewind();
    void onStepForward();
    void onStepBack();
    void onStop();
    void onPositionIndicatorChanged(int value);
    void onReplayRateChanged(int value);
    void refreshSensorRatios();
    void generateTelemetryFrame();

  protected:

    Ui::TelemetrySimulator * ui;
    QTimer timer;
    QTimer logTimer;
    QTimer gpsTimer;
    SimulatorInterface *simulator;
    bool m_simuStarted;
    bool m_logReplayEnable;

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

    class FlvssEmulator
    {
      public:
        static const uint32_t MAXCELLS = 8;
        uint32_t setAllCells_GetNextPair(double cellValues[MAXCELLS]);

      private:
        void encodeAllCells();
        void splitIntoCells(double totalVolts);
        static uint32_t encodeCellPair(uint8_t cellNum, uint8_t firstCellNo, double cell1, double cell2);
        double cellFloats[MAXCELLS];
        uint32_t nextCellNum;
        uint32_t numCells;
        uint32_t cellData1;
        uint32_t cellData2;
        uint32_t cellData3;
        uint32_t cellData4;
    };  // FlvssEmulator

    class GPSEmulator
    {
      public:
        GPSEmulator();
        uint32_t getNextPacketData(uint32_t packetType);
        void setGPSDateTime(QString dateTime);
        void setGPSLatLon(QString latLon);
        void setGPSCourse(double course);
        void setGPSSpeedKMH(double speed);
        void setGPSAltitude(double altitude);

      private:
        QDateTime dt;
        bool sendLat;
        bool sendDate;
        double lat;
        double lon;
        double course;
        double speedKNTS;
        double altitude; // in meters
        uint32_t encodeLatLon(double latLon, bool isLat);
        uint32_t encodeDateTime(uint8_t yearOrHour, uint8_t monthOrMinute, uint8_t dayOrSecond, bool isDate);
    };  // GPSEmulator
private slots:
    void on_saveTelemetryvalues_clicked();
    void on_loadTelemetryvalues_clicked();
    void on_GPSpushButton_clicked();
    void on_gps_course_valueChanged(double arg1);
};  // TelemetrySimulator

#endif // _TELEMETRYSIMU_H_

