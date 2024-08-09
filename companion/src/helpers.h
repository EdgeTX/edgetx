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

#include "eeprominterface.h"
#include <QCheckBox>
#include <QSpinBox>
#include <QTableWidget>
#include <QGridLayout>
#include <QDebug>
#include <QTime>
#include <QElapsedTimer>
#include <QStandardItemModel>
#include <QDialog>
#include <QComboBox>

extern const QColor colors[CPN_MAX_CURVES];

#define TMR_NUM_OPTION  (TMRMODE_COUNT+2*9+2*getCurrentFirmware()->getCapability(LogicalSwitches)-1)

//convert from mode 1 to mode generalSettings.stickMode
//NOTICE!  =>  1..4 -> 1..4
#define CONVERT_AIRMODE(x)      (((x)<=4) ? modn12x3[generalSettings.stickMode][((x)-1)] : (x))
#define CONVERT_SURFACEMODE(x)  (((x)<=4) ? modn12x3[4 + generalSettings.stickMode][((x)-1)] : (x))

#define CURVE_BASE   7
#define CH(x) (SRC_CH1+(x)-1-(SRC_SWC-SRC_3POS))
#define CV(x) (CURVE_BASE+(x)-1)

#define CURVE5(x) ((x)-1)
#define CURVE9(x) (MAX_CURVE5+(x)-1)

#define TRIM_ON  0
#define TRIM_OFF 1
#define TRIM_OFFSET 2

#define TRIM_MODE_NONE  0x1F  // 0b11111
#define TRIM_MODE_3POS  (2 * CPN_MAX_FLIGHT_MODES)

bool displayT16ImportWarning();

class CompanionIcon: public QIcon {
  public:
    CompanionIcon(const QString &baseimage);
    void addImage(const QString &baseimage, Mode mode = Normal, State state = Off);
};

class FilteredItemModel;

class GVarGroup: public QObject {

  Q_OBJECT

  public:
    GVarGroup(QCheckBox * weightGV, QAbstractSpinBox * weightSB, QComboBox * weightCB, int & weight, const ModelData & model,
              const int deflt, const int mini, const int maxi, const double step = 1.0, FilteredItemModel * gvarModel = nullptr);

    void setWeight(int val);

    void setMinimum(int min) {
      mini = min;
    }

    void setMaximum(int max) {
      maxi = max;
    }

  signals:
    void valueChanged();

  protected slots:
    void gvarCBChanged(int);
    void valuesChanged();

  protected:
    QCheckBox *weightGV;
    QAbstractSpinBox *weightSB;
    QSpinBox *sb;
    QDoubleSpinBox *dsb;
    QComboBox *weightCB;
    int & weight;
    int deflt;
    int mini;
    int maxi;
    double step;
    bool lock;
};

namespace Helpers
{
  void populateFileComboBox(QComboBox * b, const QSet<QString> & set, const QString & current);
  void getFileComboBoxValue(QComboBox * b, char * dest, int length);

  void exportAppSettings(QWidget * dlgParent = nullptr);

  QString getChecklistsPath();
  QString getChecklistFilename(const ModelData * model);
  QString getChecklistFilePath(const ModelData * model);
  QString removeAccents(const QString & str);
  unsigned int getBitmappedValue(const unsigned int & field, const unsigned int index = 0, const unsigned int numbits = 1, const unsigned int offset = 0);
  void setBitmappedValue(unsigned int & field, unsigned int value, unsigned int index = 0, unsigned int numbits = 1, unsigned int offset = 0);
  int getFirstPosValueIndex(QComboBox * cbo);

}  // namespace Helpers

// TODO : move globals to Helpers namespace

void startSimulation(QWidget * parent, RadioData & radioData, int modelIdx);

#ifdef __APPLE__
// Flag when simulator is running
bool isSimulatorRunning();
#endif

// Format a pixmap to fit on the current firmware
QPixmap makePixMap(const QImage & image);

int version2index(const QString & version);
const QString index2version(int index);

bool qunlink(const QString & fileName);

QString generateProcessUniqueTempFileName(const QString & fileName);
bool isTempFileName(const QString & fileName);

QString getSoundsPath(const GeneralSettings &generalSettings);
QSet<QString> getFilesSet(const QString &path, const QStringList &filter, int maxLen);


class QTimeS : public QTime
{
  public:
    QTimeS(int s) { int h = s/3600; s %= 3600; int m = s/60; s %=60; setHMS(h, m, s); }
    QTimeS(const QTime & q) : QTime(q) {}
    int seconds() const { return hour()*3600 + minute()*60 + second(); }
};

