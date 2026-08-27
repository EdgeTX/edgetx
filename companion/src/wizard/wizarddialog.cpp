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

#include "wizarddialog.h"
#include "helpers.h"
#include "namevalidator.h"

WizardDialog::WizardDialog(const GeneralSettings & settings, unsigned int modelId, const ModelData & modelData, QWidget *parent):
  QWizard(parent),
  mix(settings, modelId, modelData),
  settings(settings)
{
  setWindowIcon(CompanionIcon("wizard.png"));
  setWindowTitle(tr("Model Wizard"));

  setPage(Page_Models, new ModelSelectionPage(this, "models", tr("Model Type"), tr("Enter model name and model type.")));
  setPage(Page_Throttle, new ThrottlePage(this, "throttle", tr("Throttle"), tr("Has your model got a motor or an engine?"), Page_Wingtypes));
  setPage(Page_Wingtypes, new WingtypeSelectionPage(this, "wingtype", tr("Wing Type"), tr("Is your model a flying wing/deltawing or has it a standard wing configuration?")));
  setPage(Page_Ailerons, new AileronsPage(this, "ailerons", tr("Ailerons"), tr("Has your model got ailerons?"), Page_Flaps));
  setPage(Page_Flaps, new FlapsPage(this, "flaps", tr("Flaps"), tr("Has your model got flaps?"), Page_Airbrakes));
  setPage(Page_Airbrakes, new AirbrakesPage(this, "airbrakes", tr("Airbrakes"), tr("Has your model got airbrakes?"), Page_Tails));
  setPage(Page_Elevons, new ElevonsPage(this, "elevons", tr("Flying-wing / Delta-wing"), tr("Select the elevons channels"), Page_Rudder));
  setPage(Page_Rudder, new RudderPage(this, "rudder", tr("Rudder"), tr("Does your model have a rudder?"), Page_Options));
  setPage(Page_Tails, new TailSelectionPage(this, "tails", tr("Tail Type"), tr("Select which type of tail your model is equiped with.")));
  setPage(Page_Tail, new TailPage(this, "tail", tr("Tail"), tr("Select channels for tail control."), Page_Options));
  setPage(Page_Vtail, new VTailPage(this, "vtail", tr("V-Tail"), tr("Select channels for tail control."), Page_Options));
  setPage(Page_Simpletail, new SimpleTailPage(this, "simpletail", tr("Tail"), tr("Select elevator channel."), Page_Options));
  setPage(Page_Cyclic, new CyclicPage(this, "cyclic", tr("Cyclic"), tr("Which type of swash control is installed in your helicopter?"), Page_Gyro));
  setPage(Page_Gyro, new GyroPage(this, "gyro", tr("Tail Gyro"), tr("Has your helicopter got an adjustable gyro for the tail?"), Page_Flybar));
  setPage(Page_Flybar, new FlybarSelectionPage(this, "flybar", tr("Rotor Type"), tr("Has your helicopter got a flybar?")));
  setPage(Page_Fblheli, new FblPage(this, "fblheli", tr("Helicopter"), tr("Select the controls for your helicopter"), Page_Options));
  setPage(Page_Helictrl, new HeliPage(this, "helictrl", tr("Helicopter"), tr("Select the controls for your helicopter"), Page_Options));
  setPage(Page_Multirotor, new MultirotorPage(this, "multirotor", tr("Multirotor"), tr("Select the control channels for your multirotor"), Page_Options));
  setPage(Page_Options, new OptionsPage(this, "options", tr("Model Options"), tr("Select additional options"), Page_Conclusion));
  setPage(Page_Conclusion, new ConclusionPage(this, "conclusion", tr("Save Changes"), tr(
    "Manually check the direction of each control surface and reverse any channels that make controls move in the wrong direction. "
    "Remove the propeller/propellers before you try to control your model for the first time.<br>"
    "Please note that continuing removes all old model settings!"), -1));
  setStartId(Page_Models);

  // Workaround for Qt radio button rendering issue on Mac
  Q_FOREACH(QAbstractButton * rb, findChildren<QAbstractButton *>()) {
    rb->setAttribute(Qt::WA_LayoutUsesWidgetRect);
  }

  setWizardStyle(ModernStyle);
  setOption(QWizard::NoCancelButton, false);
  setOption(HaveHelpButton, true);
  connect(this, SIGNAL(helpRequested()), this, SLOT(showHelp()));
}

