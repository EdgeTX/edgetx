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

#include "appdata.h"
#include "updates/updateoptionsdialog.h"

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
  if (savedValue.isValid() && savedValue.convert(currValue.metaType()) && savedValue != currValue)
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

// static void dumpMetadata(QObject *obj) {
//  for (int i = obj->metaObject()->propertyOffset() /* 0 */, e = obj->metaObject()->propertyCount(); i < e; ++i)
//    qDebug() << obj->metaObject()->property(i).name() << "=" << obj->metaObject()->property(i).read(obj);
//  for (int i = obj->metaObject()->methodOffset() /* 0 */, e = obj->metaObject()->methodCount(); i < e; ++i) {
//    QMetaMethod::Access a = obj->metaObject()->method(i).access(); QMetaMethod::MethodType t = obj->metaObject()->method(i).methodType();
//    qDebug().noquote() << (a == 0 ? "  private" : a == 1 ? "protected" : "   public") << (t == 1 ? "signal:" : t == 2 ? "slot:  " : ":      ")
//                       << obj->metaObject()->method(i).typeName() << obj->metaObject()->method(i).methodSignature();
//  }
// }

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
  // grp is propertyGroup() thus why size minus 2
  const QString grp = list.size() > 1 ? list.mid(0, list.size() - 2).join("/") : QStringLiteral("General");  // "General" is the default (no-name) group, used by AppData top-level settings.
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
      // dumpMetadata(obj);
      for (int i = obj->metaObject()->propertyOffset(), e = obj->metaObject()->propertyCount(); i < e; ++i) {
        const QString name = QString(obj->metaObject()->property(i).name());
        const QString key = propertyKeyName(obj, name);
        if (!key.isEmpty())
          grpMap.insert(key, name);
      }
      map.insert(it.key(), grpMap);
      // qDebug() << it.key() << grpMap;
    }
  }
  return map;
}


// ** JStickData class********************

JStickData::JStickData() : CompStoreObj(), index(-1)
{
}

bool JStickData::existsOnDisk()
{
  return (m_settings.value(settingsPath() % stick_axe_key(), -1).toInt() > -1);
}

JButtonData::JButtonData() : CompStoreObj(), index(-1)
{
}

bool JButtonData::existsOnDisk()
{
  return (m_settings.value(settingsPath() % button_idx_key(), -1).toInt() > -1);
}

NamedJStickData::NamedJStickData() : CompStoreObj(), index(-1)
{
}

bool NamedJStickData::existsOnDisk()
{
  return (m_settings.value(settingsPath() % stick_axe_key(), -1).toInt() > -1);
}

NamedJButtonData::NamedJButtonData() : CompStoreObj(), index(-1)
{
}

bool NamedJButtonData::existsOnDisk()
{
  return (m_settings.value(settingsPath() % button_idx_key(), -1).toInt() > -1);
}

NamedJSData::NamedJSData() : CompStoreObj(), index(-1)
{
}

bool NamedJSData::existsOnDisk()
{
  return (m_settings.value(settingsPath() % jsName(), -1).toInt() > -1);
}

// ** ComponentReleaseData class********************

ComponentReleaseData::ComponentReleaseData() : CompStoreObj(), index(-1)
{
}

// The default copy operator can not be used since the index variable would be destroyed
ComponentReleaseData & ComponentReleaseData::operator= (const ComponentReleaseData & rhs)
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

// ** Profile class********************

Profile::Profile() : CompStoreObj(), index(-1)
{
}

Profile::Profile(const Profile & rhs) : CompStoreObj(), index(-1)
{
  *this = rhs;
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

  for (int i = 0; i < MAX_COMPONENTS; i++) {
    compRelease[i] = rhs.compRelease[i];
  }

  return *this;
}

bool Profile::existsOnDisk()
{
  return m_settings.contains(settingsPath() % "Name");
}

ComponentReleaseData & Profile::getCompRelease(int index)
{
  if (index > -1 && index < MAX_COMPONENTS)
    return compRelease[index];

  return compRelease[0];
}

