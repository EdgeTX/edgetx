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

#include <QtGui>
#if defined _MSC_VER
  #include <io.h>
  #include <stdio.h>
#elif defined __GNUC__
  #include <unistd.h>
#endif

#include "appdata.h"
#include "macros.h"
#include "helpers.h"
#include "simulatorinterface.h"
#include "simulatormainwindow.h"
#include "storage/sdcard.h"
#include "filtereditemmodels.h"

#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QDir>
#include <QRegularExpression>
#include <QProcess>
#include <QtGlobal>
#include <QFile>

using namespace Helpers;

Stopwatch gStopwatch("global");

const QColor colors[CPN_MAX_CURVES] = {
  QColor(0,0,127),
  QColor(0,127,0),
  QColor(127,0,0),
  QColor(0,127,127),
  QColor(127,0,127),
  QColor(127,127,0),
  QColor(127,127,127),
  QColor(0,0,255),
  QColor(0,127,255),
  QColor(127,0,255),
  QColor(0,255,0),
  QColor(0,255,127),
  QColor(127,255,0),
  QColor(255,0,0),
  QColor(255,0,127),
  QColor(255,127,0),
  QColor(0,0,127),
  QColor(0,127,0),
  QColor(127,0,0),
  QColor(0,127,127),
  QColor(127,0,127),
  QColor(127,127,0),
  QColor(127,127,127),
  QColor(0,0,255),
  QColor(0,127,255),
  QColor(127,0,255),
  QColor(0,255,0),
  QColor(0,255,127),
  QColor(127,255,0),
  QColor(255,0,0),
  QColor(255,0,127),
  QColor(255,127,0),
};


/*
 * CompanionIcon
*/

static QString iconThemeFolder(int theme_set)
{
  switch(theme_set) {
    case 0:
      return QStringLiteral("classic");
    case 2:
      return QStringLiteral("monowhite");
    case 3:
      return QStringLiteral("monochrome");
    case 4:
      return QStringLiteral("monoblue");
    default:
      return QStringLiteral("yerico");
  }
}

CompanionIcon::CompanionIcon(const QString &baseimage)
{
  addImage(baseimage);
}

void CompanionIcon::addImage(const QString & baseimage, Mode mode, State state)
{
  const QString theme = iconThemeFolder(g.theme());
  addFile(":/themes/"+theme+"/16/"+baseimage, QSize(16,16), mode, state);
  addFile(":/themes/"+theme+"/24/"+baseimage, QSize(24,24), mode, state);
  addFile(":/themes/"+theme+"/32/"+baseimage, QSize(32,32), mode, state);
  addFile(":/themes/"+theme+"/48/"+baseimage, QSize(48,48), mode, state);
}


/*
 * GVarGroup
*/

GVarGroup::GVarGroup(QCheckBox * weightGV, QAbstractSpinBox * weightSB, QComboBox * weightCB, int & weight, const ModelData & model,
                     const int deflt, const int mini, const int maxi, const double step, FilteredItemModel * gvarModel):
  QObject(),
  weightGV(weightGV),
  weightSB(weightSB),
  sb(dynamic_cast<QSpinBox *>(weightSB)),
  dsb(dynamic_cast<QDoubleSpinBox *>(weightSB)),
  weightCB(weightCB),
  weight(weight),
  deflt(deflt),
  mini(mini),
  maxi(maxi),
  step(step),
  lock(true)
{
  if (gvarModel && gvarModel->rowCount() > 0) {
    weightCB->setModel(gvarModel);
    connect(weightGV, SIGNAL(stateChanged(int)), this, SLOT(gvarCBChanged(int)));
    connect(weightCB, SIGNAL(currentIndexChanged(int)), this, SLOT(valuesChanged()));
  }
  else {
    weightGV->hide();
    if (weight > maxi || weight < mini) {
      weight = deflt;
    }
  }

  setWeight(weight);

  connect(weightSB, SIGNAL(editingFinished()), this, SLOT(valuesChanged()));

  lock = false;
}

