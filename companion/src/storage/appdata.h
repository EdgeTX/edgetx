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

/*! \defgroup AppData Application Settings Manager
  \brief All temporary and permanent global variables are defined here to make initialization and storage safe and visible.
  \note Do not access variables in QSettings directly.
  \sa CompStoreObj
  @{
*/

#pragma once

#include "constants.h"
#include "macros.h"
#include "simulator.h"
#include "version.h"

#include <QCoreApplication>
#include <QObject>
#include <QSettings>
#include <QStandardPaths>

//! CPN_SETTINGS_REVISION is used to track settings changes independently of EdgeTX version. It should be reset to zero whenever settings are migrated to new COMPANY or PRODUCT.
//! \note !! Increment this value if properties are removed or refactored. It will trigger a conversion/cleanup of any stored settings. \sa AppData::convertSettings()
#define CPN_SETTINGS_REVISION       2 // Note: bumped for fix during 2.8 RCs

//! CPN_SETTINGS_VERSION is used for settings data version tracking.
#define CPN_SETTINGS_VERSION        ((VERSION_NUMBER << 8) | CPN_SETTINGS_REVISION)

#define COMPANY                     QStringLiteral("EdgeTX")
#define COMPANY_DOMAIN              QStringLiteral("edgetx.org")
#define PRODUCT_NO_VERS             QStringLiteral("Companion")
#define PRODUCT                     QStringLiteral("Companion " QT_STRINGIFY(VERSION_MAJOR) "." QT_STRINGIFY(VERSION_MINOR))
#define APP_COMPANION               QStringLiteral("EdgeTX Companion")
#define APP_SIMULATOR               QStringLiteral("EdgeTX Simulator")

//! Default location for EdgeTX-related user documents (settigns, logs, etc)
#define CPN_DOCUMENTS_LOCATION      QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) % "/" % COMPANY
//! Location for settings backup files. TODO: make option or remember last location.
#define CPN_SETTINGS_BACKUP_DIR     CPN_DOCUMENTS_LOCATION % "/backup"
#define CPN_SETTINGS_INI_FILE       QString(PRODUCT % " " % QCoreApplication::translate("Companion", "settings") % " %1.ini")
#define CPN_SETTINGS_INI_PATH       QString(CPN_SETTINGS_BACKUP_DIR % "/" % CPN_SETTINGS_INI_FILE)

#define MAX_PROFILES 32
#define MAX_JS_AXES 10
#define MAX_JS_BUTTONS 32
#define MAX_COMPONENTS 10
#define MAX_COMPONENT_ASSETS 5
#define MAX_NAMED_JOYSTICKS 10

// It important that these function names are consistent everywhere.
#define PROP_FSIG_INIT_IMPL         _init()
#define PROP_FSIG_KEY_IMPL          _key()
#define PROP_FSIG_DFLT_IMPL         _default()
// this set is used in macros
#define PROP_FSIG_INIT(NAME)        CONCATENATE(NAME, PROP_FSIG_INIT_IMPL)
#define PROP_FSIG_KEY(NAME)         CONCATENATE(NAME, PROP_FSIG_KEY_IMPL)
#define PROP_FSIG_DFLT(NAME)        CONCATENATE(NAME, PROP_FSIG_DFLT_IMPL)
// this set is used at runtime
#define PROP_FSIG_INIT_STR(NAME)    qPrintable(QString("%1" QT_STRINGIFY(PROP_FSIG_INIT_IMPL)).arg(NAME))
#define PROP_FSIG_KEY_STR(NAME)     qPrintable(QString("%1" QT_STRINGIFY(PROP_FSIG_KEY_IMPL)).arg(NAME))
#define PROP_FSIG_DFLT_STR(NAME)    qPrintable(QString("%1" QT_STRINGIFY(PROP_FSIG_DFLT_IMPL)).arg(NAME))

//! Helper macro to cast the default value of property \a NAME to its actual \a TYPE .
#define PROP_DFLT_CAST(NAME, TYPE)  PROP_FSIG_DFLT(NAME).value<TYPE >()


