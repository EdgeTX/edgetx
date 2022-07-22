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
#include "appdata.h"
#include "minizinterface.h"
#include "helpers.h"
#include "updatefirmware.h"
#include "updatecompanion.h"
#include "updatesdcard.h"
#include "updatesounds.h"
#include "updatethemes.h"
#include "updatemultiprotocol.h"

#include <QMessageBox>
#include <QtNetwork/QNetworkProxyFactory>
#include <QJsonDocument>
#include <QFileDialog>
#include <QFileInfo>
#include <QRegularExpression>

/*

  UpdateParameters

*/

UpdateParameters::UpdateParameters(QWidget * parent) :
  QWidget(parent)
{
  data.flags = 0;
  data.decompressDirUseDwnld = true;
  data.updateDirUseSD =  true;
}

UpdateParameters& UpdateParameters::operator=(const UpdateParameters& source)
{
  // Perform a deep copy

  // check for self-assignment


  if (this != &source) {
    data.flags = source.data.flags;
    data.currentRelease = source.data.currentRelease;
    data.updateRelease = source.data.updateRelease;
    data.downloadDir = source.data.downloadDir;
    data.decompressDirUseDwnld = source.data.decompressDirUseDwnld;
    data.decompressDir = source.data.decompressDir;
    data.updateDirUseSD = source.data.updateDirUseSD;
    data.updateDir = source.data.updateDir;

    while (!data.assets.isEmpty()) {
      data.assets.removeLast();
    }

    for (int i = 0; i < source.data.assets.size(); i++) {
      data.assets.append(source.data.assets[i]);
    }
  }

  return *this;
}

UpdateParameters::AssetParams & UpdateParameters::addAsset()
{
  AssetParams ap;
  ap.flags = 0;
  ap.filterType = UFT_None;
  ap.maxExpected = -1;
  ap.copyFilterType = UFT_None;

  data.assets.append(ap);

  return data.assets[data.assets.size() - 1];
}

//  static
QString UpdateParameters::buildFilterPattern(const UpdateParameters::UpdateFilterType filterType, const QString & filter)
{
  QString pattern = filter;

  switch ((int)filterType) {
    case UpdateParameters::UFT_Exact:
      pattern.prepend("^");
      pattern.append("$");
      break;
    case UpdateParameters::UFT_Startswith:
      pattern.prepend("^");
      break;
    case UpdateParameters::UFT_Endswith:
      pattern.append("$");
      break;
    case UpdateParameters::UFT_Contains:
    case UpdateParameters::UFT_Expression:
      break;
    default:
      pattern = "";
  }

  return pattern;
}

//  static
QStringList UpdateParameters::updateFilterTypeList()
{
  return { tr("None"), tr("Exact"), tr("Startswith"), tr("Endswith"), tr("Contains"), tr("Expression") };
}

//  static
QString UpdateParameters::updateFilterTypeToString(UpdateFilterType uft)
{
  return updateFilterTypeList().value(uft, CPN_STR_UNKNOWN_ITEM);
}

/*

  UpdateInterface

*/

UpdateInterface::UpdateInterface(QWidget * parent) :
  QWidget(parent),
  reply(nullptr),
  buffer(new QByteArray()),
  file(nullptr)
{
  progress = nullptr;
  settingsIdx = -1;

  QNetworkProxyFactory::setUseSystemConfiguration(true);

  releases = new ReleasesMetaData(this);
  assets = new AssetsMetaData(this);
  dfltParams = new UpdateParameters(this);
  runParams = new UpdateParameters(this);

  getRadioProfileSettings();

  setReleasesNightlyName("");
  dfltParams->data.flags = UPDFLG_Common;
  resultsPerPage = -1;

  initParamFolders(dfltParams);
}

UpdateInterface::~UpdateInterface()
{
  if (reply)
    delete reply;
  delete buffer;
  if (file)
    delete file;
  delete releases;
  delete assets;
  delete dfltParams;
  delete runParams;
}

bool UpdateInterface::autoUpdate(ProgressWidget * progress)
{
  this->progress = progress;

  if (dfltParams->data.flags & UPDFLG_Update && !isLatestRelease())
    return update();

  return true;
}

bool UpdateInterface::manualUpdate(ProgressWidget * progress)
{
  this->progress = progress;

  if (runParams->data.flags & UPDFLG_Update)
    return update();

  return true;
}

bool UpdateInterface::update()
{
  if (progress) {
    progress->setInfo(tr("Processing updates for: %1").arg(name));
    progress->setValue(0);
    progress->setMaximum(100);
  }

  reportProgress(tr("Processing updates for: %1").arg(name), QtInfoMsg);

  if ((runParams->data.flags & UPDFLG_Preparation) && !preparation()) {
    reportProgress(tr("%1 step preparation failed").arg(name), QtCriticalMsg);
    return false;
  }

  if ((runParams->data.flags & UPDFLG_Download) && !download()) {
    reportProgress(tr("%1 step download failed").arg(name), QtCriticalMsg);
    return false;
  }

  if ((runParams->data.flags & UPDFLG_Decompress) && !decompress()) {
    reportProgress(tr("%1 step decompress failed").arg(name), QtCriticalMsg);
    return false;
  }

  if ((runParams->data.flags & UPDFLG_CopyDest) && !copyToDestination()) {
    reportProgress(tr("%1 step copy to destination failed").arg(name), QtCriticalMsg);
    return false;
  }

  if ((runParams->data.flags & UPDFLG_Housekeeping) && !housekeeping()) {
    reportProgress(tr("%1 step housekeeping failed").arg(name), QtCriticalMsg);
    return false;
  }

  if ((runParams->data.flags & UPDFLG_AsyncInstall) && !asyncInstall()) {
    reportProgress(tr("%1 step start async failed").arg(name), QtCriticalMsg);
    return false;
  }

  reportProgress(tr("%1 update successful").arg(name), QtInfoMsg);

  QMessageBox::information(this, CPN_STR_APP_NAME, tr("%1 update successful").arg(name));

  return true;
}

