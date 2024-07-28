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

#include "process_sync.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QLabel>
#include <QToolButton>

class ProgressWidget;
class AutoComboBox;

class FileSyncDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(SyncProcess::SyncOptions syncOptions READ syncOptions WRITE setSyncOptions RESET resetOptions NOTIFY optionsWereReset)
    Q_PROPERTY(bool extraOptionsVisible READ extraOptionsVisible WRITE toggleExtraOptions NOTIFY extraOptionsToggled)
    Q_PROPERTY(QString folderNameA READ folderNameA WRITE setFolderNameA NOTIFY folderNameAChanged)
    Q_PROPERTY(QString folderNameB READ folderNameB WRITE setFolderNameB NOTIFY folderNameBChanged)

  public:
    FileSyncDialog(QWidget * parent = nullptr, const SyncProcess::SyncOptions & syncOptions = SyncProcess::SyncOptions());

    inline SyncProcess::SyncOptions syncOptions() const { return m_syncOptions; }
    void setSyncOptions(const SyncProcess::SyncOptions & syncOptions);
    void resetOptions();

    inline bool extraOptionsVisible() const { return m_extraOptionsVisible; }
    void toggleExtraOptions(bool show);

    inline QString folderNameA() const { return m_folderNameA; }
    void setFolderNameA(const QString & name);

    inline QString folderNameB() const { return m_folderNameB; }
    void setFolderNameB(const QString & name);

  public slots:
    virtual void reject() override;
    void setStatusText(const QString & text, int type = QtWarningMsg);
    bool validate();

  signals:
    void startSync();
    void stopSync();
    void folderNameAChanged(const QString & name);
    void folderNameBChanged(const QString & name);
    void extraOptionsToggled(bool show);
    void optionsWereReset();

  private slots:
    void setupUi();
    void toggleSync(bool);
    void onSyncStarted();
    void onSyncFinished();
    void updateRunningState();
    void onStatusUpdate(const SyncProcess::SyncStatus & stat);
    void adjustSizeDelayed();
    void setSyncDirList();

  private:
    SyncProcess::SyncOptions m_syncOptions;
    QString m_folderNameA;
    QString m_folderNameB;
    bool m_running;
    bool m_extraOptionsVisible;

    QGroupBox * ui_optionsPanel;
    QWidget * ui_extraOptions;
    ProgressWidget * ui_progress;
    QDialogButtonBox * ui_buttonBox;
    QToolButton * ui_btnStartStop;
    QToolButton * ui_btnClose;
    QLabel * ui_statusLabel;
    AutoComboBox * ui_syncDir;
};