const ComponentReleaseData & Profile::getCompRelease(int index) const
{
  if (index > -1 && index < MAX_COMPONENTS)
    return compRelease[index];

  return compRelease[0];
}

// ** ComponentAssetData class********************

ComponentAssetData::ComponentAssetData() : CompStoreObj(), index(-1)
{
}

// The default copy operator can not be used since the index variable would be destroyed
ComponentAssetData & ComponentAssetData::operator= (const ComponentAssetData & rhs)
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

bool ComponentAssetData::existsOnDisk()
{
  return (m_settings.contains(settingsPath() % "desc"));
}

// ** ComponentData class********************

ComponentData::ComponentData() : CompStoreObj(), index(-1)
{
  qRegisterMetaType<ComponentData::ReleaseChannel>("ComponentData::ReleaseChannel");
}

// The default copy operator can not be used since the index variable would be destroyed
ComponentData & ComponentData::operator= (const ComponentData & rhs)
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

bool ComponentData::existsOnDisk()
{
  return (m_settings.contains(settingsPath() % "name"));
}

ComponentAssetData & ComponentData::getAsset(int index)
{
  if (index > -1 && index < MAX_COMPONENT_ASSETS)
    return asset[index];
  return asset[0];
}

const ComponentAssetData & ComponentData::getAsset(int index) const
{
  if (index > -1 && index < MAX_COMPONENT_ASSETS)
    return asset[index];
  return asset[0];
}

void ComponentData::initAllAssets()
{
  for (int i = 0; i < MAX_COMPONENT_ASSETS; i++) {
    asset[i].resetAll();
  }
}

// ** AppData class********************

AppData::AppData() :
  CompStoreObj(),
  m_sessionId(0)
{
  qRegisterMetaType<AppData::NewModelAction>("AppData::NewModelAction");
  qRegisterMetaType<AppData::UpdateCheckFreq>("AppData::UpdateCheckFreq");
  qRegisterMetaType<AppData::SimuGenericKeysPos>("AppData::SimuGenericKeysPos");

  CompStoreObj::addObjectMapping(propertyGroup(), this);

  firstUse = !hasCurrentSettings();

  // Make sure the settings backup folder exists
  if (firstUse && !QDir().mkpath(CPN_SETTINGS_BACKUP_DIR))
    qWarning() << "Could not create settings backup path" << CPN_SETTINGS_BACKUP_DIR;

  // Configure the profiles
  for (int i = 0; i < MAX_PROFILES; i++) {
    profile[i].setIndex(i);

    for (int j = 0; j < MAX_COMPONENTS; j++) {
      profile[i].compRelease[j].setIndexes(i, j);
    }
  }

  // Configure the joysticks
  for (int i = 0; i < MAX_JS_AXES; i++)
    joystick[i].setIndex(i);

  for (int i = 0; i < MAX_JS_BUTTONS; i++)
    jsButton[i].setIndex(i);

  for (int i = 0; i < MAX_NAMED_JOYSTICKS; i++) {
    namedJS[i].setIndex(i);

    for (int j = 0; j < MAX_JS_AXES; j++)
      namedJS[i].joystick[j].setIndexes(j, i);

    for (int j = 0; j < MAX_JS_BUTTONS; j++)
      namedJS[i].jsButton[j].setIndexes(j, i);
  }

  // Configure the updates
  for (int i = 0; i < MAX_COMPONENTS; i++) {
    component[i].setIndex(i);

    for (int j = 0; j < MAX_COMPONENT_ASSETS; j++) {
      component[i].asset[j].setIndexes(i, j);
    }
  }
}