void GVarGroup::gvarCBChanged(int state)
{
  if (!lock) {
    weightCB->setVisible(state);
    if (weightGV->isChecked()) {
      //  set CB to +GV1
      int cnt = getCurrentFirmware()->getCapability(Gvars);
      if (weightCB->count() > cnt)
        weightCB->setCurrentIndex(cnt);
      else
        weightCB->setCurrentIndex(0);
    }
    weightSB->setVisible(!state);
    valuesChanged();
  }
}

void GVarGroup::valuesChanged()
{
  if (!lock) {
    if (weightGV->isChecked())
      weight = weightCB->itemData(weightCB->currentIndex()).toInt();
    else if (sb)
      weight = sb->value();
    else
      weight = round(dsb->value() / step);

    emit valueChanged();
  }
}

void GVarGroup::setWeight(int val)
{
  lock = true;

  int tval;

  if (val > maxi || val < mini) {
    tval = deflt;
    weightGV->setChecked(true);
    weightSB->hide();
    weightCB->setCurrentIndex(weightCB->findData(val));
    if (weightCB->currentIndex() == -1)
      weightCB->setCurrentIndex(getCurrentFirmware()->getCapability(Gvars));
    weightCB->show();
  }
  else {
    tval = val;
    weightGV->setChecked(false);
    weightSB->show();
    weightCB->hide();
  }

  if (sb) {
    sb->setMinimum(mini);
    sb->setMaximum(maxi);
    sb->setValue(tval);
  }
  else {
    dsb->setMinimum(mini*step);
    dsb->setMaximum(maxi*step);
    dsb->setValue(tval*step);
  }

  lock = false;
}

/*
 * Helpers namespace functions
*/

static bool caseInsensitiveLessThan(const QString &s1, const QString &s2)
{
  return s1.toLower() < s2.toLower();
}

void Helpers::populateFileComboBox(QComboBox * b, const QSet<QString> & set, const QString & current)
{
  b->clear();
  b->addItem(CPN_STR_NONE_ITEM);

  bool added = false;
  // Convert set into list and sort it alphabetically case insensitive
  QStringList list(set.begin(), set.end());
  std::sort(list.begin(), list.end(), caseInsensitiveLessThan);
  foreach (QString entry, list) {
    b->addItem(entry);
    if (entry == current) {
      b->setCurrentIndex(b->count()-1);
      added = true;
    }
  }

  if (!added && !current.isEmpty()) {
    b->addItem(current);
    b->setCurrentIndex(b->count()-1);
  }
}

void Helpers::getFileComboBoxValue(QComboBox * b, char * dest, int length)
{
  memset(dest, 0, length+1);
  if (b->currentText() != CPN_STR_NONE_ITEM) {
    strncpy(dest, b->currentText().toLatin1(), length);
  }
}

void Helpers::exportAppSettings(QWidget * dlgParent)
{
  static QString lastExpFile = CPN_SETTINGS_INI_PATH.arg(QDateTime::currentDateTime().toString("dd-MMM-yy"));
  const QString expFile = QFileDialog::getSaveFileName(dlgParent, QCoreApplication::translate("Companion", "Select or create a file for exported Settings:"), lastExpFile, CPN_STR_APP_SETTINGS_FILTER);
  if (expFile.isEmpty())
    return;

  lastExpFile = expFile;
  QString resultMsg;
  if (g.exportSettingsToFile(expFile, resultMsg)) {
    QMessageBox::information(dlgParent, CPN_STR_APP_NAME, resultMsg);
    return;
  }
  resultMsg.append("\n" % QCoreApplication::translate("Companion", "Press the 'Retry' button to choose another file."));
  if (QMessageBox::warning(dlgParent, CPN_STR_APP_NAME, resultMsg, QMessageBox::Cancel, QMessageBox::Retry) == QMessageBox::Retry)
    exportAppSettings(dlgParent);
}