void WizardDialog::showHelp()
{
  QString message;

  switch (currentId()) {
    case Page_Models:
      message = tr("Enter a name for your model and select model type.");
      break;
    case Page_Throttle:
      message = tr("Select the receiver channel that is connected to your ESC or throttle servo.<br><br>"
                   "Throttle - Spektrum: CH1, Futaba: CH3");
      break;
    case Page_Wingtypes:
      message = tr("Most aircraft have a main wing and a tail with control surfaces. Flying wings and delta winged aircraft only have a single wing. "
                   "The main control surface on a standard wing controls the roll of the aircraft. This surface is called an aileron.<br>"
                   "The control surface of a delta wing controls both roll and pitch. This surface is called an elevon. ");
      break;
    case Page_Ailerons:
      message = tr("Models use one or two channels to control the ailerons.<br>"
                   "A so called Y-cable can be used to connect a single receiver channel to two separate aileron servos. "
                   "If your servos are connected by a Y-cable you should select the single-servo option.<br><br>"
                   "Aileron - Spektrum: CH2, Futaba: CH1");
      break;
    case Page_Flaps:
      message = tr("This wizard assumes that your flaps are controlled by a switch. "
                   "If your flaps are controlled by a potentiometer you can change that manually later.");
      break;
    case Page_Airbrakes:
      message = tr("Air brakes are used to reduce the speed of advanced sail planes.<br>"
                   "They are very uncommon on other types of planes.");
      break;
    case Page_Elevons:
      message = tr("Models use two channels to control the elevons.<br>"
                   "Select these two channels");
      break;
    case Page_Rudder:
      message = tr("Select the receiver channel that is connected to your rudder.<br><br>"
                   "Rudder - Spektrum: CH4, Futaba: CH4");
      break;
    case Page_Tails:
      message = tr("Select the tail type of your plane.");
      break;
    case Page_Tail:
      message = tr("Select the Rudder and Elevator channels.<br><br>"
                   "Rudder - Spektrum: CH4, Futaba: CH4<br>"
                   "Elevator - Spektrum: CH3, Futaba: CH2");
      break;
    case Page_Vtail:
      message = tr("Select the Rudder and Elevator channels.<br><br>"
                   "Rudder - Spektrum: CH4, Futaba: CH4<br>"
                   "Elevator - Spektrum: CH3, Futaba: CH2");
      break;
    case Page_Simpletail:
      message = tr("Select the Elevator channel.<br><br>"
                   "Elevator - Spektrum: CH3, Futaba: CH2");
      break;
    case Page_Flybar:
      message = tr("TBD.");
      break;
    case Page_Cyclic:
      message = tr("TBD.");
      break;
    case Page_Gyro:
      message = tr("TBD.");
      break;
    case Page_Fblheli:
      message = tr("TBD.");
      break;
    case Page_Helictrl:
      message = tr("TBD.");
      break;
    case Page_Multirotor:
      message = tr("Select the control channels for your multirotor.<br><br>"
                   "Throttle - Spektrum: CH1, Futaba: CH3<br>"
                   "Yaw - Spektrum: CH4, Futaba: CH4<br>"
                   "Pitch - Spektrum: CH3, Futaba: CH2<br>"
                   "Roll - Spektrum: CH2, Futaba: CH1");
      break;
    case Page_Options:
      message = tr("TBD.");
      break;
    case Page_Conclusion:
      message = tr("TBD.");
      break;
    default:
      message = tr("There is no help available for the current page.");
  }

  QMessageBox::information(this, tr("Model Wizard Help"), message);
}

StandardPage::StandardPage(WizardPage currentPage, WizardDialog *dlg, QString image, QString title, QString text, int nextPage):
  QWizardPage(),
  wizDlg(dlg),
  pageCurrent(currentPage),
  pageFollower(nextPage)
{
  setTitle(title);
  setPixmap(QWizard::WatermarkPixmap, QPixmap(QString(":/images/wizard/%1.png").arg(image)));
  topLabel = new QLabel(text+"<br>");
  topLabel->setWordWrap(true);

  QVBoxLayout *layout = new QVBoxLayout;
  layout->addWidget(topLabel);
  setLayout(layout);
}

int StandardPage::getDefaultChannel(const Input input)
{
  return wizDlg->settings.getDefaultChannel(input-1);
}

int StandardPage::nextFreeChannel(int channel)
{
  for(int i=channel;i<8; i++)
    if (wizDlg->mix.channel[i].page == Page_None)
      return i;
  return -1;
}

int StandardPage::totalChannelsAvailable()
{
  int c = 0;
  for(int i=0; i<WIZ_MAX_CHANNELS; i++)
    if (wizDlg->mix.channel[i].page == Page_None)
      c++;

  return c;
}

void StandardPage::populateCB(QComboBox *cb, int preferred)
{
  cb->clear();

  // Add all unbooked channels and select the preferred one
  for (int i=0; i<WIZ_MAX_CHANNELS; i++) {
    if (wizDlg->mix.channel[i].page == Page_None) {
      cb->addItem(tr("Channel %1").arg(i+1), i);
      if (preferred == i) {
        cb->setCurrentIndex(cb->count()-1);
      }
    }
  }
  // Avoid selecting prebooked channels
  int channel = cb->itemData(cb->currentIndex()).toInt();
  if (wizDlg->mix.channel[channel].prebooked) {
    for (int i=cb->count()-1; i>=0; i--){
      channel = cb->itemData(i).toInt();
      if (!wizDlg->mix.channel[channel].prebooked) {
        cb->setCurrentIndex(i);
        break;
      }
    }
  }
  // Prebook the selected channel
  channel = cb->itemData(cb->currentIndex()).toInt();
  wizDlg->mix.channel[channel].prebooked = true;
}

