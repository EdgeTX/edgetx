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

#include "updateinterface.h"
#include "repobuild.h"
#include "repogithub.h"
#include "helpers.h"
#include "minizinterface.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QRegularExpression>
#include <QValidator>
#include <QEventLoop>
#include <QTimer>

UpdateInterface::UpdateInterface(QWidget * parent, ComponentIdentity id, QString name, Repo::RepoType repoType,
                                 const QString & path, const QString & nightly, const int resultsPerPage) :
  QWidget(parent),
  m_id(id),
  m_name(name),
  m_params(new UpdateParameters(this)),
  m_status(new UpdateStatus(this)),
  m_network(new UpdateNetwork(this, m_status)),
  m_repo(repoType == Repo::REPO_TYPE_GITHUB ? static_cast<Repo*>(new RepoGitHub(this, m_status, m_network, path, nightly, resultsPerPage)) :
                                              static_cast<Repo*>(new RepoBuild(this, m_status, m_network, path, nightly, resultsPerPage)))
{
}

UpdateInterface::~UpdateInterface()
{
  delete m_params;
  delete m_repo;
  delete m_network;
  delete m_status;
}

void UpdateInterface::appSettingsInit()
{
  if (!isSettingsIndexValid()) {
    m_status->reportProgress(tr("Component id: %1 exceeds maximum application settings components: %2!").arg(m_id).arg(MAX_COMPONENTS), QtCriticalMsg);
    return;
  }

  if (!g.component[m_id].existsOnDisk()) {
    g.component[m_id].init();
  }

  if (!g.component[m_id].asset[0].existsOnDisk())
    assetSettingsInit();
}

void UpdateInterface::assetSettingsLoad()
{
  if (!isSettingsIndexValid())
    return;

  m_params->assets.clear();

  for (int i = 0; i < MAX_COMPONENT_ASSETS && g.component[m_id].asset[i].existsOnDisk(); i++) {
    UpdateParameters::AssetParams &ap = m_params->addAsset();
    ComponentAssetData &cad = g.component[m_id].asset[i];

    ap.processes = cad.processes();
    ap.flags = cad.flags();
    ap.filterType = (UpdateParameters::UpdateFilterType)cad.filterType();
    ap.filter = cad.filter();
    ap.maxExpected = cad.maxExpected();
    ap.destSubDir = cad.destSubDir();
    ap.copyFilterType = (UpdateParameters::UpdateFilterType)cad.copyFilterType();
    ap.copyFilter = cad.copyFilter();
  }
}

void UpdateInterface::assetSettingsSave()
{
  if (!isSettingsIndexValid())
    return;

  for (int i = 0; i < MAX_COMPONENT_ASSETS && g.component[m_id].asset[i].existsOnDisk(); i++) {
    const UpdateParameters::AssetParams &ap = m_params->assets.at(i);
    ComponentAssetData &cad = g.component[m_id].asset[i];

    //  DO NOT overwrite cad.processes
    cad.flags(ap.flags);
    cad.filterType(ap.filterType);
    cad.filter(ap.filter);
    cad.maxExpected(ap.maxExpected);
    cad.destSubDir(ap.destSubDir);
    cad.copyFilterType(ap.copyFilterType);
    cad.copyFilter(ap.copyFilter);
  }
}

int UpdateInterface::asyncInstall()
{
  return true;
}

int UpdateInterface::build()
{
  m_status->progressMessage(tr("Building assets"));

  if (!buildFlaggedAssets()) {
    m_status->reportProgress(tr("Unable to build flagged assets"), QtCriticalMsg);
    return false;
  }

  return true;
}

bool UpdateInterface::buildFlaggedAssets()
{
  m_repo->assets()->setFilterFlags(UPDFLG_Build);
  m_status->reportProgress(tr("Asset filter applied: %1 - %2 found").arg(updateFlagToString(UPDFLG_Build)).arg(m_repo->assets()->count()), QtDebugMsg);

  for (int i = 0; i < m_repo->assets()->count(); ++i) {
    if (!buildFlaggedAsset(i))
      return false;
  }

  return true;
}

bool UpdateInterface::buildFlaggedAsset(const int row)
{
  return m_repo->assets()->build(row);
}

bool UpdateInterface::checkCreateDirectory(const QString & dir, const UpdateFlags flag)
{
  if (m_params->flags & flag) {
    if (dir.isEmpty()) {
      m_status->reportProgress(tr("%1 directory not configured in application settings!").arg(updateFlagToString(flag)), QtCriticalMsg);
      return false;
    }
    else if (!QDir().mkpath(dir)) {
      m_status->reportProgress(tr("Failed to create %1 directory %2!").arg(updateFlagToString(flag)).arg(dir), QtCriticalMsg);
      return false;
    }
  }

  return true;
}