unsigned int Helpers::getBitmappedValue(const unsigned int & field, const unsigned int index, const unsigned int numbits, const unsigned int offset)
{
  int mask = -1;
  mask = mask << numbits;
  mask = ~mask;
  return (field >> (numbits * index + offset)) & (unsigned int)mask;
}

void Helpers::setBitmappedValue(unsigned int & field, unsigned int value, unsigned int index, unsigned int numbits, unsigned int offset)
{
  int mask = -1;
  mask = mask << numbits;
  mask = ~mask;

  unsigned int fieldmask = ((unsigned int)mask << (numbits * index + offset));
  field = (field & ~fieldmask) | (value << (numbits * index + offset));
}

// return index of 'NONE' ie zero or first positive data entry
// assumes list sequence of [negative] [NONE] positive values
int Helpers::getFirstPosValueIndex(QComboBox * cbo)
{
  const int cnt = cbo->count();

  if (cnt == 0)
    return -1;

  // no negative values so use 1st entry
  if (cbo->itemData(0).toInt() >= 0)
    return 0;

  // no positve values exception NONE so use last entry (safety net)
  if (cbo->itemData(cnt - 1).toInt() <= 0)
    return cnt - 1;

  // start search from mid point with positive bias
  const int idx = (cnt / 2) + 1;
  const int val = cbo->itemData(idx).toInt();

  if (val == 0)
    return idx;

  // cannot assume the list has an equal number of +/- values
  // therefore walk in either direction based on mid point value
  const int step = val > 0 ? -1 : 1;

  for (int i = idx + step; i >= 0 && i < cnt; i += step) {
    if (cbo->findData(i) == 0) {
      return i;
    } else if (step < 0 && cbo->itemData(i).toInt() < 0) {
      if (i + 1 < cnt) {
        return i + 1;
      } else {
        return i;
      }
    } else if (step > 0 && cbo->itemData(i).toInt() >= 0) {
      return i;
    }
  }

  // just in case
  return -1;
}

QString Helpers::concatPath(QString & str1, QString & str2, bool onlyonesep)
{
  return (str1 % ((!str1.endsWith("/") || !onlyonesep) ? "/" : "") % str2);
}

QString Helpers::concatPath(const QString & str1, const QString & str2, bool onlyonesep)
{
  return (str1 % ((!str1.endsWith("/") || !onlyonesep) ? "/" : "") % str2);
}

QString Helpers::concatPath(const QString & str1, QString & str2, bool onlyonesep)
{
  return (str1 % ((!str1.endsWith("/") || !onlyonesep) ? "/" : "") % str2);
}

QString Helpers::concatPath(QString & str1, const QString & str2, bool onlyonesep)
{
  return (str1 % ((!str1.endsWith("/") || !onlyonesep) ? "/" : "") % str2);
}

#ifdef __APPLE__
// Flag when simulator is running
static bool simulatorRunning = false;
bool isSimulatorRunning() { return simulatorRunning; }
#endif