bool StandardPage::bookChannel(QComboBox * cb, Input input1, int weight1, Input input2, int weight2 )
{
  int channel = cb->itemData(cb->currentIndex()).toInt();
  if (channel<0 || channel>=WIZ_MAX_CHANNELS)
    return false;
  if (wizDlg->mix.channel[channel].page != Page_None)
    return false;

  wizDlg->mix.channel[channel].page = pageCurrent;
  wizDlg->mix.channel[channel].input1 = input1;
  wizDlg->mix.channel[channel].input2 = input2;
  wizDlg->mix.channel[channel].weight1 = weight1;
  wizDlg->mix.channel[channel].weight2 = weight2;

  return true;
}

void StandardPage::releaseBookings()
{
  for (int i=0; i<WIZ_MAX_CHANNELS; i++) {
    if (wizDlg->mix.channel[i].page == pageCurrent) {
      wizDlg->mix.channel[i].clear();
    }
  }
}

void StandardPage::releasePreBookings()
{
  for (int i=0; i<WIZ_MAX_CHANNELS; i++) {
    wizDlg->mix.channel[i].prebooked = false;
  }
}

void StandardPage::initializePage()
{
  releasePreBookings(); // All prebookings are void after page has initialized
}

void StandardPage::cleanupPage()
{
  releaseBookings();
}

int StandardPage::nextId() const
{
  return pageFollower;
}

ModelSelectionPage::ModelSelectionPage(WizardDialog *dlg, QString image, QString title, QString text)
  : StandardPage(Page_Models, dlg, image, title, text)
{
  Firmware *fw = getCurrentFirmware();
  nameLineEdit = new QLineEdit;
  nameLineEdit->setValidator(new NameValidator(fw->getBoard()));
  nameLineEdit->setMaxLength(fw->getCapability(ModelName));
  planeRB = new QRadioButton(tr("Plane"));
  planeRB->setChecked(true);
  multirotorRB = new QRadioButton(tr("Multirotor"));
  helicopterRB = new QRadioButton(tr("Helicopter"));

  registerField("evaluate.name*", nameLineEdit);

  QLayout *l = layout();
  l->addWidget(new QLabel(tr("Model Name:")));
  l->addWidget(nameLineEdit);
  l->addWidget(new QLabel(""));
  l->addWidget(new QLabel(tr("Model Type:")));
  l->addWidget(planeRB);
  l->addWidget(multirotorRB);
  l->addWidget(helicopterRB);
}

void ModelSelectionPage::initializePage()
{
  nameLineEdit->setText(wizDlg->mix.name);
}

bool ModelSelectionPage::validatePage()
{
  memset(wizDlg->mix.name, 0, sizeof(wizDlg->mix.name));
  strncpy( wizDlg->mix.name, nameLineEdit->text().toLatin1(), sizeof(wizDlg->mix.name) - 1);

  if (multirotorRB->isChecked())
    wizDlg->mix.vehicle = MULTICOPTER;
  else if (helicopterRB->isChecked())
    wizDlg->mix.vehicle = HELICOPTER;
  else
    wizDlg->mix.vehicle = PLANE;
  return true;
}

int ModelSelectionPage::nextId() const
{
  if (helicopterRB->isChecked())
    return Page_Cyclic;
  else if (multirotorRB->isChecked())
    return Page_Multirotor;
  else
    return Page_Throttle;
}

WingtypeSelectionPage::WingtypeSelectionPage(WizardDialog *dlg, QString image, QString title, QString text)
  : StandardPage(Page_Wingtypes, dlg, image, title, text)
{
  standardWingRB = new QRadioButton(tr("Standard Wing"));
  standardWingRB->setChecked(true);
  deltaWingRB = new QRadioButton(tr("Flying Wing / Deltawing"));

  QLayout *l = layout();
  l->addWidget(standardWingRB);
  l->addWidget(deltaWingRB);
}

int WingtypeSelectionPage::nextId() const
{
  if (deltaWingRB->isChecked())
    return Page_Elevons;
  else
    return Page_Ailerons;
}

TailSelectionPage::TailSelectionPage(WizardDialog *dlg, QString image, QString title, QString text)
  : StandardPage(Page_Tails, dlg, image, title, text)
{
  standardTailRB = new QRadioButton(tr("Elevator and Rudder"));
  standardTailRB->setChecked(true);
  simpleTailRB = new QRadioButton(tr("Only Elevator"));
  vTailRB = new QRadioButton(tr("V-tail"));

  QLayout *l = layout();
  l->addWidget(new QLabel(tr("Tail Type:")));
  l->addWidget(standardTailRB);
  l->addWidget(simpleTailRB);
  l->addWidget(vTailRB);
}

int TailSelectionPage::nextId() const
{
  if (simpleTailRB->isChecked())
    return Page_Simpletail;
  else if (vTailRB->isChecked())
    return Page_Vtail;
  else
    return Page_Tail;
}

