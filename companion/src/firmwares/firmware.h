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
#include "helpers_json.h"

#include <QtCore>

constexpr char FWDEFNSDIR[] { ":/fwdefs" };

class Board;

class Firmware : public JsonBase
{
  Q_DECLARE_TR_FUNCTIONS(Firmware)

  public:

    typedef QMap<QString, QString> OptionTooltip;

    typedef QList<QString> OptionsGroup;
    typedef QList<OptionsGroup> OptionsList;

    struct Limits {
      int max_count    {0};
      int max_name_len {0};
    };

    struct Curves {
      Limits limits     {CPN_MAX_CURVES, 5};
      int    max_points {512};
    };

    struct LuaScripts{
      Limits limits      {CPN_MAX_SCRIPTS, 0};
      int    max_inputs  {CPN_MAX_SCRIPT_INPUTS};
      int    max_outputs {CPN_MAX_SCRIPT_OUTPUTS};
    };

    struct ModelImage {
      Limits      limits   {0, 14};
      std::string filters  {""};
      bool        image    {true}; // any radios where no image???
      bool        keepExtn {false};
    };

    struct Outputs {
      Limits limits      {CPN_MAX_CHNOUT, 6};
      int    ppmCenter   {512};
      int    ppmFrameLen {40};
    };

    struct TeleCstmScrns {
      Limits limits      {0, 0};
      int    max_bars    {0};
      int    max_perLine {0};
      int    max_lines   {0};
    };

    // TODO constants
    struct FirmwareDefn {
      std::string   id                {"unknown"};
      std::string   name              {"unknown"};
      std::string   boardId           {""};                 // default firmware id
      std::string   dwnldId           {""};                 // default firmware id
      std::string   simuId            {""};                 // default firmware id

      bool          categories        {true};               // same thing?
      bool          labels            {true};               // same thing?
      bool          modelsList        {false};              // depreciated - read yaml but not write check

      Curves        curves            {CPN_MAX_CURVES, 5, 512};
      int           max_specialFuncs  {CPN_MAX_SPECIAL_FUNCTIONS};
      int           extTrimsRange     {512};
      int           failsafeChans     {32};
      int           max_globalFuncs   {CPN_MAX_SPECIAL_FUNCTIONS};
      Limits        gvars             {CPN_MAX_GVARS, 3};
      Limits        inputs            {CPN_MAX_INPUTS, 4};
      Limits        logicalSW         {CPN_MAX_LOGICAL_SWITCHES, 3};
      LuaScripts    luaScripts;
      int           max_keyShortcuts  {6};                        // MAX_KEYSHORTCUTS;
      Limits        mixes             {CPN_MAX_MIXERS, 6};
      ModelImage    modelImage;
      int           max_modelName     {15};                       // 15, 12 or 10
      int           max_modelSlots    {CPN_MAX_MODELS};           // B&W or 0 colour
      Limits        modes             {CPN_MAX_FLIGHT_MODES, 10}; // rename constant
      int           offsetWeight      {500};
      Outputs       outputs           {CPN_MAX_CHNOUT, 3};
      int           max_quickMenuFavs {12};                       // MAX_QMFAVOURITES; // B&W 0
      Limits        sensors           {CPN_MAX_SENSORS, 3};       // 40 or 60 and check len
      int           slowRange         {250};
      int           slowScale         {10};
      TeleCstmScrns teleCstmScrns;
      Limits        timers            {CPN_MAX_TIMERS, 8};
      int           max_topBarZones   {0};
      int           max_trainerInputs {16};
      int           trimsRange        {128};
      int           max_voicesFileLen {8};

      OptionsList options;

      FirmwareDefn() = default;
    };

    explicit Firmware(const QString & id, const QString & path, const bool isSupported = true);
    virtual ~ Firmware() {}

    Board * getBoard() const { return m_board; }
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

    static Firmware * getFirmwareForId(const QString & id);
    static Firmware * getFirmware(const QString & id = QString());
    static bool isAvailable(const QString & id);

    static QList<Firmware *> getRegisteredFirmwares();

    static Firmware * getCurrent() { return m_current; }
    static void setCurrent(Firmware * firmware);
    static void setCurrent(const QString & id);

    static Firmware * getDefault() { return m_default; }
    static void setDefault(Firmware * firmware) { m_default = firmware; }

    static QList<QString> getLanguageList() { return m_languages; }

    static QString getOptionTooltip(const QString opt);

  private:
    QString m_id;       // has edgetx- prefix for backwards compatibility TODO conversion to get rid of
    QString m_path;
    bool m_supported;   // false - hide from list of available firmwares but available for conversion
    FirmwareDefn m_defn;
    bool m_loaded;
    bool m_valid;
    Board *m_board;     // pointer to BoardFactories Board

    bool isOptionDuplicate(const OptionsGroup & grp, const QString & val);
    bool isOptionDuplicate(const OptionsList & options, const QString & val);
    void loadCurves(QJsonObject::const_iterator & it);
    void loadGroup(QJsonObject::const_iterator & it, Limits & grp,
                   const int cntMax = 199, const int nameMax = 20,
                   const int cntMin = 0, const int nameMin = 0);
    void loadLuaScripts(QJsonObject::const_iterator & it);
    void loadModelImage(QJsonObject::const_iterator & it);
    void loadOptions(QJsonObject::const_iterator & it);
    void loadOptionGroup(QJsonArray::const_iterator & it, OptionsGroup & grp);
    void loadOutputs(QJsonObject::const_iterator & it);
    void loadTeleCstmScrns(QJsonObject::const_iterator & it);
    bool postLoad();

    inline static Firmware * m_current = nullptr;
    inline static Firmware * m_default = nullptr;

    static QList<QString> m_languages;

    // tooltip translation cannot be performed at runtime
    // so convert and load mapping at compile time
    // key   name
    // value tooltip
    static const OptionTooltip registeredOptions;
  };

inline Firmware* getCurrentFirmware() { return Firmware::getCurrent(); }
inline Board* getCurrentBoard() { return getCurrentFirmware()->getBoard(); }
