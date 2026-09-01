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

#include "capability.h"
#include "boards.h"
#include "constants.h"
#include "helpers_json.h"

#include <QtCore>

class FirmwareFactories;

constexpr char FWDEFNSDIR[] { ":/fwdefs" };

class Firmware : public JsonBase
{
  Q_DECLARE_TR_FUNCTIONS(Firmware)

  public:

    typedef QMap<QString, QString> OptionTooltip;

  // TODO replace struct with QString
    struct Option {
      QString name;
      QString tooltip;              // copy registeredOptions
      unsigned variant = 0;         // depreciated

      explicit Option(QString & name, const QString & description, unsigned variant = 0) :
        name(name), tooltip(description), variant(variant) { }
    };

    // TODO
    // typedef QList<QString> OptionsGroup;
    typedef QList<Option> OptionsGroup;
    typedef QList<OptionsGroup> OptionsList;

    struct BaseGrp {
      int cnt;
      int nameLen;
    };

    struct Curves {
      int cnt     = CPN_MAX_CURVES;
      int nameLen = 5;
      int points  = 512;
    };

    struct LuaScripts{
      int cnt     = CPN_MAX_SCRIPTS;
      int inputs  = CPN_MAX_SCRIPT_INPUTS;
      int outputs = CPN_MAX_SCRIPT_OUTPUTS;
    };

    struct ModelImage {
      QString filters  = "";
      bool    image    = true; // any radios where no image???
      bool    keepExtn = false;
      int     nameLen  = 14; // 10
    };

    struct Outputs {
      int cnt         = CPN_MAX_CHNOUT;
      int nameLen     = 6;
      int ppmCenter   = 512;
      int ppmFrameLen = 40;
    };

    struct TeleCstmScrns {
      int cnt           = 0;
      int bars          = 0;
      int perLine       = 0;
      int lines         = 0;
    };

    // TODO constants
    struct FirmwareDefn {
      QString       id            = "unknown";          // use m_id as it has edgetx- prefix
      QString       name          = "unknown";
      QString       bddefn        = "";                 // boards\defn default firmware id
      QString       hwdefn        = "";                 // radio\src default firmware id
      QString       dwnldId       = "";                 // default firmware id
      QString       simuId        = "";                 // default firmware id

      bool          categories    = true;               // same thing?
      bool          labels        = true;               // same thing?
      bool          modelsList    = false;              // depreciated - read yaml but not write check

      Curves        curves        = { CPN_MAX_CURVES, 5, 512 };
      int           customFuncs   = CPN_MAX_SPECIAL_FUNCTIONS;
      int           extTrimsRange = 512;
      int           failsafeChans = 32;
      int           globalFuncs   = CPN_MAX_SPECIAL_FUNCTIONS;
      BaseGrp       gvars         = { CPN_MAX_GVARS, 3};
      BaseGrp       inputs        = { CPN_MAX_INPUTS, 4 };
      BaseGrp       logicalSW     = { CPN_MAX_LOGICAL_SWITCHES, 3 }; // check len
      LuaScripts    luaScripts;
      int           keyShortcuts  = 6; // MAX_KEYSHORTCUTS;
      BaseGrp       mixes         = { CPN_MAX_MIXERS, 6};
      ModelImage    modelImage;
      int           modelNameLen  = 15; // 12 or 10
      int           modelSlots    = CPN_MAX_MODELS; // B&W or 0 colour
      BaseGrp       modes         = { CPN_MAX_FLIGHT_MODES, 10 }; // rename constant
      int           offsetWeight  = 500;
      Outputs       outputs       = { CPN_MAX_CHNOUT, 3};
      int           quickMenuFavs = 12; // MAX_QMFAVOURITES; // B&W 0
      BaseGrp       sensors       = { CPN_MAX_SENSORS, 3 };   // 40 or 60 and check len
      int           slowRange     = 250;
      int           slowScale     = 10;
      TeleCstmScrns teleCstmScrns;
      BaseGrp       timers        = { CPN_MAX_TIMERS, 8 };
      int           topBarZones   = 0;
      int           trainerInputs = 16;
      int           trimsRange    = 128;
      int           voicesFileLen = 8;

      OptionsList options;

      FirmwareDefn() = default;
    };