void startSimulation(QWidget * parent, RadioData & radioData, int modelIdx)
{
  QString simulatorId = SimulatorLoader::findSimulatorByName(getCurrentFirmware()->getSimulatorId());
  if (simulatorId.isEmpty()) {
    QMessageBox::warning(NULL,
                         CPN_STR_TTL_WARNING,
                         QCoreApplication::translate("Companion", "Simulator for this firmware is not yet available"));
    return;
  }

  RadioData * simuData = new RadioData(radioData);
  unsigned int flags = 0;

  if (modelIdx >= 0) {
    flags |= SIMULATOR_FLAGS_NOTX;
    simuData->setCurrentModel(modelIdx);
  }

  SimulatorMainWindow * dialog = new SimulatorMainWindow(parent, simulatorId, flags);
  dialog->setWindowModality(Qt::ApplicationModal);
  dialog->setAttribute(Qt::WA_DeleteOnClose);

  QObject::connect(dialog, &SimulatorMainWindow::destroyed, [simuData] (void) {
#ifdef __APPLE__
    simulatorRunning = false;
#endif
    // TODO simuData and Horus tmp directory is deleted on simulator close OR we could use it to get back data from the simulation
    delete simuData;
  });

  QString resultMsg;
  if (dialog->getExitStatus(&resultMsg)) {
    if (resultMsg.isEmpty())
      resultMsg = QCoreApplication::translate("Companion", "Uknown error during Simulator startup.");
    QMessageBox::critical(NULL, QCoreApplication::translate("Companion", "Simulator Error"), resultMsg);
    dialog->deleteLater();
  } else if (dialog->setRadioData(simuData)) {
#ifdef __APPLE__
    simulatorRunning = true;
#endif
    dialog->show();
  } else {
    QMessageBox::critical(NULL, QCoreApplication::translate("Companion", "Data Load Error"), QCoreApplication::translate("Companion", "Error occurred while starting simulator."));
    dialog->deleteLater();
  }
}

QPixmap makePixMap(const QImage & image)
{
  Board::Type board = getCurrentBoard();
  QImage result = image.scaled(Boards::getCapability(board, Board::LcdWidth), Boards::getCapability(board, Board::LcdHeight));

  if (Boards::getCapability(board, Board::LcdDepth) == 4) {
    result = result.convertToFormat(QImage::Format_RGB32);
    for (int i = 0; i < result.width(); ++i) {
      for (int j = 0; j < result.height(); ++j) {
        QRgb col = result.pixel(i, j);
        int gray = qGray(col);
        result.setPixel(i, j, qRgb(gray, gray, gray));
      }
    }
  }
  else {
    result = result.convertToFormat(QImage::Format_Mono);
  }

  return QPixmap::fromImage(result);
}

int version2index(const QString & version)
{
  int result = 999;
  QStringList parts;
  QString mainVersion = version;
  if (version.contains("RC")) {
    parts = version.split("RC");
    result = parts[1].toInt() + 900; // RC0 = 900; RC1=901,..
    mainVersion = parts[0];
  }
  else if (version.contains("N")) {
    parts = version.split("N");
    result = parts[1].toInt(); // nightly build up to 899
    mainVersion = parts[0];
  }
  else if (version.contains("-")) {
    parts = version.split("-");
    mainVersion = parts[0];
  }
  parts = mainVersion.split('.');
  if (parts.size() > 2)
    result += 1000 * parts[2].toInt();
  if (parts.size() > 1)
    result += 100000 * parts[1].toInt();
  if (parts.size() > 0)
    result += 10000000 * parts[0].toInt();
  return result;
}

const QString index2version(int index)
{
  QString result;
  QString templt("%1.%2.%3");
  if (index >= 19900000) {
    int nightly = index % 1000;
    index /= 1000;
    int revision = index % 100;
    index /= 100;
    int minor = index % 100;
    int major = index / 100;
    result = templt.arg(major).arg(minor).arg(revision);
    if (nightly > 0 && nightly < 900) {
      result += "N" + QString::number(nightly);
    }
    else if (nightly >= 900 && nightly < 999) {
      result += "RC" + QString::number(nightly-900);
    }
  }
  else if (index >= 19900) {
    int revision = index % 100;
    index /= 100;
    int minor = index % 100;
    int major = index / 100;
    result = templt.arg(major).arg(minor).arg(revision);
  }
  return result;
}

bool qunlink(const QString & fileName)
{
  return QFile::remove(fileName);
}

QString generateProcessUniqueTempFileName(const QString & fileName)
{
  QString sanitizedFileName = fileName;
  sanitizedFileName.remove('/');
  return QDir::tempPath() + QString("/%1-").arg(QCoreApplication::applicationPid()) + sanitizedFileName;
}

