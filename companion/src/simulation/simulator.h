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

#include <QColor>
#include <QDataStream>
#include <QDebug>
#include <QFile>
#include <QIcon>
#include <QPair>
#include <QString>

#define SIMULATOR_FLAGS_NOTX         0x01  // simulating a single model from Companion
#define SIMULATOR_FLAGS_STANDALONE   0x02  // started from stanalone simulator

#define SIMULATOR_OPTIONS_VERSION    3

namespace Simulator
{

  typedef QPair<QString, QString> keymapHelp_t;

  class SimulatorStyle
  {
    public:
      SimulatorStyle() { }
      static QString const basePath() { return ":/themes/default"; }
      static QString const styleSheet()
      {
        QString css;
        QFile fh(QString("%1/style.css").arg(basePath()));
        if (fh.open(QFile::ReadOnly | QFile::Text)) {
          css = fh.readAll();
          fh.close();
        }
#ifdef __APPLE__
        fh.setFileName(QString("%1/style-osx.css").arg(basePath()));
        if (fh.open(QFile::ReadOnly | QFile::Text)) {
          css.append(fh.readAll());
          fh.close();
        }
#endif
        return css;
      }
  };  // SimulatorStyle

  class SimulatorIcon : public QIcon
  {
    public:
      SimulatorIcon(const QString & baseImage)
      {
        QString baseFile = QString("%1/%2").arg(basePath(), baseImage);
        addFile(QString("%1.svg").arg(baseFile));

        QString addfile = QString("%1-on.svg").arg(baseFile);
        if (QFile(addfile).exists())
          addFile(addfile, QSize(), QIcon::Normal, QIcon::On);

        addfile = QString("%1-active.svg").arg(baseFile);
        if (QFile(addfile).exists())
          addFile(addfile, QSize(), QIcon::Active, QIcon::Off);

        addfile = QString("%1-disabled.svg").arg(baseFile);
        if (QFile(addfile).exists())
          addFile(addfile, QSize(), QIcon::Disabled, QIcon::Off);
      }

      static QString const basePath() { return ":/images/simulator/icons/svg"; }

      static QSize const toolbarIconSize(int setting)
      {
        switch(setting) {
          case 0:
            return QSize(16, 16);
          case 2:
            return QSize(32, 32);
          case 3:
            return QSize(48, 48);
          case 1:
          default:
            return QSize(24, 24);
        }
      }
  };  // class SimulatorIcon

}  // namespace Simulator

struct SimulatorOptions
{
    // NOTE: now that the main property/settings system is easier to use, it would probably be better to add new settings directly to Profiles.
    // TODO: perhaps refactor this some day.

    enum StartupDataTypes {
      START_WITH_FILE = 0,
      START_WITH_FOLDER,
      START_WITH_SDPATH,
      START_WITH_RAWDATA
    };

    // v1 fields
    quint8  startupDataType = START_WITH_FILE;
    QString firmwareId;
    QString dataFile;
    QString dataFolder;
    QString sdPath;
    QByteArray windowGeometry;        // SimulatorMainWindow geometry
    QList<QByteArray> controlsState;  // saved switch/pot/stick settings
    QColor lcdColor;
    // added in v2
    QByteArray windowState;           // SimulatorMainWindow dock/toolbar/options UI state
    QByteArray dbgConsoleState;       // DebugOutput UI state
    QByteArray radioOutputsState;     // RadioOutputsWidget UI state
    // added in v3
    QString simulatorId;

    quint16 loadedVersion() const { return m_version; }  //! loaded structure definition version (0 if none/error)

    friend QDataStream & operator << (QDataStream &out, const SimulatorOptions & o)
    {
      out << quint16(SIMULATOR_OPTIONS_VERSION) << o.startupDataType << o.firmwareId << o.dataFile << o.dataFolder
          << o.sdPath << o.windowGeometry << o.controlsState << o.lcdColor << o.windowState << o.dbgConsoleState << o.radioOutputsState
          << o.simulatorId;
      return out;
    }

    friend QDataStream & operator >> (QDataStream &in, SimulatorOptions & o)
    {
      in >> o.m_version;
      if (o.m_version && o.m_version <= SIMULATOR_OPTIONS_VERSION) {
        in >> o.startupDataType >> o.firmwareId >> o.dataFile >> o.dataFolder
           >> o.sdPath >> o.windowGeometry >> o.controlsState >> o.lcdColor;
        if (o.m_version >= 2)
          in >> o.windowState >> o.dbgConsoleState >> o.radioOutputsState;
        if (o.m_version >= 3)
          in >> o.simulatorId;
      }
      else {
        qWarning() << "Error loading SimulatorOptions, saved version not valid:" << o.m_version << "Expected <=" << SIMULATOR_OPTIONS_VERSION;
        o.m_version = 0;
      }
      return in;
    }

    inline bool operator == (const SimulatorOptions &rhs) const
    {
      bool ret = (dataFile == rhs.dataFile && dataFolder == rhs.dataFolder && sdPath == rhs.sdPath &&
                  windowGeometry == rhs.windowGeometry && lcdColor == rhs.lcdColor &&
                  windowState == rhs.windowState && dbgConsoleState == rhs.dbgConsoleState && radioOutputsState == rhs.radioOutputsState);
      if (!ret || controlsState.size() != rhs.controlsState.size()) {
        return false;
      }
      int i = 0;
      for (const QByteArray &cs : controlsState) {
        if (cs != rhs.controlsState.at(i++)) {
          return false;
        }
      }
      return true;
    }
    inline bool operator != (const SimulatorOptions &rhs) const { return !(*this == rhs); }

    friend QDebug operator << (QDebug d, const SimulatorOptions & o)
    {
      QDebugStateSaver saver(d);
      d.nospace() << "SimulatorOptions: firmwareId=" << o.firmwareId << "; simulatorId=" << o.simulatorId << "; dataFile=" << o.dataFile << "; dataFolder=" << o.dataFolder
                  << "; sdPath=" << o.sdPath << "; startupDataType=" << o.startupDataType;
      return d;
    }

    // this is a silly operator for this case, but required for QMetaType::registerComparators() to use our == comparison operators when casting as QVariant.
    inline bool operator < (const SimulatorOptions &rhs) const { return firmwareId < rhs.firmwareId; }

  private:
    quint16 m_version = 0;
};

Q_DECLARE_METATYPE(SimulatorOptions)
