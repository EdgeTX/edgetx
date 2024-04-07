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

#include "updatecloudbuild.h"
#include "repo.h"
#include "appdata.h"
#include "eeprominterface.h"

UpdateCloudBuild::UpdateCloudBuild(QWidget * parent) :
  UpdateInterface(parent, CID_CloudBuild, tr("CloudBuild"), Repo::REPO_TYPE_BUILD,
                  QString("https://cloudbuild.edgetx.org/api"), "nightly"),
  m_objBody(nullptr),
  m_docResp(nullptr)
{
  init(); // call after UpdateInterface ctor due to virtual functions
}

UpdateCloudBuild::~UpdateCloudBuild()
{
  if (m_objBody)
    delete m_objBody;
  if (m_docResp)
    delete m_docResp;
}

void UpdateCloudBuild::assetSettingsInit()
{
  if (!isSettingsIndexValid())
    return;

  g.component[id()].initAllAssets();

  ComponentAssetData &cad = g.component[id()].asset[0];

  cad.desc("binary");
  cad.processes((UPDFLG_Common_Asset | UPDFLG_Build) &~ UPDFLG_Decompress);
  cad.flags(cad.processes() | UPDFLG_CopyFiles | UPDFLG_Locked);
  cad.filterType(UpdateParameters::UFT_Exact);
  cad.filter("%FWFLAVOUR%");
  cad.destSubDir("FIRMWARE");
  cad.copyFilterType(UpdateParameters::UFT_Pattern);
  cad.copyFilter("^%FWFLAVOUR%-%LANGUAGE%.*\\.bin$");
  cad.maxExpected(1);

  qDebug() << "Asset settings initialised";
}

bool UpdateCloudBuild::buildFlaggedAsset(const int row)
{
  m_jobStatus.clear();

  m_radio = repo()->assets()->name(row);

  status()->progressMessage(tr("Building firmware target %1").arg(m_radio));

  QJsonObject target;

  if (!repo()->config()->value("targets").isUndefined() && repo()->config()->value("targets").isObject()) {
    const QJsonObject &targets = repo()->config()->value("targets").toObject();
    if (!targets.value(m_radio).isUndefined() && targets.value(m_radio).isObject()) { // use filtered Asset
      target = targets.value(m_radio).toObject();
    }
    else {
      status()->reportProgress(tr("No build support for target %1").arg(m_radio), QtCriticalMsg);
      return false;
    }
  }
  else {
    status()->reportProgress(tr("Unexpected format for build targets meta data"), QtCriticalMsg);
    return false;
  }

  //g.currentProfile()
  Firmware *fw = getCurrentFirmware();

  QJsonArray arrTags;

  if (!target.value("tags").isUndefined() && target.value("tags").isArray())
    arrTags = target.value("tags").toArray();
  else {
    status()->reportProgress(tr("Build target %1 has no valid tags").arg(m_radio), QtDebugMsg);
  }

  // add tags based on radio profile and do not duplicate defaults
  if (fw->getCapability(HasBluetooth) && !arrTags.contains(QJsonValue("bluetooth")))
    arrTags.append(QJsonValue("bluetooth"));

  //===============================
  //  TODO tags more to be added!!!
  //===============================

  QJsonObject objBuildFlags;
  m_buildFlags.clear();

  if (!target.value("build_flags").isUndefined() && target.value("build_flags").isObject())
    objBuildFlags = target.value("build_flags").toObject();

  // add language based on radio profile
  if (!repo()->config()->value("flags").isUndefined() && repo()->config()->value("flags").isObject()) {
    const QJsonObject &flags = repo()->config()->value("flags").toObject();
    if (!flags.value("language").isUndefined() && flags.value("language").isObject()) {
      const QJsonObject &language = flags.value("language").toObject();
      if (language.value("build_flag").isUndefined() || !language.value("build_flag").isString()) {
        status()->reportProgress(tr("Language build_flag value not found or format unsupported"), QtCriticalMsg);
        return false;
      }
      if (language.value("values").isUndefined() || !language.value("values").isArray()) {
        status()->reportProgress(tr("Language values not found or format unsupported"), QtCriticalMsg);
        return false;
      }
      if (language.value("values").toArray().contains(params()->language.toUpper())) {
        objBuildFlags.insert(language.value("build_flag").toString(), QJsonValue(params()->language.toUpper()));
        m_buildFlags.append(QString("-%1").arg(params()->language.toLower()));
      }
      else {
        status()->reportProgress(tr("Radio profile language %1 not supported").arg(params()->language.toUpper()), QtCriticalMsg);
        return false;
      }
    }
    else {
      status()->reportProgress(tr("Language build flag not found or format unsupported"), QtCriticalMsg);
      return false;
    }
  }
  else {
    status()->reportProgress(tr("No build flags found or format unsupported"), QtCriticalMsg);
    return false;
  }

  if (m_objBody)
    delete m_objBody;

  m_objBody = new QJsonObject();
  m_objBody->insert("release", QJsonValue(params()->releaseUpdate));
  m_objBody->insert("target", QJsonValue(m_radio));
  m_objBody->insert("tags", QJsonValue(arrTags));
  m_objBody->insert("build_flags", QJsonValue(objBuildFlags));

  QJsonDocument *docBody = new QJsonDocument(*m_objBody);

  if (m_docResp)
    delete m_docResp;

  m_docResp = new QJsonDocument();

  network()->submitRequest(tr("Submit firmware build"), repo()->urlJobs(), docBody, m_docResp);

  if (!network()->isSuccess()) {
    status()->reportProgress(tr("Failed to initiate build job"), QtCriticalMsg);
    return false;
  }

  if (m_docResp->isObject()) {
    const QJsonObject &obj = m_docResp->object();

    if (!obj.value("error").isUndefined()) {
      status()->reportProgress(tr("Build error: %1").arg(obj.value("error").toString()), QtCriticalMsg);
      return false;
    }
    else if (obj.value("target").isUndefined() ||
             (!obj.value("target").isUndefined() && obj.value("target").toString() != m_radio) ||
             obj.value("release").isUndefined() ||
             (!obj.value("release").isUndefined() && obj.value("release").toString() != params()->releaseUpdate)) {
      status()->reportProgress(tr("Process status not returned when submitting build job"), QtCriticalMsg);
      return false;
    }
    else if (!obj.value("status").isUndefined() && obj.value("status").isString()) {
      m_jobStatus = obj.value("status").toString();
    }
  }
  else {
    status()->reportProgress(tr("Unexpected response when submitting build job"), QtCriticalMsg);
    return false;
  }

  if (isStatusInProgress())
    waitForBuildFinish();

  status()->reportProgress(tr("Firmware build finished with status %1").arg(m_jobStatus), QtDebugMsg);

  if (m_jobStatus == "BUILD_SUCCESS" && setAssetDownload())
    return true;;

  return false;
}