//  static
QString UpdateInterface::updateFlagsToString(UpdateFlags val)
{
  switch ((int)val) {
    case UPDFLG_None:
      return "None";
    case UPDFLG_Update:
      return "Update";
    case UPDFLG_Locked:
      return "Locked";
    case UPDFLG_Preparation:
      return "Preparation";
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
    default:
      return CPN_STR_UNKNOWN_ITEM;
  }
}

//  static
QString UpdateInterface::downloadDataTypeToString(DownloadDataType val)
{
  switch ((int)val) {
    case DDT_Binary:
      return "Binary";
    case DDT_Content:
      return "Content";
    case DDT_MetaData:
      return "Meta Data";
    default:
      return CPN_STR_UNKNOWN_ITEM;
  }
}

//  static
QString UpdateInterface::semanticVersion(QString version)
{
  QStringList strl = version.split(".");

  while (strl.count() < 4) {
    strl.append("0");
  }

  return strl.join(".");
}

void UpdateInterface::reportProgress(const QString & text, const int type)
{
  if (logLevel == QtDebugMsg || (logLevel == QtInfoMsg && type > QtDebugMsg) || (type < QtInfoMsg && type >= logLevel)) {
    if (progress)
      progress->addMessage(text, type);
    else
      qDebug() << text;
  }

  if (type == QtCriticalMsg || type == QtFatalMsg)
    criticalMsg(text);
}

void UpdateInterface::progressMessage(const QString & text)
{
  if (progress)
    progress->setInfo(text);
  else
    qDebug() << text;
}

void UpdateInterface::criticalMsg(const QString & msg)
{
  QMessageBox::critical(nullptr, CPN_STR_APP_NAME % ": " % tr("Update Interface"), msg);
}

void UpdateInterface::setName(QString name)
{
  this->name = name;
  setSettingsIdx();
  currentRelease();
}

void UpdateInterface::initParamFolders(UpdateParameters * params)
{
  if (g.downloadDir().trimmed().isEmpty())
    g.downloadDirReset(true);

  params->data.downloadDir = g.downloadDir().trimmed();

  params->data.decompressDirUseDwnld = g.decompressDirUseDwnld();

  if (params->data.decompressDirUseDwnld)
    params->data.decompressDir = params->data.downloadDir;
  else {
    if (g.decompressDir().trimmed().isEmpty())
      g.decompressDirReset(true);

    params->data.decompressDir = g.decompressDir().trimmed();
  }

  params->data.updateDirUseSD = g.updateDirUseSD();

  if (params->data.updateDirUseSD && (!g.currentProfile().sdPath().trimmed().isEmpty()))
      params->data.updateDir = g.currentProfile().sdPath().trimmed();
  else {
    if (g.updateDir().trimmed().isEmpty())
      g.updateDirReset(true);

    params->data.updateDir = g.updateDir().trimmed();
  }
}

void UpdateInterface::resetRunEnvironment()
{
  progress = nullptr;
  logLevel = g.updLogLevel();
  qDebug() << tr("Log level: %1").arg(logLevel);

  *runParams = *dfltParams;
  runParams->data.flags &= ~UPDFLG_Update;

  if (g.updDelDownloads())
    runParams->data.flags |= UpdateInterface::UPDFLG_DelDownloads;
  else
    runParams->data.flags &= ~UpdateInterface::UPDFLG_DelDownloads;
}

int UpdateInterface::getSettingsIdx()
{
  int idx = g.getComponentIndex(name);

  if (idx < 0) {
    for (idx = 0; idx < MAX_COMPONENTS && g.component[idx].existsOnDisk(); idx++)
      ;
    if (idx >= MAX_COMPONENTS) {
      reportProgress(tr("No free slot to save interface settings!"), QtCriticalMsg);
      idx = -1;
    }
    else {
      g.component[idx].init();
      g.component[idx].name(name);
    }
  }

  return idx;
}

void UpdateInterface::setSettingsIdx()
{
  const int idx = getSettingsIdx();
  settingsIdx = idx;
  releases->setSettingsIndex(idx);
}

void UpdateInterface::getRadioProfileSettings()
{
  //  eg edgetx-ts16s-en
  QStringList strl = QStringList(g.currentProfile().fwType().split('-'));

  if (strl.size() < 1) {
    reportProgress(tr("Settings radio profile firmware type unexpected format"), QtWarningMsg);
    return;
  }

  if (strl.size() > 1) {
    fwFlavour = strl.at(1);
    fwFlavour = fwFlavour.replace('+', 'p');
  }

  if (strl.size() > 2)
    language = strl.at(2);
}