class GpsCoord
{
public:
  GpsCoord(): latitude(0), longitude(0) {}
  double latitude;    // Precede South latitudes and West longitudes with a minus sign. Latitudes range from -90 to 90.
  double longitude;   // Longitudes range from -180 to 180.
};

class GpsGlitchFilter
{
public:
  GpsGlitchFilter() : lastValid(false), glitchCount(0) {}
  bool isGlitch(GpsCoord coord);

private:
  bool lastValid;
  int glitchCount;
  double lastLat;
  double lastLon;
};

class GpsLatLonFilter
{
public:
  GpsLatLonFilter() {}
  bool isValid(GpsCoord coord);

private:
  double lastLat;
  double lastLon;
};



GpsCoord extractGpsCoordinates(const QString & position);

class TableLayout: public QObject
{
    Q_OBJECT
public:
  TableLayout(QWidget * parent, int rowCount, const QStringList & headerLabels);
  // ~TableLayout() ;

  void addWidget(int row, int column, QWidget * widget, Qt::Alignment alignment = Qt::Alignment());
  void addLayout(int row, int column, QLayout * layout, Qt::Alignment alignment = Qt::Alignment());

  void resizeColumnsToContents();
  void setColumnWidth(int col, int width);
  void setColumnWidth(int col, QString str);
  void pushRowsUp(int row);
  void pushColumnsLeft(int col);
  void setColumnStretch(int col, int stretch);

private:
#if defined(TABLE_LAYOUT)
  QTableWidget * tableWidget;
#else
  QGridLayout * gridWidget;
#endif
};


class Stopwatch
{
public:
  Stopwatch(const QString & name) :
    name(name), total(0) {
    timer.start();
  };
  ~Stopwatch() {}

  void restart() {
    total = 0;
    timer.restart();
  };

  void report() {
    qint64 elapsed = timer.restart();
    total += elapsed;
    qDebug() << name << QString("%1 ms [%2 ms]").arg(elapsed).arg(total);
  };

  void report(const QString & text) {
    qint64 elapsed = timer.restart();
    total += elapsed;
    qDebug() << name << text << QString("%1 ms [%2 ms]").arg(elapsed).arg(total);
  };

private:
  QString name;
  QElapsedTimer timer;
  qint64 total;
};

extern Stopwatch gStopwatch;

class SemanticVersion
{
  public:
    explicit SemanticVersion(const QString vers);
    explicit SemanticVersion() {}
    ~SemanticVersion() {}

    bool isValid(const QString vers);
    bool isValid();
    bool fromString(const QString vers);
    QString toString() const;
    unsigned int toInt() const;
    bool fromInt(const unsigned int val);
    bool isEmpty(const QString vers);
    bool isEmpty();
    bool isPreRelease(const QString vers);
    bool isPreRelease();

    SemanticVersion& operator=(const SemanticVersion& rhs);

    bool operator==(const SemanticVersion& rhs) {
      return compare(rhs) == 0;
    }

    bool operator!=(const SemanticVersion& rhs) {
      return compare(rhs) != 0;
    }

    bool operator>(const SemanticVersion& rhs) {
      return compare(rhs) > 0;
    }

    bool operator>=(const SemanticVersion& rhs) {
      return compare(rhs) >= 0;
    }

    bool operator<(const SemanticVersion& rhs) {
      return compare(rhs) < 0;
    }

    bool operator<=(const SemanticVersion& rhs) {
      return compare(rhs) <= 0;
    }

  private:
    enum PreReleaseTypes {
      PR_ALPHA = 0,
      PR_BETA,
      PR_RC,
      PR_NONE
    };

    const QStringList PreReleaseTypesStringList = { "alpha", "beta", "rc"};

    struct Version {
      int major            = 0;
      int minor            = 0;
      int patch            = 0;
      int preReleaseType   = PR_NONE;
      int preReleaseNumber = 0;
    };

    Version version;

    int compare(const SemanticVersion& other);
    inline QString preReleaseTypeToString() const { return PreReleaseTypesStringList.value(version.preReleaseType, ""); }
    inline int preReleaseTypeToInt(QString preRelType) const { return PreReleaseTypesStringList.indexOf(preRelType); }

};

class StatusDialog: public QDialog
{
    Q_OBJECT

  public:
    StatusDialog(QWidget * parent = nullptr, const QString title = "", QString msgtext = "", const int width = 200);
    virtual ~StatusDialog();

    void update(QString text);

  private:
    QLabel *msg;
};