bool UpdateInterface::copyAsset()
{
  if (m_repo->assets()->flags() & UPDFLG_CopyStructure)
    return copyStructure();
  else if (m_repo->assets()->flags() & UPDFLG_CopyFiles)
    return copyFiles();

  return true;
}

bool UpdateInterface::copyFiles()
{
  //m_status->reportProgress(tr("Copy files"), QtDebugMsg);
  m_status->setValue(0);
  m_status->setMaximum(100);

  QString srcPath;

  if (m_repo->assets()->flags() & UPDFLG_Decompress)
    srcPath = QString("%1/A%2/%3").arg(m_decompressDir).arg(m_repo->assets()->id()).arg(QFileInfo(m_repo->assets()->name()).completeBaseName());
  else if (m_repo->assets()->flags() & UPDFLG_Download)
    srcPath = QString("%1/A%2").arg(m_downloadDir).arg(m_repo->assets()->id());
  else {
    m_status->reportProgress(tr("Unable to determine source directory for asset %1").arg(m_repo->assets()->name()), QtCriticalMsg);
    return false;
  }

  m_status->reportProgress(tr("Copy files from %1").arg(srcPath), QtDebugMsg);

  QString destPath = m_updateDir;

  if (!m_repo->assets()->subDirectory().isEmpty())
    destPath.append("/" % m_repo->assets()->subDirectory());

  if (!QDir().mkpath(destPath)) {
    m_status->reportProgress(tr("Failed to create directory %1").arg(QDir::toNativeSeparators(destPath)), QtCriticalMsg);
    return false;
  }

  m_status->reportProgress(tr("Check/create directory: %1").arg(destPath), QtDebugMsg);
  m_status->reportProgress(tr("Directories checked/created: %1").arg(1), QtDebugMsg);

  QRegularExpression filter(m_repo->assets()->copyFilter(), QRegularExpression::CaseInsensitiveOption);
  m_status->reportProgress(tr("Copy filter pattern: %1").arg(filter.pattern()), QtDebugMsg);

  QDirIterator itcnt(srcPath, QDir::Files, QDirIterator::Subdirectories);

  int cnt = 0;

  //m_status->reportProgress(tr("Calculating number of files"), QtDebugMsg);

  while (itcnt.hasNext())
  {
    QFileInfo src(itcnt.next());

    if ((!filter.pattern().isEmpty()) && (!src.fileName().contains(filter)))
      continue;

    cnt++;
  }

  m_status->setMaximum(cnt);

  QDirIterator itfiles(srcPath, QDir::Files, QDirIterator::Subdirectories);

  cnt = 0;

  while (itfiles.hasNext())
  {
    QFileInfo src(itfiles.next());

    if ((!filter.pattern().isEmpty()) && (!src.fileName().contains(filter)))
      continue;

    QFile dest(destPath % "/" % src.fileName());

    if (dest.exists()) {
      if (!dest.remove()) {
        m_status->reportProgress(tr("Failed to delete existing file %1").arg(dest.fileName()), QtCriticalMsg);
        return false;
      }
      m_status->reportProgress(tr("Deleted file: %1").arg(dest.fileName()), QtDebugMsg);
    }

    if (!QFile::copy(src.absoluteFilePath(), dest.fileName())) {
      m_status->reportProgress(tr("Failed to copy file %1").arg(src.absoluteFilePath()), QtCriticalMsg);
      return false;
    }

    m_status->reportProgress(tr("Copied %1 to %2").arg(src.fileName()).arg(dest.fileName()), QtDebugMsg);
    cnt++;
    m_status->setValue(cnt);
  }

  if (!cnt) {
    m_status->reportProgress(tr("No downloaded or decompressed files matched filter '%1'").arg(m_repo->assets()->copyFilter()), QtCriticalMsg);
    return false;
  }

  m_status->setValue(m_status->maximum());
  m_status->reportProgress(tr("Files copied: %1").arg(cnt), QtInfoMsg);

  return true;
}

bool UpdateInterface::copyFlaggedAssets()
{
  m_repo->assets()->setFilterFlags(UPDFLG_CopyDest);
  m_status->reportProgress(tr("Asset filter applied: %1 - %2 found").arg(updateFlagToString(UPDFLG_CopyDest)).arg(m_repo->assets()->count()), QtDebugMsg);

  for (int i = 0; i < m_repo->assets()->count(); ++i) {
    m_repo->assets()->getSetId(i);
    if (!copyAsset())
      return false;
  }

  return true;
}