FlybarSelectionPage::FlybarSelectionPage(WizardDialog *dlg, QString image, QString title, QString text)
  : StandardPage(Page_Flybar, dlg, image, title, text)
{
  flybarRB = new QRadioButton(tr("Has Flybar"));
  flybarRB->setChecked(true);
  noFlybarRB = new QRadioButton(tr("Flybarless"));

  QLayout *l = layout();
  l->addWidget(new QLabel(tr("Flybar:")));
  l->addWidget(flybarRB);
  l->addWidget(noFlybarRB);
}

int FlybarSelectionPage::nextId() const
{
  if (flybarRB->isChecked())
    return Page_Helictrl;
  else
    return Page_Fblheli;
}

ThrottlePage::ThrottlePage(WizardDialog *dlg, QString image, QString title, QString text, int nextPage)
  : StandardPage(Page_Throttle, dlg, image, title, text, nextPage)
{
  motorRB = new QRadioButton(tr("Yes"));
  noMotorRB = new QRadioButton(tr("No"));
  motorRB->setChecked(true);
  throttleCB = new QComboBox();

  QLayout *l = layout();
  l->addWidget(motorRB);
  l->addWidget(noMotorRB);
  l->addWidget(new QLabel(tr("<br>Throttle Channel:")));
  l->addWidget(throttleCB);

  connect(motorRB, SIGNAL(toggled(bool)), this, SLOT(onMotorStateChanged(bool)));
}

void ThrottlePage::initializePage()
{
  populateCB(throttleCB, getDefaultChannel(THROTTLE_INPUT));
  StandardPage::initializePage();
}

bool ThrottlePage::validatePage()
{
  releaseBookings();
  if (motorRB->isChecked())
    return bookChannel(throttleCB, THROTTLE_INPUT, 100);
  else
    return true;
}

void ThrottlePage::onMotorStateChanged(bool toggled)
{
  throttleCB->setEnabled(toggled);
}

AileronsPage::AileronsPage(WizardDialog *dlg, QString image, QString title, QString text, int nextPage)
  : StandardPage(Page_Ailerons, dlg, image, title, text, nextPage)
{
  noAileronsRB = new QRadioButton(tr("No"));
  oneAileronRB = new QRadioButton(tr("Yes, controlled by a single channel"));
  twoAileronsRB = new QRadioButton(tr("Yes, controlled by two channels"));
  noAileronsRB->setChecked(true);

  aileron1CB = new QComboBox();
  aileron2CB = new QComboBox();
  aileron1CB->setEnabled(false);
  aileron2CB->setEnabled(false);

  QLayout *l = layout();
  l->addWidget(noAileronsRB);
  l->addWidget(oneAileronRB);
  l->addWidget(twoAileronsRB);
  l->addWidget(new QLabel(tr("<br>First Aileron Channel:")));
  l->addWidget(aileron1CB);
  l->addWidget(new QLabel(tr("Second Aileron Channel:")));
  l->addWidget(aileron2CB);

  connect(noAileronsRB, SIGNAL(toggled(bool)), this, SLOT(noAileronChannel()));
  connect(oneAileronRB, SIGNAL(toggled(bool)), this, SLOT(oneAileronChannel()));
  connect(twoAileronsRB, SIGNAL(toggled(bool)), this, SLOT(twoAileronChannels()));
}

void AileronsPage::initializePage()
{
  populateCB(aileron1CB, getDefaultChannel(AILERONS_INPUT));
  populateCB(aileron2CB, nextFreeChannel(4));
  StandardPage::initializePage();
}

bool AileronsPage::validatePage()
{
  releaseBookings();
  if (noAileronsRB->isChecked()) {
    return true;
  }
  if (oneAileronRB->isChecked()) {
    return (bookChannel(aileron1CB, AILERONS_INPUT, 100 ));
  }
  return( bookChannel(aileron1CB, AILERONS_INPUT, 100 ) &&
    bookChannel(aileron2CB, AILERONS_INPUT, -100 ));
}

void AileronsPage::noAileronChannel()
{
  aileron1CB->setEnabled(false);
  aileron2CB->setEnabled(false);
}

void AileronsPage::oneAileronChannel()
{
  aileron1CB->setEnabled(true);
  aileron2CB->setEnabled(false);
}

void AileronsPage::twoAileronChannels()
{
  aileron1CB->setEnabled(true);
  aileron2CB->setEnabled(true);
}

FlapsPage::FlapsPage(WizardDialog *dlg, QString image, QString title, QString text, int nextPage):
  StandardPage(Page_Flaps, dlg, image, title, text, nextPage)
{
  noFlapsRB = new QRadioButton(tr("No"));
  oneFlapRB = new QRadioButton(tr("Yes, controlled by a single channel"));
  twoFlapsRB = new QRadioButton(tr("Yes, controlled by two channels"));
  noFlapsRB->setChecked(true);

  flap1CB = new QComboBox();
  flap2CB = new QComboBox();
  flap1CB->setEnabled(false);
  flap2CB->setEnabled(false);

  QLayout *l = layout();
  l->addWidget(noFlapsRB);
  l->addWidget(oneFlapRB);
  l->addWidget(twoFlapsRB);
  l->addWidget(new QLabel(tr("<br>First Flap Channel:")));
  l->addWidget(flap1CB);
  l->addWidget(new QLabel(tr("Second Flap Channel:")));
  l->addWidget(flap2CB);

  connect(noFlapsRB, SIGNAL(toggled(bool)), this, SLOT(noFlapChannel()));
  connect(oneFlapRB, SIGNAL(toggled(bool)), this, SLOT(oneFlapChannel()));
  connect(twoFlapsRB, SIGNAL(toggled(bool)), this, SLOT(twoFlapChannels()));
}

