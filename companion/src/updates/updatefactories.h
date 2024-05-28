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

#include "updateinterface.h"

#include <QtCore>

class UpdateFactoryInterface
{
  public:
    explicit UpdateFactoryInterface() {}
    virtual ~UpdateFactoryInterface() {}

    virtual const int id() = 0;
    virtual UpdateInterface* const instance() = 0;
    virtual const QString name() = 0;
};

template <class T>
class UpdateFactory : public UpdateFactoryInterface
{
  public:
    explicit UpdateFactory(QWidget * parent) :
      UpdateFactoryInterface(),
      m_instance(new T(parent)) {}

    virtual ~UpdateFactory() {}

    virtual const int id() { return m_instance->id(); }
    virtual UpdateInterface* const instance() { return m_instance; }
    virtual const QString name() { return m_instance->name(); }

  private:
    UpdateInterface* const m_instance;
};

class UpdateFactories : public QWidget
{
    Q_OBJECT

  public:
    explicit UpdateFactories(QWidget * parent = nullptr);
    virtual ~UpdateFactories();

    UpdateInterface* const instance(const int id);
    const bool isUpdateAvailable(QMap<QString, int> & names);
    void radioProfileChanged();
    void registerUpdateFactories();
    void registerUpdateFactory(UpdateFactoryInterface * factory);
    void resetAllEnvironments();
    const QMap<QString, int> sortedComponentsList(bool updateableOnly = false);
    void unregisterUpdateFactories();
    bool updateAll(ProgressWidget * progress = nullptr);

  private:
    QVector<UpdateFactoryInterface *> registeredUpdateFactories;
};