    explicit Firmware(const QString & id, const QString & path, const bool isSupported = true);
    virtual ~ Firmware() {}

    const QString bddefn() const { return m_defn.bddefn; }
    const QString dwnldid() const { return m_defn.dwnldId; }
    const QString hwdefn() const { return m_defn.hwdefn; }
    const QString id() const { return m_id; } // do not use m_defn.id as it does not have edgetx- prefix
    const QString name() const { return m_defn.name; }
    const QString simuid() const { return m_defn.simuId; }

    int getCapability(Capability value) const;
    QString getCapabilityStr(Capability value) const;

    OptionsList optionGroups() const { return m_defn.options; }

    // parse the contents of the loaded json
    bool loadDefinition();
    bool loadDefinition(const QString & path);

    bool isLoaded() { return m_loaded; }
    bool isValid() { return m_valid; }
    bool isSupported() { return m_supported; }

    // static functions
    static Firmware * getFirmwareForId(const QString & id);
    static Firmware * getFirmware(const QString & id = QString());

    static Firmware * getCurrent() { return m_current; }
    static void setCurrent(Firmware * firmware);
    static void setCurrent(const QString & id);

    static Firmware * getDefault() { return m_default; }
    static void setDefault(Firmware * firmware) { m_default = firmware; }

    // ========================
    // until Boards refactored
    Board::Type getBoard() const { return Boards::getBoardForHwDefn(m_defn.hwdefn); }
    // ========================

    // ========================
    // deprecated v3.0
    // [[deprecated("Deprecated from v3.0 use getCurrent() instead")]]
    static Firmware * getCurrentVariant() { return m_current; }
    static void setCurrentVariant(Firmware * value) { setCurrent(value); }
    static Firmware * getDefaultVariant() { return m_default; }
    static void setDefaultVariant(Firmware * value) { setDefault(value); }
    const Firmware * getFirmwareBase() const { return m_current; }
    Firmware * getFirmwareVariant(const QString & id) { return m_current; }
    unsigned int getVariantNumber() { return 0; }
    const QString getDownloadId() { return dwnldid(); }
    QString getFlavour() { return id(); }
    const QString getId() const { return id(); }
    const QString getHwDefnId() { return hwdefn(); }
    QString getLanguage() const;
    const QString getName() const { return name(); }
    const QString getSimulatorId() { return simuid(); }
    static Firmware * getFirmwareForFlavour(const QString & flavour) { return getFirmwareForId(flavour); }
    static QList<Firmware*>getRegisteredFirmwares();
    QList<const char *> languageList() const { return m_languages; }
    // end depreciated v3.0
    // ========================

  private:
    QString m_id;       // has edgetx- prefix for backwards compatibility
    QString m_path;
    bool m_supported;   // false - hide from list of available firmwares but available for conversion
    FirmwareDefn m_defn;
    bool m_loaded;
    bool m_valid;

    inline static Firmware * m_current = nullptr;
    inline static Firmware * m_default = nullptr;
    static QList<const char *> m_languages;

    // tooltip translation cannot be performed at runtime
    // so convert and load mapping at compile time
    // key   name
    // value tooltip
    static const OptionTooltip registeredOptions;

    bool isOptionDuplicate(const OptionsGroup & grp, const QString & val);
    bool isOptionDuplicate(const OptionsList & options, const QString & val);
    void loadCurves(QJsonObject::const_iterator & it);
    void loadGroup(QJsonObject::const_iterator & it, BaseGrp & grp,
                   const int cntMax = 199, const int nameMax = 20,
                   const int cntMin = 0, const int nameMin = 0);
    void loadLuaScripts(QJsonObject::const_iterator & it);
    void loadModelImage(QJsonObject::const_iterator & it);
    void loadOptions(QJsonObject::const_iterator & it);
    void loadOptionGroup(QJsonArray::const_iterator & it, OptionsGroup & grp);
    void loadOutputs(QJsonObject::const_iterator & it);
    void loadTeleCstmScrns(QJsonObject::const_iterator & it);
    bool postLoad();
};

inline Firmware * getCurrentFirmware() { return Firmware::getCurrent(); }

// after Boards refactored
// inline QString getCurrentBoard() { return Firmware::board(); }

// before Boards refactored
inline Board::Type getCurrentBoard() { return Firmware::getCurrent()->getBoard(); }