bool isTempFileName(const QString & fileName)
{
  return fileName.startsWith(QDir::tempPath());
}

QString getSoundsPath(const GeneralSettings &generalSettings)
{
  QString path = g.profile[g.id()].sdPath() + "/SOUNDS/";
  QString lang = generalSettings.ttsLanguage;
  if (lang.isEmpty())
    lang = "en";
  path.append(lang);
  return path;
}

QSet<QString> getFilesSet(const QString &path, const QStringList &filter, int maxLen)
{
  QSet<QString> result;
  QDir dir(path);
  if (dir.exists()) {
    foreach (QString filename, dir.entryList(filter, QDir::Files)) {
      QFileInfo file(filename);
      QString name = file.completeBaseName();
      if (name.length() <= maxLen) {
        result.insert(name);
      }
    }
  }
  return result;
}

// based on /src/repos/edgetx/edgetx/radio/src/gui/colorlcd/radio/radio_tools.cpp
// loadLuaTools()
QStringList getListLuaTools()
{
  const QString tdir = g.profile[g.id()].sdPath() + "/SCRIPTS/TOOLS";
  QStringList result;
  QDir dir(tdir);

  if (dir.exists()) {
    foreach (QString name, dir.entryList(QStringList() << "*.lua",
             QDir::Files | QDir::AllDirs | QDir::NoDotAndDotDot | QDir::NoSymLinks)) {
      QString path = tdir % "/" % name;
      QFileInfo fi(path);
      bool inFolder = fi.isDir();

      QString toolname;

      if (inFolder) {
        // check if .lua with same name exists - skip folder to avoid duplicate entries
        if (QFileInfo::exists(path % ".lua"))
          continue;
        // Default name is folder name
        toolname = QDir(path).dirName();
        // must have main.lua
        path.append("/main.lua");
        if (!QFileInfo::exists(path))
          continue;
      } else {
        // Default name is file name
        toolname = QFileInfo(path).completeBaseName();
      }

      // look for tool name in lua file to override default name
      QFile file(path);

      if (file.open(QFile::ReadOnly)) {
        QByteArray ba = file.read(1024);
        file.close();

        if (ba.size() > 0) {
          QByteArrayView view("TNS|");
          int startpos = ba.indexOf(view);

          if (startpos > -1) {
            view = "|TNE";
            int endpos = ba.indexOf(view, startpos + 1);

            if (endpos > -1) {
              toolname = ba.mid(startpos + 4, endpos - (startpos + 4)).data();
            }
          }
        }
      }

      result.append(toolname);
    }
  }

  return result;
}

bool GpsGlitchFilter::isGlitch(GpsCoord coord)
{
  if ((fabs(coord.latitude) < 0.1) && (fabs(coord.longitude) < 0.1)) {
    return true;
  }

  if (lastValid) {
    if (fabs(coord.latitude - lastLat) > 0.01) {
      // qDebug() << "GpsGlitchFilter(): latitude glitch " << coord.latitude << lastLat;
      if ( ++glitchCount < 10) {
        return true;
      }
    }
    if (fabs(coord.longitude - lastLon) > 0.01) {
      // qDebug() << "GpsGlitchFilter(): longitude glitch " << coord.longitude << lastLon;
      if ( ++glitchCount < 10) {
        return true;
      }
    }
  }
  lastLat = coord.latitude;
  lastLon = coord.longitude;
  lastValid = true;
  glitchCount = 0;
  return false;
}

bool GpsLatLonFilter::isValid(GpsCoord coord)
{
  if (lastLat == coord.latitude) {
    return false;
  }
  if (lastLon == coord.longitude) {
    return false;
  }
  lastLat = coord.latitude;
  lastLon = coord.longitude;
  return true;
}