/*!
  \brief The PROPERTY_META4 macro declares and defines member property meta data functions, and declares the member variable. It also sets up the Q_PROPERTY declaration, essential for introspection.

  It produces the following code for each invokation (where \e name is the property name and \e T is the type.):
  \code
    public:
      static const QVariant name_default();   // Returns the default value as a QVariant.
      static const QString name_key();        // Returns the settings storage key name (may or may not be same as \a name )
      Q_SIGNAL void nameChanged(const T &);   // Value change notifier signal.
    private:
      T m_name;                               // Member variable.
  \endcode

  If you only use this macro (w/out a \p PROPERTY*() macro) then you need to provide the getter, setter, and reset functions yourself. The initializer functoin is optional.
  Those function names must match the expected signatures as defined in the \e Q_PROPERTY() macro.

  \param TYPE   The data type (T).
  \param NAME  Name of the property.
  \param KEY   Settings key name, a const string. Can be the same as \a NAME.
  \param DFLT  Default value, must be compatible with \a TYPE.

  \note \a Macro should be placed in the private declarations section.
  \note \a TYPE must be a standard or registered Qt type. See notes in CompStoreObj.

  \sa PROPERTY4()  \sa PROPERTY_META()
*/
#define PROPERTY_META4(TYPE, NAME, KEY, DFLT)                                           \
  public:                                                                               \
    Q_INVOKABLE static inline const QVariant PROP_FSIG_DFLT(NAME)                       \
      { return QVariant::fromValue(DFLT); }                                             \
    Q_INVOKABLE static inline const QString PROP_FSIG_KEY(NAME)                         \
      { return QStringLiteral(KEY); }                                                   \
  Q_SIGNALS: void NAME##Changed(const TYPE & val);                                      \
  private:                                                                                        \
    Q_PROPERTY(TYPE NAME READ NAME WRITE NAME RESET NAME##Reset NOTIFY NAME##Changed STORED true) \
    TYPE m_##NAME;

/*!
  \brief Convenience equivalent to \p PROPERTY_META4() macro when the \a NAME and \a KEY are the same (recommended).
  \sa PROPERTY()  \sa PROPERTY_META4()
*/
#define PROPERTY_META(TYPE, NAME, DFLT)     PROPERTY_META4(TYPE, NAME, #NAME, DFLT)


/*!
  \brief This macro extends \p PROPERTY_META4() to add initializer, getter, setter, and reset functions.
  It produces the following additional code for each invokation (where \e name is the property name and \e T is the type.):
  \code
    public:
      inline T name() const;            // Getter function.
      void name(const T &, bool=true);  // Sets value and optionally saves to storage. If new value == default value, and store == true, the storage is cleared.
      void nameReset(bool=true);        // Reset to default and optionally save.
    protected:
      void name_init();                 // Sets value from saved settings, if any, otherwise sets default value. Calls CompStoreObj::load()
  \endcode

  \note \a Macro should be placed in the private declarations section.
  \sa PROPERTY(), PROPERTY_META4()
*/
#define PROPERTY4(TYPE, NAME, KEY, DFLT)                                               \
  PROPERTY_META4(TYPE, NAME, KEY, DFLT)                                                \
  public:                                                                              \
    inline TYPE NAME() const { return m_##NAME; }                                      \
  public Q_SLOTS:                                                                      \
    void NAME(const TYPE &val, bool store = true) {                                    \
      if (val == m_##NAME) return;                                                     \
      m_##NAME = val;                                                                  \
      emit NAME##Changed(val);                                                         \
      if (store)                                                                       \
        CompStoreObj::storeProperty(this, QStringLiteral(#NAME));                      \
    }                                                                                  \
    inline void NAME##Reset(bool store = true)                                         \
      { NAME(PROP_DFLT_CAST(NAME, TYPE), store); }                                     \
  protected Q_SLOTS:                                                                   \
    void PROP_FSIG_INIT(NAME) {                                                        \
      load(this, QStringLiteral(#NAME), PROP_FSIG_KEY(NAME), PROP_FSIG_DFLT(NAME));    \
    }                                                                                  \
  private:

/*!
  \brief Convenience equivalent to \p PROPERTY4() macro when the \a NAME and \a KEY are the same (recommended).
  \param TYPE   The data type (T).
  \param NAME  Name of the property, and also the settings key name.
  \param DFLT  Default value, must be compatible with \a TYPE.
  \sa PROPERTY4()
*/
#define PROPERTY(TYPE, NAME, DFLT)      PROPERTY4(TYPE, NAME, #NAME, DFLT)

//! Convenience macros for QString types with either null or custom default value. \sa PROPERTY()
#define PROPERTYSTR(NAME)               PROPERTY(QString, NAME, QStringLiteral(""))
#define PROPERTYSTRD(NAME, DFLT)        PROPERTY(QString, NAME, QString(DFLT))
//! Convenience macros for QString types where key name != property name and either null or custom default value. \sa PROPERTY4()
#define PROPERTYSTR2(NAME, KEY)         PROPERTY4(QString, NAME, KEY, QStringLiteral(""))
#define PROPERTYSTR3(NAME, KEY, DFLT)   PROPERTY4(QString, NAME, KEY, QString(DFLT))

//! Convenience macros for QByteArray types with null value. \sa PROPERTY()
#define PROPERTYQBA(NAME)               PROPERTY(QByteArray, NAME, QByteArray())
#define PROPERTYQBA2(NAME, KEY)         PROPERTY4(QByteArray, NAME, KEY, QByteArray())


/*!
  \brief CompStoreObj manages staic meta data on member properties (extending Qt's own property system) and provides utility functions for subclasses.

  The extension is essentially used to provide some "integration" with \e QSettings by providing storage path/key mappings to properties.
  It also adds ability to access property default values, auto-initialization from saved settings (with fallback to default), and provides change notification signals.
  A reset slot is provided for all properties, which sets the value back to default and is invokable through the Qt property system.

  The process of managing which settings key is used to store the property is complicated by properties which were originally configured to use a different key name than propety name.
  Hence we need to keep a mapping of all names to keys, even if they're identical (which should be the case with any newly/recently added properties.

  \note Property types must be a standard or registered Qt type which are compatible with \e QVariant and \e QSettings. To use custom types, you need to \e Q_DECLARE_METATYPE()
          them and add \e QDataStream streaming operators and (possibly) comparison operators so that they can be handled by \e QVariant and read/written by \e QSettings.
        For an example of using a custom type with stream operators see the \p SimulatorOptions class, or e.g. https://stackoverflow.com/questions/18144377/writing-and-reading-custom-class-to-qsettings .
        For an example of using a custom enum type, see \p AppData::DownloadBranchType .

  \sa AppData
*/
class CompStoreObj: public QObject
{
  Q_OBJECT
  public slots:
    virtual inline void init() { initAllProperties(this); }
    virtual inline void resetAll() { resetAllProperties(this); }
    virtual inline void storeAll() { storeAllProperties(this); }

  protected:
    CompStoreObj();

    mutable QSettings m_settings;

    //! Subclasses must reimplement this to return a unique name, typically used in settings storage as the base path.
    virtual QString propertyGroup() const = 0;

    //! reimplement this function to return the default settings group (used in \p pathForKey() if the \a group is not specified).
    virtual QString settingsPath() const { return propertyGroup() % "/"; }

    //! Utility function to return a fully qualified settings path for given \a key in optional \a group. If \a group is null, \p settingsPath() is used. Reimpliment to provide custom paths.
    virtual QString pathForKey(const QString &key, const QString &group = QString()) const;

    //! Set property to value saved in persistent storage, if any, otherwise to \a def default value. Calls \p pathForKey() to determine full settings path from  given \a key and \a group.
    void load(CompStoreObj *obj, const QString & name, const QString & key, const QVariant & def, const QString & group = QString());

    //! Save property value \a newValue to \a key in persistent storage. Calls \p pathForKey() to determine full settings path from  given \a key and \a group.
    void store(const QVariant & newValue, const QString & key, const QString & group = QString()) const;

    //! Deletes any saved settings in \a key, which is (optionally) a subkey of \a group. Calls \p pathForKey() to determine full settings path from  given \a key and \a group.
    void clear(const QString & key, const QString & group = QString()) const;

    //! Deletes settings \a group if it is empty.
    void removeGroupIfEmpty(const QString & group) const;

    //// Static property meta data handling functions. This first set of functons operates on specific object instances.

    //! Calls the initialization function on a property declared with a Q_PROPERTY() macro for given \a obj object.
    static bool initProperty(CompStoreObj * obj, const QString & name);
    //! Calls the initialization function on all properties declared with a Q_PROPERTY() macro (and by extension with our own PROPERTY() et.al.).
    static void initAllProperties(CompStoreObj * obj);

    //! Invokes the RESET function on a property declared with a Q_PROPERTY() macro for given \a obj object.
    static bool resetProperty(CompStoreObj * obj, const QString & name);
    //! Invokes the RESET function on all properties declared with a Q_PROPERTY() macro for given \a obj object.
    static void resetAllProperties(CompStoreObj * obj);

    //! Invokes the \p store() function on a property declared with a Q_PROPERTY() macro for given \a obj object.
    static bool storeProperty(CompStoreObj * obj, const QString & name);
    //! Invokes the \p store() function on all properties declared with a Q_PROPERTY() macro for given \a obj object.
    static void storeAllProperties(CompStoreObj * obj);

    //// Methods below here operate only on static object data, not specific object instances. They are only used to look up meta data about an obect.

    //! Each subclass should register their settings group name (root path). This way we can easily look up which object a property key belongs to.
    //! The actual instance of \a obj doesn't matter, we just use static data and only store this mapping once.
    static inline void addObjectMapping(const QString & groupName, CompStoreObj * obj) { group2ObjMap().insert(groupName, obj); }
    //! Get the CompStoreObj associated with \a group, if any. Returns null pointer if \a group is invalid. Note that this object is only useful for static data lookup, it is not guaranteed to be any specific instance.
    static inline CompStoreObj * getObjectByGroup(const QString & group)               { return group2ObjMap().value(group, nullptr); }

    //! Returns true if property exists based on settings \a path. Special exception for groups with dynamic property names, eg. FwRevision. \sa propertyPathIsValidNonDefault(), propertyExists(), groupHasDynamicProperties()
    static bool propertyPathIsValid(const QString & path);
    //! Returns true if property exists (based on settings \a path) and \a newVal != the default property value. Special exception for groups with dynamic property names, eg. FwRevision.
    static bool propertyPathIsValidNonDefault(const QString & path, const QVariant & newVal);

    //! Return true if  \a name property exists in the given \a obj object.
    static bool propertyExists(CompStoreObj * obj, const QString & name);
    //! Return true if  \a name property exists within settings \a group.  Group names must be declared with \p addObjectMapping()
    static inline bool propertyExists(const QString & group, const QString & name) { return propertyExists(getObjectByGroup(group), name); }

    //! Return true if a property exists by checking the given settings \a key in the given \a obj object.
    static inline bool propertyKeyExists(CompStoreObj * obj, const QString & key)    { return keyToNameMap().value(obj->propertyGroup()).contains(key); }
    //! Return true if a property exists by checking the given settings \a key within settings \a group.
    static inline bool propertyKeyExists(const QString & group, const QString & key) { return keyToNameMap().value(group).contains(key); }

    //! Get the default value for a property \a name in the given \a obj object.
    static const QVariant propertyDefaultValue(CompStoreObj * obj, const QString & name);
    //! Get the default value for a property \a name within \a group.
    static inline const QVariant propertyDefaultValue(const QString & group, const QString & name) { return propertyDefaultValue(getObjectByGroup(group), name); }

    //! Get the default value for a property with settings key name of \a key in the given \a obj object.
    static inline const QVariant propertyKeyDefaultValue(CompStoreObj * obj, const QString & key)    { return propertyDefaultValue(obj, keyToNameMap().value(obj->propertyGroup()).value(key)); }
    //! Get the default value for a property with settings key name of \a key within \a group. Group names must be declared with \p addObjectMapping()
    static inline const QVariant propertyKeyDefaultValue(const QString & group, const QString & key) { return propertyDefaultValue(group, keyToNameMap().value(group).value(key)); }

    //! Get the settings key name for property \a name for the given \a obj object.
    static const QString propertyKeyName(CompStoreObj * obj, const QString & name);
    //! Get the settings key name for property \a name within the \a group.
    static inline const QString propertyKeyName(const QString & group, const QString & name) { return propertyKeyName(getObjectByGroup(group), name); }

    //! Groups which have dynamic property names always return true in \p propertyPathIsValid() and \p propertyPathIsValidNonDefault(). So far only FwRevision uses this.
    static void addDynamicPropertyGroup(const QString & group)          { dynamicPropGroups().append(group); }
    //! Check if \a group has dynamic properties.
    static inline bool groupHasDynamicProperties(const QString & group) { return dynamicPropGroups().contains(group); }

    //! Utility function. Given a full settings path ([group/][.../]key), returns the primary group (or "General" if no group) and
    //!   the last key value in the path (which would correspond to a property in the object which owns the group).
    //! \retval QPair({<group name>, <property key>})
    static QPair<QString, QString> splitGroupedPath(const QString & path);

  private:
    // static lookup tables storage
    static QHash<QString, CompStoreObj *> & group2ObjMap();                   //! Maps propertyGroup() name to the CompStoreObj subclass which owns it.
    static const QHash<QString, QHash<QString, QString> > & keyToNameMap();   //! Maps key names to property names for each property group.
    static QVector<QString> & dynamicPropGroups();                            //! \sa addDynamicPropertyGroup()
};

class AppData;


//! \brief FwRevision class stores data about downloaded firmware binaries. It uses dynamic key names and does not have any properties.
class FwRevision: public CompStoreObj
{
  Q_OBJECT
  public slots:
    inline int get(const QString & fwType) const { return m_settings.value(settingsPath() % fwType, 0).toInt(); }
    inline void set(const QString & fwType, const int fwRevision) const { store(QString("%1").arg(fwRevision), fwType); }
    inline void remove(const QString & tag) const { clear(tag); }
    inline void resetAll() override { clear(propertyGroup(), ""); }

  protected:
    explicit FwRevision() : CompStoreObj() { CompStoreObj::addDynamicPropertyGroup(propertyGroup()); }
    QString propertyGroup() const override { return QStringLiteral("FwRevisions"); }
    friend class AppData;
};


//! \brief JStickData class stores properties related to each joystick axis (calibration/assignment/direction).
class JStickData: public CompStoreObj
{
  Q_OBJECT
  public slots:
    bool existsOnDisk();

  public:
    void clear() {
      stick_axe(-1);
      stick_med(-32768);
      stick_max(32767);
      stick_med(0);
      stick_inv(0);
    }

  protected:
    explicit JStickData();
    void setIndex(int idx) { index = idx; }
    inline QString propertyGroup() const override { return QStringLiteral("JsCalibration"); }
    inline QString settingsPath()  const override { return QString("%1/%2/").arg(propertyGroup()).arg(index); }
    friend class AppData;
    friend class NamedJSData;

  private:
    PROPERTY(int, stick_axe, -1)
    PROPERTY(int, stick_min, -32768)
    PROPERTY(int, stick_med, 0)
    PROPERTY(int, stick_max, 32767)
    PROPERTY(int, stick_inv, 0)

    int index;
};

//! \brief JButtonData class stores properties related to each joystick button (button number).
class JButtonData: public CompStoreObj
{
  Q_OBJECT
  public slots:
    bool existsOnDisk();

  public:
    void clear() {
      button_idx(-1);
    }

  protected:
    explicit JButtonData();
    void setIndex(int idx) { index = idx; }
    inline QString propertyGroup() const override { return QStringLiteral("JsButton"); }
    inline QString settingsPath()  const override { return QString("%1/%2/").arg(propertyGroup()).arg(index); }
    friend class AppData;
    friend class NamedJSData;

  private:
    PROPERTY(int, button_idx, -1)

    int index;
};

//! \brief NamedJStickData class stores properties related to each joystick axis (calibration/assignment/direction).
class NamedJStickData: public CompStoreObj
{
  Q_OBJECT
  public slots:
    bool existsOnDisk();

  protected:
    explicit NamedJStickData();
    void setIndex(int idx, int nmIdx) { index = idx; namedIdx = nmIdx; }
    inline QString propertyGroup() const override { return QStringLiteral("NamedJSData/%1").arg(namedIdx); }
    inline QString settingsPath()  const override { return QString("%1/JsCalibration/%2/").arg(propertyGroup()).arg(index); }
    friend class AppData;
    friend class NamedJSData;

  private:
    PROPERTY(int, stick_axe, -1)
    PROPERTY(int, stick_min, -32768)
    PROPERTY(int, stick_med, 0)
    PROPERTY(int, stick_max, 32767)
    PROPERTY(int, stick_inv, 0)

    int namedIdx;
    int index;
};

//! \brief NamedJButtonData class stores properties related to each joystick button (button number).
class NamedJButtonData: public CompStoreObj
{
  Q_OBJECT
  public slots:
    bool existsOnDisk();

  protected:
    explicit NamedJButtonData();
    void setIndex(int idx, int nmIdx) { index = idx; namedIdx = nmIdx; }
    inline QString propertyGroup() const override { return QStringLiteral("NamedJSData/%1").arg(namedIdx); }
    inline QString settingsPath()  const override { return QString("%1/JsButton/%2/").arg(propertyGroup()).arg(index); }
    friend class AppData;
    friend class NamedJSData;

  private:
    PROPERTY(int, button_idx, -1)

    int namedIdx;
    int index;
};

class NamedJSData: public CompStoreObj
{
  Q_OBJECT
  public slots:
    bool existsOnDisk();

  protected:
    explicit NamedJSData();
    void setIndex(int idx) { index = idx; }
    inline QString propertyGroup() const override { return QStringLiteral("NamedJSData"); }
    inline QString settingsPath()  const override { return QString("%1/%2/").arg(propertyGroup()).arg(index); }
    friend class AppData;

  public:
    NamedJStickData joystick[MAX_JS_AXES];
    NamedJButtonData jsButton[MAX_JS_BUTTONS];

  private:
    PROPERTYSTRD(jsName, "")
    PROPERTY(unsigned int, jsLastUsed, 0)

    int index;
};


//! \brief Profile class stores properties related to each Radio Profile.
//! \todo TODO: Remove or refactor stored radio settings system (#4583)
class Profile: public CompStoreObj
{
  Q_OBJECT
  public:
    Profile & operator=(const Profile & rhs);

  public slots:
    bool existsOnDisk();
    void resetFwVariables();

  protected:
    explicit Profile();
    explicit Profile(const Profile & rhs);
    void setIndex(int idx) { index = idx; }
    inline QString propertyGroup() const override { return QStringLiteral("Profiles"); }
    inline QString settingsPath()  const override { return QString("%1/profile%2/").arg(propertyGroup()).arg(index); }
    friend class AppData;

  private:
    PROPERTYSTR2(name,       "Name")
    PROPERTYSTR2(splashFile, "SplashFileName")
    PROPERTYSTR(fwName)
    PROPERTYSTR(fwType)
    PROPERTYSTR(sdPath)
    PROPERTYSTR(pBackupDir)

    PROPERTY (int, defaultInternalModule, 0)
    PROPERTY (int, externalModuleSize, 1) // added 2.9 - Board::EXTMODSIZE_STD used for existing profiles
    PROPERTY4(int, channelOrder, "default_channel_order",  0)
    PROPERTY4(int, defaultMode,  "default_mode",           1)
    PROPERTY (int, volumeGain,   10)

    PROPERTY (bool, burnFirmware,  false)
    PROPERTY (bool, penableBackup, false)
    PROPERTY (bool, runSDSync,  false)

    // Simulator variables
    PROPERTY(SimulatorOptions, simulatorOptions,  SimulatorOptions())
    PROPERTY(bool, telemSimEnabled,         false)
    PROPERTY(bool, telemSimPauseOnHide,     true)
    PROPERTY(bool, telemSimResetRssiOnStop, false)

    // Firmware Variables
    PROPERTYSTR2(beeper,        "Beeper")
    PROPERTYSTR2(countryCode,   "countryCode")
    PROPERTYSTR2(display,       "Display")
    PROPERTYSTR2(haptic,        "Haptic")
    PROPERTYSTR2(speaker,       "Speaker")
    PROPERTYSTR2(stickPotCalib, "StickPotCalib")
    PROPERTYSTR2(timeStamp,     "TimeStamp")
    PROPERTYSTR2(trainerCalib,  "TrainerCalib")
    PROPERTYSTR2(controlTypes,  "ControlTypes")
    PROPERTYSTR2(controlNames,  "ControlNames")

    PROPERTY4(int, gsStickMode,   "GSStickMode",    0)
    PROPERTY4(int, ppmMultiplier, "PPM_Multiplier", 0)
    PROPERTY4(int, vBatWarn,      "vBatWarn",       0)  // not a typo.. vBat vs. Vbat
    PROPERTY4(int, vBatMin,       "VbatMin",        0)
    PROPERTY4(int, vBatMax,       "VbatMax",        0)
    PROPERTY4(int, txCurrentCalibration, "currentCalib",  0)
    PROPERTY4(int, txVoltageCalibration, "VbatCalib",     0)

    PROPERTYSTRD(jsName, "")

    int index;

    static const QStringList fwVarsList()  //! for resetFwVariables()... TODO: make this go away
    {
      static const QStringList list({
        "Beeper", "countryCode", "Display", "Haptic", "Speaker", "TimeStamp", "TrainerCalib", "StickPotCalib",
        "ControlTypes", "ControlNames", "GSStickMode", "PPM_Multiplier", "vBatWarn", "VbatMin", "VbatMax", "currentCalib", "VbatCalib"
      });
      return list;
    }
};

//! \brief ComponentAssetData class stores properties related to each updateable component.
class ComponentAssetData: public CompStoreObj
{
  Q_OBJECT
  public:
    ComponentAssetData & operator=(const ComponentAssetData & rhs);

  public slots:
    bool existsOnDisk();

  protected:
    explicit ComponentAssetData();
    void setCompIndex(int idx) { compIndex = idx; }
    void setIndex(int idx) { index = idx; }
    void setIndexes(int compIdx, int idx) { compIndex = compIdx; index = idx; }
    inline QString propertyGroup() const override { return QString("Components/component%1").arg(compIndex); }
    inline QString settingsPath()  const override { return QString("%1/asset%2/").arg(propertyGroup()).arg(index); }
    friend class ComponentData;
    friend class AppData;

  private:
    PROPERTYSTR (      desc)
    PROPERTY    (int,  processes,           0)
    PROPERTY    (int,  flags,           0)
    PROPERTY    (int,  filterType,      0)
    PROPERTYSTR (      filter)
    PROPERTY    (int,  maxExpected,     0)
    PROPERTYSTR (      destSubDir)
    PROPERTY    (int,  copyFilterType,  0)
    PROPERTYSTR (      copyFilter)

    int compIndex;
    int index;
};

//! \brief ComponentData class stores properties related to each updateable component.
class ComponentData: public CompStoreObj
{
  Q_OBJECT
  public:
    ComponentData & operator=(const ComponentData & rhs);

    enum ReleaseChannel {
      RELEASE_CHANNEL_STABLE,
      RELEASE_CHANNEL_PRERELEASE,
      RELEASE_CHANNEL_NIGHTLY
    };
    Q_ENUM(ReleaseChannel)

    void releaseClear();
    static QStringList releaseChannelsList() { return { tr("Releases"), tr("Pre-release"), tr("Nightly") } ; }

    inline ReleaseChannel boundedReleaseChannel() const {
      return qBound(RELEASE_CHANNEL_STABLE, releaseChannel(), RELEASE_CHANNEL_NIGHTLY); }

    ComponentAssetData asset[MAX_COMPONENT_ASSETS];

    ComponentAssetData & getAsset(int index);
    const ComponentAssetData & getAsset(int index) const;
    void initAllAssets();

  public slots:
    bool existsOnDisk();

  protected:
    explicit ComponentData();
    void setIndex(int idx) { index = idx; }
    inline QString propertyGroup() const override { return QStringLiteral("Components"); }
    inline QString settingsPath()  const override { return QString("%1/component%2/").arg(propertyGroup()).arg(index); }
    friend class AppData;

  private:
    PROPERTY    (bool,           checkForUpdate,  false)
    PROPERTY    (ReleaseChannel, releaseChannel,  RELEASE_CHANNEL_STABLE)
    PROPERTYSTRD(                release,         "unknown")
    PROPERTY    (int,            releaseId,       0)
    PROPERTY    (bool,           prerelease,      false)
    PROPERTYSTRD(                version,         "0")
    PROPERTYSTRD(                date,            "")

    int index;

    CREATE_ENUM_FRIEND_STREAM_OPS(ComponentData::ReleaseChannel)
};

/*!
  \brief The AppData class acts as the main interface for all application settings.
  \note Only one instance of this class should ever exist. It is accessible application-wide through the glabal variable \p g .

  \todo TODO: constructor should be protected/private, this class acts as a singleton.
  \todo TODO: formalize access to Profile and JStickData via getter functions.
  \todo TODO: better profile handling... e.g. a dynamic list instead of fixed number (see above TODO).
  \todo TODO: move \p backLight to \p Profile
*/
class AppData: public CompStoreObj
{
  Q_OBJECT
  public:
    enum NewModelAction {
      MODEL_ACT_NONE,
      MODEL_ACT_WIZARD,
      MODEL_ACT_EDITOR,
      MODEL_ACT_TEMPLATE,
      MODEL_ACT_PROMPT
    };
    Q_ENUM(NewModelAction)

    enum UpdateCheckFreq {
      UPDATE_CHECK_MANUAL,
      UPDATE_CHECK_STARTUP,
      UPDATE_CHECK_DAILY,
      UPDATE_CHECK_WEEKLY,
      UPDATE_CHECK_MONTHLY
    };
    Q_ENUM(UpdateCheckFreq)

    static QStringList newModelActionsList() { return { tr("None"), tr("Wizard"), tr("Editor"), tr("Template"), tr("Prompt") } ; }
    static QStringList updateCheckFreqsList() { return { tr("Manual"), tr("Startup"), tr("Daily"), tr("Weekly"), tr("Monthly") } ; }
    // refer enum QtMsgType
    static QStringList updateLogLevelsList() { return { tr("Debug"), tr("Warning"), tr("Critical"), tr("Fatal"), tr("Information") } ; }

    explicit AppData();
    void init() override;
    void initAll();
    void resetAllSettings();
    void storeAllSettings();
    void resetUpdatesSettings();

    inline bool isFirstUse()         const { return firstUse; }
    inline QString previousVersion() const { return upgradeFromVersion; }
    bool hasCurrentSettings() const;

    //! Get the currently active radio profile ID. This may or may not be the same as \p id(). \sa currentProfile()
    inline int sessionId() const { return m_sessionId; }
    //! Set the current profile ID, but do not save it in persisted settings. To set and save the ID, use \p id(int).
    Q_SLOT void sessionId(int index);
    //! Reset to stored ID
    Q_SLOT inline void sessionIdReset() { sessionId(id()); }

    //! Get a modifiable (non-const) reference to the currently active Profile.  \sa sessionId()
    inline Profile & currentProfile() { return getProfile(m_sessionId); }
    //! Get a non-modifiable (const) reference to the currently active Profile.  \sa sessionId()
    inline const Profile & currentProfile() const { return getProfile(m_sessionId); }
    //! Get a modifiable (non-const) reference to the Profile at \a index. Returns the default profile if \a index is invalid.
    Profile & getProfile(int index);
    //! Get a non-modifiable (const) reference to the Profile at \a index. Returns the default profile if \a index is invalid.
    const Profile & getProfile(int index) const;
    //! List of all active profiles mapped by index.
    QMap<int, QString> getActiveProfiles() const;
    // Move the currently selected profile to the top of the list
    void moveCurrentProfileToTop();

    //! Get a modifiable (non-const) reference to the ComponentData at \a index. Returns component[0] if \a index is invalid.
    ComponentData & getComponent(int index);
    //! Get a non-modifiable (const) reference to the ComponentData at \a index. Returns component[0] if \a index is invalid.
    const ComponentData & getComponent(int index) const;

    //! Find the set of settings from the last previous version installed, if any. Used at virgin startup to offer import option.
    bool findPreviousVersionSettings(QString * version) const;
    //! Converts any old/refactored settings to new ones and removes anything stale if \a removeUnused is true. This is only important when
    //! the meanings of property values change, or to do cleanup when properties are removed.
    void convertSettings(QSettings & settings);
    //! Removes all stored properties which either no longer exist or are set to default value.
    void clearUnusedSettings(QSettings & settings);

    bool importSettings(const QString & fromVersion);
    bool importSettings(QSettings * fromSettings);
    bool exportSettings(QSettings * toSettings, bool clearDestination = true);
    bool exportSettingsToFile(const QString & expFile, QString & resultMsg);

    Profile    profile[MAX_PROFILES];
    JStickData joystick[MAX_JS_AXES];
    JButtonData jsButton[MAX_JS_BUTTONS];
    NamedJSData namedJS[MAX_NAMED_JOYSTICKS];
    FwRevision fwRev;
    ComponentData component[MAX_COMPONENTS];

    void clearJSData() {
      for (int i = 0; i < MAX_JS_AXES; i += 1)
        joystick[i].clear();
      for (int i = 0; i < MAX_JS_BUTTONS; i += 1)
        jsButton[i].clear();
    }
    void saveNamedJS();
    void loadNamedJS();

  signals:
    void currentProfileChanged();

  protected:
    inline QString propertyGroup() const override { return QStringLiteral("General"); }
    inline QString settingsPath()  const override { return QString(); }

    void saveNamedJS(int i);
    void loadNamedJS(int i);

  private:

    PROPERTY_META(int, sessionId, 0)    // currently active radio profile ID, NOT saved to persistent storage -- Initialize before profileId!
    PROPERTY4(int, id, "profileId", 0)  // saved radio profile id, loaded at next Companion startup

    PROPERTY4(QStringList, recentFiles,  "recentFileList", QStringList())

    PROPERTYQBA2(mainWinGeo,   "mainWindowGeometry")
    PROPERTYQBA2(mainWinState, "mainWindowState")
    PROPERTYQBA2(modelEditGeo, "modelEditGeometry")
    PROPERTYQBA (mdiWinGeo)
    PROPERTYQBA (mdiWinState)
    PROPERTYQBA (compareWinGeo)

    PROPERTYSTR3(armMcu,          "arm_mcu",            QStringLiteral("at91sam3s4-9x"))
    PROPERTYSTR2(avrArguments,    "avr_arguments")
    PROPERTYSTR2(avrPort,         "avr_port")
    PROPERTYSTR2(avrdudeLocation, "avrdudeLocation")
    PROPERTYSTR3(dfuArguments,    "dfu_arguments",      QStringLiteral("-a 0"))
    PROPERTYSTR2(dfuLocation,     "dfu_location")
    PROPERTYSTR2(sambaLocation,   "samba_location")
    PROPERTYSTR3(sambaPort,       "samba_port",         QStringLiteral("\\USBserial\\COM23"))

    PROPERTYSTR2(libDir,          "libraryPath")

    PROPERTYSTR3(backupDir,       "backupPath",         CPN_DOCUMENTS_LOCATION % "/backups")
    PROPERTYSTR3(eepromDir,       "lastDir",            CPN_DOCUMENTS_LOCATION % "/eeproms")
    PROPERTYSTR3(flashDir,        "lastFlashDir",       CPN_DOCUMENTS_LOCATION % "/flash")
    PROPERTYSTR3(imagesDir,       "lastImagesDir",      CPN_DOCUMENTS_LOCATION % "/images")
    PROPERTYSTR3(logDir,          "lastLogDir",         CPN_DOCUMENTS_LOCATION % "/debuglogs")
    PROPERTYSTRD(appLogsDir,                            CPN_DOCUMENTS_LOCATION % "/debuglogs")
    PROPERTYSTR3(snapshotDir,     "snapshotpath",       CPN_DOCUMENTS_LOCATION % "/snapshots")
    PROPERTYSTRD(templatesDir,                          CPN_DOCUMENTS_LOCATION % "/templates")
    PROPERTYSTRD(downloadDir,                           CPN_DOCUMENTS_LOCATION % "/downloads")
    PROPERTYSTRD(decompressDir,                         CPN_DOCUMENTS_LOCATION % "/decompress")
    PROPERTYSTRD(updateDir,                             CPN_DOCUMENTS_LOCATION % "/updates")
    PROPERTY    (bool, decompressDirUseDwnld,           true)
    PROPERTY    (bool, updateDirUseSD,                  true)
    PROPERTY    (bool, runAppInstaller,                 false)
    PROPERTY    (int,  updLogLevel,                     4)
    PROPERTY    (bool, updDelDownloads,                 false)
    PROPERTY    (bool, updDelDecompress,                false)
    PROPERTYSTR (lastUpdateDir)

    PROPERTYSTR (locale)
    PROPERTYSTR (gePath)
    PROPERTYSTRD(mcu,        QStringLiteral("m64"))
    PROPERTYSTRD(programmer, QStringLiteral("usbasp"))

    PROPERTY(NewModelAction, newModelAction, MODEL_ACT_WIZARD)

    PROPERTY4(int, embedSplashes,         "embedded_splashes",        0)
    PROPERTY4(int, fwServerFails,         "fwserver",                 0)
    PROPERTY4(int, iconSize,              "icon_size",                2)
    PROPERTY4(int, historySize,           "history_size",             10)
    PROPERTY (int, generalEditTab,                                    0)
    PROPERTY (int, theme,                                             1)
    PROPERTY (int, warningId,                                         0)

    PROPERTY4(bool, jsSupport,            "js_support",               false)
    PROPERTY4(bool, showSplash,           "show_splash",              true)
    PROPERTY4(bool, sortProfiles,         "sort_profiles",            false)
    PROPERTY4(bool, snapToClpbrd,         "snapshot_to_clipboard",    false)

    PROPERTY(UpdateCheckFreq, updateCheckFreq, UPDATE_CHECK_MANUAL)
    PROPERTYSTR(lastUpdateCheck)

    PROPERTY4(bool, promptProfile,        "startup_prompt_profile",   false)

    PROPERTY(bool, enableBackup,               false)
    PROPERTY(bool, backupOnFlash,              true)
    PROPERTY(bool, outputDisplayDetails,       false)
    PROPERTY(bool, checkHardwareCompatibility, true)
    PROPERTY(bool, removeModelSlots,           true)
    PROPERTY(bool, maximized,                  false)
    PROPERTY(bool, tabbedMdi,                  false)
    PROPERTY(bool, appDebugLog,                false)
    PROPERTY(bool, fwTraceLog,                 false)

    // Simulator global (non-profile) settings
    PROPERTY(QStringList, simuDbgFilters, QStringList())
    PROPERTY(int, backLight,       0)
    PROPERTY(int, simuLastProfId, -1)
    PROPERTY(bool, simuSW,      true)
    PROPERTY(bool, disableJoystickWarning, false)

    // Message box confirmations
    PROPERTY(bool, confirmWriteModelsAndSettings, true)

    bool firstUse;
    QString upgradeFromVersion;

    CREATE_ENUM_FRIEND_STREAM_OPS(AppData::NewModelAction)
    CREATE_ENUM_FRIEND_STREAM_OPS(AppData::UpdateCheckFreq)
};

extern AppData g;
