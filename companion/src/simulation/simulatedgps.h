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

#include <QTimer>
#include <QDateTime>

class SimulatedGPS : public QObject
{
  Q_OBJECT

 public:
  SimulatedGPS();
  ~SimulatedGPS();

  QDateTime dt;
  double lat;
  double lon;
  double courseDegrees;
  double speedKMH;
  double altitude; // in meters
  uint8_t satellites;
  bool running;

 signals:
  void positionChanged(const QString latLon);
  void dateTimeChanged(const QString dateTime);
  void dateTimeChanged(const QDateTime dateTime);
  void courseDegreesChanged(double course);

 public slots:
  void setDateTime(QDateTime dateTime);
  void setDateTime(QString dateTime);
  void setLatLon(QString latLon);
  void setCourseDegrees(double course);
  void setSpeedKMH(double speed);
  void setAltitude(double altitude);
  void setSatelliteCount(int sats);
  void start();
  void stop();

 protected slots:
  void update();
  void emitPositionChange();
  void emitDateTimeChange();

 private:
  QTimer timer;
};
