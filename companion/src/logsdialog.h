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

#include <QtCore>
#include <QDialog>
#include "qcustomplot.h"

#define INVALID_MIN 999999
#define INVALID_MAX -999999

namespace Ui {
  class LogsDialog;
}

class LogsDialog : public QDialog
{
  Q_OBJECT

  enum yaxes_t {
    firstLeft = 0,
    firstRight,
    secondLeft,
    secondRight,
    AXES_LIMIT // = 4
  };

  struct coords_t {
    QVector<double> x, y;
    double min_y;
    double max_y;
    yaxes_t yaxis;
    QString name;
  };

  struct minMax_t {
    double min;
    double max;
  };

  struct plotsCollection {
    QVarLengthArray<coords_t> coords;
    double min_x;
    double max_x;
    bool tooManyRanges;
  };

public:
  explicit LogsDialog(QWidget *parent = 0);
  ~LogsDialog();

private slots:
  void titleDoubleClicked(QMouseEvent *evt);
  void axisLabelDoubleClick(QCPAxis* axis, QCPAxis::SelectablePart part);
  void legendDoubleClick(QCPLegend* legend, QCPAbstractLegendItem* item);
  void selectionChanged();
  void mousePress(QMouseEvent * event);
  void mouseWheel();
  void removeAllGraphs();
  void plotLogs();
  void fileOpen();
  void saveSession();
  void sessionsCurrentIndexChanged(int index);
  void mapsButtonClicked();
  void yAxisChangeRanges(QCPRange range);

private:
  QList<QStringList> csvlog;
  Ui::LogsDialog *ui;
  QCPAxisRect *axisRect;
  QCPLegend *rightLegend;
  bool plotLock;
  QString logFilename;

  QVarLengthArray<Qt::GlobalColor> colors;
  QPen pen;

  double yAxesRatios[AXES_LIMIT];
  minMax_t yAxesRanges[AXES_LIMIT];

  QCPItemTracer * tracerMaxAlt;
  QCPItemTracer * cursorA;
  QCPItemTracer * cursorB;
  QCPItemStraightLine * cursorLine;

  bool cvsFileParse();
  QList<QStringList> filterGePoints(const QList<QStringList> & input);
  void exportToGoogleEarth();
  QDateTime getRecordTimeStamp(int index);
  QString generateDuration(const QDateTime & start, const QDateTime & end);
  void setFlightSessions();

  void addMaxAltitudeMarker(const coords_t & c, QCPGraph * graph);
  void countNumberOfThrows(const coords_t & c, QCPGraph * graph);
  void addCursor(QCPItemTracer ** cursor, QCPGraph * graph, const QColor & color);
  void addCursorLine(QCPItemStraightLine ** line, QCPGraph * graph, const QColor & color);
  void placeCursor(double x, bool second);
  QString formatTimeDelta(double timeDelta);
  void updateCursorsLabel();


};