bool UpdateInterface::copyStructure()
{
  m_status->reportProgress(tr("Copy directory structure"), QtDebugMsg);
  m_status->setValue(0);
  m_status->setMaximum(100);

  QDir srcPath(QString("%1/A%2/%3").arg(m_decompressDir).arg(m_repo->assets()->id()).arg(QFileInfo(m_repo->assets()->name()).completeBaseName()));

  if (!srcPath.exists()) {
    m_status->reportProgress(tr("Decompressed file structure not found at %1").arg(QDir::toNativeSeparators(srcPath.path())), QtCriticalMsg);
    return false;
  }

  m_status->reportProgress(tr("Copy file structure from: %1").arg(QDir::toNativeSeparators(srcPath.path())), QtDebugMsg);

  QString baseDir = m_updateDir;

  if (!m_repo->assets()->subDirectory().isEmpty())
    baseDir.append("/" % m_repo->assets()->subDirectory());

  //  create directory structure first as there is no order to QDirIterator if both files and directories selected
  //  also supports creation of empty directories
  QDirIterator itdirs(srcPath.path(), QDir::AllDirs | QDir::NoDot | QDir::NoDotDot, QDirIterator::Subdirectories);

  int cnt = 0;

  while (itdirs.hasNext())
  {
    QFileInfo src(itdirs.next());

    QString subPath(src.absoluteFilePath());
    subPath.replace(srcPath.path(), "");

    QString dir = baseDir % subPath;

    if (!QDir().mkpath(dir)) {
      m_status->reportProgress(tr("Failed to create directory %1").arg(dir), QtCriticalMsg);
      return false;
    }

    m_status->reportProgress(tr("Check/create directory: %1").arg(dir), QtDebugMsg);
    cnt++;
  }

  m_status->reportProgress(tr("Directories checked/created: %1").arg(cnt), QtInfoMsg);

  QRegularExpression filter(m_repo->assets()->copyFilter(), QRegularExpression::CaseInsensitiveOption);
  //m_status->reportProgress(tr("Copy filter pattern: %1").arg(filter.pattern()), QtDebugMsg);

  QDirIterator itcnt(srcPath.path(), QDir::Files, QDirIterator::Subdirectories);

  cnt = 0;

  //m_status->reportProgress(tr("Calculating number of files"), QtDebugMsg);

  while (itcnt.hasNext())
  {
    QFileInfo src(itcnt.next());

    if ((!filter.pattern().isEmpty()) && (!src.fileName().contains(filter)))
      continue;

    cnt++;
  }

  m_status->setMaximum(cnt);

  QDirIterator itfiles(srcPath.path(), QDir::Files, QDirIterator::Subdirectories);

  cnt = 0;

  while (itfiles.hasNext())
  {
    QFileInfo src(itfiles.next());

    if ((!filter.pattern().isEmpty()) && (!src.fileName().contains(filter)))
      continue;

    QString subPath(src.absoluteFilePath());
    subPath.replace(srcPath.path(), "");

    QFile dest(baseDir % subPath);

    if (dest.exists()) {
      if (!dest.remove()) {
        m_status->reportProgress(tr("Failed to delete existing file %1").arg(dest.fileName()), QtCriticalMsg);
        return false;
      }
    }

    if (!QFile::copy(src.absoluteFilePath(), dest.fileName())) {
      m_status->reportProgress(tr("Failed to copy file %1").arg(src.absoluteFilePath()), QtCriticalMsg);
      return false;
    }

    m_status->reportProgress(tr("Copied %1 to %2").arg(src.fileName()).arg(dest.fileName()), QtDebugMsg);
    cnt++;
    m_status->setValue(cnt);
  }

  m_status->setValue(m_status->maximum());
  m_status->reportProgress(tr("Files copied: %1").arg(cnt), QtInfoMsg);

  return true;
}

int UpdateInterface::copyToDestination()
{
  m_status->progressMessage(tr("Copying to destination"));

  if (!copyFlaggedAssets()) {
    m_status->reportProgress(tr("Unable to copy assets"), QtDebugMsg);
    return false;
  }

  return true;
}

int UpdateInterface::decompress()
{
  m_status->progressMessage(tr("Decompressing assets"));

  if (!decompressFlaggedAssets()) {
    m_status->reportProgress(tr("Unable to decompress flagged assets"), QtDebugMsg);
    return false;
  }

  return true;
}

const QString UpdateInterface::decompressDir() const
{
  return m_decompressDir;
}

bool UpdateInterface::decompressFlaggedAssets()
{
  m_repo->assets()->setFilterFlags(UPDFLG_Decompress);
  m_status->reportProgress(tr("Asset filter applied: %1 - %2 found").arg(updateFlagToString(UPDFLG_Decompress)).arg(m_repo->assets()->count()), QtDebugMsg);

  for (int i = 0; i < m_repo->assets()->count(); ++i) {
    if (!decompressAsset(i))
      return false;
  }

  return true;
}

