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

#include <QtWidgets>
#include "wizarddata.h"

class WizardDialog : public QWizard
{
  Q_OBJECT

  public:
    WizMix mix;
    WizardDialog(const GeneralSettings & settings, const unsigned int modelId, const ModelData & modelData, QWidget *parent = 0);
    const GeneralSettings & settings;

  private slots:
    void showHelp();
};

class StandardPage: public QWizardPage
{
  Q_OBJECT

  public:
    StandardPage(WizardPage curPage, WizardDialog *dlg, QString image, QString title, QString text, int nextPage=-1);
    WizardDialog *wizDlg;

  protected:
    void releaseBookings();
    void releasePreBookings();
    void initializePage();
    void cleanupPage();
    int getDefaultChannel(const Input input);
    int nextFreeChannel(int channel = 0);
    void populateCB(QComboBox * cb, int preferred=-1);
    bool bookChannel(QComboBox * cb, Input input1, int weight1, Input input2=NO_INPUT, int weight2=0);
    int totalChannelsAvailable();

  private:
    QLabel *topLabel;
    WizardPage pageCurrent;
    int pageFollower;
    int nextId() const;
};

class ModelSelectionPage: public StandardPage
{
  Q_OBJECT
public:
  ModelSelectionPage(WizardDialog *dlg, QString image, QString title, QString text);
private:
  QLineEdit *nameLineEdit;
  QRadioButton *planeRB;
  QRadioButton *multirotorRB;
  QRadioButton *helicopterRB;
  void initializePage();
  bool validatePage();
  int nextId() const;
};

class WingtypeSelectionPage: public StandardPage
{
  Q_OBJECT
public:
  WingtypeSelectionPage(WizardDialog *dlg, QString image, QString title, QString text);
private:
  QRadioButton *deltaWingRB;
  QRadioButton *standardWingRB;
  int nextId() const;
};

class TailSelectionPage: public StandardPage
{
  Q_OBJECT
public:
  TailSelectionPage(WizardDialog *dlg, QString image, QString title, QString text);
private:
  QRadioButton *vTailRB;
  QRadioButton *standardTailRB;
  QRadioButton *simpleTailRB;
  int nextId() const;
};

class FlybarSelectionPage: public StandardPage
{
  Q_OBJECT
public:
  FlybarSelectionPage(WizardDialog *dlg, QString image, QString title, QString text);
private:
  QRadioButton *flybarRB;
  QRadioButton *noFlybarRB;
  int nextId() const;
};

class ThrottlePage: public StandardPage
{
  Q_OBJECT

  public:
    ThrottlePage(WizardDialog *dlg, QString image, QString title, QString text, int nextPage=-1);
    void initializePage();
    bool validatePage();

  protected slots:
    void onMotorStateChanged(bool toggled);

  private:
    QRadioButton *motorRB;
    QRadioButton *noMotorRB;
    QComboBox *throttleCB;
};

class AileronsPage: public StandardPage
{
  Q_OBJECT
public:
  AileronsPage(WizardDialog *dlg, QString image, QString title, QString text, int nextPage=-1);
  void initializePage();
  bool validatePage();

protected slots:
  void noAileronChannel();
  void oneAileronChannel();
  void twoAileronChannels();

private:
  QRadioButton *oneAileronRB;
  QRadioButton *twoAileronsRB;
  QRadioButton *noAileronsRB;
  QComboBox *aileron1CB;
  QComboBox *aileron2CB;
};

class FlapsPage: public StandardPage
{
  Q_OBJECT
public:
  FlapsPage(WizardDialog *dlg, QString image, QString title, QString text, int nextPage=-1);
  void initializePage();
  bool validatePage();

protected slots:
  void noFlapChannel();
  void oneFlapChannel();
  void twoFlapChannels();

private:
  QRadioButton *oneFlapRB;
  QRadioButton *twoFlapsRB;
  QRadioButton *noFlapsRB;
  QComboBox *flap1CB;
  QComboBox *flap2CB;
};

class AirbrakesPage: public StandardPage
{
  Q_OBJECT
public:
  AirbrakesPage(WizardDialog *dlg, QString image, QString title, QString text, int nextPage=-1);
  void initializePage();
  bool validatePage();

protected slots:
  void noAirbrakeChannel();
  void oneAirbrakeChannel();
  void twoAirbrakeChannels();

private:
  QRadioButton *oneAirbrakeRB;
  QRadioButton *twoAirbrakesRB;
  QRadioButton *noAirbrakesRB;
  QComboBox *airbrake1CB;
  QComboBox *airbrake2CB;
};