void UpdateInterface::clearRelease()
{
  g.component[settingsIdx].clearRelease();
  currentRelease();
}

const QString UpdateInterface::currentRelease()
{
  dfltParams->data.currentRelease = g.component[settingsIdx].release();
  runParams->data.currentRelease = dfltParams->data.currentRelease;
  return dfltParams->data.currentRelease;
}

const QString UpdateInterface::latestRelease()
{
  if (repoReleasesMetaData()) {
    releases->getSetId();
    if (releases->id())
      dfltParams->data.updateRelease = releases->name();
    else
      dfltParams->data.updateRelease = "";
  }

  return dfltParams->data.updateRelease;
}

const QString UpdateInterface::updateRelease()
{
  if (runParams->data.updateRelease.isEmpty()) {
    if (dfltParams->data.updateRelease.isEmpty())
      latestRelease();

    runParams->data.updateRelease = dfltParams->data.updateRelease;
  }

  return runParams->data.updateRelease;
}

const bool UpdateInterface::isUpdateAvailable()
{
  if (g.component[settingsIdx].checkForUpdate())
    return !isLatestRelease();
  else {
    return false;
  }
}

const bool UpdateInterface::isLatestVersion(const QString & installed, const QString & latest)
{
  QStringList a = semanticVersion(installed).split(".");
  QStringList b = semanticVersion(latest).split(".");

  // not equal comparision used to force update if client upgraded and the release is subsequently withdrawn or subscribed to nightlies
  for (int i = 0; i < 4; i++) {
    if (a.at(i) != b.at(i)) {
      return false;
    }
  }

  return true;
}

const bool UpdateInterface::isLatestRelease()
{
  //  subsequent processes rely on this call being successful so advise nothing to do
  if (!repoReleasesMetaData()) {
    return true;
  }

  QString currentVer = currentVersion();
  QString latestVer = releases->version();
  // nightlies often have the same version so also check id
  if (isLatestVersion(currentVer, latestVer) && g.component[settingsIdx].id() == releases->id()) {
    return true;
  }
  else {
    return false;
  }
}

const QString UpdateInterface::currentVersion()
{
  return g.component[settingsIdx].version();
}

const QStringList UpdateInterface::getReleases()
{
  repoReleasesMetaData();
  return releases->list();
}

bool UpdateInterface::repoReleasesMetaData()
{
  if (releases->refreshRequired()) {
    if (!downloadReleasesMetaData()) {
      reportProgress(tr("Unable to download release meta data"), QtDebugMsg);
      releases->setId(0);
      return false;
    }
  }

  releases->getSetId();

  return true;
}

bool UpdateInterface::repoReleaseAssetsMetaData()
{
  if (!downloadReleaseAssetsMetaData(releases->id())) {
    reportProgress(tr("Unable to download release channel assets information"), QtDebugMsg);
    return false;
  }

  return true;
}

bool UpdateInterface::setRunFolders()
{
  QString fldr(releases->name());
  fldr = fldr.replace("\"", "");
  fldr = fldr.replace("\'", "");
  fldr = fldr.replace(" ", "_");
  fldr = fldr.replace("(", "_");
  fldr = fldr.replace(")", "_");
  fldr = fldr.replace("[", "_");
  fldr = fldr.replace("]", "_");

  downloadDir = QString("%1/%2/%3").arg(runParams->data.downloadDir).arg(name).arg(fldr);

  if (!checkCreateDirectory(downloadDir, UPDFLG_Download))
    return false;

  reportProgress(tr("Download directory: %1").arg(downloadDir), QtDebugMsg);

  decompressDir = QString("%1/%2/%3").arg(runParams->data.decompressDir).arg(name).arg(fldr);

  if (!checkCreateDirectory(decompressDir, UPDFLG_Decompress))
    return false;

  reportProgress(tr("Decompress directory: %1").arg(decompressDir), QtDebugMsg);

  updateDir = runParams->data.updateDir;

  if (!checkCreateDirectory(updateDir, UPDFLG_CopyDest))
    return false;

  reportProgress(tr("Update directory: %1").arg(updateDir), QtDebugMsg);

  return true;
}

bool UpdateInterface::checkCreateDirectory(const QString & dir, const UpdateFlags flag)
{
  if (runParams->data.flags & flag) {
    if (dir.isEmpty()) {
      reportProgress(tr("%1 directory not configured in application settings!").arg(updateFlagsToString(flag)), QtCriticalMsg);
      return false;
    }
    else if (!QDir().mkpath(dir)) {
      reportProgress(tr("Failed to create %1 directory %2!").arg(updateFlagsToString(flag)).arg(dir), QtCriticalMsg);
      return false;
    }
  }

  return true;
}

bool UpdateInterface::downloadReleasesMetaData()
{
  progressMessage(tr("Download releases metadata"));
  downloadMetaData(MDT_Releases, releases->urlReleases());
  return downloadSuccess;
}

bool UpdateInterface::downloadReleaseLatestMetaData()
{
  progressMessage(tr("Download latest release metadata"));
  downloadMetaData(MDT_Release, releases->urlReleaseLatest());
  return downloadSuccess;
}

bool UpdateInterface::downloadReleaseMetaData(const int releaseId)
{
  progressMessage(tr("Download release %1 metadata").arg(releaseId));
  releases->setId(releaseId);
  downloadMetaData(MDT_Release, releases->urlRelease());
  return downloadSuccess;
}