double toDecimalCoordinate(const QString & value)
{
  if (value.isEmpty()) return 0.0;
  double temp = int(value.left(value.length()-1).toDouble() / 100);
  double result = temp + (value.left(value.length() - 1).toDouble() - temp * 100) / 60.0;
  QChar direction = value.at(value.size()-1);
  if ((direction == 'S') || (direction == 'W')) {
    result = -result;
  }
  return result;
}

GpsCoord extractGpsCoordinates(const QString & position)
{
  GpsCoord result;
  QStringList parts = position.split(' ');
  if (parts.size() == 2) {
    QString value = parts.at(0).trimmed();
    QChar direction = value.at(value.size()-1);
    if (direction == 'E' || direction == 'W') {
      // OpenTX 2.1 format: "NNN.MMM[E|W] NNN.MMM[N|S]" <longitude> <latitude>
      result.latitude = toDecimalCoordinate(parts.at(1).trimmed());
      result.longitude = toDecimalCoordinate(parts.at(0).trimmed());
    }
    else {
      // OpenTX 2.2 format: "DD.DDDDDD DD.DDDDDD" <latitude> <longitude> both in Signed degrees format (DDD.dddd)
      // Precede South latitudes and West longitudes with a minus sign.
      // Latitudes range from -90 to 90.
      // Longitudes range from -180 to 180.
      result.latitude = parts.at(0).trimmed().toDouble();
      result.longitude = parts.at(1).trimmed().toDouble();
    }
  }
  return result;
}

TableLayout::TableLayout(QWidget * parent, int rowCount, const QStringList & headerLabels) :
  QObject(parent)
{
#if defined(TABLE_LAYOUT)
  tableWidget = new QTableWidget(parent);
  QVBoxLayout * layout = new QVBoxLayout();
  layout->addWidget(tableWidget);
  layout->setContentsMargins(0, 0, 0, 0);
  parent->setLayout(layout);

  tableWidget->setRowCount(rowCount);
  tableWidget->setColumnCount(headerLabels.size());
  tableWidget->setShowGrid(false);
  tableWidget->verticalHeader()->setVisible(false);
  tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
  tableWidget->setSelectionMode(QAbstractItemView::NoSelection);
  tableWidget->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
  tableWidget->setStyleSheet("QTableWidget {background-color: transparent;}");
  tableWidget->setHorizontalHeaderLabels(headerLabels);
#else
  gridWidget = new QGridLayout(parent);

  int col = 0;
  foreach(QString text, headerLabels) {
    addColumnHead(text, col++);
  }
#endif
}

void TableLayout::addWidget(int row, int column, QWidget * widget, Qt::Alignment alignment)
{
#if defined(TABLE_LAYOUT)
  QHBoxLayout * layout = new QHBoxLayout(tableWidget);
  layout->addWidget(widget);
  addLayout(row, column, layout);
#else
  gridWidget->addWidget(widget, row + 1, column, alignment);
#endif
}

void TableLayout::addWidget(int fromRow, int fromColumn, int rowSpan, int colSpan,
                            QWidget * widget, Qt::Alignment alignment)
{
#if defined(TABLE_LAYOUT)
#else
  gridWidget->addWidget(widget, fromRow + 1, fromColumn, rowSpan, colSpan, alignment);
#endif
}

void TableLayout::addLayout(int row, int column, QLayout * layout, Qt::Alignment alignment)
{
#if defined(TABLE_LAYOUT)
  layout->setContentsMargins(1, 3, 1, 3);
  QWidget * containerWidget = new QWidget(tableWidget);
  containerWidget->setLayout(layout);
  tableWidget->setCellWidget(row, column, containerWidget);
#else
  gridWidget->addLayout(layout, row + 1, column, alignment);
#endif
}

void TableLayout::resizeColumnsToContents()
{
#if defined(TABLE_LAYOUT)
  tableWidget->resizeColumnsToContents();
#else
#endif
}