void FlapsPage::initializePage()
{
  populateCB(flap1CB, nextFreeChannel(4));
  populateCB(flap2CB, nextFreeChannel(4));
  StandardPage::initializePage();
}

bool FlapsPage::validatePage() {
  releaseBookings();
  if (noFlapsRB->isChecked()) {
    return true;
  }
  if (oneFlapRB->isChecked()) {
    return (bookChannel(flap1CB, FLAPS_INPUT, 100 ));
  }
  return( bookChannel(flap1CB, FLAPS_INPUT, 100 ) &&
    bookChannel(flap2CB, FLAPS_INPUT, 100 ));
}

void FlapsPage::noFlapChannel()
{
  flap1CB->setEnabled(false);
  flap2CB->setEnabled(false);
}

void FlapsPage::oneFlapChannel()
{
  flap1CB->setEnabled(true);
  flap2CB->setEnabled(false);
}

void FlapsPage::twoFlapChannels()
{
  flap1CB->setEnabled(true);
  flap2CB->setEnabled(true);
}

AirbrakesPage::AirbrakesPage(WizardDialog *dlg, QString image, QString title, QString text, int nextPage):
  StandardPage(Page_Airbrakes, dlg, image, title, text, nextPage)
{
  noAirbrakesRB = new QRadioButton(tr("No"));
  oneAirbrakeRB = new QRadioButton(tr("Yes, controlled by a single channel"));
  twoAirbrakesRB = new QRadioButton(tr("Yes, controlled by two channels"));
  noAirbrakesRB->setChecked(true);

  airbrake1CB = new QComboBox();
  airbrake2CB = new QComboBox();
  airbrake1CB->setEnabled(false);
  airbrake2CB->setEnabled(false);

  QLayout *l = layout();
  l->addWidget(noAirbrakesRB);
  l->addWidget(oneAirbrakeRB);
  l->addWidget(twoAirbrakesRB);
  l->addWidget(new QLabel(tr("<br>First Airbrake Channel:")));
  l->addWidget(airbrake1CB);
  l->addWidget(new QLabel(tr("Second Airbrake Channel:")));
  l->addWidget(airbrake2CB);

  connect(noAirbrakesRB, SIGNAL(toggled(bool)), this, SLOT(noAirbrakeChannel()));
  connect(oneAirbrakeRB, SIGNAL(toggled(bool)), this, SLOT(oneAirbrakeChannel()));
  connect(twoAirbrakesRB, SIGNAL(toggled(bool)), this, SLOT(twoAirbrakeChannels()));
}

void AirbrakesPage::initializePage()
{
  populateCB(airbrake1CB, nextFreeChannel(4));
  populateCB(airbrake2CB, nextFreeChannel(4));
  StandardPage::initializePage();
}

bool AirbrakesPage::validatePage()
{
  releaseBookings();
  if (noAirbrakesRB->isChecked()) {
    return true;
  }
  if (oneAirbrakeRB->isChecked()) {
    return (bookChannel(airbrake1CB, AIRBRAKES_INPUT, 100 ));
  }
  return( bookChannel(airbrake1CB, AIRBRAKES_INPUT, 100 ) &&
    bookChannel(airbrake2CB, AIRBRAKES_INPUT, 100 ));
}

void AirbrakesPage::noAirbrakeChannel()
{
  airbrake1CB->setEnabled(false);
  airbrake2CB->setEnabled(false);
}

void AirbrakesPage::oneAirbrakeChannel()
{
  airbrake1CB->setEnabled(true);
  airbrake2CB->setEnabled(false);
}

void AirbrakesPage::twoAirbrakeChannels()
{
  airbrake1CB->setEnabled(true);
  airbrake2CB->setEnabled(true);
}

ElevonsPage::ElevonsPage(WizardDialog *dlg, QString image, QString title, QString text, int nextPage):
  StandardPage(Page_Elevons, dlg, image, title, text, nextPage)
{
  elevon1CB = new QComboBox();
  elevon2CB = new QComboBox();

  QLayout *l = layout();
  l->addWidget(new QLabel(tr("<br>First Elevon Channel:")));
  l->addWidget(elevon1CB);
  l->addWidget(new QLabel(tr("Second Elevon Channel:")));
  l->addWidget(elevon2CB);
}

void ElevonsPage::initializePage()
{
  populateCB(elevon1CB, getDefaultChannel(ELEVATOR_INPUT));
  populateCB(elevon2CB, getDefaultChannel(AILERONS_INPUT));
  StandardPage::initializePage();
}