bool UpdateInterface::downloadReleaseAssetsMetaData(const int releaseId)
{
  progressMessage(tr("Download release %1 assets metadata").arg(releaseId));
  downloadMetaData(MDT_ReleaseAssets, assets->urlReleaseAssets(releaseId, resultsPerPage));
  return downloadSuccess;
}

bool UpdateInterface::downloadAssetMetaData(const int assetId)
{
  progressMessage(tr("Download asset %1 metadata").arg(assetId));
  assets->setId(assetId);
  downloadMetaData(MDT_Asset, assets->urlAsset());
  return downloadSuccess;
}

void UpdateInterface::downloadMetaData(const MetaDataType mdt, const QString & url)
{
  reportProgress(tr("Download metadata for: %1").arg(metaDataTypeToString(mdt).toLower()));
  download(DDT_MetaData, mdt, url, GH_ACCEPT_HEADER_METADATA, QString());
}

bool UpdateInterface::downloadTextFileToBuffer(const QString & path)
{
  progressMessage(tr("Download file %1").arg(path));
  reportProgress(tr("Download file: %1").arg(path), QtInfoMsg);

  downloadFileToBuffer(assets->urlContent(path));
  return downloadSuccess;
}

void UpdateInterface::downloadFileToBuffer(const QString & url)
{
  download(DDT_Content, 0, url, GH_ACCEPT_HEADER_RAW, QString());
}

void UpdateInterface::download(const DownloadDataType type, const int subtype, const QString & urlStr,
                               const char * header, const QString & filePath)
{
  if (progress) {
    progress->setValue(0);
    progress->setMaximum(100);
  }

  buffer->clear();
  downloadSuccess = false;

  if (type == DDT_SaveToFile) {
    file = new QFile(filePath);
    if (!file->open(QIODevice::WriteOnly)) {
      reportProgress(tr("Unable to open the download file %1 for writing.\nError: %2").arg(filePath).arg(file->errorString()), QtCriticalMsg);
      return;
    }
  }
  else
    file = nullptr;

  url.setUrl(urlStr);

  if (!url.isValid()) {
    reportProgress(tr("Invalid URL: %1").arg(urlStr), QtCriticalMsg);
    return;
  }
  else
    reportProgress(tr("URL: %1").arg(urlStr), QtDebugMsg);

  request.setUrl(url);
  request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferNetwork);
  request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

  request.setRawHeader(QByteArray("Accept"), QByteArray(header));

  reply = manager.get(request);

  connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), [=] (QNetworkReply::NetworkError code) {
    //  leave it to the finished slot to deal with error condition
    reportProgress(tr("Network error has occurred. Error code: %1").arg(code), QtDebugMsg);
  });

  connect(reply, &QNetworkReply::readyRead, [=]() {
    if (type == DDT_SaveToFile) {
      file->write(reply->readAll());
    }
    else {
      const QByteArray qba = reply->readAll();
      buffer->append(qba);
    }
  });

  connect(reply, &QNetworkReply::finished, [&]() {
    onDownloadFinished(reply, type, subtype);
  });

  connect(reply, &QNetworkReply::downloadProgress, [=](const qint64 bytesRead, const qint64 totalBytes) {
    if (progress) {
      progress->setMaximum(totalBytes);
      progress->setValue(bytesRead);
    }
  });

  while (!reply->isFinished()) {
    qApp->processEvents();
  }
}

void UpdateInterface::onDownloadFinished(QNetworkReply * reply, DownloadDataType type, int subtype)
{
  if (type == DDT_SaveToFile) {
    file->flush();
    file->close();
  }

  if (progress)
    progress->setValue(progress->maximum());

  if (reply->error()) {
    reportProgress(tr("Unable to download %1.\nError:%2\n%3").arg(downloadDataTypeToString(type)).arg(reply->error()).arg(reply->errorString()), QtCriticalMsg);

    if (type == DDT_SaveToFile) {
      file->remove();
      delete file;
      file = nullptr;
    }

    return;
  }

  if (type == DDT_MetaData) {
    parseMetaData(subtype);
  }
  else if (type == DDT_SaveToFile) {
    delete file;
    file = nullptr;
    downloadSuccess = true;
  }
  else {
    downloadSuccess = true;
  }
}

void UpdateInterface::parseMetaData(int mdt)
{
  reportProgress(tr("Parse and load data models with metadata"), QtDebugMsg);

  QJsonDocument *json = new QJsonDocument();

  if (!convertDownloadToJson(json)) {
    return;
  }

  switch (mdt) {
    case MDT_Release:
    case MDT_Releases:
      releases->parseMetaData(mdt, json);
      break;
    case MDT_Asset:
    case MDT_ReleaseAssets:
      assets->parseMetaData(mdt, json);
      break;
  }

  delete json;
  downloadSuccess = true;
}

bool UpdateInterface::convertDownloadToJson(QJsonDocument * json)
{
  QJsonParseError res;

  *json = QJsonDocument::fromJson(*buffer, &res);

  if (res.error || json->isNull()) {
    reportProgress(tr("Unable to convert downloaded data to json format.\nError:%1\n%2").arg(res.error).arg(res.errorString()), QtCriticalMsg);
    return false;
  }

  return true;
}

