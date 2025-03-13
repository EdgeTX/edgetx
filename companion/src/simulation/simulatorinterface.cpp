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
#include "customdebug.h"
#include "version.h"

#include <QDebug>
#include <QLibraryInfo>

#if defined _MSC_VER || !defined __GNUC__
  #include <windows.h>
#endif

QMap<QString, QPair<QString, QLibrary *>> SimulatorLoader::registeredSimulators;

QStringList SimulatorLoader::getAvailableSimulators()
{
  return registeredSimulators.keys();
}

int SimulatorLoader::registerSimulators(const QDir & dir)
{
  QStringList filters;
#if defined(__APPLE__)
  filters << "libedgetx-*-simulator.dylib";
#elif defined(WIN32) || defined(__CYGWIN__)
  filters << "libedgetx-*-simulator.dll";
#else
  filters << "libedgetx-*-simulator.so";
#endif
  registeredSimulators.clear();

  qCDebug(simulatorInterfaceLoader) << "Searching for simulators in" << dir.path() << "matching pattern" << filters;

  foreach(QString filename, dir.entryList(filters, QDir::Files)) {
    QString simuName(filename.mid(3, filename.lastIndexOf('-') - 3));
    QString libPath(dir.path() + "/" + filename);

    if (getAvailableSimulators().contains(simuName))
      continue;

    registeredSimulators.insert(simuName, {libPath, nullptr});
  }

  qCDebug(simulatorInterfaceLoader) << "Found libraries:" << (registeredSimulators.size() ? registeredSimulators.keys() : QStringList() << "none");
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
    dir.setPath(QCoreApplication::applicationDirPath() + "/" SIMULATOR_LIB_SEARCH_PATH);
  }
#endif
  registerSimulators(dir.absolutePath());
}

void SimulatorLoader::unregisterSimulators()
{
  for(QPair<QString, QLibrary *> lib : registeredSimulators) {
    if (lib.second)
      delete lib.second;
  }
}

QString SimulatorLoader::findSimulatorByName(const QString & name)
{
  int pos;
  QString ret;
  QString simuName = name;

  while(1) {
    qCDebug(simulatorInterfaceLoader) << "searching" << simuName << "simulator";
    if (registeredSimulators.contains(simuName)) {
      ret = simuName;
      break;
    }
    if ((pos = simuName.lastIndexOf('-')) <= 0)
      break;
    simuName = simuName.mid(0, pos);
    if (simuName.count('-') == 0)
      break;
  }
  return ret;
}

SimulatorInterface * SimulatorLoader::loadSimulator(const QString & name)
{
  SimulatorInterface * si = nullptr;
  QString simuName = findSimulatorByName(name);

  if (simuName.isEmpty()) {
    qWarning() << "Simulator" << name << "not found.";
    return si;
  }

  QPair<QString, QLibrary *> libInfo = registeredSimulators.value(simuName, {QString(), nullptr});
  QString libPath = libInfo.first;
  QLibrary *lib = libInfo.second;
  qCDebug(simulatorInterfaceLoader) << "Trying to load simulator in " << libPath;

  if (!lib) {
    lib = new QLibrary(libPath);
    if (lib)
      registeredSimulators.insert(simuName, {libPath, lib});
    else {
      qWarning() << "Unable to load library";
      return si;
    }
  }

  SimulatorFactory * factory;
  RegisterSimulator registerFunc = (RegisterSimulator)lib->resolve("registerSimu");
  if (registerFunc && (factory = registerFunc()) && (si = factory->create())) {
    quint8 instance = lib->property("instances_used").toUInt();
    lib->setProperty("instances_used", ++instance);
    qCDebug(simulatorInterfaceLoader) << "Loaded" << factory->name() << "simulator instance" << instance;
    delete factory;
  }
  else {
    qWarning() << "Library error" << libPath << lib->errorString();
  }

  return si;
}

bool SimulatorLoader::unloadSimulator(const QString & name)
{
  bool ret = false;
  QString simuName = findSimulatorByName(name);

  if (simuName.isEmpty())
    return ret;

  QLibrary * lib = registeredSimulators.value(simuName).second;

  if (lib && lib->isLoaded()) {
    quint8 instance = lib->property("instances_used").toUInt();
    lib->setProperty("instances_used", --instance);
    if (!instance) {
      ret = lib->unload();
      qCDebug(simulatorInterfaceLoader) << "Unloading" << simuName << "(" << lib->fileName() << ")" << "result:" << ret;
    }
    else {
      ret = true;
      qCDebug(simulatorInterfaceLoader) << "Simulator" << simuName << "instances remaining:" << instance;
    }
  }
  else {
    qCDebug(simulatorInterfaceLoader) << "Simulator library for " << simuName << "already unloaded.";
  }

  return ret;
}