bool ElevonsPage::validatePage()
{
  releaseBookings();
  return (bookChannel(elevon1CB, AILERONS_INPUT, 50, ELEVATOR_INPUT, 50) &&
      bookChannel(elevon2CB, AILERONS_INPUT, -50, ELEVATOR_INPUT, 50));
}

RudderPage::RudderPage(WizardDialog *dlg, QString image, QString title, QString text, int nextPage):
  StandardPage(Page_Rudder, dlg, image, title, text, nextPage)
{
  noRudderRB = new QRadioButton(tr("No"));
  hasRudderRB = new QRadioButton(tr("Yes"));
  noRudderRB->setChecked(true);

  rudderCB = new QComboBox();
  rudderCB->setEnabled(false);

  QLayout *l = layout();
  l->addWidget(noRudderRB);
  l->addWidget(hasRudderRB);
  l->addWidget(new QLabel(tr("<br>Rudder Channel:")));
  l->addWidget(rudderCB);
  connect(noRudderRB,  SIGNAL(toggled(bool)), this, SLOT(noRudder()));
  connect(hasRudderRB, SIGNAL(toggled(bool)), this, SLOT(hasRudder()));
}

void RudderPage::initializePage()
{
  populateCB(rudderCB, getDefaultChannel(RUDDER_INPUT));
  StandardPage::initializePage();
}

bool RudderPage::validatePage() {
  releaseBookings();
  if (noRudderRB->isChecked())
    return true;

  return (bookChannel(rudderCB, RUDDER_INPUT, 100));
}

void RudderPage::noRudder()
{
  rudderCB->setEnabled(false);
}

void RudderPage::hasRudder()
{
  rudderCB->setEnabled(true);
}

VTailPage::VTailPage(WizardDialog *dlg, QString image, QString title, QString text, int nextPage)
  : StandardPage(Page_Tail, dlg, image, title, text, nextPage)
{
  tail1CB = new QComboBox();
  tail2CB = new QComboBox();

  QLayout *l = layout();
  l->addWidget(new QLabel(tr("First Tail Channel:")));
  l->addWidget(tail1CB);
  l->addWidget(new QLabel(tr("Second Tail Channel:")));
  l->addWidget(tail2CB);
}

void VTailPage::initializePage()
{
  populateCB(tail1CB, getDefaultChannel(ELEVATOR_INPUT));
  populateCB(tail2CB, getDefaultChannel(AILERONS_INPUT));
  StandardPage::initializePage();
}

bool VTailPage::validatePage()
{
  releaseBookings();
  return (bookChannel(tail1CB, ELEVATOR_INPUT, 50, RUDDER_INPUT, 50) &&
    bookChannel(tail2CB, ELEVATOR_INPUT, 50, RUDDER_INPUT, -50 ));
}

TailPage::TailPage(WizardDialog *dlg, QString image, QString title, QString text, int nextPage):
  StandardPage(Page_Tail, dlg, image, title, text, nextPage)
{
  elevatorCB = new QComboBox();
  rudderCB = new QComboBox();

  l = layout();
  l->addWidget(new QLabel(tr("Rudder Channel:")));
  l->addWidget(rudderCB);
  l->addWidget(new QLabel(tr("Elevator Channel:")));
  l->addWidget(elevatorCB);

  errorMessage = NULL;
}

void TailPage::initializePage()
{
  populateCB(elevatorCB, getDefaultChannel(ELEVATOR_INPUT));
  populateCB(rudderCB, getDefaultChannel(RUDDER_INPUT));
  StandardPage::initializePage();

  if (totalChannelsAvailable() < 2) {
    errorMessage = new QLabel(tr("Only one channel still available!<br>"
      "You probably should configure your model without using the wizard."));
    errorMessage->setStyleSheet("QLabel { color : red; }");
    l->addWidget(errorMessage);
  }
}

bool TailPage::validatePage()
{
  releaseBookings();
  return( bookChannel(elevatorCB, ELEVATOR_INPUT, 100) &&
    bookChannel(rudderCB, RUDDER_INPUT,   100));
}

void TailPage::cleanupPage()
{
  if (errorMessage)
  {
    l->removeWidget(errorMessage);
    delete errorMessage;
    errorMessage = NULL;
  }
  StandardPage::cleanupPage();
}

SimpleTailPage::SimpleTailPage(WizardDialog *dlg, QString image, QString title, QString text, int nextPage)
  : StandardPage(Page_Simpletail, dlg, image, title, text, nextPage)
{
  elevatorCB = new QComboBox();

  QLayout *l = layout();
  l->addWidget(new QLabel(tr("Elevator Channel:")));
  l->addWidget(elevatorCB);
}

void SimpleTailPage::initializePage()
{
  populateCB(elevatorCB, getDefaultChannel(ELEVATOR_INPUT));
  StandardPage::initializePage();
}

bool SimpleTailPage::validatePage()
{
  releaseBookings();
  return( bookChannel(elevatorCB, ELEVATOR_INPUT, 100 ));
}

