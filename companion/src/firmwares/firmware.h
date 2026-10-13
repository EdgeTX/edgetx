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

#include "../shared/capabilities.h"
#include "constants.h"

#include <QtCore>

constexpr char FWDEFNSDIR[] { ":/fwdefs" };

class Board;

class Firmware
{
  Q_DECLARE_TR_FUNCTIONS(Firmware)

  public:

    typedef QMap<QString, QString> OptionTooltip;

    typedef QList<QString> OptionsGroup;
    typedef QList<OptionsGroup> OptionsList;

    struct defn_sizes_t {
      int cnt  {0};
      int name {0};
    };

    struct defn_range_t {
      int min  {0};
      int max  {0};
      int dflt {0};
    };

    struct CurvesDefn {
      defn_sizes_t sizes       {CPN_MAX_CURVES, 5};
      int          minPoints   {2};
      int          maxPoints   {17};
      int          totalPoints {512};
    };

    struct LuaScriptsDefn {
      defn_sizes_t sizes      {CPN_MAX_SCRIPTS, 0};
      int          maxInputs  {CPN_MAX_SCRIPT_INPUTS};
      int          maxOutputs {CPN_MAX_SCRIPT_OUTPUTS};
    };
    struct ModelImageDefn {
      int         name     {14};
      std::string filters  {"*.bmp"};
      bool        image    {true};  // b&w - true color - false ??
      bool        keepExtn {false}; // b&w - false color - true
    };

    struct TeleCstmScrnsDefn {
      defn_sizes_t sizes      {0, 0};
      int          maxBars    {0};
      int          maxPerLine {0};
      int          maxLines   {0};
    };

    // TODO constants
    struct FirmwareDefn {
      std::string       id                {"unknown"};
      std::string       name              {"unknown"};
      std::string       boardId           {""};                 // default id
      std::string       dwnldId           {""};                 // default id
      std::string       simuId            {""};                 // default id

      bool              categories        {false};               // same thing?
      bool              labels            {false};               // same thing?
      bool              modelsList        {false};               // depreciated - read yaml but not write check

      defn_sizes_t      channels          {CPN_MAX_CHNOUT, 6};
      CurvesDefn        curves;
      defn_sizes_t      specialFuncs      {CPN_MAX_SPECIAL_FUNCTIONS, 0};
      defn_sizes_t      expos             {CPN_MAX_EXPOS, 10};
      defn_sizes_t      globalFuncs       {CPN_MAX_SPECIAL_FUNCTIONS, 0};
      defn_sizes_t      globalVars        {CPN_MAX_GVARS, 3};
      defn_sizes_t      inputs            {CPN_MAX_INPUTS, 4};
      bool              lcdtoVideo        {false};
      defn_sizes_t      logicalSwitches   {CPN_MAX_LOGICAL_SWITCHES, 3};
      LuaScriptsDefn    luaScripts;
      int               keyShortcuts      {6};                        // MAX_KEYSHORTCUTS;
      defn_sizes_t      mixes             {CPN_MAX_MIXERS, 6};
      ModelImageDefn    modelImage;
      int               modelName         {15};                       // 15, 12 or 10
      int               modelSlots        {CPN_MAX_MODELS};           // B&W or 0 colour
      defn_sizes_t      flightModes       {CPN_MAX_FLIGHT_MODES, 10}; // rename to operatingModes
      defn_sizes_t      outputs;
      int               quickMenuFavs     {12};                       // MAX_QMFAVOURITES; // B&W 0
      defn_sizes_t      sensors           {CPN_MAX_SENSORS, 3};       // 40 or 60 and check len
      TeleCstmScrnsDefn teleCstmScrns;
      defn_sizes_t      timers            {CPN_MAX_TIMERS, 8};
      int               topBarZones       {0};
      int               voicesFileLen     {8};

      OptionsList options;
    };

    explicit Firmware(const QString & id);
    virtual ~ Firmware() {}

    Board * getBoard(bool forceLoad = true) const;
    const QString getDownloadId() const { return m_defn.dwnldId.c_str(); }
    const QString getId() const { return m_id; } // do not use m_defn.id as it does not have edgetx- prefix
    const QString getFullName() const;
    const QString getName() const { return m_defn.name.c_str(); }
    const QString getSimulatorId() const { return m_defn.simuId.c_str(); }

    int getCapability(Capability value) const;
    QString getCapabilityStr(Capability value) const;

    const OptionsList optionGroups() const { return m_defn.options; }

    // parse the contents of the loaded json
    bool loadDefinition();
    bool loadDefinition(const QString & path);

    bool isLoaded() { return m_loaded; }
    bool isValid() { return m_valid; }
    bool isSupported() { return m_supported; }

    FirmwareDefn getDefinition() { return m_defn; }

    static Firmware * getFirmware(const QString & id, bool forceLoad = true);
    static bool isAvailable(const QString & id);

    static QList<Firmware *> getRegisteredFirmwares();

    static Firmware * getCurrent() { return m_current; }
    static void setCurrent(Firmware * firmware);
    static void setCurrent(const QString & id);

    static Firmware * getFallback() { return m_fallback; }
    static void setFallback(Firmware * firmware) { m_fallback = firmware; }

    static QList<QString> getLanguageList() { return m_languages; }

    static QString getOptionTooltip(const QString & opt);

  private:
    QString m_id;
    bool m_supported;   // false - hide from list of available firmwares but available for conversion
    bool m_loaded;
    bool m_valid;
    FirmwareDefn m_defn;
    Board *m_board;     // pointer to BoardFactories Board

    bool isOptionDuplicate(const OptionsGroup & grp, const QString & val);
    bool isOptionDuplicate(const OptionsList & options, const QString & val);
    void loadCurves(QJsonObject::const_iterator & it);
    void loadSizes(QJsonObject::const_iterator & it, defn_sizes_t & sizes,
                    const int cntMax = 99999, const int nameMax = 100,
                    const int cntMin = 0, const int nameMin = 0);
    void loadLuaScripts(QJsonObject::const_iterator & it);
    void loadModelImage(QJsonObject::const_iterator & it);
    void loadBuildOptions(QJsonObject::const_iterator & it);
    void loadBuildOptionGroup(QJsonArray::const_iterator & it, OptionsGroup & grp);
    void loadTeleCstmScrns(QJsonObject::const_iterator & it);
    bool postLoad();
    void dumpCapabilities();

    inline static Firmware * m_current = nullptr;
    inline static Firmware * m_fallback = nullptr;

    static QList<QString> m_languages;
    static const OptionTooltip registeredOptions;
  };

inline Firmware* getCurrentFirmware() { return Firmware::getCurrent(); }
inline Board* getCurrentBoard() { return Firmware::getCurrent()->getBoard(); }