void TableLayout::setColumnWidth(int col, int width)
{
#if defined(TABLE_LAYOUT)
  tableWidget->setColumnWidth(col, width);
#else
  gridWidget->setColumnMinimumWidth(col, width);
#endif
}

void TableLayout::setColumnWidth(int col, QString str)
{
#if defined(TABLE_LAYOUT)
#else
  QFontMetrics *f = new QFontMetrics(QFont());
  QSize sz = f->size(Qt::TextSingleLine, str);
  setColumnWidth(col, sz.width());
#endif
}

void TableLayout::pushRowsUp(int row)
{
#if defined(TABLE_LAYOUT)
#else
  // Push the rows up
  QSpacerItem * spacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  gridWidget->addItem(spacer, row, 0);
#endif
}

void TableLayout::pushColumnsLeft(int col)
{
#if defined(TABLE_LAYOUT)
#else
  // Push the columns to the left
  QSpacerItem * spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
  gridWidget->addItem(spacer, 0, col);
#endif
}

void TableLayout::setColumnStretch(int col, int stretch)
{
#if defined(TABLE_LAYOUT)
#else
  gridWidget->setColumnStretch(col, stretch);
#endif
}

void TableLayout::addColumnHead(QString text, int col, int colSpan)
{
  QLabel *label = new QLabel();
  label->setFrameShape(QFrame::Panel);
  label->setFrameShadow(QFrame::Raised);
  label->setMidLineWidth(0);
  label->setAlignment(Qt::AlignCenter);
  label->setMargin(2);
  label->setText(text);
  // if (!minimize)
  //   label->setMinimumWidth(100);
  gridWidget->addWidget(label, 0, col, 1, colSpan);
}

void TableLayout::updateColumnHeading(int col, QString &text)
{
#if defined(TABLE_LAYOUT)
#else
  QLayoutItem *item = gridWidget->itemAtPosition(0, col);

  if (item) {
    QWidget *widget = item->widget();
    if (widget) {
      QLabel *lbl = qobject_cast<QLabel*>(widget);
      if (lbl) {
        lbl->setText(text);
      }
    }
  }
#endif
}

QString Helpers::removeAccents(const QString & str)
{
  // UTF-8 ASCII Table
  static const QHash<QString, QVariant> map = {
    {"a", QRegularExpression("[áâãàä]")},
    {"A", QRegularExpression("[ÁÂÃÀÄ]")},
    {"e", QRegularExpression("[éèêě]")},
    {"E", QRegularExpression("[ÉÈÊĚ]")},
    {"o", QRegularExpression("[óôõö]")},
    {"O", QRegularExpression("[ÓÔÕÖ]")},
    {"u", QRegularExpression("[úü]")},
    {"U", QRegularExpression("[ÚÜ]")},
    {"i", "í"}, {"I", "Í"},
    {"c", "ç"}, {"C", "Ç"},
    {"y", "ý"}, {"Y", "Ý"},
    {"s", "š"}, {"S", "Š"},
    {"r", "ř"}, {"R", "Ř"}
  };

  QString result(str);
  for (QHash<QString, QVariant>::const_iterator it = map.cbegin(), en = map.cend(); it != en; ++it) {
    if (it.value().canConvert<QRegularExpression>())
      result.replace(it.value().toRegularExpression(), it.key());
    else
      result.replace(it.value().toString(), it.key());
  }
  return result;
}

StatusDialog::StatusDialog(QWidget * parent, const QString title, QString msgtext, const int width) :
  QDialog(parent)
{
  setWindowTitle(title);
  QVBoxLayout *layout = new QVBoxLayout(this);
  msg = new QLabel(this);
  msg->setFixedWidth(width);
  msg->setContentsMargins(50, 50, 50, 50);
  update(msgtext);
  layout->addWidget(msg);
  show();
}

StatusDialog::~StatusDialog()
{
}

void StatusDialog::update(QString text)
{
  msg->setText(text);
}