CyclicPage::CyclicPage(WizardDialog *dlg, QString image, QString title, QString text, int nextPage):
  StandardPage(Page_Cyclic, dlg, image, title, text, nextPage)
{
  cyclic90RB = new QRadioButton(tr("90"));
  cyclic90RB->setChecked(true);
  cyclic120RB = new QRadioButton(tr("120"));
  cyclic120XRB = new QRadioButton(tr("120x"));
  cyclic140RB = new QRadioButton(tr("140"));

  QLayout *l = layout();
  l->addWidget(cyclic90RB);
  l->addWidget(cyclic120RB);
  l->addWidget(cyclic120XRB);
  l->addWidget(cyclic140RB);
}

void CyclicPage::initializePage()
{
  StandardPage::initializePage();
}

bool CyclicPage::validatePage()
{
  releaseBookings();
  return true;
}

GyroPage::GyroPage(WizardDialog *dlg, QString image, QString title, QString text, int nextPage):
  StandardPage(Page_Gyro, dlg, image, title, text, nextPage)
{
  noGyroRB = new QRadioButton(tr("No"));
  noGyroRB->setChecked(true);
  switchGyroRB = new QRadioButton(tr("Yes, controled by a switch"));
  potGyroRB = new QRadioButton(tr("Yes, controlled by a pot"));

  QLayout *l = layout();
  l->addWidget(noGyroRB);
  l->addWidget(switchGyroRB);
  l->addWidget(potGyroRB);
}

void GyroPage::initializePage()
{
  StandardPage::initializePage();
}

bool GyroPage::validatePage() {
  releaseBookings();
  return true;
}

FblPage::FblPage(WizardDialog *dlg, QString image, QString title, QString text, int nextPage):
  StandardPage(Page_Fblheli, dlg, image, title, text, nextPage)
{
  throttleCB = new QComboBox();
  yawCB = new QComboBox();
  pitchCB = new QComboBox();
  rollCB = new QComboBox();

  QLayout *l = layout();
  l->addWidget(new QLabel(tr("Throttle Channel:")));
  l->addWidget(throttleCB);
  l->addWidget(new QLabel(tr("Yaw Channel:")));
  l->addWidget(yawCB);
  l->addWidget(new QLabel(tr("Pitch Channel:")));
  l->addWidget(pitchCB);
  l->addWidget(new QLabel(tr("Roll Channel:")));
  l->addWidget(rollCB);
}

void FblPage::initializePage()
{
  populateCB(throttleCB, getDefaultChannel(THROTTLE_INPUT));
  populateCB(yawCB, getDefaultChannel(RUDDER_INPUT));
  populateCB(pitchCB, getDefaultChannel(ELEVATOR_INPUT));
  populateCB(rollCB, getDefaultChannel(AILERONS_INPUT));
  StandardPage::initializePage();
}

bool FblPage::validatePage()
{
  releaseBookings();
  return( bookChannel(throttleCB, THROTTLE_INPUT, 100 ) &&
    bookChannel(      yawCB,      RUDDER_INPUT,   100 ) &&
    bookChannel(      pitchCB,    ELEVATOR_INPUT, 100 ) &&
    bookChannel(      rollCB,     AILERONS_INPUT,  100 ));
}

HeliPage::HeliPage(WizardDialog *dlg, QString image, QString title, QString text, int nextPage):
  StandardPage(Page_Helictrl, dlg, image, title, text, nextPage)
{
  throttleCB = new QComboBox();
  yawCB = new QComboBox();
  pitchCB = new QComboBox();
  rollCB = new QComboBox();

  QLayout *l = layout();
  l->addWidget(new QLabel(tr("Throttle Channel:")));
  l->addWidget(throttleCB);
  l->addWidget(new QLabel(tr("Yaw Channel:")));
  l->addWidget(yawCB);
  l->addWidget(new QLabel(tr("Pitch Channel:")));
  l->addWidget(pitchCB);
  l->addWidget(new QLabel(tr("Roll Channel:")));
  l->addWidget(rollCB);
}

void HeliPage::initializePage()
{
  populateCB(throttleCB, getDefaultChannel(THROTTLE_INPUT));
  populateCB(yawCB, getDefaultChannel(RUDDER_INPUT));
  populateCB(pitchCB, getDefaultChannel(ELEVATOR_INPUT));
  populateCB(rollCB, getDefaultChannel(AILERONS_INPUT));
  StandardPage::initializePage();
}

bool HeliPage::validatePage()
{
  releaseBookings();
  return( bookChannel(throttleCB, THROTTLE_INPUT, 100 ) &&
    bookChannel(      yawCB,      RUDDER_INPUT,   100 ) &&
    bookChannel(      pitchCB,    ELEVATOR_INPUT, 100 ) &&
    bookChannel(      rollCB,     AILERONS_INPUT,  100 ));
}

