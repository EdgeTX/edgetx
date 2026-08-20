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

#include "firmware.h"

class FirmwareFactory
{
  public:
    explicit FirmwareFactory(const QString & id, const QString & path, const bool isSupported = true);

    virtual ~FirmwareFactory() {}

    Firmware* firmware() const { return m_firmware; }

  private:
    Firmware *m_firmware;
};

class FirmwareFactories
{
  public:
    explicit FirmwareFactories();
    virtual ~FirmwareFactories();

    Firmware* firmware(const QString & id) const;

    bool registerFirmware(const QString & id, const QString & path, const bool isSupported = true);
    bool registerFactory(FirmwareFactory * factory);
    void unregisterFactories();
    void registerAllFirmwares();
    bool loadDefinition(const QString & id);

    QMap<QString, QString> registeredFirmware();

    // depeciated v3.0
    QList<Firmware*> getRegisteredFirmwares();

  private:
    QList<FirmwareFactory *> registeredFactories;
};

extern FirmwareFactories* gFirmwareFactories;