bool UpdateInterface::getSetAssets(const int flags, const UpdateParameters::UpdateFilterType assetsFilterType, const QString & assetsFilter, const int maxAssetsExpected,
                                   const QString & subDirectory, const UpdateParameters::UpdateFilterType copyFilterType, const QString & copyFilter)
{
  QString pattern(UpdateParameters::buildFilterPattern(assetsFilterType, assetsFilter));
  assets->setFilterPattern(pattern);
  reportProgress(tr("Asset filter applied: %1").arg(pattern), QtDebugMsg);

  if (assets->count() < 0) {
    reportProgress(tr("No assets not found in release '%1' using filter pattern '%2'").arg(releases->name().arg(assetsFilter)), QtCriticalMsg);
    return false;
  }
  else if (maxAssetsExpected > -1 && assets->count() > maxAssetsExpected) {
    reportProgress(tr("%1 assets found when %2 expected in release '%3' using filter pattern '%4'")
                        .arg(assets->count()).arg(maxAssetsExpected).arg(releases->name().arg(assetsFilter)), QtCriticalMsg);
    return false;
  }

  reportProgress(tr("Assets found: %1").arg(assets->count()), QtDebugMsg);

  for (int i = 0; i < assets->count(); i++) {
    assets->getSetId(i);
    if (!assets->setFlags(flags)) {
      reportProgress(tr("Unable to set processing flags for asset %1").arg(assets->filename()), QtCriticalMsg);
      return false;
    }

    if (!assets->setSubDirectory(subDirectory)) {
      reportProgress(tr("Unable to set sub directory for asset %1").arg(assets->filename()), QtCriticalMsg);
      return false;
    }

    if (!assets->setCopyFilter(UpdateParameters::buildFilterPattern(copyFilterType, copyFilter))) {
      reportProgress(tr("Unable to set copy filter for asset %1").arg(assets->filename()), QtCriticalMsg);
      return false;
    }
  }

  return true;
}

bool UpdateInterface::downloadFlaggedAssets()
{
  assets->setFilterFlags(UPDFLG_Download);
  reportProgress(tr("Asset filter applied: %1 Assets found: %2").arg(updateFlagsToString(UPDFLG_Download)).arg(assets->count()), QtDebugMsg);

  for (int i = 0; i < assets->count(); ++i) {
    if (!downloadAsset(i))
      return false;
  }

  return true;
}

bool UpdateInterface::downloadAsset(int row)
{
  assets->getSetId(row);
  downloadBinaryToFile(assets->urlAsset(), assets->filename());
  return downloadSuccess;
}

void UpdateInterface::downloadBinaryToFile(const QString & url, const QString & filename)
{
  downloadSuccess = false;

  QFileInfo f(QString("%1/A%2/%3").arg(downloadDir).arg(assets->id()).arg(filename));

  progressMessage(tr("Downloading: %1").arg(filename));
  reportProgress(tr("Download: %1").arg(filename), QtInfoMsg);

  if (f.exists()) {
    reportProgress(tr("File exists: %1").arg(filename), QtDebugMsg);
    int reply = QMessageBox::question(this, CPN_STR_APP_NAME, tr("File %1 exists. Download again?").arg(filename),
                              QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::No);
    if (reply == QMessageBox::Cancel) {
      reportProgress(tr("User action: Cancel"), QtDebugMsg);
      downloadSuccess = false;
      return;
    }
    else if (reply == QMessageBox::No) {
      reportProgress(tr("User action: Use previous download"), QtDebugMsg);
      downloadSuccess = true;
      return;
    }
    else {
      reportProgress(tr("User action: Download again"), QtDebugMsg);
    }
  }
  else if (!QDir().mkpath(f.path())) {
    reportProgress(tr("Failed to create directory %1!").arg(f.path()), QtCriticalMsg);
    return;
  }

  download(DDT_SaveToFile, 0, url, GH_ACCEPT_HEADER_BINARY, f.absoluteFilePath());
}



bool UpdateInterface::decompressFlaggedAssets()
{
  assets->setFilterFlags(UPDFLG_Decompress);
  reportProgress(tr("Asset filter applied: %1 Assets found: %2").arg(updateFlagsToString(UPDFLG_Decompress)).arg(assets->count()), QtDebugMsg);

  for (int i = 0; i < assets->count(); ++i) {
    if (!decompressAsset(i))
      return false;
  }

  return true;
}

bool UpdateInterface::decompressAsset(int row)
{
  assets->getSetId(row);

  if (!(assets->flags() & UPDFLG_Decompress))
    return true;

  QString filename = assets->filename();

  if (filename.isEmpty())
    return false;

  QFileInfo f(QString("%1/A%2/%3").arg(downloadDir).arg(assets->id()).arg(assets->filename()));

  if (!f.exists())
    return false;

  reportProgress(tr("Decompress: %1").arg(filename), QtInfoMsg);

  decompressArchive(f.absoluteFilePath(), QString("%1/A%2").arg(decompressDir).arg(assets->id()));

  return true;
}

bool UpdateInterface::decompressArchive(const QString & archivePath, const QString & destDir)
{
  QString dest = destDir;

  if (!dest.endsWith("/"))
    dest.append("/");

  dest.append(QFileInfo(archivePath).completeBaseName());

  MinizInterface mIface(progress, MinizInterface::PCM_SIZE, logLevel);

  if (!mIface.unzipArchiveToPath(archivePath, dest)) {
    criticalMsg(tr("Failed to decompress %1").arg(archivePath));
    return false;
  }

  return true;
}

