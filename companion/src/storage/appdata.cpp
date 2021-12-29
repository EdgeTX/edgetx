/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#include "appdata.h"

#include <QDateTime>
#include <QDir>
#include <QMetaObject>
#include <QMetaProperty>

#define SETTINGS_VERSION_KEY          QStringLiteral("settings_version")
#define SETTINGS_TIMESTAMP_KEY        QStringLiteral("settingsLastSave")

// Global data and storage object
AppData g;

// ** CompStoreObj class********************

CompStoreObj::CompStoreObj() :
  QObject(),
  m_settings(COMPANY, PRODUCT)
{ }

QString CompStoreObj::pathForKey(const QString & key, const QString & group) const
{
  QString path = (group.isNull() ? settingsPath() : group);
  if (!path.isEmpty() && !path.endsWith('/'))
    path.append('/');
  return path.append(key);
}

void CompStoreObj::load(CompStoreObj * obj, const QString & name, const QString & key, const QVariant & def, const QString & group)
{
  const int idx = obj->metaObject()->indexOfProperty(qPrintable(name));
  if (key.isEmpty() || idx < 0)
    return;
  const QMetaProperty & prop = obj->metaObject()->property(idx);
  const QVariant currValue = prop.read(obj);
  QVariant savedValue = m_settings.value(pathForKey(key, group), def);
  if (savedValue.isValid() && savedValue.convert(currValue.userType()) && savedValue != currValue)
    prop.write(obj, savedValue);
}