bool UpdateInterface::decompressAsset(int row)
{
  m_repo->assets()->getSetId(row);

  if (!(m_repo->assets()->flags() & UPDFLG_Decompress))
    return true;

  QString filename = m_repo->assets()->name();

  if (filename.isEmpty())
    return false;

  QFileInfo f(QString("%1/A%2/%3").arg(m_downloadDir).arg(m_repo->assets()->id()).arg(filename));

  if (!f.exists())
    return false;

  m_status->progressMessage(tr("Decompressing %1").arg(filename));
  m_status->reportProgress(tr("Decompress: %1").arg(filename), QtInfoMsg);

  return decompressArchive(f.absoluteFilePath(), QString("%1/A%2").arg(m_decompressDir).arg(m_repo->assets()->id()));
}

bool UpdateInterface::decompressArchive(const QString & archivePath, const QString & destDir)
{
  QString dest = destDir;

  if (!dest.endsWith("/"))
    dest.append("/");

  dest.append(QFileInfo(archivePath).completeBaseName());

  MinizInterface mIface(m_status->progress(), MinizInterface::PCM_SIZE, m_params->logLevel);

  if (!mIface.unzipArchiveToPath(archivePath, dest)) {
    m_status->reportProgress(tr("Failed to decompress %1").arg(archivePath), QtCriticalMsg);
    return false;
  }

  return true;
}

const QString UpdateInterface::downloadDir() const
{
  return m_downloadDir;
}

bool UpdateInterface::downloadFlaggedAssets()
{
  m_repo->assets()->setFilterFlags(UPDFLG_Download);
  m_status->reportProgress(tr("Asset filter applied: %1 - %2 found").arg(updateFlagToString(UPDFLG_Download)).arg(m_repo->assets()->count()), QtDebugMsg);

  for (int i = 0; i < m_repo->assets()->count(); ++i) {
    if (!downloadFlaggedAsset(i))
      return false;
  }

  return true;
}

bool UpdateInterface::downloadFlaggedAsset(const int row)
{
  return m_repo->assets()->downloadToFile(row, m_downloadDir);
}

int UpdateInterface::download()
{
  m_status->progressMessage(tr("Downloading assets"));

  if (!downloadFlaggedAssets()) {
    m_status->reportProgress(tr("Unable to download flagged assets"), QtDebugMsg);
    return false;
  }

  return true;
}

bool UpdateInterface::filterAssets(const UpdateParameters::AssetParams & ap)
{
  QString pattern(m_params->buildFilterPattern(ap.filterType, ap.filter));
  m_repo->assets()->setFilterPattern(pattern);
  m_status->reportProgress(tr("Asset filter applied: %1 - %2 found").arg(pattern).arg(m_repo->assets()->count()), QtDebugMsg);

  if (m_repo->assets()->count() < 1) {
    m_status->reportProgress(tr("No assets found in release '%1' using filter '%2'").arg(m_repo->releases()->name()).arg(pattern), QtCriticalMsg);
    return false;
  }
  else if (ap.maxExpected > 0 && m_repo->assets()->count() > ap.maxExpected) {
    m_status->reportProgress(tr("%1 assets found when %2 expected in release '%3' using filter '%4'")
                        .arg(m_repo->assets()->count() + 1).arg(ap.maxExpected).arg(m_repo->releases()->name().arg(pattern)), QtCriticalMsg);
    return false;
  }

  return true;
}

bool UpdateInterface::flagAssets()
{
  m_status->progressMessage(tr("Flagging assets"));

  for (int i = 0; i < m_params->assets.size(); i++) {
    const UpdateParameters::AssetParams & ap = m_params->assets.at(i);
    if (!setFilteredAssets(ap))
      return false;
  }

  return true;
}

