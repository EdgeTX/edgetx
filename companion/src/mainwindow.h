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

#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include <QtWidgets>
#include <QDateTime>
#include "downloaddialog.h"
#include "eeprominterface.h"

#define SPLASH_TIME 5

class MdiChild;
QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QMdiArea;
class QMdiSubWindow;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    friend class FirmwarePreferencesDialog;

    Q_OBJECT

  public:
    MainWindow();
   ~MainWindow();

  signals:
    void firmwareDownloadCompleted();
    void firmwareChanged();
    void startSync();
    void onInternalModuleChanged();

  protected:
    QString getCompanionUpdateBaseUrl() const;
    QString seekCodeString(const QByteArray & qba, const QString & label) const;

  protected slots:
    void dowloadLastFirmwareUpdate();
    void startFirmwareDownload();
    virtual void closeEvent(QCloseEvent *event);
    virtual void changeEvent(QEvent *e);
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dropEvent(QDropEvent *event);

  private slots:
    void openDocURL();
    void initWindowOptions();
    void retranslateUi(bool showMsg = false);

    void setLanguage(const QString & langString);
    void onLanguageChanged(QAction * act);
    void setTheme(int index);
    void onThemeChanged(QAction * act);
    void setIconThemeSize(int index);
    void onIconSizeChanged(QAction * act);
    void setTabbedWindows(bool on);
    void onChangeWindowAction(QAction * act);
    void updateWindowActions();
    void updateWindowActionTitle(const QMdiSubWindow * win, QAction * act = NULL);
    void onSubwindowTitleChanged();
    void onSubwindowModified();
    void onCurrentProfileChanged();

    void checkForUpdates();
    void checkForFirmwareUpdate();

    void checkForCompanionUpdateFinished(QNetworkReply * reply);
    void checkForFirmwareUpdateFinished(QNetworkReply * reply);

    void displayWarnings();
    void doAutoUpdates();
    void doUpdates();
    void updateDownloaded();
    void firmwareDownloadAccepted();
    void newFile();
    void openFile();
    void save();
    void saveAs();
    void saveAll();
    void closeFile();
    void openRecentFile();
    bool loadProfileId(const unsigned pid);
    void loadProfile();
    void logFile();
    void writeSettings();
    void readSettings();
    void writeFlash(QString fileToFlash="");
    void readFlash();
    void writeBackup();
    void readBackup();
    void burnConfig();
    void burnList();
    void sdsync();
    void changelog();
    void customizeSplash();
    void about();
    void compare();
    void appPrefs();
    void fwPrefs();
    void updateMenus();
    void createProfile();
    void copyProfile();
    void deleteProfile(const int pid);
    void deleteCurrentProfile();
    void exportSettings();
    void importSettings();
    void autoClose();
    void chooseProfile();

    void openUpdatesWaitDialog();
    void closeUpdatesWaitDialog();
    void onUpdatesError(const QString & err);
    void openFile(const QString & fileName, bool updateLastUsedDir = false);

  private:
    QAction * addAct(const QString & icon, const char * slot = NULL, const QKeySequence & shortcut = 0, QObject * slotObj = NULL, const char * signal = NULL);
    QAction * addActToGroup(QActionGroup * aGroup, const QString & sName, const QString & lName, const char * propName = 0,
                            const QVariant & propValue = QVariant(), const QVariant & dfltValue = QVariant(), const QKeySequence & shortcut = 0);
    void trAct(QAction * act, const QString & text, const QString & descript);

    QMenu * createLanguageMenu(QWidget * parent = Q_NULLPTR);

    void createActions();
    void createMenus();
    void createToolBars();
    void showReadyStatus();
    void updateRecentFileActions();
    void updateProfilesActions();

    int newProfile(bool loadProfile = true);
    QString strippedName(const QString & fullFileName);

    MdiChild * createMdiChild();
    MdiChild * activeMdiChild();
    QMdiSubWindow * findMdiChild(const QString & fileName);
    bool anyChildrenDirty();

    bool readSettingsFromRadio(const QString & filename);
    bool readFirmwareFromRadio(const QString & filename);

    bool checkProfileRadioExists(int profId);

    QMdiArea *mdiArea;

    QString installer_fileName;
    DownloadDialog * downloadDialog_forWait;
    unsigned int checkForUpdatesState;
    QString firmwareVersionString;

    QNetworkAccessManager *networkManager;

    QVector<QAction *> recentFileActs;
    QVector<QAction *> profileActs;
    QList<QAction *> fileWindowActions;

    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *settingsMenu;
    QMenu *burnMenu;
    QMenu *helpMenu;
    QMenu *windowMenu;
    QMenu *iconThemeSizeMenu;
    QMenu *themeMenu;
    QMenu *recentFilesMenu;
    QMenu *profilesMenu;
    QActionGroup * windowsListActions;
    QToolBar *fileToolBar;
    QToolBar *editToolBar;
    QToolBar *burnToolBar;
    QToolBar *helpToolBar;
    QAction *newAct;
    QAction *openAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *closeAct;
    QAction *recentFilesAct;
    QAction *exitAct;
    QAction *appPrefsAct;
    QAction *fwPrefsAct;
    QAction *checkForUpdatesAct;
    QAction *sdsyncAct;
    QAction *changelogAct;
    QAction *compareAct;
    QAction *editSplashAct;
    QAction *writeSettingsAct;
    QAction *readSettingsAct;
    QAction *burnConfigAct;
    QAction *burnListAct;
    QAction *writeFlashAct;
    QAction *readFlashAct;
    QAction *writeBUToRadioAct;
    QAction *readBUToFileAct;
    QAction *aboutAct;
    QAction *logsAct;
    QAction *profilesMenuAct;
    QAction *createProfileAct;
    QAction *copyProfileAct;
    QAction *deleteProfileAct;
    QAction *exportSettingsAct;
    QAction *importSettingsAct;
    QAction *openDocURLAct;
    QAction *actTabbedWindows;
    QAction *actTileWindows;
    QAction *actCascadeWindows;
    QAction *actCloseAllWindows;
};

#endif // _MAINWINDOW_H_