class ElevonsPage: public StandardPage
{
  Q_OBJECT

  public:
    ElevonsPage(WizardDialog *dlg, QString image, QString title, QString text, int nextPage=-1);
    void initializePage();
    bool validatePage();

  private:
    QComboBox *elevon1CB;
    QComboBox *elevon2CB;
};

class RudderPage: public StandardPage
{
  Q_OBJECT

  public:
    RudderPage(WizardDialog *dlg, QString image, QString title, QString text, int nextPage=-1);
    void initializePage();
    bool validatePage();

    protected slots:
    void noRudder();
    void hasRudder();

  private:
    QRadioButton *noRudderRB;
    QRadioButton *hasRudderRB;
    QComboBox *rudderCB;
};

class TailPage: public StandardPage
{
  Q_OBJECT
public:
  TailPage(WizardDialog *dlg, QString image, QString title, QString text, int nextPage=-1);
  void initializePage();
  void cleanupPage();
  bool validatePage();
private:
  QComboBox *elevatorCB;
  QComboBox *rudderCB;
  QLayout *l;
  QLabel *errorMessage;
};

class VTailPage: public StandardPage
{
  Q_OBJECT
public:
  VTailPage(WizardDialog *dlg, QString image, QString title, QString text, int nextPage=-1);
  void initializePage();
  bool validatePage();
private:
  QComboBox *tail1CB;
  QComboBox *tail2CB;
};

class SimpleTailPage: public StandardPage
{
  Q_OBJECT
public:
  SimpleTailPage(WizardDialog *dlg, QString image, QString title, QString text, int nextPage=-1);
  void initializePage();
  bool validatePage();
private:
  QComboBox *elevatorCB;
};

class CyclicPage: public StandardPage
{
  Q_OBJECT
public:
  CyclicPage(WizardDialog *dlg, QString image, QString title, QString text, int nextPage=-1);
  void initializePage();
  bool validatePage();
private:
  QRadioButton *cyclic90RB;
  QRadioButton *cyclic120RB;
  QRadioButton *cyclic120XRB;
  QRadioButton *cyclic140RB;
};

class GyroPage: public StandardPage
{
  Q_OBJECT
public:
  GyroPage(WizardDialog *dlg, QString image, QString title, QString text, int nextPage=-1);
  void initializePage();
  bool validatePage();
private:
  QRadioButton *noGyroRB;
  QRadioButton *switchGyroRB;
  QRadioButton *potGyroRB;
};

class FblPage: public StandardPage
{
  Q_OBJECT
public:
  FblPage(WizardDialog *dlg, QString image, QString title, QString text, int nextPage=-1);
  void initializePage();
  bool validatePage();
private:
  QComboBox *throttleCB;
  QComboBox *yawCB;
  QComboBox *pitchCB;
  QComboBox *rollCB;
};

class HeliPage: public StandardPage
{
  Q_OBJECT
public:
  HeliPage(WizardDialog *dlg, QString image, QString title, QString text, int nextPage=-1);
  void initializePage();
  bool validatePage();
private:
  QComboBox *throttleCB;
  QComboBox *yawCB;
  QComboBox *pitchCB;
  QComboBox *rollCB;
};

class MultirotorPage: public StandardPage
{
  Q_OBJECT
public:
  MultirotorPage(WizardDialog *dlg, QString image, QString title, QString text, int nextPage=-1);
  void initializePage();
  bool validatePage();
private:
  QComboBox *throttleCB;
  QComboBox *yawCB;
  QComboBox *pitchCB;
  QComboBox *rollCB;
};

class OptionsPage: public StandardPage
{
  Q_OBJECT
public:
  OptionsPage(WizardDialog *dlg, QString image, QString title, QString text, int nextPage=-1);
  void initializePage();
  bool validatePage();
private:
  QCheckBox *throttleCutRB;
  QCheckBox *flightTimerRB;
  QCheckBox *throttleTimerRB;
};

class ConclusionPage: public StandardPage
{
  Q_OBJECT
public:
  ConclusionPage(WizardDialog *dlg, QString image, QString title, QString text, int nextPage=-1);
  void initializePage();
  bool validatePage();
private:
  QCheckBox *proceedCB;
  QLabel *textLabel;
};

class WizardPrinter:QObject
{
  Q_OBJECT
public:
  WizardPrinter( WizMix * );
  QString print();
private:
  WizMix *mix;
  QString inputName( Input );
  QString vehicleName( Vehicle );
  QString printChannel( Input, int, Input, int );
};
