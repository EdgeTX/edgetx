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
#include <QValidator>

/*

  UpdateParameters

*/

UpdateParameters::UpdateParameters(QObject * parent) :
  QObject(parent)
{
  logLevel = 0;
  flags = 0;
  decompressDirUseDwnld = true;
  updateDirUseSD =  true;
}

UpdateParameters::AssetParams & UpdateParameters::addAsset()
{
  AssetParams ap;
  ap.processes = 0;
  ap.flags = 0;
  ap.filterType = UFT_None;
  ap.maxExpected = 0;
  ap.copyFilterType = UFT_None;
  assets.append(ap);

  return assets[assets.size() - 1];
}

QString UpdateParameters::buildFilterPattern(const UpdateFilterType filterType, const QString & filter)
{
  QString pattern(filter);

  pattern.replace("%FWFLAVOUR%", fwFlavour);
  pattern.replace("%LANGUAGE%", language);

  switch ((int)filterType) {
    case UFT_Exact:
      pattern.prepend("^");
      pattern.append("$");
      break;
    case UFT_Startswith:
      pattern.prepend("^");
      break;
    case UFT_Endswith:
      pattern.append("$");
      break;
    case UFT_Contains:
    case UFT_Expression:
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
  progress(nullptr),
  reply(nullptr),
  buffer(new QByteArray()),
  file(nullptr),
  m_id(CID_Unknown),
  m_name("")
{
  QNetworkProxyFactory::setUseSystemConfiguration(true);

  releases = new ReleasesMetaData(this);
  assets = new AssetsMetaData(this);
  params = new UpdateParameters(this);
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
  delete params;
}

void UpdateInterface::init(ComponentIdentity id, QString name, QString repo, QString nightly, int resultsPerPage)
{
  setId(id);
  setName(name);

  initAppSettings();

  releases->init(repo, nightly, resultsPerPage, id);
  assets->init(repo, resultsPerPage);

  currentRelease();
}

void UpdateInterface::initAppSettings()
{
  if (!isValidSettingsIndex()) {
    reportProgress(tr("Component id: %1 exceeds maximum application settings components: %2!").arg(m_id).arg(MAX_COMPONENTS), QtCriticalMsg);
    return;
  }

  if (!g.component[m_id].existsOnDisk()) {
    g.component[m_id].init();
  }

  if (!g.component[m_id].asset[0].existsOnDisk())
    initAssetSettings();
}

void UpdateInterface::loadAssetSettings()
{
  if (!isValidSettingsIndex())
    return;

  params->assets.clear();

  for (int i = 0; i < MAX_COMPONENT_ASSETS && g.component[m_id].asset[i].existsOnDisk(); i++) {
    UpdateParameters::AssetParams &ap = params->addAsset();
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

void UpdateInterface::saveAssetSettings()
{
  if (!isValidSettingsIndex())
    return;

  for (int i = 0; i < MAX_COMPONENT_ASSETS && g.component[m_id].asset[i].existsOnDisk(); i++) {
    const UpdateParameters::AssetParams &ap = params->assets.at(i);
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

bool UpdateInterface::update(ProgressWidget * progress)
{
  if (!(params->flags & UPDFLG_Update))
    return true;

  this->progress = progress;

  if (progress) {
    progress->setInfo(tr("Processing updates for: %1").arg(m_name));
    progress->setValue(0);
    progress->setMaximum(100);
  }

  reportProgress(tr("Processing updates for: %1").arg(m_name), QtInfoMsg);

  if (!preparation()) {
    reportProgress(tr("%1 preparation failed").arg(m_name), QtCriticalMsg);
    return false;
  }

  if (!download()) {
    reportProgress(tr("%1 download failed").arg(m_name), QtCriticalMsg);
    return false;
  }

  if (!decompress()) {
    reportProgress(tr("%1 decompress failed").arg(m_name), QtCriticalMsg);
    return false;
  }

  if (!copyToDestination()) {
    reportProgress(tr("%1 copy to destination failed").arg(m_name), QtCriticalMsg);
    return false;
  }

  //  perform before async install in case Companion restarted
  if (!saveReleaseSettings()) {
    reportProgress(tr("Failed to save release settings"), QtDebugMsg);
    return false;
  }

  if (!asyncInstall()) {
    reportProgress(tr("%1 start async failed").arg(m_name), QtCriticalMsg);
    return false;
  }

  if (!housekeeping()) {
    reportProgress(tr("%1 housekeeping failed").arg(m_name), QtCriticalMsg);
    return false;
  }

  reportProgress(tr("%1 update successful").arg(m_name), QtInfoMsg);

  if (!progress)
    QMessageBox::information(progress, CPN_STR_APP_NAME, tr("%1 update successful").arg(m_name));

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
  if (params->logLevel == QtDebugMsg ||
     (params->logLevel == QtInfoMsg && type > QtDebugMsg) ||
     (type < QtInfoMsg && type >= params->logLevel)) {
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
  QMessageBox::critical(progress, tr("Update Interface"), msg);
}

bool UpdateInterface::isUpdateable()
{
  return g.component[m_id].checkForUpdate();
}

void UpdateInterface::resetEnvironment()
{
  //  reset from previous run if any
  params->flags &= ~UPDFLG_Update;
  progress = nullptr;

  params->logLevel = g.updLogLevel();
  setReleaseChannel(g.component[m_id].releaseChannel());
  params->updateRelease = "";
  setFlavourLanguage();
  loadAssetSettings();
  setParamFolders();

  if (g.updDelDownloads())
    params->flags |= UpdateInterface::UPDFLG_DelDownloads;
  else
    params->flags &= ~UpdateInterface::UPDFLG_DelDownloads;

  if (g.updDelDecompress())
    params->flags |= UpdateInterface::UPDFLG_DelDecompress;
  else
    params->flags &= ~UpdateInterface::UPDFLG_DelDecompress;
}

void UpdateInterface::setReleaseChannel(int channel)
{
  params->releaseChannel = channel;
  //repoReleasesMetaData();
  releases->setReleaseChannel(channel);
}

void UpdateInterface::setReleaseId(QString val)
{
  releases->getSetId(val);
}

void UpdateInterface::setFlavourLanguage()
{
  const Firmware * baseFw = getCurrentFirmware()->getFirmwareBase();
  const QStringList currVariant = getCurrentFirmware()->getId().split('-');

  params->fwFlavour = "";

  if (currVariant.size() > 1) {
    params->fwFlavour = currVariant.at(1);
    params->fwFlavour = params->fwFlavour.replace('+', 'p');
  }

  params->language = "";

  for (const char *lang : baseFw->languageList()) {
    if (currVariant.last() == lang) {
      params->language = currVariant.last();
      break;
    }
  }
}

void UpdateInterface::setParamFolders()
{
  if (g.downloadDir().trimmed().isEmpty())
    g.downloadDirReset(true);

  params->downloadDir = g.downloadDir().trimmed();

  params->decompressDirUseDwnld = g.decompressDirUseDwnld();

  if (params->decompressDirUseDwnld)
    params->decompressDir = params->downloadDir;
  else {
    if (g.decompressDir().trimmed().isEmpty())
      g.decompressDirReset(true);

    params->decompressDir = g.decompressDir().trimmed();
  }

  if (g.currentProfile().sdPath().trimmed().isEmpty())
    params->updateDirUseSD = false;
  else
    params->updateDirUseSD = g.updateDirUseSD();

  if (params->updateDirUseSD) {
    params->sdDir = g.currentProfile().sdPath().trimmed();
    params->updateDir = params->sdDir;
  }
  else {
    if (g.updateDir().trimmed().isEmpty())
      g.updateDirReset(true);

    params->updateDir = g.updateDir().trimmed();
  }
}

void UpdateInterface::clearRelease()
{
  g.component[m_id].clearRelease();
  currentRelease();
}

const QString UpdateInterface::currentRelease()
{
  params->currentRelease = g.component[m_id].release();
  return params->currentRelease;
}

QString UpdateInterface::latestRelease()
{
  if (!repoReleasesMetaData())
    return tr("unknown");

  return releases->name();
}

const QString UpdateInterface::updateRelease()
{
  if (params->updateRelease.isEmpty())
    params->updateRelease = latestRelease();

  return params->updateRelease;
}

const bool UpdateInterface::isUpdateAvailable()
{
  if (g.component[m_id].checkForUpdate())
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
  if (!repoReleasesMetaData())
    return false;

  QString currentVer = currentVersion();
  QString latestVer = releases->version();
  // nightlies often have the same version so also check id
  if (isLatestVersion(currentVer, latestVer) && g.component[m_id].releaseId() == releases->id()) {
    return true;
  }
  else {
    return false;
  }
}

const QString UpdateInterface::currentVersion()
{
  return g.component[m_id].version();
}

const QStringList UpdateInterface::getReleases()
{
  if (!repoReleasesMetaData())
    return QStringList();

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

    releases->getSetId();
  }

  return true;
}

bool UpdateInterface::repoReleaseAssetsMetaData()
{
  if (!repoReleasesMetaData())
    return false;

  if (!downloadReleaseAssetsMetaData(releases->id())) {
    reportProgress(tr("Unable to download release assets information"), QtDebugMsg);
    return false;
  }

  return true;
}

bool UpdateInterface::setRunFolders()
{
  QRegExp rx("[0-9a-zA-Z_\\-.]+");
  // the validator treats the regexp as "^[0-9a-zA-Z_\-.]+$"
  QRegExpValidator v(rx);
  int pos = 0;

  QString fldr(releases->name().trimmed());

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
      //reportProgress(tr("Unable to use release name %1 for directory name using default %2")
      //               .arg(releases->name().trimmed()).arg(QString("R%1").arg(releases->id())), QtDebugMsg);
      fldr = QString("R%1").arg(releases->id());
    }
  }

  downloadDir = QString("%1/%2/%3").arg(params->downloadDir).arg(m_name).arg(fldr);

  if (!checkCreateDirectory(downloadDir, UPDFLG_Download))
    return false;

  //reportProgress(tr("Download directory: %1").arg(downloadDir), QtDebugMsg);

  decompressDir = QString("%1/%2/%3").arg(params->decompressDir).arg(m_name).arg(fldr);

  if (!checkCreateDirectory(decompressDir, UPDFLG_Decompress))
    return false;

  //reportProgress(tr("Decompress directory: %1").arg(decompressDir), QtDebugMsg);

  updateDir = params->updateDir;

  if (!checkCreateDirectory(updateDir, UPDFLG_CopyDest))
    return false;

  //reportProgress(tr("Update directory: %1").arg(updateDir), QtDebugMsg);

  return true;
}

bool UpdateInterface::checkCreateDirectory(const QString & dir, const UpdateFlags flag)
{
  if (params->flags & flag) {
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

 bool UpdateInterface::getReleaseJsonAsset(const QString assetName, QJsonDocument * json)
{
  if (!repoReleaseAssetsMetaData())
    return false;

  if (!assets->getSetId(assetName))
    return false;

  if (!downloadAssetToBuffer(assets->id())) {
    return false;
  }

  if (!convertDownloadToJson(json)) {
    return false;
  }

  return true;
}

 bool UpdateInterface::getRepoJsonFile(const QString filename, QJsonDocument * json)
{
  if (!downloadTextFileToBuffer(filename)) {
    return false;
  }

  if (!convertDownloadToJson(json)) {
    return false;
  }

  return true;
}

bool UpdateInterface::downloadReleasesMetaData()
{
  //progressMessage(tr("Download releases metadata"));
  downloadMetaData(MDT_Releases, releases->urlReleases());
  return downloadSuccess;
}

bool UpdateInterface::downloadReleaseLatestMetaData()
{
  //progressMessage(tr("Download latest release metadata"));
  downloadMetaData(MDT_Release, releases->urlReleaseLatest());
  return downloadSuccess;
}

bool UpdateInterface::downloadReleaseMetaData(const int releaseId)
{
  //progressMessage(tr("Download release %1 metadata").arg(releaseId));
  releases->setId(releaseId);
  downloadMetaData(MDT_Release, releases->urlRelease());
  return downloadSuccess;
}

bool UpdateInterface::downloadReleaseAssetsMetaData(const int releaseId)
{
  //progressMessage(tr("Download release %1 assets metadata").arg(releaseId));
  downloadMetaData(MDT_ReleaseAssets, assets->urlReleaseAssets(releaseId));
  return downloadSuccess;
}

bool UpdateInterface::downloadAssetMetaData(const int assetId)
{
  //progressMessage(tr("Download asset %1 metadata").arg(assetId));
  assets->setId(assetId);
  downloadMetaData(MDT_Asset, assets->urlAsset());
  return downloadSuccess;
}

void UpdateInterface::downloadMetaData(const MetaDataType mdt, const QString & url)
{
  //reportProgress(tr("Download metadata for: %1").arg(metaDataTypeToString(mdt).toLower()), QtDebugMsg);
  download(DDT_MetaData, mdt, url, GH_ACCEPT_HEADER_METADATA, QString());
}

bool UpdateInterface::downloadAssetToBuffer(const int assetId)
{
  //progressMessage(tr("Download asset %1").arg(assetId));
  assets->setId(assetId);
  download(DDT_Content, 0, assets->urlAsset(), GH_ACCEPT_HEADER_BINARY, QString());
  return downloadSuccess;
}

bool UpdateInterface::downloadTextFileToBuffer(const QString & path)
{
  //progressMessage(tr("Download file %1").arg(path));
  //reportProgress(tr("Download file: %1").arg(path), QtInfoMsg);

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
  //reportProgress(tr("Parse and load data models with metadata"), QtDebugMsg);

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

bool UpdateInterface::getSetAssets(const UpdateParameters::AssetParams & ap)
{
  QString pattern(params->buildFilterPattern(ap.filterType, ap.filter));
  assets->setFilterPattern(pattern);
  reportProgress(tr("Asset filter: %1").arg(pattern), QtDebugMsg);

  if (assets->count() < 1) {
    reportProgress(tr("No assets found in release '%1' using filter '%2'").arg(releases->name()).arg(pattern), QtCriticalMsg);
    return false;
  }
  else if (ap.maxExpected > 0 && assets->count() > ap.maxExpected) {
    reportProgress(tr("%1 assets found when %2 expected in release '%3' using filter '%4'")
                        .arg(assets->count() + 1).arg(ap.maxExpected).arg(releases->name().arg(pattern)), QtCriticalMsg);
    return false;
  }

  //reportProgress(tr("Assets found: %1").arg(assets->count()), QtDebugMsg);

  for (int i = 0; i < assets->count(); i++) {
    assets->getSetId(i);
    if (!assets->setFlags(ap.flags)) {
      reportProgress(tr("Unable to set processing flags for asset %1").arg(assets->filename()), QtCriticalMsg);
      return false;
    }

    if (!assets->setSubDirectory(ap.destSubDir)) {
      reportProgress(tr("Unable to set sub directory for asset %1").arg(assets->filename()), QtCriticalMsg);
      return false;
    }

    if (!assets->setCopyFilter(params->buildFilterPattern(ap.copyFilterType, ap.copyFilter))) {
      reportProgress(tr("Unable to set copy filter for asset %1").arg(assets->filename()), QtCriticalMsg);
      return false;
    }
  }

  return true;
}

bool UpdateInterface::downloadFlaggedAssets()
{
  assets->setFilterFlags(UPDFLG_Download);
  //reportProgress(tr("Asset filter applied: %1 Assets found: %2").arg(updateFlagsToString(UPDFLG_Download)).arg(assets->count()), QtDebugMsg);

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
    if (progress) {
      int reply = QMessageBox::question(progress, CPN_STR_APP_NAME, tr("File %1 exists. Download again?").arg(filename),
                                QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::No);
      if (reply == QMessageBox::Cancel) {
        //reportProgress(tr("User action: Cancel"), QtDebugMsg);
        downloadSuccess = false;
        return;
      }
      else if (reply == QMessageBox::No) {
        //reportProgress(tr("User action: Use previous download"), QtDebugMsg);
        downloadSuccess = true;
        return;
      }
      else {
        ;
        //reportProgress(tr("User action: Download again"), QtDebugMsg);
      }
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
  //reportProgress(tr("Asset filter applied: %1 Assets found: %2").arg(updateFlagsToString(UPDFLG_Decompress)).arg(assets->count()), QtDebugMsg);

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

  progressMessage(tr("Decompressing %1").arg(filename));
  reportProgress(tr("Decompress: %1").arg(filename), QtInfoMsg);

  return decompressArchive(f.absoluteFilePath(), QString("%1/A%2").arg(decompressDir).arg(assets->id()));
}

bool UpdateInterface::decompressArchive(const QString & archivePath, const QString & destDir)
{
  QString dest = destDir;

  if (!dest.endsWith("/"))
    dest.append("/");

  dest.append(QFileInfo(archivePath).completeBaseName());

  MinizInterface mIface(progress, MinizInterface::PCM_SIZE, params->logLevel);

  if (!mIface.unzipArchiveToPath(archivePath, dest)) {
    reportProgress(tr("Failed to decompress %1").arg(archivePath), QtCriticalMsg);
    return false;
  }

  return true;
}

bool UpdateInterface::copyFlaggedAssets()
{
  assets->setFilterFlags(UPDFLG_CopyDest);
  //reportProgress(tr("Asset filter applied: %1 Assets found: %2").arg(updateFlagsToString(UPDFLG_CopyDest)).arg(assets->count()), QtDebugMsg);

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
  //reportProgress(tr("Copy directory structure"), QtDebugMsg);

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
  //reportProgress(tr("Copy filter pattern: %1").arg(filter.pattern()), QtDebugMsg);

  QDirIterator itcnt(srcPath.path(), QDir::Files, QDirIterator::Subdirectories);

  cnt = 0;

  //reportProgress(tr("Calculating number of files"), QtDebugMsg);

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
  //reportProgress(tr("Copy files"), QtDebugMsg);

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
  //reportProgress(tr("Copy filter pattern: %1").arg(filter.pattern()), QtDebugMsg);

  QDirIterator itcnt(srcPath, QDir::Files, QDirIterator::Subdirectories);

  int cnt = 0;

  //reportProgress(tr("Calculating number of files"), QtDebugMsg);

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

  if (!releases->getSetId(params->updateRelease)) {
    reportProgress(tr("Set release id from update release '%1' failed").arg(params->updateRelease), QtCriticalMsg);
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

  for (int i = 0; i < params->assets.size(); i++) {
    const UpdateParameters::AssetParams & ap = params->assets.at(i);
    if (!getSetAssets(ap))
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
    progress->setMaximum(2);
  }

  if (progress)
    progress->setValue(++cnt);

  g.lastUpdateDir(params->updateDir);  //  used by sdsync

  if (params->flags & UPDFLG_DelDownloads) {
    reportProgress(tr("Delete download directory: %1").arg(downloadDir), QtDebugMsg);
    QDir d(downloadDir);
    if (!d.removeRecursively())
      reportProgress(tr("Failed to delete downloads folder %1").arg(downloadDir), QtCriticalMsg);
  }

  if (progress)
    progress->setValue(++cnt);

  if (params->flags & UPDFLG_DelDecompress) {
    if (!params->decompressDirUseDwnld) {
      reportProgress(tr("Delete decompress directory: %1").arg(decompressDir), QtDebugMsg);
      QDir d(decompressDir);
      if (!d.removeRecursively())
        reportProgress(tr("Failed to delete decompress folder %1").arg(decompressDir), QtCriticalMsg);
    }
    else if (!(params->flags & UPDFLG_DelDownloads)) {
      assets->setFilterFlags(UPDFLG_Decompress);
      for (int i = 0; i < assets->count(); i++) {
        assets->getSetId(i);
        QString path = QString("%1/A%2/%3").arg(decompressDir).arg(assets->id()).arg(QFileInfo(assets->filename()).completeBaseName());
        reportProgress(tr("Delete decompress folder: %1").arg(path), QtDebugMsg);
        //QDir d(path);
        if (!QDir(path).removeRecursively())
          reportProgress(tr("Failed to delete decompress folder %1").arg(path), QtCriticalMsg);
      }
    }
  }

  if (progress)
    progress->setValue(++cnt);

  return true;
}

//  as unique per component override if used
bool UpdateInterface::asyncInstall()
{
  return true;
}

bool UpdateInterface::saveReleaseSettings()
{
  reportProgress(tr("Save release settings"), QtDebugMsg);
  g.component[m_id].release(releases->name());
  g.component[m_id].version(releases->version());
  g.component[m_id].releaseId(releases->id());
  g.component[m_id].date(releases->date());

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
  foreach (UpdateFactoryInterface * registeredFactory, registeredUpdateFactories) {
    if (registeredFactory->id() == factory->id()) {
      qDebug() << "Duplicate factory - id:" << factory->id() << "name:" << factory->name();
      return;
    }
  }
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

UpdateInterface * UpdateFactories::interface(const int id)
{
  foreach (UpdateFactoryInterface * factory, registeredUpdateFactories) {
    if (id == factory->id())
      return factory->instance();
  }
  qDebug() << "Critical error - Interface not found for id:" << id;
  return nullptr;
}

const QString UpdateFactories::name(const int id)
{
  UpdateInterface * iface = interface(id);
  if (iface)
    return iface->name();

  return "";
}

void UpdateFactories::saveAssetSettings(const int id)
{
  UpdateInterface * iface = interface(id);
  if (iface)
    iface->saveAssetSettings();
}

UpdateParameters * const UpdateFactories::getParams(const int id)
{
  UpdateInterface * iface = interface(id);
  if (iface)
    return iface->getParams();

  return nullptr;
}

void UpdateFactories::resetEnvironment(const int id)
{
  UpdateInterface * iface = interface(id);
  if (iface)
    iface->resetEnvironment();
}

void UpdateFactories::resetAllEnvironments()
{
  foreach (UpdateFactoryInterface * factory, registeredUpdateFactories) {
    resetEnvironment(factory->id());
  }
}

void UpdateFactories::setRunUpdate(const int id)
{
  UpdateInterface * iface = interface(id);
  if (iface)
    iface->setRunUpdate();
}

const QMap<QString, int> UpdateFactories::sortedComponentsList(bool updateableOnly)
{
  QMap<QString, int> map;

  foreach (UpdateFactoryInterface * factory, registeredUpdateFactories) {
    if (updateableOnly && !factory->instance()->isUpdateable())
      continue;
    map.insert(factory->name(), factory->id());
  }

  return map;
}

void UpdateFactories::clearRelease(const int id)
{
  UpdateInterface * iface = interface(id);
  if (iface)
    iface->clearRelease();
}

void UpdateFactories::setReleaseChannel(const int id, int channel)
{
  UpdateInterface * iface = interface(id);
  if (iface)
    iface->setReleaseChannel(channel);
}

void UpdateFactories::setReleaseId(const int id, QString val)
{
  UpdateInterface * iface = interface(id);
  if (iface)
    iface->setReleaseId(val);
}

const QString UpdateFactories::currentRelease(const int id)
{
  UpdateInterface * iface = interface(id);
  if (iface)
    return iface->currentRelease();

  return "";
}

const QString UpdateFactories::updateRelease(const int id)
{
  UpdateInterface * iface = interface(id);
  if (iface)
    return iface->updateRelease();

  return "";
}

const bool UpdateFactories::isLatestRelease(const int id)
{
  UpdateInterface * iface = interface(id);
  if (iface)
    return iface->isLatestRelease();

  return true;
}

const QString UpdateFactories::latestRelease(const int id)
{
  UpdateInterface * iface = interface(id);
  if (iface)
    return iface->latestRelease();

  return "";
}

const QStringList UpdateFactories::releases(const int id)
{
  UpdateInterface * iface = interface(id);
  if (iface)
    return iface->getReleases();

  return QStringList();
}

bool UpdateFactories::update(const int id, ProgressWidget * progress)
{
  UpdateInterface * iface = interface(id);
  if (iface)
    return iface->update(progress);

  return false;
}

bool UpdateFactories::updateAll(ProgressWidget * progress)
{
  bool ret = false;

  foreach (UpdateFactoryInterface * factory, registeredUpdateFactories) {
    ret = factory->instance()->update(progress);
    if (!ret)
      break;
  }

  return ret;
}

const bool UpdateFactories::isUpdateAvailable(QMap<QString, int> & list)
{
  bool ret = false;

  list.clear();

  foreach (UpdateFactoryInterface * factory, registeredUpdateFactories) {
    if (factory->instance()->isUpdateable() && factory->instance()->isUpdateAvailable()) {
      list.insert(factory->name(), factory->id());
      ret = true;
    }
  }

  return ret;
}

bool UpdateFactories::getReleaseJsonAsset(const int id, const QString assetName, QJsonDocument * json)
{
  UpdateInterface * iface = interface(id);
  if (iface)
    return iface->getReleaseJsonAsset(assetName, json);

  return false;
}

bool UpdateFactories::getRepoJsonFile(const int id, const QString filename, QJsonDocument * json)
{
  UpdateInterface * iface = interface(id);
  if (iface)
    return iface->getRepoJsonFile(filename, json);

  return false;
}
