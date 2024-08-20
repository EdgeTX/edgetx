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

#include <QWidget>
#include <QDateTime>

#include "simulatorinterface.h"
#include "telemetryprovider.h"
#include "simulatedgps.h"

// Max telemetry packet size
#define CROSSFIRE_PACKET_SIZE 64

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
    void generateTelemetryGPSFrame(uint8_t *packet, double latitude, double longitude, double ground_speed, double ground_course, double altitude, int satellite_count);
    void resetRssi();
    QHash<QString, QString> * getSupportedLogItems();
    void loadItemFromLog(QString itemName, QString value);
    QString getLogfileIdentifier();

  protected slots:
    void generateTelemetryFrame(SimulatorInterface * simulator);
    void loadUiFromSimulator(SimulatorInterface * simulator);

  protected:
    Ui::TelemetryProviderCrossfire * ui;
    SimulatedGPS gps;
    QHash<QString, QString> supportedLogItems;

  private slots:
    void on_button_gpsRunStop_clicked();
    void on_button_saveTelemetryValues_clicked();
    void on_button_loadTelemetryValues_clicked();
};