bool UpdateInterface::copyFlaggedAssets()
{
  assets->setFilterFlags(UPDFLG_CopyDest);
  reportProgress(tr("Asset filter applied: %1 Assets found: %2").arg(updateFlagsToString(UPDFLG_CopyDest)).arg(assets->count()), QtDebugMsg);

  for (int i = 0; i < assets->count(); ++i) {
    assets->getSetId(i);
    if (!copyAsset())
      return false;
  }

  return true;
}

bool UpdateInterface::copyAsset()
{
  if (assets->flags() & UPDFLG_CopyStructure)
    return copyStructure();
  else if (assets->flags() & UPDFLG_CopyFiles)
    return copyFiles();

  return true;
}

bool UpdateInterface::copyStructure()
{
  reportProgress(tr("Copy directory structure"), QtInfoMsg);

  if (progress) {
    progress->setValue(0);
    progress->setMaximum(100);
  }

  QDir srcPath(QString("%1/A%2/%3").arg(decompressDir).arg(assets->id()).arg(QFileInfo(assets->filename()).completeBaseName()));

  if (!srcPath.exists()) {
    reportProgress(tr("Decompressed file structure not found at %1").arg(QDir::toNativeSeparators(srcPath.path())), QtCriticalMsg);
    return false;
  }

  reportProgress(tr("Copy file structure from: %1").arg(QDir::toNativeSeparators(srcPath.path())), QtDebugMsg);

  QString baseDir = updateDir;

  if (!assets->subDirectory().isEmpty())
    baseDir.append("/" % assets->subDirectory());

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
      reportProgress(tr("Failed to create directory %1").arg(dir), QtCriticalMsg);
      return false;
    }

    reportProgress(tr("Check/create directory: %1").arg(dir), QtDebugMsg);
    cnt++;
  }

  reportProgress(tr("Directories checked/created: %1").arg(cnt), QtInfoMsg);

  QRegularExpression filter(assets->copyFilter(), QRegularExpression::CaseInsensitiveOption);
  reportProgress(tr("Copy filter pattern: %1").arg(filter.pattern()), QtDebugMsg);

  QDirIterator itcnt(srcPath.path(), QDir::Files, QDirIterator::Subdirectories);

  cnt = 0;

  reportProgress(tr("Calculating number of files"), QtDebugMsg);

  while (itcnt.hasNext())
  {
    QFileInfo src(itcnt.next());

    if ((!filter.pattern().isEmpty()) && (!src.fileName().contains(filter)))
      continue;

    cnt++;
  }

  if (progress)
    progress->setMaximum(cnt);

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
        reportProgress(tr("Failed to delete existing file %1").arg(dest.fileName()), QtCriticalMsg);
        return false;
      }
    }

    if (!QFile::copy(src.absoluteFilePath(), dest.fileName())) {
      reportProgress(tr("Failed to copy file %1").arg(src.absoluteFilePath()), QtCriticalMsg);
      return false;
    }

    reportProgress(tr("Copied %1 to %2").arg(src.fileName()).arg(dest.fileName()), QtDebugMsg);
    cnt++;
    if (progress)
      progress->setValue(cnt);
  }

  if (progress)
    progress->setValue(progress->maximum());

  reportProgress(tr("Files copied: %1").arg(cnt), QtInfoMsg);

  return true;
}

bool UpdateInterface::copyFiles()
{
  reportProgress(tr("Copy files"), QtInfoMsg);

  if (progress) {
    progress->setValue(0);
    progress->setMaximum(100);
  }

  QString srcPath;

  if (assets->flags() & UPDFLG_Decompress)
    srcPath = QString("%1/A%2/%3").arg(decompressDir).arg(assets->id()).arg(QFileInfo(assets->filename()).completeBaseName());
  else if (assets->flags() & UPDFLG_Download)
    srcPath = QString("%1/A%2").arg(downloadDir).arg(assets->id());
  else {
    reportProgress(tr("Unable to determine source directory for asset %1").arg(assets->filename()), QtCriticalMsg);
    return false;
  }

  reportProgress(tr("Copy files from %1").arg(srcPath), QtDebugMsg);

  QString destPath = updateDir;

  if (!assets->subDirectory().isEmpty())
    destPath.append("/" % assets->subDirectory());

  if (!QDir().mkpath(destPath)) {
    reportProgress(tr("Failed to create directory %1").arg(QDir::toNativeSeparators(destPath)), QtCriticalMsg);
    return false;
  }

  reportProgress(tr("Check/create directory: %1").arg(destPath), QtDebugMsg);
  reportProgress(tr("Directories checked/created: %1").arg(1), QtDebugMsg);

  QRegularExpression filter(assets->copyFilter(), QRegularExpression::CaseInsensitiveOption);
  reportProgress(tr("Copy filter pattern: %1").arg(filter.pattern()), QtDebugMsg);

  QDirIterator itcnt(srcPath, QDir::Files, QDirIterator::Subdirectories);

  int cnt = 0;

  reportProgress(tr("Calculating number of files"), QtDebugMsg);

  while (itcnt.hasNext())
  {
    QFileInfo src(itcnt.next());

    if ((!filter.pattern().isEmpty()) && (!src.fileName().contains(filter)))
      continue;

    cnt++;
  }

  if (progress)
    progress->setMaximum(cnt);

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
        reportProgress(tr("Failed to delete existing file %1").arg(dest.fileName()), QtCriticalMsg);
        return false;
      }
      reportProgress(tr("Deleted file: %1").arg(dest.fileName()), QtDebugMsg);
    }

    if (!QFile::copy(src.absoluteFilePath(), dest.fileName())) {
      reportProgress(tr("Failed to copy file %1").arg(src.absoluteFilePath()), QtCriticalMsg);
      return false;
    }

    reportProgress(tr("Copied %1 to %2").arg(src.fileName()).arg(dest.fileName()), QtDebugMsg);
    cnt++;
    if (progress)
      progress->setValue(cnt);
  }

  if (!cnt) {
    reportProgress(tr("No downloaded or decompressed files matched filter '%1'").arg(assets->copyFilter()), QtCriticalMsg);
    return false;
  }

  if (progress)
    progress->setValue(progress->maximum());

  reportProgress(tr("Files copied: %1").arg(cnt), QtInfoMsg);

  return true;
}