void CompStoreObj::store(const QVariant & newValue, const QString & key, const QString & group) const
{
  m_settings.setValue(pathForKey(key, group), newValue);
  m_settings.setValue(SETTINGS_TIMESTAMP_KEY, QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
}

void CompStoreObj::clear(const QString &key, const QString &group) const
{
  m_settings.remove(pathForKey(key, group));
  m_settings.setValue(SETTINGS_TIMESTAMP_KEY, QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
}

void CompStoreObj::removeGroupIfEmpty(const QString &group) const
{
  m_settings.beginGroup(group);
  if (!m_settings.allKeys().size())
    m_settings.remove("");
  m_settings.endGroup();
}

//static void dumpMetadata(QObject *obj) {
//  for (int i = obj->metaObject()->propertyOffset() /* 0 */, e = obj->metaObject()->propertyCount(); i < e; ++i)
//    qDebug() << obj->metaObject()->property(i).name() << "=" << obj->metaObject()->property(i).read(obj);
//  for (int i = obj->metaObject()->methodOffset() /* 0 */, e = obj->metaObject()->methodCount(); i < e; ++i) {
//    QMetaMethod::Access a = obj->metaObject()->method(i).access(); QMetaMethod::MethodType t = obj->metaObject()->method(i).methodType();
//    qDebug().noquote() << (a == 0 ? "  private" : a == 1 ? "protected" : "   public") << (t == 1 ? "signal:" : t == 2 ? "slot:  " : ":      ")
//                       << obj->metaObject()->method(i).typeName() << obj->metaObject()->method(i).methodSignature();
//  }
//}

// static methods

bool CompStoreObj::initProperty(CompStoreObj * obj, const QString & name)
{
  if (obj && !name.isEmpty()) {
    const int initIdx = obj->metaObject()->indexOfMethod(PROP_FSIG_INIT_STR(name));
    if (initIdx > -1)
      return obj->metaObject()->method(initIdx).invoke(obj);
  }
  return false;
}

void CompStoreObj::initAllProperties(CompStoreObj * obj)
{
  if (!obj)
    return;
  for (int i = obj->metaObject()->propertyOffset(), e = obj->metaObject()->propertyCount(); i < e; ++i)
    initProperty(obj, QString(obj->metaObject()->property(i).name()));
}

bool CompStoreObj::storeProperty(CompStoreObj * obj, const QString & name)
{
  if (!obj || name.isEmpty())
    return false;

  const QMetaProperty & prop = obj->metaObject()->property(obj->metaObject()->indexOfProperty(qPrintable(name)));
  const QString key = propertyKeyName(obj, name);
  QVariant currValue;
  if (!prop.isValid() || !prop.isStored() || key.isEmpty() || !(currValue = prop.read(obj)).isValid())
    return false;
  if (currValue == propertyDefaultValue(obj, name)) {
    obj->clear(key);
    obj->removeGroupIfEmpty(obj->settingsPath());
  }
  else {
    obj->store(currValue, key);
  }
  return false;
}

void CompStoreObj::storeAllProperties(CompStoreObj * obj)
{
  if (!obj)
    return;
  for (int i = obj->metaObject()->propertyOffset(), e = obj->metaObject()->propertyCount(); i < e; ++i)
    storeProperty(obj, QString(obj->metaObject()->property(i).name()));
}

bool CompStoreObj::resetProperty(CompStoreObj * obj, const QString & name)
{
  if (obj && !name.isEmpty()) {
    const QMetaProperty & prop = obj->metaObject()->property(obj->metaObject()->indexOfProperty(qPrintable(name)));
    if (prop.isValid() && prop.isResettable())
      return prop.reset(obj);
  }
  return false;
}

void CompStoreObj::resetAllProperties(CompStoreObj * obj)
{
  if (!obj)
    return;
  for (int i = obj->metaObject()->propertyOffset(), e = obj->metaObject()->propertyCount(); i < e; ++i) {
    const QMetaProperty & prop = obj->metaObject()->property(i);
    if (prop.isValid() && prop.isResettable())
      prop.reset(obj);
  }
}

bool CompStoreObj::propertyPathIsValid(const QString & path)
{
  const QPair<QString, QString> grp = splitGroupedPath(path);  // {<group name>, <property key>}
  return (groupHasDynamicProperties(grp.first) || propertyKeyExists(grp.first, grp.second));
}

bool CompStoreObj::propertyPathIsValidNonDefault(const QString & path, const QVariant & newVal)
{
  const QPair<QString, QString> grp = splitGroupedPath(path);  // {<group name>, <property key>}
  return (groupHasDynamicProperties(grp.first) || (propertyKeyExists(grp.first, grp.second) && newVal != propertyKeyDefaultValue(grp.first, grp.second)));
}

bool CompStoreObj::propertyExists(CompStoreObj * obj, const QString & name)
{
  if (!obj)
    return false;
  return obj->metaObject()->indexOfProperty(qPrintable(name)) > -1;
}

const QVariant CompStoreObj::propertyDefaultValue(CompStoreObj * obj, const QString & name)
{
  QVariant ret;
  if (!obj || name.isEmpty())
    return ret;
  const int idx = obj->metaObject()->indexOfMethod(PROP_FSIG_DFLT_STR(name));
  if (idx > -1)
    obj->metaObject()->method(idx).invoke(obj, Qt::DirectConnection, Q_RETURN_ARG(QVariant, ret));
  //qDebug() << ret;
  return ret;
}

const QString CompStoreObj::propertyKeyName(CompStoreObj * obj, const QString & name)
{
  QString ret;
  if (!obj || name.isEmpty())
    return ret;
  const int idx = obj->metaObject()->indexOfMethod(PROP_FSIG_KEY_STR(name));
  if (idx > -1)
    obj->metaObject()->method(idx).invoke(obj, Qt::DirectConnection, Q_RETURN_ARG(QString, ret));
  //qDebug() << ret;
  return ret;
}

QPair<QString, QString> CompStoreObj::splitGroupedPath(const QString & path)
{
  const QStringList list = path.split('/');
  const QString grp = list.size() > 1 ? list.first() : QStringLiteral("General");  // "General" is the default (no-name) group, used by AppData top-level settings.
  return QPair<QString, QString>(grp, list.last());
}

QHash<QString, CompStoreObj *> & CompStoreObj::group2ObjMap()
{
  static QHash<QString, CompStoreObj *> map;
  return map;
}

QVector<QString> &CompStoreObj::dynamicPropGroups()
{
  static QVector<QString> list;
  return list;
}

const QHash<QString, QHash<QString, QString> > & CompStoreObj::keyToNameMap()
{
  static QHash<QString, QHash<QString, QString> > map;
  if (map.isEmpty()) {
    QHashIterator<QString, CompStoreObj *> it(group2ObjMap());
    while (it.hasNext()) {
      it.next();
      QHash<QString, QString> grpMap;
      CompStoreObj * obj = it.value();
      //dumpMetadata(obj);
      for (int i = obj->metaObject()->propertyOffset(), e = obj->metaObject()->propertyCount(); i < e; ++i) {
        const QString name = QString(obj->metaObject()->property(i).name());
        const QString key = propertyKeyName(obj, name);
        if (!key.isEmpty())
          grpMap.insert(key, name);
      }
      map.insert(it.key(), grpMap);
      //qDebug() << it.key() << grpMap;
    }
  }
  return map;
}


// ** JStickData class********************

JStickData::JStickData() : CompStoreObj(), index(-1)
{
  CompStoreObj::addObjectMapping(propertyGroup(), this);
}

bool JStickData::existsOnDisk()
{
  return (m_settings.value(settingsPath() % stick_axe_key(), -1).toInt() > -1);
}


// ** Profile class********************

Profile::Profile() : CompStoreObj(), index(-1)
{
  CompStoreObj::addObjectMapping(propertyGroup(), this);
}

// The default copy operator can not be used since the index variable would be destroyed
Profile & Profile::operator= (const Profile & rhs)
{
  for (int i = metaObject()->propertyOffset(), e = metaObject()->propertyCount(); i < e; ++i) {
    const QMetaProperty & prop = metaObject()->property(i);
    if (!prop.isValid() || !prop.isWritable()) {
      qWarning() << "Could not copy property" << QString(prop.name()) << "isValid:" << prop.isValid() << "isWritable:" << prop.isWritable();
      continue;
    }
    prop.write(this, prop.read(&rhs));
  }
  return *this;
}

bool Profile::existsOnDisk()
{
  return m_settings.contains(settingsPath() % "Name");
}

void Profile::resetFwVariables()
{
  for (const QString & name : fwVarsList())
    CompStoreObj::resetProperty(this, name);
}


// ** AppData class********************

AppData::AppData() :
  CompStoreObj(),
  m_sessionId(0)
{
  QMetaType::registerComparators<SimulatorOptions>();
  qRegisterMetaTypeStreamOperators<SimulatorOptions>("SimulatorOptions");
  qRegisterMetaTypeStreamOperators<AppData::DownloadBranchType>("AppData::DownloadBranchType");
  qRegisterMetaTypeStreamOperators<AppData::NewModelAction>("AppData::NewModelAction");

  CompStoreObj::addObjectMapping(propertyGroup(), this);

  firstUse = !hasCurrentSettings();

  // Make sure the settings backup folder exists
  if (firstUse && !QDir().mkpath(CPN_SETTINGS_BACKUP_DIR))
    qWarning() << "Could not create settings backup path" << CPN_SETTINGS_BACKUP_DIR;

  // Configure the profiles
  for (int i=0; i<MAX_PROFILES; i++)
    profile[i].setIndex(i);

  // Configure the joysticks
  for (int i=0; i<MAX_JOYSTICKS; i++)
    joystick[i].setIndex(i);
}

static QString fmtHex(quint32 num)
{
  return QString::number(num, 16).toUpper();
}

void AppData::init()
{
  qInfo().noquote() << "Settings init with" << m_settings.organizationName() << m_settings.applicationName()
                    << "Saved version:"   << fmtHex(m_settings.value(SETTINGS_VERSION_KEY).toUInt()) << "Current version:" << fmtHex(CPN_SETTINGS_VERSION);

  // This connection doesn't work in the constructor because AppData is created before QApplication. Globals suck like that. Compensate by using Qt::UniqueConnection because init() may be called multiple times within app lifetime.
  connect(this, &AppData::idChanged, this, static_cast<void (AppData::*)(int)>(&AppData::sessionId), Qt::UniqueConnection);

  // update anything that may have changed since last saved version and update the version marker if needed.
  convertSettings(m_settings);

  initAll();
}

void AppData::initAll()
{
  // Initialize all variables. Use default values if no saved settings.
  CompStoreObj::initAllProperties(this);
  // Initialize the profiles
  for (int i=0; i<MAX_PROFILES; i++)
    profile[i].init();
  // Initialize the joysticks
  for (int i=0; i<MAX_JOYSTICKS; i++)
    joystick[i].init();
}

void AppData::resetAllSettings()
{
  resetAll();
  fwRev.resetAll();
  for (int i=0; i<MAX_PROFILES; i++)
    profile[i].resetAll();
  for (int i=0; i<MAX_JOYSTICKS; i++)
    joystick[i].resetAll();
  firstUse = true;
}

void AppData::storeAllSettings()
{
  storeAll();
  for (int i=0; i<MAX_PROFILES; i++)
    profile[i].storeAll();
  for (int i=0; i<MAX_JOYSTICKS; i++)
    joystick[i].storeAll();
}

bool AppData::hasCurrentSettings() const
{
  return m_settings.contains(SETTINGS_VERSION_KEY);
}

void AppData::sessionId(int index)
{
  if (index < 0 || index >= MAX_PROFILES || index == m_sessionId)
    return;
  m_sessionId = index;
  emit sessionIdChanged(index);
  emit currentProfileChanged();
}

Profile & AppData::getProfile(int index)
{
  if (index > -1 && index < MAX_PROFILES)
    return profile[index];
  return profile[0];
}

const Profile & AppData::getProfile(int index) const
{
  if (index > -1 && index < MAX_PROFILES)
    return profile[index];
  return profile[0];
}

QMap<int, QString> AppData::getActiveProfiles() const
{
  QMap<int, QString> active;
  for (int i=0; i<MAX_PROFILES; i++) {
    if (g.profile[i].existsOnDisk())
      active.insert(i, g.profile[i].name());
  }
  return active;
}

void AppData::convertSettings(QSettings & settings)
{
  quint32 savedVer = settings.value(SETTINGS_VERSION_KEY, 0).toUInt();
  if (savedVer == CPN_SETTINGS_VERSION)
    return;
  if (savedVer > CPN_SETTINGS_VERSION) {
    qWarning().noquote() << "Saved settings version is newer than current, skipping conversions. Saved:" << fmtHex(savedVer) << "Current:" << fmtHex(CPN_SETTINGS_VERSION);
    return;
  }

  // We only want to remove old/deprecated settngs if the {major}{minor} part of the settings version number has changed.
  // This preserves backwards compatiblity within minor version releases (where only revision number changes).
  const unsigned savedMajMin = (savedVer >> 16);
  const unsigned currMajMin = (CPN_SETTINGS_VERSION >> 16);
  const bool removeUnused = (savedMajMin < currMajMin);

  qInfo().noquote().nospace() << "Converting settings " << settings.applicationName()
                              << " from v" << fmtHex(savedVer) << " (" << fmtHex(savedMajMin) << ") to v"
                              <<  fmtHex(CPN_SETTINGS_VERSION) << " (" << fmtHex(currMajMin)  << "). Removing unused: " << removeUnused;

  // firmwares renamed from opentx-* to edgetx-* at 2.6
  if (savedMajMin <= 0x207) {        // Note: change merged post 2.6 rc 1 and version bumped to 2.7 the Nightly users also require upgrade
    qInfo().noquote() << "Converting profiles";
    static const QString profileFwTypePath = QStringLiteral("Profiles/profile%1/fwType");
    for (int i = 0; i < MAX_PROFILES; i++) {
      if (settings.contains(profileFwTypePath.arg(i))) {
        const QVariant oldValue = settings.value(profileFwTypePath.arg(i));
        if (oldValue.isValid()) {
          const QString oldval = settings.value(profileFwTypePath.arg(i)).toString();
          QString newval = oldval;
          newval.replace("opentx-", "edgetx-");
          if (oldval != newval) {
            settings.setValue(profileFwTypePath.arg(i), newval);
            qInfo().noquote().nospace() << "Converted entry " << profileFwTypePath.arg(i)
                                        << " from (" << oldval << ")"
                                        << " to (" << newval << ")";
          }
        }
      }
    }
  }

  if (removeUnused)
    clearUnusedSettings(settings);

  // This is a version marker, used to convert settings between versions. Do NOT remove it, ever! Also see notes on CPN_SETTINGS_VERSION macro.
  settings.setValue(SETTINGS_VERSION_KEY, CPN_SETTINGS_VERSION);
}

void AppData::clearUnusedSettings(QSettings & settings)
{
  // Go through and clean up anything that doesn't exist or is set to default value.
  foreach (const QString & key, settings.allKeys()) {
    if (key == ".")  // special Windows registry key, don't delete it
      continue;
    const QVariant newVal = settings.value(key);
    // Remove key if property does not exist or is the default value.
    if (!newVal.isValid() || !CompStoreObj::propertyPathIsValidNonDefault(key, newVal))
      settings.remove(key);
  }
}

bool AppData::findPreviousVersionSettings(QString * version) const
{
  //  TODO need a more dynamic method since ETX moves so quickly
  static const QStringList versList({QStringLiteral("2.6"), QStringLiteral("2.5"), QStringLiteral("2.4")});

  for (const QString &ver : versList) {
    const QString prod("Companion " % ver);
    // make sure we do not try to import from ourselves otherwise settings WILL get corrupted
    if (prod == PRODUCT)
      continue;
    QSettings settings(COMPANY, prod);
    if (settings.contains(SETTINGS_VERSION_KEY)) {
      *version = ver;
      return true;
    }
    else {
      settings.clear();
    }
  }

  return false;
}

bool AppData::importSettings(const QString & fromVersion)
{
  QString fromProduct =  PRODUCT_NO_VERS % " " % fromVersion;
  //qDebug() << "From Product:" << fromProduct;
  upgradeFromVersion.clear();

  upgradeFromVersion = fromVersion;

  QSettings fromSettings(COMPANY, fromProduct);
  return importSettings(&fromSettings);
}

bool AppData::importSettings(QSettings * fromSettings)
{
  if (!fromSettings)
    return false;

  // Create temporary settings because we may modify them before import.
  QSettings tempSettings(COMPANY, PRODUCT % "_import");
  for (const QString & key : fromSettings->allKeys()) {
    tempSettings.setValue(key, fromSettings->value(key));
    //qInfo().noquote() << "Import key(" << key << ") value(" << fromSettings->value(key) << ")";
  }

  // convert settings first to simplify import process
  convertSettings(tempSettings);

  if (fromSettings->status() != QSettings::NoError || tempSettings.status() != QSettings::NoError) {
    tempSettings.clear();
    return false;
  }

  // import settings, just copy since they have already been converted/cleaned.
  for (const QString & key : tempSettings.allKeys()) {
    if (key != ".")  // special Windows registry key, don't import it
      m_settings.setValue(key, tempSettings.value(key));
  }

  tempSettings.clear();

  if (m_settings.status() != QSettings::NoError)
    return false;

  firstUse = !hasCurrentSettings();
  return true;
}

bool AppData::exportSettings(QSettings * toSettings, bool clearDestination)
{
  if (!toSettings)
    return false;

  m_settings.sync();
  if (clearDestination)
    toSettings->clear();
  foreach (const QString & key, m_settings.allKeys()) {
    const QVariant newVal = m_settings.value(key);
    // Skip export if property does not exist or is the default value.
    if (newVal.isValid() && CompStoreObj::propertyPathIsValidNonDefault(key, newVal))
      toSettings->setValue(key, newVal);
    //else qDebug() << "SKIPPING:" << key << newVal;
  }
  // Write the version and timestamp to export file -- they will NOT be imported, but may be useful for converting future imports.
  toSettings->setValue(SETTINGS_VERSION_KEY, m_settings.value(SETTINGS_VERSION_KEY));
  toSettings->setValue(SETTINGS_TIMESTAMP_KEY, m_settings.value(SETTINGS_TIMESTAMP_KEY));
  toSettings->sync();

  return (toSettings->status() == QSettings::NoError);
}

bool AppData::exportSettingsToFile(const QString & expFile, QString & resultMsg)
{
  QSettings toSettings(expFile, QSettings::IniFormat);
  if (exportSettings(&toSettings)) {
    resultMsg = tr("Application Settings have been saved to\n %1").arg(expFile);
    return true;
  }
  resultMsg = tr("Could not save Application Settings to file \"%1\"").arg(expFile) % " ";
  if (toSettings.status() == QSettings::AccessError)
    resultMsg.append(tr("because the file could not be saved (check access permissions)."));
  else
    resultMsg.append(tr("for unknown reasons."));
  return false;
}
