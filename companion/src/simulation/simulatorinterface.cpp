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

#ifndef SIMULATOR_INTERFACE_LOADER_METHOD
  #define SIMULATOR_INTERFACE_LOADER_DYNAMIC    1  // How to load simulator libraries: 1=dynamic load and unload; 0=load once (old way)
#endif

QMap<QString, QLibrary *> SimulatorLoader::registeredSimulators;

QStringList SimulatorLoader::getAvailableSimulators()
{
  return registeredSimulators.keys();
}

int SimulatorLoader::registerSimulators(const QDir & dir)
{
  QStringList filters;
#if defined(__APPLE__)
  filters << "*-simulator.dylib";
#elif defined(WIN32) || defined(__CYGWIN__)
  filters << "*-simulator.dll";
#else
  filters << "*-simulator.so";
#endif
  registeredSimulators.clear();

  qCDebug(simulatorInterfaceLoader) << "Searching for simulators in" << dir.path() << "matching pattern" << filters;

  foreach(QString filename, dir.entryList(filters, QDir::Files)) {
    QLibrary * lib = new QLibrary( dir.path() + "/" + filename);

    qCDebug(simulatorInterfaceLoader) << "Trying to register simulator in " << filename;

    SimulatorFactory * factory;
    RegisterSimulator registerFunc = (RegisterSimulator)lib->resolve("registerSimu");

    if (registerFunc && (factory = registerFunc())) {
      if (getAvailableSimulators().contains(factory->name()))
        continue;

      lib->setProperty("instances_used", 0);
      registeredSimulators.insert(factory->name(), lib);
      delete factory;
#if SIMULATOR_INTERFACE_LOADER_DYNAMIC
      lib->unload();
#endif
      qCDebug(simulatorInterfaceLoader) << "Registered" << registeredSimulators.lastKey() << "simulator in " << lib->fileName() << "and unloaded:" << !lib->isLoaded();
    }
    else {
      qWarning() << "Library error" << lib->fileName() << lib->errorString();
      delete lib;
    }

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
  dir = QLibraryInfo::location(QLibraryInfo::PrefixPath) + "/Resources";
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
  foreach(QLibrary * lib, registeredSimulators)
    delete lib;
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
  SimulatorInterface * si = NULL;
  QString libname = findSimulatorByName(name);

  if (libname.isEmpty()) {
    qWarning() << "Simulator" << name << "not found.";
    return si;
  }

  QLibrary * lib = registeredSimulators.value(libname, NULL);
  if (!lib) {
    qWarning() << "Simulator library is NULL";
    return si;
  }

  qCDebug(simulatorInterfaceLoader) << "Trying to load simulator in " << lib->fileName();

  SimulatorFactory * factory;
  RegisterSimulator registerFunc = (RegisterSimulator)lib->resolve("registerSimu");
  if (registerFunc && (factory = registerFunc()) && (si = factory->create())) {
    quint8 instance = lib->property("instances_used").toUInt();
    lib->setProperty("instances_used", ++instance);
    qCDebug(simulatorInterfaceLoader) << "Loaded" << factory->name() << "simulator instance" << instance;
    delete factory;
  }
  else {
    qWarning() << "Library error" << lib->fileName() << lib->errorString();
  }
  return si;
}

bool SimulatorLoader::unloadSimulator(const QString & name)
{
  bool ret = false;
#if SIMULATOR_INTERFACE_LOADER_DYNAMIC
  QString simuName = findSimulatorByName(name);
  if (simuName.isEmpty())
    return ret;

  QLibrary * lib = registeredSimulators.value(simuName, NULL);

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
#else
  qCDebug(simulatorInterfaceLoader) << "Keeping simulator library" << simuName << "loaded.";
#endif

  return ret;
}
