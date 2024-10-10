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

#include "updatefactories.h"
#include "updatefirmware.h"
#include "updatecompanion.h"
#include "updatesdcard.h"
#include "updatesounds.h"
#include "updatethemes.h"
#include "updatemultiprotocol.h"
#include "updatecloudbuild.h"

UpdateFactories::UpdateFactories(QWidget * parent) :
  QWidget(parent)
{
  registerUpdateFactories();
}

UpdateFactories::~UpdateFactories()
{
  unregisterUpdateFactories();
}

UpdateInterface* const UpdateFactories::instance(const int id)
{
  foreach (UpdateFactoryInterface * factory, registeredUpdateFactories) {
    if (id == factory->id())
      return factory->instance();
  }
  qDebug() << "Critical error - Interface not found for id:" << id;
  return nullptr;
}

const bool UpdateFactories::isUpdateAvailable(QMap<QString, int> & list)
{
  bool ret = false;

  list.clear();

  foreach (UpdateFactoryInterface * factory, registeredUpdateFactories) {
    if (factory->instance()->isUpdateable() && factory->instance()->isUpdateAvailable()) {
      list.insert(factory->name(), factory->id());
      ret = true;
    }
  }

  return ret;
}

void UpdateFactories::radioProfileChanged()
{
  foreach (UpdateFactoryInterface * factory, registeredUpdateFactories) {
    factory->instance()->radioProfileChanged();
  }
}

void UpdateFactories::registerUpdateFactory(UpdateFactoryInterface * factory)
{
  foreach (UpdateFactoryInterface * registeredFactory, registeredUpdateFactories) {
    if (registeredFactory->id() == factory->id()) {
      qDebug() << "Duplicate factory - id:" << factory->id() << "name:" << factory->name();
      return;
    }
  }
  registeredUpdateFactories.append(factory);
  qDebug() << "Registered update factory:" << factory->name();
}

void UpdateFactories::registerUpdateFactories()
{
  //  Note: register in logical updating sequence
  //        SDCard should be first as it may clean up/erase commmon destination directory
  registerUpdateFactory(new UpdateFactory<UpdateSDCard>(this));
  registerUpdateFactory(new UpdateFactory<UpdateFirmware>(this));
  registerUpdateFactory(new UpdateFactory<UpdateSounds>(this));
  registerUpdateFactory(new UpdateFactory<UpdateThemes>(this));
  registerUpdateFactory(new UpdateFactory<UpdateMultiProtocol>(this));
  registerUpdateFactory(new UpdateFactory<UpdateCloudBuild>(this));

  //  Note: Companion must be last as its install requires the app to be closed and thus would interrupt the update loop
  registerUpdateFactory(new UpdateFactory<UpdateCompanion>(this));
}

void UpdateFactories::resetAllEnvironments()
{
  foreach (UpdateFactoryInterface * factory, registeredUpdateFactories) {
    factory->instance()->resetEnvironment();
  }
}

const QMap<QString, int> UpdateFactories::sortedComponentsList(bool updateableOnly)
{
  QMap<QString, int> map;

  foreach (UpdateFactoryInterface * factory, registeredUpdateFactories) {
    if (updateableOnly && !factory->instance()->isUpdateable())
      continue;
    map.insert(factory->name(), factory->id());
  }

  return map;
}

bool UpdateFactories::updateAll(ProgressWidget * progress)
{
  bool ret = false;

  foreach (UpdateFactoryInterface * factory, registeredUpdateFactories) {
    ret = factory->instance()->update(progress);
    if (!ret)
      break;
  }

  return ret;
}

void UpdateFactories::unregisterUpdateFactories()
{
  foreach (UpdateFactoryInterface * factory, registeredUpdateFactories)
    delete factory;
}
