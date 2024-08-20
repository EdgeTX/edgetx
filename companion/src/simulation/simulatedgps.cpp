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

#include "simulatedgps.h"
#include <QtMath>
#include <QDebug>

SimulatedGPS::SimulatedGPS()
{
  lat = 0;
  lon = 0;
  courseDegrees = 0;
  speedKMH = 0;
  altitude = 0;
  satellites = 0;
  dt = QDateTime::currentDateTime();
  running = false;
  timer.setInterval(250);
  connect(&timer, &QTimer::timeout, this, &SimulatedGPS::update);
}

SimulatedGPS::~SimulatedGPS()
{
}

void SimulatedGPS::setDateTime(QDateTime dateTime)
{
  dt = dateTime;
}

void SimulatedGPS::setDateTime(QString dateTime)
{
  if (dateTime.startsWith("*")) {
    dt = QDateTime::currentDateTime();
  } else {
    dt = QDateTime::fromString(dateTime, Qt::ISODate);
  }
}

void SimulatedGPS::setLatLon(QString latLon)
{
  QStringList coords = latLon.split(",");
  if (coords.length() < 2) {
    coords = latLon.split(" ");
  }
  lat = 0.0;
  lon = 0.0;
  if (coords.length() > 1) {
    lat = coords[0].simplified().toDouble();
    lon = coords[1].simplified().toDouble();
  } else {
    stop();
  }
}

void SimulatedGPS::setCourseDegrees(double course)
{
  courseDegrees = course;
}

void SimulatedGPS::setSpeedKMH(double speed)
{
  speedKMH = speed;
}

void SimulatedGPS::setAltitude(double altitude)
{
  this->altitude = altitude;
}

void SimulatedGPS::setSatelliteCount(int sats)
{
  satellites = sats;
}

void SimulatedGPS::start()
{
  running = true;
  timer.start();
}

void SimulatedGPS::stop()
{
  running = false;
  timer.stop();
}

void SimulatedGPS::update()
{
  if (!running) {
    return;
  }

  dt = QDateTime::currentDateTime().toTimeSpec(Qt::UTC);
  emitDateTimeChange();

  double b2 = lat;
  double c2 = lon;
  double d3 = speedKMH / 14400;
  double f3 = courseDegrees;
  double j2 = 6378.1;
  double b3 = qRadiansToDegrees(qAsin( qSin(qDegreesToRadians(b2))*qCos(d3/j2) + qCos(qDegreesToRadians(b2))*qSin(d3/j2)*qCos(qDegreesToRadians(f3))));
  double bb3 = b3;
  if (bb3 < 0) {
    bb3 = bb3 * -1;
  }
  if (bb3 > 89.99) {
    f3 = f3 + 180;
    if (f3 > 360) {
      f3 = f3 - 360;
    }
    courseDegrees = f3;
    emit courseDegreesChanged(courseDegrees);
  }
  double c3 = qRadiansToDegrees(qDegreesToRadians(c2) + qAtan2(qSin(qDegreesToRadians(f3))*qSin(d3/j2)*qCos(qDegreesToRadians(b2)),qCos(d3/j2)-qSin(qDegreesToRadians(b2))*qSin(qDegreesToRadians(b3))));
  if (c3 > 180) {
    c3 = c3 - 360;
  }
  if (c3 < -180) {
    c3 = c3 + 360;
  }
  lat = b3;
  lon = c3;

  emitPositionChange(); 
}

void SimulatedGPS::emitPositionChange()
{
  QString lats = QString::number(lat, 'f', 8);
  QString lons = QString::number(lon, 'f', 8);
  QString qs = lats + "," + lons;

  emit positionChanged(qs);
}

void SimulatedGPS::emitDateTimeChange()
{
  QString formatted = dt.toString(Qt::ISODate);
  emit dateTimeChanged(formatted);
  emit dateTimeChanged(dt);
}