bool UpdateInterface::preparation()
{
  progressMessage(tr("Preparing"));
  int cnt = 0;

  if (progress) {
    progress->setValue(cnt);
    progress->setMaximum(5);
  }

  if (!setRunFolders()) {
    reportProgress(tr("Set run folders failed"), QtDebugMsg);
    return false;
  }

  if (progress)
    progress->setValue(++cnt);

  if (!repoReleasesMetaData()) {
    return false;
  }

  if (progress)
    progress->setValue(++cnt);

  if (!releases->getSetId(runParams->data.updateRelease)) {
    reportProgress(tr("Set release id from update release name failed"), QtDebugMsg);
    return false;
  }

  if (progress)
    progress->setValue(++cnt);

  if (!repoReleaseAssetsMetaData())
    return false;

  if (progress)
    progress->setValue(++cnt);

  if (!flagAssets())
    return false;

  if (progress)
    progress->setValue(++cnt);

  return true;
}

bool UpdateInterface::flagAssets()
{
  progressMessage(tr("Flagging assets"));

  for (int i = 0; i < runParams->data.assets.size(); i++) {
    const UpdateParameters::AssetParams &ap = runParams->data.assets.at(i);
    if (!getSetAssets(ap.flags, ap.filterType, ap.filter, ap.maxExpected, ap.destSubDir, ap.copyFilterType, ap.copyFilter))
      return false;
  }

  return true;
}

bool UpdateInterface::download()
{
  progressMessage(tr("Downloading assets"));

  if (!downloadFlaggedAssets()) {
    reportProgress(tr("Unable to download flagged assets"), QtDebugMsg);
    return false;
  }

  return true;
}

bool UpdateInterface::decompress()
{
  progressMessage(tr("Decompressing assets"));

  if (!decompressFlaggedAssets()) {
    reportProgress(tr("Unable to decompress flagged assets"), QtDebugMsg);
    return false;
  }

  return true;
}

bool UpdateInterface::copyToDestination()
{
  progressMessage(tr("Copying to destination"));

  if (!copyFlaggedAssets()) {
    reportProgress(tr("Unable to copy assets"), QtDebugMsg);
    return false;
  }

  return true;
}

bool UpdateInterface::housekeeping()
{
  progressMessage(tr("Housekeeping"));
  int cnt = 0;

  if (progress) {
    progress->setValue(cnt);
    progress->setMaximum(3);
  }

  if (!saveReleaseSettings()) {
    reportProgress(tr("Failed to save release settings"), QtDebugMsg);
    return false;
  }

  if (progress)
    progress->setValue(++cnt);

  if (runParams->data.flags & UPDFLG_DelDownloads) {
    reportProgress(tr("Delete download directory: %1").arg(downloadDir), QtDebugMsg);
    QDir d(downloadDir);
    if (!d.removeRecursively())
      reportProgress(tr("Failed to delete downloads folder %1").arg(downloadDir), QtCriticalMsg);
  }

  if (progress)
    progress->setValue(++cnt);

  if (!g.decompressDirUseDwnld() && (runParams->data.flags & UPDFLG_DelDownloads)) {
    reportProgress(tr("Delete decompress directory: %1").arg(decompressDir), QtDebugMsg);
    QDir d(decompressDir);
    if (!d.removeRecursively())
      reportProgress(tr("Failed to delete decompress folder %1").arg(decompressDir), QtCriticalMsg);
  }

  if (progress)
    progress->setValue(++cnt);

  return true;
}

bool UpdateInterface::asyncInstall()
{
  return true;
}

bool UpdateInterface::saveReleaseSettings()
{
  reportProgress(tr("Save release settings"), QtDebugMsg);
  g.component[settingsIdx].release(releases->name());
  g.component[settingsIdx].version(releases->version());
  g.component[settingsIdx].id(releases->id());
  g.component[settingsIdx].date(releases->date());

  return true;
}

/*
  UpdateFactories
*/

UpdateFactories::UpdateFactories(QWidget * parent) :
  QWidget(parent)
{
  registerUpdateFactories();
}

UpdateFactories::~UpdateFactories()
{
  unregisterUpdateFactories();
}