void UpdateCloudBuild::cancel()
{
  cleanup();
  status()->reportProgress(tr("Build firmware cancelled"), QtWarningMsg);
}

void UpdateCloudBuild::checkStatus()
{
  getStatus();

  if (!isStatusInProgress())
    cleanup();
}

void UpdateCloudBuild::cleanup()
{
  if (m_timer.isActive())
    m_timer.stop();

  if (m_eventLoop.isRunning())
   m_eventLoop.quit();
}

bool UpdateCloudBuild::getStatus()
{
  QJsonDocument *docBody = new QJsonDocument(*m_objBody);

  if (m_docResp)
    delete m_docResp;

  m_docResp = new QJsonDocument();
  network()->submitRequest(tr("Submit get build status"), repo()->urlStatus(), docBody, m_docResp);

  if (m_docResp->isObject()) {
    const QJsonObject &obj = m_docResp->object();

    if (!obj.value("status").isUndefined() && obj.value("status").isString()) {
      m_jobStatus = obj.value("status").toString();
    }
  }
  else {
    m_jobStatus = "STATUS_UNKNOWN";
    status()->reportProgress(tr("Build status not returned"), QtCriticalMsg);
    return false;
  }

  return true;
}

bool UpdateCloudBuild::isStatusInProgress()
{
  if (m_jobStatus == "WAITING_FOR_BUILD" || m_jobStatus == "BUILD_IN_PROGRESS")
    return true;

  return false;
}

bool UpdateCloudBuild::setAssetDownload()
{
  QString name = QString("%1%2-%3.bin").arg(m_radio).arg(m_buildFlags/* has a leading hyphen*/).arg(repo()->releases()->name());

  repo()->assets()->setDownloadName(name);

  const QJsonObject &obj = m_docResp->object();

  if (!obj.value("artifacts").isUndefined() && obj.value("artifacts").isArray()) {
    const QJsonArray &artifacts = obj.value("artifacts").toArray();
    if (artifacts.count() > 1) {
      status()->reportProgress(tr("Build status contains %1 artifact when only 1 expected").arg(artifacts.count()), QtWarningMsg);
    }
    for (int i = 0; i < artifacts.count(); i++) {
      const QJsonObject &artifact = artifacts[i].toObject();
      if (!artifact.value("slug").isUndefined() && artifact.value("slug").isString() && artifact.value("slug").toString() == "firmware") {
        if (!artifact.value("download_url").isUndefined() && artifact.value("download_url").isString()) {
          repo()->assets()->setDownloadUrl(artifact.value("download_url").toString());
        }
        else {
          status()->reportProgress(tr("Build status does not contain download url"), QtCriticalMsg);
          return false;
        }
      }
      else {
        status()->reportProgress(tr("Build status does not contain firmware artifact"), QtCriticalMsg);
        return false;
      }
    }
  }
  else {
    status()->reportProgress(tr("Build status does not contain artifacts"), QtCriticalMsg);
    return false;
  }

  return true;
}

void UpdateCloudBuild::waitForBuildFinish()
{
  status()->progressMessage(tr("Waiting for firmware build to finish..."));
  m_timer.setInterval(2000);  // check status every 2 seconds

  connect(this, &UpdateInterface::stop, this, &UpdateCloudBuild::cancel);
  connect(&m_timer, &QTimer::timeout, this, &UpdateCloudBuild::checkStatus);

  m_timer.start();
  m_eventLoop.exec();

  disconnect(this, &UpdateInterface::stop, this, &UpdateCloudBuild::cancel);
  disconnect(&m_timer, nullptr);
}
