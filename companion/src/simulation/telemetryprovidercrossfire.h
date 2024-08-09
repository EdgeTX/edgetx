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

#ifndef _TELEMETRY_CROSSFIRE_H_
#define _TELEMETRY_CROSSFIRE_H_

#include <QWidget>
#include <QDateTime>

#include "simulatorinterface.h"
#include "telemetryprovider.h"

// Max telemetry packet size
#define CROSSFIRE_PACKET_SIZE 16

namespace Ui {
  class TelemetryProviderCrossfire;
}

class TelemetryProviderCrossfire : public QWidget, public TelemetryProvider
{
  Q_OBJECT

  public:

    explicit TelemetryProviderCrossfire(QWidget * parent);
    virtual ~TelemetryProviderCrossfire();

  signals:
    void telemetryDataChanged(const quint8 protocol, const QByteArray data);

  public slots:
    void generateTelemetryLinkStatisticsFrame(uint8_t *packet, uint8_t rss1, uint8_t rss2, uint8_t rqly, int8_t rsnr, uint8_t ant, uint8_t rfmd, uint8_t tpwr, uint8_t trss, uint8_t tqly, int8_t tsnr);
    void generateTelemetryBatterySensorFrame(uint8_t *packet, double voltage, double current, int32_t used_capacity, int8_t battery_percent);
    void generateTelemetryAttitudeFrame(uint8_t *packet, double pitch, double roll, double yaw);
    void generateTelemetryFlightModeFrame(uint8_t *packet, const QString &mode);
    void generateTelemetryBarometerFrame(uint8_t *packet, double altitude, double vspeed);
    void resetRssi();

  protected slots:
    void updateGps();
    void generateTelemetryFrame(SimulatorInterface * simulator);
    void loadUiFromSimulator(SimulatorInterface * simulator);

  protected:
    Ui::TelemetryProviderCrossfire * ui;
    QTimer gpsTimer;

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
    void on_GPSpushButton_clicked();
    void on_gps_course_valueChanged(double arg1);
};

#endif