MultirotorPage::MultirotorPage(WizardDialog *dlg, QString image, QString title, QString text, int nextPage):
  StandardPage(Page_Multirotor, dlg, image, title, text, nextPage)
{
  throttleCB = new QComboBox();
  yawCB = new QComboBox();
  pitchCB = new QComboBox();
  rollCB = new QComboBox();

  QLayout *l = layout();
  l->addWidget(new QLabel(tr("Throttle Channel:")));
  l->addWidget(throttleCB);
  l->addWidget(new QLabel(tr("Yaw Channel:")));
  l->addWidget(yawCB);
  l->addWidget(new QLabel(tr("Pitch Channel:")));
  l->addWidget(pitchCB);
  l->addWidget(new QLabel(tr("Roll Channel:")));
  l->addWidget(rollCB);
}

void MultirotorPage::initializePage()
{
  populateCB(throttleCB, getDefaultChannel(THROTTLE_INPUT));
  populateCB(yawCB, getDefaultChannel(RUDDER_INPUT));
  populateCB(pitchCB, getDefaultChannel(ELEVATOR_INPUT));
  populateCB(rollCB, getDefaultChannel(AILERONS_INPUT));
  StandardPage::initializePage();
}

bool MultirotorPage::validatePage()
{
  releaseBookings();
  return( bookChannel(throttleCB, THROTTLE_INPUT, 100 ) &&
    bookChannel(yawCB,            RUDDER_INPUT,   100 ) &&
    bookChannel(pitchCB,          ELEVATOR_INPUT, 100 ) &&
    bookChannel(rollCB,           AILERONS_INPUT,  100 ));
}

OptionsPage::OptionsPage(WizardDialog *dlg, QString image, QString title, QString text, int nextPage):
  StandardPage(Page_Options, dlg, image, title, text, nextPage)
{
  throttleCutRB = new QCheckBox(tr("Throttle Cut"));
  throttleTimerRB = new QCheckBox(tr("Throttle Timer"));
  flightTimerRB = new QCheckBox(tr("Flight Timer"));

  QLayout *l = layout();
  l->addWidget(throttleCutRB);
  l->addWidget(throttleTimerRB);
  l->addWidget(flightTimerRB);
}

void OptionsPage::initializePage(){
  StandardPage::initializePage();
}

bool OptionsPage::validatePage(){
  wizDlg->mix.options[THROTTLE_CUT_OPTION] = throttleCutRB->isChecked();
  wizDlg->mix.options[THROTTLE_TIMER_OPTION] = throttleTimerRB->isChecked();
  wizDlg->mix.options[FLIGHT_TIMER_OPTION] = flightTimerRB->isChecked();
  return true;
}

ConclusionPage::ConclusionPage(WizardDialog *dlg, QString image, QString title, QString text, int nextPage)
  : StandardPage(Page_Conclusion, dlg, image, title, text, nextPage)
{
  textLabel = new QLabel();
  proceedCB = new QCheckBox(tr("OK, I understand."));
  registerField("evaluate.proceed*", proceedCB);

  QLayout *l = layout();
  l->addWidget(textLabel);
  l->addWidget(proceedCB);
}

void ConclusionPage::initializePage()
{
  WizardPrinter p(&wizDlg->mix);
  textLabel->setText(p.print());
  StandardPage::initializePage();
}

bool ConclusionPage::validatePage()
{
  wizDlg->mix.complete = true;
  return true;
}


QString WizardPrinter::inputName(Input input)
{
  switch (input) {
    case THROTTLE_INPUT:
      return "THR";
    case RUDDER_INPUT:
      return "RUD";
    case ELEVATOR_INPUT:
      return "ELE";
    case AILERONS_INPUT:
      return "AIL";
    case FLAPS_INPUT:
      return "FLP";
    case AIRBRAKES_INPUT:
      return "AIR";
    default:
      return "---";
  }
}

QString WizardPrinter::vehicleName(Vehicle vehicle)
{
  switch (vehicle) {
    case PLANE:
      return tr("Plane");
    case MULTICOPTER:
      return tr("Multicopter");
    case HELICOPTER:
      return tr("Helicopter");
    default:
      return "---";
  }
}

WizardPrinter::WizardPrinter(WizMix *wizMix)
{
  mix = wizMix;
}

QString WizardPrinter::printChannel( Input input1, int weight1, Input input2, int weight2 )
{
  QString str;
  str =  QString("[%1, %2]").arg(inputName(input1)).arg(weight1);
  if ( input2 != NO_INPUT )
    str += QString("[%1, %2]").arg(inputName(input2)).arg(weight2);
  return str;
}

QString WizardPrinter::print()
{
  QString str = tr("Model Name: ") + QString::fromUtf8(mix->name) + "\n";
  str += tr("Model Type: ") + vehicleName(mix->vehicle) + "\n";

  str += tr("Options: ") + "[";
  for (int i=0; i<WIZ_MAX_OPTIONS; i++) {
    if (mix->options[i])
      str += "X";
    else
      str += "-";
  }
  str += QString("]") + "\n";

  for (int i=0; i<WIZ_MAX_CHANNELS; i++) {
    if (mix->channel[i].page != Page_None) {
      Channel ch = mix->channel[i];
      str += tr("Channel %1: ").arg(i+1);
      str += printChannel(ch.input1, ch.weight1, ch.input2, ch.weight2 );
      str += QString("\n");
    }
  }
  return str;
}
