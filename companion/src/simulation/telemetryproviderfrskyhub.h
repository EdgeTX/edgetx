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
#define FRSKY_HUB_MAX_PACKET_SIZE 16

namespace Ui {
  class TelemetryProviderFrSkyHub;
}

class TelemetryProviderFrSkyHub : public QWidget, public TelemetryProvider
{
  Q_OBJECT

  public:

    explicit TelemetryProviderFrSkyHub(QWidget * parent);
    virtual ~TelemetryProviderFrSkyHub();

  signals:
    void telemetryDataChanged(const quint8 protocol, const QByteArray data);

  public slots:
    void resetRssi();
    QHash<QString, QString> * getSupportedLogItems();
    void loadItemFromLog(QString itemName, QString value);
    QString getLogfileIdentifier();

  protected slots:
    void generateTelemetryFrame(SimulatorInterface * simulator);
    void loadUiFromSimulator(SimulatorInterface * simulator);

  protected:
    Ui::TelemetryProviderFrSkyHub * ui;
    SimulatedGPS gps;
    QHash<QString, QString> supportedLogItems;

  private slots:
    void on_button_gpsRunStop_clicked();
    void on_button_saveTelemetryValues_clicked();
    void on_button_loadTelemetryValues_clicked();

    void sendLongLinkFrame(int rssi, double a1, double a2, int trss, int rqly, int tqly);
    void sendShortLinkFrame(int rssi, double a1, double a2);
    void sendLinkFrame(int rssi, double a1, double a2, int trss, int rqly, int tqly);
    void sendSensorPacket(uint8_t id, uint16_t value);
    void sendRPMFrame(int rpm);
    void sendFuelFrame(int fuelpercent);
    void sendTempsFrame(int temp1, int temp2);
    void sendBatteryFrame(double vfas, double curr);
    void sendBaroFrame(double vspd, double alt);
    void sendAccelFrame(double accx, double accy, double accz);
    void sendGPSFrame(SimulatedGPS & gps);
};