int UpdateInterface::housekeeping()
{
  m_status->progressMessage(tr("Housekeeping"));
  int cnt = 0;

  m_status->setValue(cnt);
  m_status->setMaximum(2);

  m_status->setValue(++cnt);

  g.lastUpdateDir(m_params->updateDir);  //  used by sdsync

  if (m_params->flags & UPDFLG_DelDownloads) {
    m_status->reportProgress(tr("Delete download directory: %1").arg(m_downloadDir), QtDebugMsg);
    QDir d(m_downloadDir);
    if (!d.removeRecursively())
      m_status->reportProgress(tr("Failed to delete downloads folder %1").arg(m_downloadDir), QtCriticalMsg);
  }

  m_status->setValue(++cnt);

  if (m_params->flags & UPDFLG_DelDecompress) {
    if (!m_params->decompressDirUseDwnld) {
      m_status->reportProgress(tr("Delete decompress directory: %1").arg(m_decompressDir), QtDebugMsg);
      QDir d(m_decompressDir);
      if (!d.removeRecursively())
        m_status->reportProgress(tr("Failed to delete decompress folder %1").arg(m_decompressDir), QtCriticalMsg);
    }
    else if (!(m_params->flags & UPDFLG_DelDownloads)) {
      m_repo->assets()->setFilterFlags(UPDFLG_Decompress);
      for (int i = 0; i < m_repo->assets()->count(); i++) {
        m_repo->assets()->getSetId(i);
        QString path = QString("%1/A%2/%3").arg(m_decompressDir).arg(m_repo->assets()->id()).arg(QFileInfo(m_repo->assets()->name()).completeBaseName());
        m_status->reportProgress(tr("Delete decompress folder: %1").arg(path), QtDebugMsg);
        if (!QDir(path).removeRecursively())
          m_status->reportProgress(tr("Failed to delete decompress folder %1").arg(path), QtCriticalMsg);
      }
    }
  }

  m_status->setValue(++cnt);

  return true;
}

const int UpdateInterface::id() const
{
  return (int)m_id;
}

void UpdateInterface::init()
{
  appSettingsInit();
  releaseCurrent();
}

const bool UpdateInterface::isReleaseLatest()
{
  if (!m_repo->releases()->retrieveMetaDataAll())
    return false;

  // release metadata and variables are cached so ensure referencing latest release for channel
  m_repo->releases()->getSetId();

  // nightlies often have the same version so also check id
  if (isVersionLatest(versionCurrent(), m_repo->releases()->version()) &&
      g.component[m_id].releaseId() == m_repo->releases()->id())
    return true;

  return false;
}

const bool UpdateInterface::isSettingsIndexValid() const
{
  return (int)m_id > -1 && (int)m_id < MAX_COMPONENTS;
}

const bool UpdateInterface::isUpdateAvailable()
{
  if (g.component[m_id].checkForUpdate())
    return !isReleaseLatest();
  else {
    return false;
  }
}

const bool UpdateInterface::isVersionLatest(const QString & installed, const QString & latest)
{
  QStringList a = versionToStringList(installed);
  QStringList b = versionToStringList(latest);

  // not equal comparision used as version identifiers are inconsistent eg release vs nightly vs older
  for (int i = 0; i < 4; i++) {
    if (a.at(i) != b.at(i)) {
      return false;
    }
  }

  return true;
}

const bool UpdateInterface::isUpdateable() const
{
  if (!isSettingsIndexValid())
    return false;

  return g.component[m_id].checkForUpdate();
}

const QString UpdateInterface::releaseLatest()
{
  if (!m_repo->releases()->retrieveMetaDataAll())
    return tr("unknown");

  return m_repo->releases()->name();
}

const QString UpdateInterface::name() const
{
  return m_name;
}

UpdateNetwork* const UpdateInterface::network() const
{
  return m_network;
}

void UpdateInterface::onStatusCancelled()
{
  m_status->setMaximum(0);
  m_stopping = true;
  emit stop();
  m_status->reportProgress(tr("Update cancelled"), QtWarningMsg);
}

UpdateParameters* const UpdateInterface::params() const
{
  return m_params;
}

int UpdateInterface::preparation()
{
  m_status->progressMessage(tr("Preparing"));
  int cnt = 0;

  m_status->setValue(cnt);
  m_status->setMaximum(5);

  if (!setRunFolders()) {
    m_status->reportProgress(tr("Set run folders failed"), QtDebugMsg);
    return false;
  }

  m_status->setValue(++cnt);

  if (!m_repo->releases()->retrieveMetaDataAll()) {
    return false;
  }

  m_status->setValue(++cnt);

  if (!m_repo->releases()->getSetId(m_params->releaseUpdate)) {
    m_status->reportProgress(tr("Set release id from update release '%1' failed").arg(m_params->releaseUpdate), QtCriticalMsg);
    return false;
  }

  m_status->setValue(++cnt);

  if (!m_repo->assets()->retrieveMetaDataAll())
    return false;

  m_status->setValue(++cnt);

  if (!flagAssets())
    return false;

  m_status->setValue(++cnt);

  return true;
}

void UpdateInterface::radioProfileChanged()
{
  resetEnvironment();
  m_repo->releases()->invalidate();
}

void UpdateInterface::releaseClear()
{
  g.component[m_id].releaseClear();
  releaseCurrent();
}

const QString UpdateInterface::releaseCurrent()
{
  m_params->releaseCurrent = g.component[m_id].release();
  return m_params->releaseCurrent;
}

