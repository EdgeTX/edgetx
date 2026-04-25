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

#include "simulatorinterface.h"
#include "wasmsimulatorinterface.h"
#include "customdebug.h"
#include "version.h"
#include "firmwares/eeprominterface.h"
#include <QLibrary>
#include <QDebug>
#include <QLibraryInfo>

QMap<QString, SimulatorFactory *> SimulatorLoader::registeredSimulators;

QStringList SimulatorLoader::getAvailableSimulators()
{
  return registeredSimulators.keys();
}

SimulatorFactory * SimulatorLoader::loadSimulatorFactory(const QString & path, const QString & name, Board::Type boardType)
{
  QLibrary lib(path);
  if (!lib.load()) {
    qWarning() << "Failed to load simulator library:" << path << lib.errorString();
    return nullptr;
  }
  typedef SimulatorFactory * (*FactoryFunc)(void);
  FactoryFunc factoryFunc = (FactoryFunc)lib.resolve("simulatorInterfaceLoader");
  if (!factoryFunc) {
    qWarning() << "Failed to resolve simulatorInterfaceLoader in:" << path;
    return nullptr;
  }
  return factoryFunc();
}

int SimulatorLoader::registerSimulators(const QDir & dir)
{
  QStringList wasmFilters;
  wasmFilters << "edgetx-*-simulator.wasm"
	      << "libedgetx-*-simulator.so";

  qCDebug(simulatorInterfaceLoader) << "Searching for WASM simulators in"
                                    << dir.path();
  foreach (QString filename, dir.entryList(wasmFilters, QDir::Files)) {
    QString simuName = filename;
    bool isSo = filename.endsWith(".so");
    if (isSo) {
      simuName.remove(0, 10);  // remove "libedgetx-"
    } else {
      simuName.remove(0, 7);   // remove "edgetx-"
    }
    simuName.truncate(simuName.lastIndexOf("-simulator"));
    if (registeredSimulators.contains(simuName))
      continue;
    QString simPath = dir.path() + "/" + filename;
    Board::Type boardType = Board::BOARD_UNKNOWN;
    Firmware * fw = Firmware::getFirmwareForId(QString("edgetx-") + simuName);
    if (fw)
      boardType = fw->getBoard();
    SimulatorFactory * factory = nullptr;
    if (isSo) {
      factory = SimulatorLoader::loadSimulatorFactory(simPath, simuName, boardType);
    } else {
      factory = new WasmSimulatorFactory(simPath, simuName, boardType);
    }
    if (factory) {
      registeredSimulators.insert(simuName, factory);
      qCDebug(simulatorInterfaceLoader) << "Registered simulator:" << simuName;
    }
  }

  qCDebug(simulatorInterfaceLoader)
      << "Found WASM modules:"
      << (registeredSimulators.size() ? registeredSimulators.keys()
                                      : QStringList() << "none");
  return registeredSimulators.size();
}

void SimulatorLoader::registerSimulators()
{
  QDir dir(QCoreApplication::applicationDirPath());
  if (registerSimulators(dir)) {
    return;
  }

#if defined(__APPLE__)
  dir.setPath(QLibraryInfo::path(QLibraryInfo::PrefixPath) + "/Resources");
#else
  if (QDir::isAbsolutePath(SIMULATOR_LIB_SEARCH_PATH)) {
    dir.setPath(SIMULATOR_LIB_SEARCH_PATH);
  } else {
    dir.setPath(QCoreApplication::applicationDirPath() + "/"
                SIMULATOR_LIB_SEARCH_PATH);
  }
#endif
  registerSimulators(dir.absolutePath());
}

void SimulatorLoader::unregisterSimulators()
{
  qDeleteAll(registeredSimulators);
  registeredSimulators.clear();
}

QString SimulatorLoader::findSimulatorByName(const QString & name)
{
  int pos;
  QString simuName = name;

  // Strip "edgetx-" prefix if present (registered names don't have it)
  if (simuName.startsWith("edgetx-"))
    simuName.remove(0, 7);

  while (1) {
    qCDebug(simulatorInterfaceLoader) << "searching" << simuName
                                      << "simulator";
    if (registeredSimulators.contains(simuName))
      return simuName;
    if ((pos = simuName.lastIndexOf('-')) <= 0)
      break;
    simuName = simuName.mid(0, pos);
  }
  return QString();
}

SimulatorInterface * SimulatorLoader::loadSimulator(const QString & name)
{
  QString simuName = findSimulatorByName(name);

  if (simuName.isEmpty()) {
    qWarning() << "Simulator" << name << "not found.";
    return nullptr;
  }

  SimulatorInterface * si = registeredSimulators[simuName]->create();
  if (si)
    qCDebug(simulatorInterfaceLoader) << "Loaded simulator:" << simuName;

  return si;
}

bool SimulatorLoader::unloadSimulator(const QString & name)
{
  Q_UNUSED(name);
  return true;
}