void UpdateFactories::registerUpdateFactory(UpdateFactoryInterface * factory)
{
  registeredUpdateFactories.append(factory);
  qDebug() << "Registered update factory:" << factory->name();
}

void UpdateFactories::registerUpdateFactories()
{
  //  Note: register in logical updating sequence
  //        SDCard should be first as it may clean up/erase commmon destination directory
  registerUpdateFactory(new UpdateFactory<UpdateSDCard>(this));
  registerUpdateFactory(new UpdateFactory<UpdateFirmware>(this));
  registerUpdateFactory(new UpdateFactory<UpdateSounds>(this));
  registerUpdateFactory(new UpdateFactory<UpdateThemes>(this));
  registerUpdateFactory(new UpdateFactory<UpdateMultiProtocol>(this));

  //  Note: Companion must be last as its install requires the app to be closed and thus would interrupt the update loop
  registerUpdateFactory(new UpdateFactory<UpdateCompanion>(this));
}

void UpdateFactories::unregisterUpdateFactories()
{
  foreach (UpdateFactoryInterface * factory, registeredUpdateFactories)
    delete factory;
}

const UpdateParameters * const UpdateFactories::getDefaultParams(const QString & name)
{
  foreach (UpdateFactoryInterface * factory, registeredUpdateFactories) {
    if (name == factory->name()) {
      return factory->instance()->getDefaultParams();
      break;
    }
  }

  return nullptr;
}

UpdateParameters * const UpdateFactories::getRunParams(const QString & name)
{
  foreach (UpdateFactoryInterface * factory, registeredUpdateFactories) {
    if (name == factory->name()) {
      return factory->instance()->getRunParams();
      break;
    }
  }

  return nullptr;
}

void UpdateFactories::resetRunEnvironment(const QString & name)
{
  foreach (UpdateFactoryInterface * factory, registeredUpdateFactories) {
    if (name == factory->name()) {
      factory->instance()->resetRunEnvironment();
      break;
    }
  }
}

void UpdateFactories::resetAllRunEnvironments()
{
  foreach (UpdateFactoryInterface * factory, registeredUpdateFactories) {
    factory->instance()->resetRunEnvironment();
  }
}

void UpdateFactories::setRunUpdate(const QString & name)
{
  foreach (UpdateFactoryInterface * factory, registeredUpdateFactories) {
    if (name == factory->name()) {
      factory->instance()->setRunUpdate();
      break;
    }
  }
}

const QMap<QString, int> UpdateFactories::sortedComponentsList()
{
  QMap<QString, int> map;

  foreach (UpdateFactoryInterface * factory, registeredUpdateFactories) {
    map.insert(factory->name(), factory->instance()->settingsIdx);
  }

  return map;
}

void UpdateFactories::clearRelease(const QString & name)
{
  foreach (UpdateFactoryInterface * factory, registeredUpdateFactories) {
    if (name == factory->name()) {
      factory->instance()->clearRelease();
      break;
    }
  }
}

const QString UpdateFactories::currentRelease(const QString & name)
{
  QString ret;

  foreach (UpdateFactoryInterface * factory, registeredUpdateFactories) {
    if (name == factory->name()) {
      ret = factory->instance()->currentRelease();
      break;
    }
  }

  return ret;
}

const QString UpdateFactories::updateRelease(const QString & name)
{
  QString ret;

  foreach (UpdateFactoryInterface * factory, registeredUpdateFactories) {
    if (name == factory->name()) {
      ret = factory->instance()->updateRelease();
      break;
    }
  }

  return ret;
}

const bool UpdateFactories::isLatestRelease(const QString & name)
{
  bool ret = true;

  foreach (UpdateFactoryInterface * factory, registeredUpdateFactories) {
    if (name == factory->name()) {
      ret = factory->instance()->isLatestRelease();
      break;
    }
  }

  return ret;
}

const QString UpdateFactories::latestRelease(const QString & name)
{
  QString ret;

  foreach (UpdateFactoryInterface * factory, registeredUpdateFactories) {
    if (name == factory->name()) {
      ret = factory->instance()->latestRelease();
      break;
    }
  }

  return ret;
}

const QStringList UpdateFactories::releases(const QString & name)
{
  QStringList ret;

  foreach (UpdateFactoryInterface * factory, registeredUpdateFactories) {
    if (name == factory->name()) {
      ret = factory->instance()->getReleases();
      break;
    }
  }

  return ret;
}

bool UpdateFactories::autoUpdate(ProgressWidget * progress)
{
  bool ok = false;

  foreach (UpdateFactoryInterface * factory, registeredUpdateFactories) {
    ok = factory->instance()->autoUpdate(progress);
    if (!ok)
      break;
  }

  return ok;
}

bool UpdateFactories::manualUpdate(ProgressWidget * progress)
{
  bool ok = false;

  foreach (UpdateFactoryInterface * factory, registeredUpdateFactories) {
    ok = factory->instance()->manualUpdate(progress);
    if (!ok)
      break;
  }

  return ok;
}

const bool UpdateFactories::updatesAvailable()
{
  bool ret = false;

  foreach (UpdateFactoryInterface * factory, registeredUpdateFactories) {
    if (factory->instance()->isUpdateAvailable()) {
      ret = true;
    }
  }

  return ret;
}