const QStringList UpdateInterface::releaseList()
{
  if (!m_repo->releases()->retrieveMetaDataAll())
    return QStringList();

  return m_repo->releases()->list();
}

int UpdateInterface::releaseSettingsSave()
{
  m_status->reportProgress(tr("Save release settings"), QtDebugMsg);
  g.component[m_id].release(m_repo->releases()->name());
  g.component[m_id].version(m_repo->releases()->version());
  g.component[m_id].releaseId(m_repo->releases()->id());
  g.component[m_id].date(m_repo->releases()->date());

  return true;
}

const QString UpdateInterface::releaseUpdate()
{
  if (m_params->releaseUpdate.isEmpty())
    m_params->releaseUpdate = releaseLatest();

  return m_params->releaseUpdate;
}

Repo* const UpdateInterface::repo() const
{
  return m_repo;
}

void UpdateInterface::resetEnvironment()
{
  //  reset from previous run if any
  m_params->flags &= ~UPDFLG_Update;
  m_params->logLevel = g.updLogLevel();

  m_status->setProgress(nullptr);
  m_status->setLogLevel(m_params->logLevel);

  setReleaseChannel(g.component[m_id].releaseChannel());
  m_params->releaseUpdate = "";
  setFirmwareFlavour();
  setLanguage();
  assetSettingsLoad();
  setParamFolders();

  if (g.updDelDownloads())
    m_params->flags |= UpdateInterface::UPDFLG_DelDownloads;
  else
    m_params->flags &= ~UpdateInterface::UPDFLG_DelDownloads;

  if (g.updDelDecompress())
    m_params->flags |= UpdateInterface::UPDFLG_DelDecompress;
  else
    m_params->flags &= ~UpdateInterface::UPDFLG_DelDecompress;
}

bool UpdateInterface::retrieveAssetsJsonFile(const QString & assetName, QJsonDocument * json)
{
  m_repo->assets()->setFilterPattern(assetName);
  return m_repo->assets()->getJson(assetName, json);
}

bool UpdateInterface::retrieveRepoJsonFile(const QString & filename, QJsonDocument * json)
{
  return m_repo->getJson(filename, json);
}

void UpdateInterface::runAsyncInstall()
{
  m_result = asyncInstall();
  if (m_result == PROC_RESULT_FAIL) m_status->reportProgress(tr("%1 start async failed").arg(m_name), QtCriticalMsg);
  emit finished();
}

void UpdateInterface::runBuild()
{
  m_result = build();
  if (m_result == PROC_RESULT_FAIL) m_status->reportProgress(tr("%1 preparation failed").arg(m_name), QtCriticalMsg);
  emit finished();
}

void UpdateInterface::runCopyToDestination()
{
  m_result = copyToDestination();
  if (m_result == PROC_RESULT_FAIL) m_status->reportProgress(tr("%1 copy to destination failed").arg(m_name), QtCriticalMsg);
  emit finished();
}

void UpdateInterface::runDecompress()
{
  m_result = decompress();
  if (m_result == PROC_RESULT_FAIL) m_status->reportProgress(tr("%1 decompress failed").arg(m_name), QtCriticalMsg);
  emit finished();
}

void UpdateInterface::runDownload()
{
  m_result = download();
  if (m_result == PROC_RESULT_FAIL) m_status->reportProgress(tr("%1 download failed").arg(m_name), QtCriticalMsg);
  emit finished();
}

void UpdateInterface::runHousekeeping()
{
  m_result = housekeeping();
  if (m_result == PROC_RESULT_FAIL) m_status->reportProgress(tr("%1 housekeeping failed").arg(m_name), QtCriticalMsg);
  emit finished();
}

void UpdateInterface::runPreparation()
{
  m_result = preparation();
  if (m_result == PROC_RESULT_FAIL) m_status->reportProgress(tr("%1 preparation failed").arg(m_name), QtCriticalMsg);
  emit finished();
}

void UpdateInterface::runReleaseSettingsSave()
{
  m_result = releaseSettingsSave();
  if (m_result == PROC_RESULT_FAIL) m_status->reportProgress(tr("%1 save release settings failed").arg(m_name), QtCriticalMsg);
  emit finished();
}