void AppData::saveNamedJS(int i)
{
  namedJS[i].jsName(currentProfile().jsName());

  for (int j = 0; j < MAX_JS_AXES; j++) {
    namedJS[i].joystick[j].stick_axe(joystick[j].stick_axe());
    namedJS[i].joystick[j].stick_max(joystick[j].stick_max());
    namedJS[i].joystick[j].stick_med(joystick[j].stick_med());
    namedJS[i].joystick[j].stick_min(joystick[j].stick_min());
    namedJS[i].joystick[j].stick_inv(joystick[j].stick_inv());
  }

  for (int j = 0; j < MAX_JS_BUTTONS; j++) {
    namedJS[i].jsButton[j].button_idx(jsButton[j].button_idx());
  }

  namedJS[i].jsLastUsed(time(NULL));
}

void AppData::saveNamedJS()
{
  for (int i = 0; i < MAX_NAMED_JOYSTICKS; i++) {
    if (namedJS[i].jsName() == currentProfile().jsName()) {
      saveNamedJS(i);
      return;
    }
  }

  for (int i = 0; i < MAX_NAMED_JOYSTICKS; i++) {
    if (namedJS[i].jsName() == "") {
      saveNamedJS(i);
      return;
    }
  }

  unsigned int oldestTime = namedJS[0].jsLastUsed();
  int oldestN = 0;
  for (int i = 1; i < MAX_NAMED_JOYSTICKS; i++) {
    if (namedJS[i].jsLastUsed() < oldestTime) {
      oldestTime = namedJS[i].jsLastUsed();
      oldestN = i;
    }
  }
  saveNamedJS(oldestN);
}

void AppData::loadNamedJS(int i)
{
  for (int j = 0; j < MAX_JS_AXES; j++) {
    joystick[j].stick_axe(namedJS[i].joystick[j].stick_axe());
    joystick[j].stick_max(namedJS[i].joystick[j].stick_max());
    joystick[j].stick_med(namedJS[i].joystick[j].stick_med());
    joystick[j].stick_min(namedJS[i].joystick[j].stick_min());
    joystick[j].stick_inv(namedJS[i].joystick[j].stick_inv());
  }

  for (int j = 0; j < MAX_JS_BUTTONS; j++) {
    jsButton[j].button_idx(namedJS[i].jsButton[j].button_idx());
  }

  namedJS[i].jsLastUsed(time(NULL));
}

void AppData::loadNamedJS()
{
  for (int i = 0; i < MAX_NAMED_JOYSTICKS; i++) {
    if (namedJS[i].jsName() == currentProfile().jsName()) {
      loadNamedJS(i);
      return;
    }
  }
}

void AppData::init()
{
  qInfo().noquote() << "Settings init with" << m_settings.organizationName() << m_settings.applicationName()
                    << "Saved version:" << settingsVersionToDisplay(m_settings.value(SETTINGS_VERSION_KEY).toUInt())
                    << "Current version:" << settingsVersionToDisplay(CPN_SETTINGS_VERSION);

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
  for (int i = 0; i < MAX_PROFILES; i++) {
    profile[i].init();

    for (int j = 0; j < MAX_COMPONENTS; j++) {
      profile[i].compRelease[j].init();
    }
  }

  // Initialize the joysticks
  for (int i = 0; i < MAX_JS_AXES; i++)
    joystick[i].init();

  for (int i = 0; i < MAX_JS_BUTTONS; i++)
    jsButton[i].init();

  for (int i = 0; i < MAX_NAMED_JOYSTICKS; i++) {
    namedJS[i].init();

    for (int j = 0; j < MAX_JS_AXES; j++)
      namedJS[i].joystick[j].init();

    for (int j = 0; j < MAX_JS_BUTTONS; j++)
      namedJS[i].jsButton[j].init();
  }

  // Initialize the updates
  for (int i = 0; i < MAX_COMPONENTS; i++) {
    component[i].init();

    for (int j = 0; j < MAX_COMPONENT_ASSETS; j++) {
      component[i].asset[j].init();
    }
  }
}

