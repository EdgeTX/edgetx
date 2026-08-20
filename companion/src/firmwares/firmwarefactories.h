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

class FirmwareFactories;

class FirmwareFactory
{
  protected:
    // only allow FirmwareFactories to instantiate
    friend FirmwareFactories;

    explicit FirmwareFactory(const QString & id);
    virtual ~FirmwareFactory() = default;

    Firmware* firmware() const { return m_firmware; }

  private:
    Firmware *m_firmware;
};

class FirmwareFactories
{
  public:
    // Delete copy constructor and assignment operators to prevent duplicates
    FirmwareFactories(const FirmwareFactories&) = delete;
    FirmwareFactories& operator=(const FirmwareFactories&) = delete;
    FirmwareFactories(FirmwareFactories&&) = delete;
    FirmwareFactories& operator=(FirmwareFactories&&) = delete;

    // Public static method acting as the single access point
    static FirmwareFactories& getInstance() {
        // Guaranteed to be destroyed and instantiated thread-safely on first use
        static FirmwareFactories instance;
        return instance;
    }

    Firmware* getFirmware(const QString & id, bool forceLoad = true) const;
    bool isAvailable(const QString & id) const;

    QMap<QString, QString> registeredFirmwares() const;
    QList<Firmware *> getRegisteredFirmwares() const;

  private:
    // Keep constructor and destructor private to prevent external instantiation
    FirmwareFactories();
    ~FirmwareFactories();

    QList<FirmwareFactory *> registeredFactories;

    void registerAllFirmwares();
    bool registerFirmware(const QString & id);
};