bool UpdateInterface::setFilteredAssets(const UpdateParameters::AssetParams & ap)
{
  if (!filterAssets(ap))
    return false;

  for (int i = 0; i < m_repo->assets()->count(); i++) {
    m_repo->assets()->getSetId(i);
    if (!m_repo->assets()->setFlags(ap.flags)) {
      m_status->reportProgress(tr("Unable to set processing flags for asset %1").arg(m_repo->assets()->name()), QtCriticalMsg);
      return false;
    }

    if (!m_repo->assets()->setSubDirectory(ap.destSubDir)) {
      m_status->reportProgress(tr("Unable to set sub directory for asset %1").arg(m_repo->assets()->name()), QtCriticalMsg);
      return false;
    }

    if (!m_repo->assets()->setCopyFilter(m_params->buildFilterPattern(ap.copyFilterType, ap.copyFilter))) {
      m_status->reportProgress(tr("Unable to set copy filter for asset %1").arg(m_repo->assets()->name()), QtCriticalMsg);
      return false;
    }
  }

  return true;
}

void UpdateInterface::setFirmwareFlavour()
{
  m_params->fwFlavour = getCurrentFirmware()->getDownloadId();
}

void UpdateInterface::setLanguage()
{
  const Firmware * baseFw = getCurrentFirmware()->getFirmwareBase();
  const QStringList currVariant = getCurrentFirmware()->getId().split('-');

  m_params->language = "";

  for (const char *lang : baseFw->languageList()) {
    if (currVariant.last() == lang) {
      m_params->language = currVariant.last();
      break;
    }
  }
}

void UpdateInterface::setParamFolders()
{
  if (g.downloadDir().trimmed().isEmpty())
    g.downloadDirReset(true);

  m_params->downloadDir = g.downloadDir().trimmed();

  m_params->decompressDirUseDwnld = g.decompressDirUseDwnld();

  if (m_params->decompressDirUseDwnld)
    m_params->decompressDir = m_params->downloadDir;
  else {
    if (g.decompressDir().trimmed().isEmpty())
      g.decompressDirReset(true);

    m_params->decompressDir = g.decompressDir().trimmed();
  }

  if (g.currentProfile().sdPath().trimmed().isEmpty())
    m_params->updateDirUseSD = false;
  else
    m_params->updateDirUseSD = g.updateDirUseSD();

  if (m_params->updateDirUseSD) {
    m_params->sdDir = g.currentProfile().sdPath().trimmed();
    m_params->updateDir = m_params->sdDir;
  }
  else {
    if (g.updateDir().trimmed().isEmpty())
      g.updateDirReset(true);

    m_params->updateDir = g.updateDir().trimmed();
  }
}

void UpdateInterface::setReleaseChannel(const int channel)
{
  m_params->releaseChannel = channel;
  m_repo->releases()->setChannel(channel);
}

void UpdateInterface::setReleaseId(QString name)
{
  m_repo->releases()->getSetId(name);
}

bool UpdateInterface::setRunFolders()
{
  QRegularExpression rx("[0-9a-zA-Z_\\-.]+");
  // the validator treats the regexp as "^[0-9a-zA-Z_\-.]+$"
  QRegularExpressionValidator v(rx);
  int pos = 0;

  QString fldr(m_repo->releases()->name().trimmed());

  if (v.validate(fldr, pos) != QValidator::Acceptable) {
    fldr.replace("\"", "");
    fldr.replace("'", "");
    fldr.replace("(", "_");
    fldr.replace(")", "_");
    fldr.replace("[", "_");
    fldr.replace("]", "_");
    fldr.replace(" ", "_");
    fldr.replace("__", "_");

    if (v.validate(fldr, pos) != QValidator::Acceptable) {
      //m_status->reportProgress(tr("Unable to use release name %1 for directory name using default %2")
      //               .arg(m_repo->releases()->name().trimmed()).arg(QString("R%1").arg(m_repo->releases()->id())), QtDebugMsg);
      fldr = QString("R%1").arg(m_repo->releases()->id());
    }
  }

  m_downloadDir = QString("%1/%2/%3").arg(m_params->downloadDir).arg(m_name).arg(fldr);

  if (!checkCreateDirectory(m_downloadDir, UPDFLG_Download))
    return false;

  //m_status->reportProgress(tr("Download directory: %1").arg(m_downloadDir), QtDebugMsg);

  m_decompressDir = QString("%1/%2/%3").arg(m_params->decompressDir).arg(m_name).arg(fldr);

  if (!checkCreateDirectory(m_decompressDir, UPDFLG_Decompress))
    return false;

  //m_status->reportProgress(tr("Decompress directory: %1").arg(m_decompressDir), QtDebugMsg);

  m_updateDir = m_params->updateDir;

  if (!checkCreateDirectory(m_updateDir, UPDFLG_CopyDest))
    return false;

  //m_status->reportProgress(tr("Update directory: %1").arg(m_updateDir), QtDebugMsg);

  return true;
}

void UpdateInterface::setRunUpdate()
{
  m_params->flags |= UPDFLG_Update;
}