void AppData::resetAllSettings()
{
  resetAll();
  fwRev.resetAll();

  for (int i = 0; i < MAX_PROFILES; i++) {
    profile[i].resetAll();

    for (int j = 0; j < MAX_COMPONENTS; j++) {
      profile[i].compRelease[j].resetAll();
    }
  }

  for (int i = 0; i < MAX_JS_AXES; i++)
    joystick[i].resetAll();

  for (int i = 0; i < MAX_JS_BUTTONS; i++)
    jsButton[i].resetAll();

  for (int i = 0; i < MAX_NAMED_JOYSTICKS; i++) {
    namedJS[i].resetAll();

    for (int j = 0; j < MAX_JS_AXES; j++)
      namedJS[i].joystick[j].resetAll();

    for (int j = 0; j < MAX_JS_BUTTONS; j++)
      namedJS[i].jsButton[j].resetAll();
  }

  for (int i = 0; i < MAX_COMPONENTS; i++) {
    component[i].resetAll();

    for (int j = 0; j < MAX_COMPONENT_ASSETS; j++) {
      component[i].asset[j].resetAll();
    }
  }

  firstUse = true;
}

void AppData::storeAllSettings()
{
  storeAll();

  for (int i = 0; i < MAX_PROFILES; i++) {
    profile[i].storeAll();

    for (int j = 0; j < MAX_COMPONENTS; j++) {
      profile[i].compRelease[j].storeAll();
    }
  }

  for (int i = 0; i < MAX_JS_AXES; i++)
    joystick[i].storeAll();

  for (int i = 0; i < MAX_JS_BUTTONS; i++)
    jsButton[i].storeAll();

  for (int i = 0; i < MAX_NAMED_JOYSTICKS; i++) {
    namedJS[i].storeAll();

    for (int j = 0; j < MAX_JS_AXES; j++)
      namedJS[i].joystick[j].storeAll();

    for (int j = 0; j < MAX_JS_BUTTONS; j++)
      namedJS[i].jsButton[j].storeAll();
  }

  for (int i = 0; i < MAX_COMPONENTS; i++) {
    component[i].storeAll();

    for (int j = 0; j < MAX_COMPONENT_ASSETS; j++)
      component[i].asset[j].storeAll();
  }
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
  loadNamedJS();
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

void AppData::moveCurrentProfileToTop()
{
  if (g.sortProfiles() && m_sessionId > 0) {
    Profile tmpProfile(g.profile[m_sessionId]);

    for (int i = m_sessionId; i > 0; i--) {
      g.profile[i] = g.profile[i - 1];
    }

    g.profile[0] = tmpProfile;
    id(0);
  }
}

void AppData::convertSettings(QSettings & settings)
{
  quint32 savedVer = settings.value(SETTINGS_VERSION_KEY, 0).toUInt();

  if (savedVer == CPN_SETTINGS_VERSION)
    return;

  if (savedVer > CPN_SETTINGS_VERSION) {
    qWarning().noquote() << "Saved settings version is newer than current, skipping conversions. Saved:"
      << settingsVersionToDisplay(savedVer) << "Current:"
      << settingsVersionToDisplay(CPN_SETTINGS_VERSION);
    return;
  }

  // We only want to remove old/deprecated settngs if the {major}{minor} part of the settings version number has changed.
  // This preserves backwards compatiblity within minor version releases (where only revision number changes).
  const unsigned savedMajMin = (savedVer >> 16);
  const unsigned currMajMin = (CPN_SETTINGS_VERSION >> 16);
  const bool removeUnused = (savedMajMin < currMajMin);

  qInfo().noquote().nospace() << "Converting settings " << settings.applicationName()
                              << " from " << settingsVersionToDisplay(savedVer)
                              << " to " << settingsVersionToDisplay(CPN_SETTINGS_VERSION)
                              << ". Removing unused: " << removeUnused;

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

  if (savedMajMin < 0x209) {
    //  2.9 component id renamed releaseId - copy value before calling clearUnusedSettings
    qInfo().noquote() << "Converting components - moving id to releaseId";
    static const QString path = QStringLiteral("Components/component%1/%2");
    for (int i = 0; i < MAX_COMPONENTS; i++) {
      if (settings.contains(path.arg(i).arg("id"))) {
        const QVariant id = settings.value(path.arg(i).arg("id"));
        settings.setValue(path.arg(i).arg("releaseId"), id);
      }
    }
  }

  if (savedMajMin < 0x300) {
    // CloudBuild copy filter changed to cater for uf2
    qInfo().noquote() << "Deleting CloudBuild settings to force refresh";
    QString path = QStringLiteral("Components/component6");
    if (settings.contains(path))
      settings.remove(path);

    // Joystick settings paths changed to fix export issue
    qInfo().noquote() << "Reorganising joystick settings";
/*
     Old structure TODO THIS IS NOT CORRECT SO FIX IT
    -------------
    JsCalibration
      |_<n>
          |_<data>
    JsButton
      |_<n>
          |_<data>
    NamedJSData
      |_<n>
        |_<data>
        |_JsCalibration
            |_<n>
              |_<data>
        |_JsButton
            |_<n>
              |_<data>

    New structure
    -------------
    JsCalibration
      |_stick<n>
          |_<data>
    JsButtons
      |_button<n>
          |_<data>
    NamedJsData
      |_name<n>
        |_<data>
        |_stick<n>
            |_<data>
        |_button<n>
            |_<data>
 */
    QString oldpath = "JsCalibration/%1/%2";
    QString newpath = "JsCalibration/stick%1/%2";
    QStringList keys = { "stick_axe", "stick_min", "stick_med", "stick_max", "stick_inv" };

    for (int i = 0; i < MAX_JS_AXES; i++) {
      for (int k = 0; k < keys.size(); k++) {
        const QString opath = oldpath.arg(i).arg(keys.at(k));
        const QString npath = newpath.arg(i).arg(keys.at(k));

        if (settings.contains(opath)) {
          settings.setValue(npath, settings.value(opath));
          settings.remove(opath);
          qInfo().noquote() << "Moved " << opath << " to " << npath;
        }
      }
    }

    oldpath = "JsButton/%1/%2";
    newpath = "JsButtons/button%1/%2";
    keys = { "button_idx" };

    for (int i = 0; i < MAX_JS_BUTTONS; i++) {
      for (int k = 0; k < keys.size(); k++) {
        const QString opath = oldpath.arg(i).arg(keys.at(k));
        const QString npath = newpath.arg(i).arg(keys.at(k));

        if (settings.contains(opath)) {
          settings.setValue(npath, settings.value(opath));
          settings.remove(opath);
          qInfo().noquote() << "Moved " << opath << " to " << npath;
        }
      }
    }

    for (int j = 0; j < MAX_NAMED_JOYSTICKS; j++) {
      oldpath = "NamedJSData/%1/%2";
      newpath = "NamedJSData/name%1/%2";
      keys = { "jsName", "jsLastUsed" };

      for (int k = 0; k < keys.size(); k++) {
        const QString opath = oldpath.arg(j).arg(keys.at(k));
        const QString npath = newpath.arg(j).arg(keys.at(k));

        if (settings.contains(opath)) {
          settings.setValue(npath, settings.value(opath));
          settings.remove(opath);
          qInfo().noquote() << "Moved " << opath << " to " << npath;
        }
      }

      oldpath = "NamedJSData/%1/JsCalibration/%2/%3";
      newpath = "NamedJSData/name%1/stick%2/%3";
      keys = { "stick_axe", "stick_min", "stick_med", "stick_max", "stick_inv" };

      for (int i = 0; i < MAX_JS_AXES; i++) {
        for (int k = 0; k < keys.size(); k++) {
          const QString opath = oldpath.arg(j).arg(i).arg(keys.at(k));
          const QString npath = newpath.arg(j).arg(i).arg(keys.at(k));

          if (settings.contains(opath)) {
            settings.setValue(npath, settings.value(opath));
            settings.remove(opath);
            qInfo().noquote() << "Moved " << opath << " to " << npath;
          }
        }
      }

      oldpath = "NamedJSData/%1/JsButton/%2/%3";
      newpath = "NamedJSData/name%1/button%2/%3";
      keys = { "button_idx" };

      for (int i = 0; i < MAX_JS_BUTTONS; i++) {
        for (int k = 0; k < keys.size(); k++) {
          const QString opath = oldpath.arg(j).arg(i).arg(keys.at(k));
          const QString npath = newpath.arg(j).arg(i).arg(keys.at(k));

          if (settings.contains(opath)) {
            settings.setValue(npath, settings.value(opath));
            settings.remove(opath);
            qInfo().noquote() << "Moved " << opath << " to " << npath;
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
  qInfo().noquote() << "Tidy settings by removing redundant and default settings";

  foreach (const QString & key, settings.allKeys()) {
    if (key == ".")  // special Windows registry key, don't delete it
      continue;
    const QVariant newVal = settings.value(key);
    // Remove key if property does not exist or is the default value.
    if (!newVal.isValid() || !CompStoreObj::propertyPathIsValidNonDefault(key, newVal)) {
      settings.remove(key);
      qInfo().noquote() << "Removed key " << key;
    }
  }
}

bool AppData::findPreviousVersionSettings(QString * version) const
{
  // make sure we do not try to import from ourselves otherwise settings WILL get corrupted
  const int vmax = 30; //abitary maximum minor version
  int vmaj = VERSION_MINOR > 1 ? VERSION_MAJOR : VERSION_MAJOR - 1;
  int vmin = VERSION_MINOR > 1 ? VERSION_MINOR - 1 : vmax;

  // qDebug() << "Search start version:" << vmaj << vmin;

  for (;(vmaj << 8 | vmin) >= (2 << 8 | 4);) {  // 2.4 earliest EdgeTX version
    const QString ver = QString("%1.%2").arg(vmaj).arg(vmin);
    const QString prod = QString("Companion %1").arg(ver);
    // qDebug() << "Searching for previous version" << ver;
    QSettings settings(COMPANY, prod);
    if (settings.contains(SETTINGS_VERSION_KEY)) {
      *version = ver;
      return true;
    }
    else {
      settings.clear();
    }

    --vmin;
    if (vmin < 0) {
      vmin = vmax;
      --vmaj;
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
    qDebug() << key << newVal;
    // Skip export if property does not exist or is the default value.
    if (newVal.isValid() && CompStoreObj::propertyPathIsValidNonDefault(key, newVal))
      toSettings->setValue(key, newVal);
    else qDebug() << "SKIPPING:" << key << newVal;
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

ComponentData & AppData::getComponent(int index)
{
  if (index > -1 && index < MAX_COMPONENTS)
    return component[index];

  return component[0];
}

const ComponentData & AppData::getComponent(int index) const
{
  if (index > -1 && index < MAX_COMPONENTS)
    return component[index];

  return component[0];
}

void AppData::resetUpdatesSettings()
{
  updateCheckFreqReset();
  downloadDirReset();
  decompressDirReset();
  decompressDirUseDwnldReset();
  updateDirReset();
  updateDirUseSDReset();
  updDelDownloadsReset();
  updLogLevelReset();

  for (int i = 0; i < MAX_COMPONENTS; i++) {
    component[i].resetAll();

    for (int j = 0; j < MAX_COMPONENT_ASSETS; j++)
      component[i].asset[j].resetAll();
  }

  for (int i = 0; i < MAX_PROFILES; i++) {
    for (int j = 0; j < MAX_COMPONENTS; j++) {
      profile[i].compRelease[j].resetAll();
    }
  }
}

const QString AppData::settingsVersionToDisplay(const unsigned int ver)
{
  return QString("v%1.%2.%3.%4")
                .arg(ver >> 24)
                .arg((ver >> 16) & ((1U << 8) - 1))
                .arg((ver >> 8) & ((1U << 8) - 1))
                .arg(ver & ((1U << 8) - 1));
}
