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
#include "../helpers/helpers_json.h"

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

    struct BacklightDefn {
      int  delay         {0};
      bool keys          {false};
      int  min           {0};
      int  mode          {0};
      int  offBrightness {0};
      int  onBrightness  {0};

      BacklightDefn() = default;
    };

    struct CurvesDefn {
      Json::sizes_t sizes       {CPN_MAX_CURVES, CURVEDATA_NAME_LEN};
      int          minPoints   {2};
      int          maxPoints   {CPN_MAX_POINTS};
      int          totalPoints {512};

      CurvesDefn() = default;
    };

    struct CustomScreensDefn {
      Json::sizes_t sizes  {MAX_CUSTOM_SCREENS, 0};
      int layoutOptions   {MAX_LAYOUT_OPTIONS};
      int layoutZones     {MAX_LAYOUT_ZONES};
      int topBarOptions   {MAX_TOPBAR_OPTIONS};
      int topBarZones     {MAX_TOPBAR_ZONES};
      int themeOptions    {MAX_THEME_OPTIONS};
      int widgetOptions   {MAX_WIDGET_OPTIONS};

      CustomScreensDefn() = default;
    };

    struct LuaScriptsDefn {
      Json::sizes_t sizes      {CPN_MAX_SCRIPTS, CPN_LEN_SCRIPT_NAME};
      int          maxInputs  {CPN_MAX_SCRIPT_INPUTS};
      int          maxOutputs {CPN_MAX_SCRIPT_OUTPUTS};

      LuaScriptsDefn() = default;
    };

    struct ModelImageDefn {
      int         name     {CPN_MAX_BITMAP_LEN};
      std::string filters  {"*.bmp"};
      bool        image    {true};  // b&w - true color - false ??
      bool        keepExtn {false}; // b&w - false color - true

      ModelImageDefn() = default;
    };

    struct TeleCstmScrnsDefn {
      Json::sizes_t sizes      {CPN_TELE_CUST_SCRNS, 0};
      int          maxBars    {CPN_TELE_CUST_SCRNS_BARS};
      int          maxPerLine {CPN_TELE_CUST_SCRNS_PERLINE};
      int          maxLines   {CPN_TELE_CUST_SCRNS_LINES};

      TeleCstmScrnsDefn() = default;
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

      BacklightDefn     backlight;
      Json::range_t     backgroundVolume  {0, 0, 0};
      Json::range_t     contrast          {0, 0, 25};
      CurvesDefn        curves;
      CustomScreensDefn customScreens;
      int               eepromSize        {0};
      Json::sizes_t     expos             {CPN_MAX_EXPOS, EXPODATA_NAME_LEN};
      int               flashSize         {0};
      Json::sizes_t     flightModes       {CPN_MAX_FLIGHT_MODES, FLIGHTMODE_NAME_LEN}; // rename to operatingModes
      int               fourCC            {0};
      Json::sizes_t     globalFuncs       {CPN_MAX_SPECIAL_FUNCTIONS, 0};
      Json::sizes_t     globalVars        {CPN_MAX_GVARS, GVAR_NAME_LEN};
      int               hatsMode          {0};
      Json::range_t     inactivityTimer   {0, 0, 10};
      Json::sizes_t     inputs            {CPN_MAX_INPUTS, INPUT_NAME_LEN};
      bool              lcdToVideo        {false};
      Json::sizes_t     logicalSwitches   {CPN_MAX_LOGICAL_SWITCHES, 0};
      LuaScriptsDefn    luaScripts;
      int               keyShortcuts      {MAX_KEYSHORTCUTS};
      Json::sizes_t     mixes             {CPN_MAX_MIXERS, MIXDATA_NAME_LEN};
      ModelImageDefn    modelImage;
      int               modelName         {MODEL_NAME_LEN};                          // 15, 12 or 10
      int               modelSlots        {CPN_MAX_MODELS};                          // B&W or 0 colour
      OptionsList       options;
      Json::sizes_t     outputs           {CPN_MAX_CHNOUT, LIMITDATA_NAME_LEN};
      bool              powerOnDelay      {false};
      int               quickMenuFavs     {MAX_QMFAVOURITES};                        // B&W 0
      Json::sizes_t     sensors           {CPN_MAX_SENSORS, SENSOR_LABEL_LEN};       // 40, 60 or 99 and check len
      Json::range_t     speakerVolume     {0, 0, 0};
      Json::sizes_t     specialFuncs      {CPN_MAX_SPECIAL_FUNCTIONS, 0};
      TeleCstmScrnsDefn teleCstmScrns;
      Json::sizes_t     timers            {CPN_MAX_TIMERS, TIMER_NAME_LEN};
      int               topBarZones       {MAX_TOPBAR_ZONES};
      Json::range_t     wavVolume         {0, 0, 0};

      FirmwareDefn() = default;
    };

    explicit Firmware(const QString & id);
    virtual ~ Firmware() {}

    Board * getBoard(bool forceLoad = true) const;
    const QString getDownloadId() const { return m_defn.dwnldId.c_str(); }
    const QString getId() const { return m_id; }
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
    static bool setCurrent(Firmware * firmware);
    static bool setCurrent(const QString & id);

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

    void dumpCapabilities();
    bool isOptionDuplicate(const OptionsGroup & grp, const QString & val);
    bool isOptionDuplicate(const OptionsList & options, const QString & val);
    void loadBacklight(QJsonObject::const_iterator & it);
    void loadBuildOptions(QJsonObject::const_iterator & it);
    void loadBuildOptionGroup(QJsonArray::const_iterator & it, OptionsGroup & grp);
    void loadCurves(QJsonObject::const_iterator & it);
    void loadCustomScreens(QJsonObject::const_iterator & it);
    void loadLuaScripts(QJsonObject::const_iterator & it);
    void loadModelImage(QJsonObject::const_iterator & it);
    void loadTeleCstmScrns(QJsonObject::const_iterator & it);
    bool postLoad();

    inline static Firmware * m_current = nullptr;
    inline static Firmware * m_fallback = nullptr;

    static QList<QString> m_languages;
    static const OptionTooltip registeredOptions;
  };

inline Firmware* getCurrentFirmware() { return Firmware::getCurrent(); }
inline Board* getCurrentBoard() { return Firmware::getCurrent()->getBoard(); }