UpdateStatus* const UpdateInterface::status() const
{
  return m_status;
}

bool UpdateInterface::update(ProgressWidget * progress)
{
  if (!(m_params->flags & UPDFLG_Update))
    return true;

  m_result = PROC_RESULT_SUCCESS;
  m_stopping = false;
  m_status->setProgress(progress);
  m_status->setLogLevel(m_params->logLevel);
  m_status->setInfo(tr("Processing updates for: %1").arg(m_name));
  m_status->setValue(0);
  m_status->setMaximum(100);

  // the default behaviour of the dialog is to close on clicking the cancel button
  // but need to keep it open to display the current process until it can be interrupted
  // and then display close button
  status()->keepOpen(true);

  connect(m_status, &UpdateStatus::cancelled, this, &UpdateInterface::onStatusCancelled);

  // need to create a separate thread for the processes so that the cancel request can be handled asap
  QEventLoop loop;

  // EXTREMELY IMPORTANT: every process within the event loop MUST emit finished() otherwise the loop will not exit
  // Note: finished does not indicate the exit status
  connect(this, &UpdateInterface::finished, [&]() { if (loop.isRunning()) loop.quit(); });

  if (okToRun()) {
    // pause before starting next process to allow time for queued events to be processed ie like the cancel request
    QTimer::singleShot(100, this, &UpdateInterface::runPreparation);
    // NOTE: this will not exit until the finished signal is detected and processed
    loop.exec();
  }

  if (okToRun()) {
    QTimer::singleShot(100, this, &UpdateInterface::runBuild);
    loop.exec();
  }

  if (okToRun()) {
    QTimer::singleShot(100, this, &UpdateInterface::runDownload);
    loop.exec();
  }

  if (okToRun()) {
    QTimer::singleShot(100, this, &UpdateInterface::runDecompress);
    loop.exec();
  }

  if (okToRun()) {
    QTimer::singleShot(100, this, &UpdateInterface::runCopyToDestination);
    loop.exec();
  }

  //  perform before async install in case Companion restarted
  if (okToRun()) {
    QTimer::singleShot(100, this, &UpdateInterface::runReleaseSettingsSave);
    loop.exec();
  }

  if (okToRun()) {
    QTimer::singleShot(100, this, &UpdateInterface::runAsyncInstall);
    loop.exec();
  }

  if (okToRun()) {
    QTimer::singleShot(100, this, &UpdateInterface::runHousekeeping);
    loop.exec();
  }

  disconnect(m_status, &UpdateStatus::cancelled, this, &UpdateInterface::onStatusCancelled);

  // allow the dialog to be closed by clicking the close button
  status()->keepOpen(false);

  if (!okToRun())
    return false;

  m_status->reportProgress(tr("%1 update successful").arg(m_name), QtInfoMsg);

  if (!m_status->progress())
    QMessageBox::information(m_status->progress(), CPN_STR_APP_NAME, tr("%1 update successful").arg(m_name));

  return true;
}

const QString UpdateInterface::updateDir() const
{
  return m_updateDir;
}

//  static
const QString UpdateInterface::updateFlagsToString(const int flags)
{
  QString str;

  for (int i = 0; i < 32; i++) {
    int flag = flags & (1 << i);
    if (flag) {
      if (!str.isEmpty())
        str.append(";");
      str.append(updateFlagToString(flag));
    }
  }

  return str;
}

//  static
const QString UpdateInterface::updateFlagToString(const int flag)
{
  switch (flag) {
    case UPDFLG_None:
      return "None";
    case UPDFLG_Update:
      return "Update";
    case UPDFLG_Locked:
      return "Locked";
    case UPDFLG_Preparation:
      return "Preparation";
    case UPDFLG_Build:
      return "Build";
    case UPDFLG_Download:
      return "Download";
    case UPDFLG_Decompress:
      return "Decompress";
    case UPDFLG_CopyDest:
      return "Copy to Destination";
    case UPDFLG_CopyFiles:
      return "Copy Files";
    case UPDFLG_CopyStructure:
      return "Copy Structure";
    case UPDFLG_Housekeeping:
      return "Housekeeping";
    case UPDFLG_AsyncInstall:
      return "Async Install";
    case UPDFLG_DelDownloads:
      return "Delete Downloads";
    case UPDFLG_DelDecompress:
      return "Delete Decompress";
    default:
      return CPN_STR_UNKNOWN_ITEM;
  }
}

const QString UpdateInterface::versionCurrent()
{
  return g.component[m_id].version();
}

// static
QStringList UpdateInterface::versionToStringList(QString version)
{
  QStringList strl = version.split(".");

  while (strl.count() < 4) {
    strl.append("0");
  }

  return strl;
}
