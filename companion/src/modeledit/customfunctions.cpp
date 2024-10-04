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

#include "customfunctions.h"
#include "helpers.h"
#include "appdata.h"

#include <TimerEdit>

CustomFunctionsPanel::CustomFunctionsPanel(QWidget * parent, ModelData * model, GeneralSettings & generalSettings, Firmware * firmware,
                                           CompoundItemModelFactory * sharedItemModels):
  GenericPanel(parent, model, generalSettings, firmware),
  functions(model ? model->customFn : generalSettings.customFn),
  mediaPlayerCurrent(-1),
  mediaPlayer(nullptr),
  modelsUpdateCnt(0)
{
  lock = true;
  fswCapability = model ? firmware->getCapability(CustomFunctions) : firmware->getCapability(GlobalFunctions);

  tabModelFactory = new CompoundItemModelFactory(&generalSettings, model);
  playSoundId = tabModelFactory->registerItemModel(CustomFunctionData::playSoundItemModel());
  harpicId = tabModelFactory->registerItemModel(CustomFunctionData::harpicItemModel());
  repeatId = tabModelFactory->registerItemModel(CustomFunctionData::repeatItemModel());
  repeatLuaId = tabModelFactory->registerItemModel(CustomFunctionData::repeatLuaItemModel());
  repeatSetScreenId = tabModelFactory->registerItemModel(CustomFunctionData::repeatSetScreenItemModel());
  gvarAdjustModeId = tabModelFactory->registerItemModel(CustomFunctionData::gvarAdjustModeItemModel());

  tabFilterFactory = new FilteredItemModelFactory();

  funcActionsId = tabFilterFactory->registerItemModel(new FilteredItemModel(sharedItemModels->getItemModel(AbstractItemModel::IMID_CustomFuncAction),
                                                      model ? CustomFunctionData::AllFunctionContexts : CustomFunctionData::GlobalFunctionsContext),
                                                      "Function Actions");
  connectItemModelEvents(tabFilterFactory->getItemModel(funcActionsId));

  funcResetParamId = tabFilterFactory->registerItemModel(new FilteredItemModel(sharedItemModels->getItemModel(AbstractItemModel::IMID_CustomFuncResetParam)),
                                                         "Reset Params");
  connectItemModelEvents(tabFilterFactory->getItemModel(funcResetParamId));

  rawSwitchId = tabFilterFactory->registerItemModel(new FilteredItemModel(sharedItemModels->getItemModel(AbstractItemModel::IMID_RawSwitch),
                                                                          model ? RawSwitch::SpecialFunctionsContext : RawSwitch::GlobalFunctionsContext),
                                                    "RawSwitch");
  connectItemModelEvents(tabFilterFactory->getItemModel(rawSwitchId));

  rawSourceAllId = tabFilterFactory->registerItemModel(new FilteredItemModel(sharedItemModels->getItemModel(AbstractItemModel::IMID_RawSource)),
                                                       "RawSource All");
  connectItemModelEvents(tabFilterFactory->getItemModel(rawSourceAllId));

  rawSourceInputsId = tabFilterFactory->registerItemModel(new FilteredItemModel(sharedItemModels->getItemModel(AbstractItemModel::IMID_RawSource),
                                                                                RawSource::InputSourceGroups),
                                                          "RawSource Inputs");
  connectItemModelEvents(tabFilterFactory->getItemModel(rawSourceInputsId));

  rawSourceGVarsId = tabFilterFactory->registerItemModel(new FilteredItemModel(sharedItemModels->getItemModel(AbstractItemModel::IMID_RawSource),
                                                                               RawSource::GVarsGroup),
                                                         "RawSource GVars");
  connectItemModelEvents(tabFilterFactory->getItemModel(rawSourceGVarsId));

  if (!firmware->getCapability(VoicesAsNumbers)) {
    tracksSet = getFilesSet(getSoundsPath(generalSettings), QStringList() << "*.wav" << "*.WAV", firmware->getCapability(VoicesMaxLength));
    for (int i = 0; i < fswCapability; i++) {
      if (functions[i].func == FuncPlayPrompt || functions[i].func == FuncBackgroundMusic) {
        QString temp = functions[i].paramarm;
        if (!temp.isEmpty()) {
          tracksSet.insert(temp);
        }
      }
    }
  }

  if (IS_STM32(firmware->getBoard())) {
    scriptsSet = getFilesSet(g.profile[g.id()].sdPath() + "/SCRIPTS/FUNCTIONS", QStringList() << "*.lua", firmware->getCapability(VoicesMaxLength));
    for (int i = 0; i < fswCapability; i++) {
      if (functions[i].func == FuncPlayScript) {
        QString temp = functions[i].paramarm;
        if (!temp.isEmpty()) {
          scriptsSet.insert(temp);
        }
      }
    }
  }

  if (IS_STM32(firmware->getBoard())) {
    scriptsSet = getFilesSet(g.profile[g.id()].sdPath() + "/SCRIPTS/RGBLED", QStringList() << "*.lua", firmware->getCapability(VoicesMaxLength));
    for (int i = 0; i < fswCapability; i++) {
      if (functions[i].func == FuncRGBLed) {
        QString temp = functions[i].paramarm;
        if (!temp.isEmpty()) {
          scriptsSet.insert(temp);
        }
      }
    }
  }

  CompanionIcon playIcon("play.png");
  playIcon.addImage("stop.png", QIcon::Normal, QIcon::On);

  QStringList headerLabels;
  headerLabels << "#" << tr("Switch") << tr("Action") << tr("Parameters") << tr("Repeat") << tr("Enable");
  TableLayout * tableLayout = new TableLayout(this, fswCapability, headerLabels);

  for (int i = 0; i < fswCapability; i++) {
    // The label
    QLabel * label = new QLabel(this);
    label->setContextMenuPolicy(Qt::CustomContextMenu);
    label->setToolTip(tr("Popup menu available"));
    label->setMouseTracking(true);
    label->setProperty("index", i);
    if (model)
      label->setText(tr("SF%1").arg(i + 1));
    else
      label->setText(tr("GF%1").arg(i + 1));
    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    connect(label, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onCustomContextMenuRequested(QPoint)));
    tableLayout->addWidget(i, 0, label);

    // The switch
    fswtchSwtch[i] = new QComboBox(this);
    fswtchSwtch[i]->setProperty("index", i);
    fswtchSwtch[i]->setModel(tabFilterFactory->getItemModel(rawSwitchId));
    fswtchSwtch[i]->setCurrentIndex(fswtchSwtch[i]->findData(functions[i].swtch.toValue()));
    fswtchSwtch[i]->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    fswtchSwtch[i]->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    fswtchSwtch[i]->setMaxVisibleItems(10);
    connect(fswtchSwtch[i], SIGNAL(currentIndexChanged(int)), this, SLOT(customFunctionEdited()));
    tableLayout->addWidget(i, 1, fswtchSwtch[i]);

    // The function
    fswtchFunc[i] = new QComboBox(this);
    fswtchFunc[i]->setProperty("index", i);
    fswtchFunc[i]->setModel(tabFilterFactory->getItemModel(funcActionsId));
    fswtchFunc[i]->setCurrentIndex(fswtchFunc[i]->findData(functions[i].func));
    fswtchFunc[i]->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    connect(fswtchFunc[i], SIGNAL(currentIndexChanged(int)), this, SLOT(functionEdited()));
    tableLayout->addWidget(i, 2, fswtchFunc[i]);

    // The parameters
    QHBoxLayout * paramLayout = new QHBoxLayout();
    tableLayout->addLayout(i, 3, paramLayout);

    fswtchGVmode[i] = new QComboBox(this);
    fswtchGVmode[i]->setProperty("index", i);
    fswtchGVmode[i]->setModel(tabModelFactory->getItemModel(gvarAdjustModeId));
    fswtchGVmode[i]->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    connect(fswtchGVmode[i], SIGNAL(currentIndexChanged(int)), this, SLOT(customFunctionEdited()));
    paramLayout->addWidget(fswtchGVmode[i]);

    fswtchParamGV[i] = new QCheckBox(this);
    fswtchParamGV[i]->setProperty("index", i);
    fswtchParamGV[i]->setText(tr("GV"));
    fswtchParamGV[i]->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    connect(fswtchParamGV[i], SIGNAL(stateChanged(int)), this, SLOT(customFunctionEdited()));
    paramLayout->addWidget(fswtchParamGV[i]);

    fswtchParam[i] = new QDoubleSpinBox(this);
    fswtchParam[i]->setProperty("index", i);
    fswtchParam[i]->setAccelerated(true);
    fswtchParam[i]->setDecimals(0);
    connect(fswtchParam[i], SIGNAL(editingFinished()), this, SLOT(customFunctionEdited()));
    paramLayout->addWidget(fswtchParam[i]);

    fswtchParamTime[i] = new TimerEdit(this);
    fswtchParamTime[i]->setProperty("index", i);
    connect(fswtchParamTime[i], SIGNAL(editingFinished()), this, SLOT(customFunctionEdited()));
    paramLayout->addWidget(fswtchParamTime[i]);

    fswtchParamT[i] = new QComboBox(this);
    fswtchParamT[i]->setProperty("index", i);
    populateFuncParamCB(fswtchParamT[i], functions[i].func, functions[i].param, functions[i].adjustMode);
    paramLayout->addWidget(fswtchParamT[i]);
    fswtchParamT[i]->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    connect(fswtchParamT[i], SIGNAL(currentIndexChanged(int)), this, SLOT(customFunctionEdited()));

    fswtchParamArmT[i] = new QComboBox(this);
    fswtchParamArmT[i]->setProperty("index", i);
    fswtchParamArmT[i]->setEditable(true);
    fswtchParamArmT[i]->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    paramLayout->addWidget(fswtchParamArmT[i]);
    connect(fswtchParamArmT[i], SIGNAL(currentIndexChanged(int)), this, SLOT(customFunctionEdited()));
    connect(fswtchParamArmT[i], SIGNAL(editTextChanged ( const QString)), this, SLOT(customFunctionEdited()));

    playBT[i] = new QToolButton(this);
    playBT[i]->setProperty("index", i);
    playBT[i]->setIcon(playIcon);
    playBT[i]->setCheckable(true);
    paramLayout->addWidget(playBT[i]);
    connect(playBT[i], &QToolButton::clicked, this, &CustomFunctionsPanel::toggleSound);

    QHBoxLayout * repeatLayout = new QHBoxLayout();
    tableLayout->addLayout(i, 4, repeatLayout);
    fswtchRepeat[i] = new QComboBox(this);
    fswtchRepeat[i]->setProperty("index", i);
    if (functions[i].func == FuncPlayScript || functions[i].func == FuncRGBLed)
      fswtchRepeat[i]->setModel(tabModelFactory->getItemModel(repeatLuaId));
    else if (functions[i].func == FuncSetScreen && !Boards::getCapability(firmware->getBoard(), Board::HasColorLcd))
      fswtchRepeat[i]->setModel(tabModelFactory->getItemModel(repeatSetScreenId));
    else
      fswtchRepeat[i]->setModel(tabModelFactory->getItemModel(repeatId));
    fswtchRepeat[i]->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    fswtchRepeat[i]->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    repeatLayout->addWidget(fswtchRepeat[i], i + 1);
    connect(fswtchRepeat[i], SIGNAL(currentIndexChanged(int)), this, SLOT(customFunctionEdited()));

    QHBoxLayout *enableLayout = new QHBoxLayout();
    tableLayout->addLayout(i, 5, enableLayout);
    fswtchEnable[i] = new QCheckBox(this);
    fswtchEnable[i]->setProperty("index", i);
    fswtchEnable[i]->setFixedWidth(200);
    enableLayout->addWidget(fswtchEnable[i], i + 1);
    connect(fswtchEnable[i], SIGNAL(stateChanged(int)), this, SLOT(customFunctionEdited()));
  }

  disableMouseScrolling();
  tableLayout->resizeColumnsToContents();
  tableLayout->setColumnWidth(3, 300);
  tableLayout->pushRowsUp(fswCapability + 1);

  update();
  lock = false;
}

CustomFunctionsPanel::~CustomFunctionsPanel()
{
  if (mediaPlayer)
    stopSound(mediaPlayerCurrent);
  delete tabModelFactory;
  delete tabFilterFactory;
}

void CustomFunctionsPanel::onMediaPlayerStateChanged(QMediaPlayer::State state)
{
  if (state != QMediaPlayer::PlayingState)
    stopSound(mediaPlayerCurrent);
}

void CustomFunctionsPanel::onMediaPlayerError(QMediaPlayer::Error error)
{
  stopSound(mediaPlayerCurrent);
  QMessageBox::critical(this, CPN_STR_TTL_ERROR, tr("Error occurred while trying to play sound, possibly the file is already opened. (Err: %1 [%2])").arg(mediaPlayer ? mediaPlayer->errorString() : "").arg(error));
}

bool CustomFunctionsPanel::playSound(int index)
{
  QString path;

  if (functions[index].func == FuncPlaySound) {
    path = QString(":/sounds/%1.wav").arg(functions[index].param);
  }
  else {
    path = g.profile[g.id()].sdPath();
    if (!QDir(path).exists())
      return false;  // unlikely

    if (firmware->getCapability(VoicesAsNumbers)) {  // AVR
      path.append(QString("/%1.wav").arg(int(fswtchParam[index]->value()), 4, 10, QChar('0')));
    }
    else {
      QString lang(generalSettings.ttsLanguage);
      if (lang.isEmpty())
        lang = "en";
      path.append(QString("/SOUNDS/%1/%2.wav").arg(lang).arg(fswtchParamArmT[index]->currentText()));
    }
  }

  if (!QFileInfo::exists(path) || !QFileInfo(path).isReadable()) {
    QMessageBox::critical(this, CPN_STR_TTL_ERROR, tr("Unable to find or open sound file:\n%1").arg(path));
    return false;
  }

  if (mediaPlayer)
    stopSound(mediaPlayerCurrent);

  mediaPlayer = new QMediaPlayer(this, QMediaPlayer::LowLatency);
  if (functions[index].func == FuncPlaySound)
    mediaPlayer->setMedia(QUrl(path.prepend("qrc")));
  else
    mediaPlayer->setMedia(QUrl::fromLocalFile(path));
  connect(mediaPlayer, &QMediaPlayer::stateChanged, this, &CustomFunctionsPanel::onMediaPlayerStateChanged);
  connect(mediaPlayer, static_cast<void(QMediaPlayer::*)(QMediaPlayer::Error)>(&QMediaPlayer::error), this, &CustomFunctionsPanel::onMediaPlayerError);
  mediaPlayerCurrent = index;
  mediaPlayer->play();
  return true;
}

void CustomFunctionsPanel::stopSound(int index)
{
  if (index > -1 && index < (int)DIM(playBT))
    playBT[index]->setChecked(false);
  mediaPlayerCurrent = -1;
  if (mediaPlayer) {
    disconnect(mediaPlayer, 0, this, 0);
    mediaPlayer->stop();
    mediaPlayer->deleteLater();
    mediaPlayer = nullptr;
  }
}

void CustomFunctionsPanel::toggleSound(bool play)
{
  if (!sender() || !sender()->property("index").isValid())
    return;
  const int index = sender()->property("index").toInt();
  if (play)
    playBT[index]->setChecked(playSound(index));
  else
    stopSound(index);
}

#define CUSTOM_FUNCTION_NUMERIC_PARAM  (1<<0)
#define CUSTOM_FUNCTION_SOURCE_PARAM   (1<<1)
#define CUSTOM_FUNCTION_FILE_PARAM     (1<<2)
#define CUSTOM_FUNCTION_TIME_PARAM     (1<<3)
#define CUSTOM_FUNCTION_GV_MODE        (1<<4)
#define CUSTOM_FUNCTION_GV_TOOGLE      (1<<5)
#define CUSTOM_FUNCTION_ENABLE         (1<<6)
#define CUSTOM_FUNCTION_REPEAT         (1<<7)
#define CUSTOM_FUNCTION_PLAY           (1<<8)
#define CUSTOM_FUNCTION_SHOW_FUNC      (1<<10)


void CustomFunctionsPanel::customFunctionEdited()
{
  if (!lock) {
    lock = true;
    int index = sender()->property("index").toInt();
    refreshCustomFunction(index, true);
    emit modified();
    lock = false;
  }
}

void CustomFunctionsPanel::functionEdited()
{
  if (!lock) {
    lock = true;
    int index = sender()->property("index").toInt();
    RawSwitch swtch = functions[index].swtch;
    functions[index].clear();
    functions[index].swtch = swtch;
    functions[index].func = (AssignFunc)fswtchFunc[index]->currentData().toInt();
    functions[index].enabled = true;
    if (functions[index].func == FuncLogs)
      functions[index].param = 10;  // 1 sec
    refreshCustomFunction(index);
    emit modified();
    lock = false;
  }
}

void CustomFunctionsPanel::refreshCustomFunction(int i, bool modified)
{
  CustomFunctionData & cfn = functions[i];
  AssignFunc func = (AssignFunc)fswtchFunc[i]->currentData().toInt();

  unsigned int widgetsMask = 0;
  if (modified) {
    cfn.swtch = RawSwitch(fswtchSwtch[i]->currentData().toInt());
    cfn.func = func;
    cfn.enabled = fswtchEnable[i]->isChecked();
  }
  else {
    fswtchSwtch[i]->setCurrentIndex(fswtchSwtch[i]->findData(cfn.swtch.toValue()));
    fswtchFunc[i]->setCurrentIndex(fswtchFunc[i]->findData(cfn.func));
    fswtchEnable[i]->setChecked(cfn.enabled);
  }

  if (!cfn.isEmpty()) {
    widgetsMask |= CUSTOM_FUNCTION_SHOW_FUNC | CUSTOM_FUNCTION_ENABLE;

    if (func >= FuncOverrideCH1 && func <= FuncOverrideCHLast) {
      if (model) {
        int channelsMax = model->getChannelsMax(true);
        fswtchParam[i]->setDecimals(0);
        fswtchParam[i]->setSingleStep(1);
        fswtchParam[i]->setMinimum(-channelsMax);
        fswtchParam[i]->setMaximum(channelsMax);
        if (modified) {
          cfn.param = fswtchParam[i]->value();
        }
        fswtchParam[i]->setValue(cfn.param);
        widgetsMask |= CUSTOM_FUNCTION_NUMERIC_PARAM;
      }
    }
    else if (func == FuncLogs) {
      fswtchParam[i]->setDecimals(1);
      fswtchParam[i]->setMinimum(0.1);
      fswtchParam[i]->setMaximum(25.5);
      fswtchParam[i]->setSingleStep(0.1);
      if (modified)
        cfn.param = fswtchParam[i]->value() * 10.0;
      fswtchParam[i]->setValue(cfn.param / 10.0);
      widgetsMask |= CUSTOM_FUNCTION_NUMERIC_PARAM;
    }
    else if (func >= FuncPushCustomSwitch1 && func <= FuncPushCustomSwitchLast) {
      fswtchParam[i]->setDecimals(1);
      fswtchParam[i]->setMinimum(0);
      fswtchParam[i]->setMaximum(25.5);
      fswtchParam[i]->setSingleStep(0.1);
      if (modified)
        cfn.param = fswtchParam[i]->value() * 10.0;
      fswtchParam[i]->setValue(cfn.param / 10.0);
      widgetsMask |= CUSTOM_FUNCTION_NUMERIC_PARAM;
    }
    else if (func >= FuncAdjustGV1 && func <= FuncAdjustGVLast) {
      int gvidx = func - FuncAdjustGV1;
      if (modified)
        cfn.adjustMode = fswtchGVmode[i]->currentData().toInt();
      fswtchGVmode[i]->setCurrentIndex(fswtchGVmode[i]->findData(cfn.adjustMode));
      widgetsMask |= CUSTOM_FUNCTION_GV_MODE;
      if (cfn.adjustMode == FUNC_ADJUST_GVAR_CONSTANT || cfn.adjustMode == FUNC_ADJUST_GVAR_INCDEC) {
        if (modified)
          cfn.param = fswtchParam[i]->value() * model->gvarData[gvidx].multiplierSet();
        fswtchParam[i]->setDecimals(model->gvarData[gvidx].prec);
        fswtchParam[i]->setSingleStep(model->gvarData[gvidx].multiplierGet());
        fswtchParam[i]->setSuffix(model->gvarData[gvidx].unitToString());
        if (cfn.adjustMode == FUNC_ADJUST_GVAR_INCDEC) {
          double rng = abs(model->gvarData[gvidx].getMax() - model->gvarData[gvidx].getMin());
          rng *= model->gvarData[gvidx].multiplierGet();
          fswtchParam[i]->setMinimum(-rng);
          fswtchParam[i]->setMaximum(rng);
        }
        else {
          fswtchParam[i]->setMinimum(model->gvarData[gvidx].getMinPrec());
          fswtchParam[i]->setMaximum(model->gvarData[gvidx].getMaxPrec());
        }
        fswtchParam[i]->setValue(cfn.param * model->gvarData[gvidx].multiplierGet());
        widgetsMask |= CUSTOM_FUNCTION_NUMERIC_PARAM;
      }
      else {
        if (modified)
          cfn.param = fswtchParamT[i]->currentData().toInt();
        populateFuncParamCB(fswtchParamT[i], func, cfn.param, cfn.adjustMode);
        widgetsMask |= CUSTOM_FUNCTION_SOURCE_PARAM;
      }
    }
    else if (func == FuncReset) {
      if (modified)
        cfn.param = fswtchParamT[i]->currentData().toInt();
      populateFuncParamCB(fswtchParamT[i], func, cfn.param);
      widgetsMask |= CUSTOM_FUNCTION_SOURCE_PARAM;
    }
    else if (func >= FuncSetTimer1 && func <= FuncSetTimerLast) {
      if (modified)
        cfn.param = fswtchParamTime[i]->timeInSeconds();
      RawSourceRange range = RawSource(SOURCE_TYPE_TIMER, func - FuncSetTimer1 + 1).getRange(model, generalSettings);
      fswtchParamTime[i]->setTimeRange((int)range.min, (int)range.max);
      fswtchParamTime[i]->setTime(cfn.param);
      widgetsMask |= CUSTOM_FUNCTION_TIME_PARAM;
    }
    else if (func == FuncVolume || func == FuncBacklight) {
      if (modified)
        cfn.param = fswtchParamT[i]->currentData().toInt();
      populateFuncParamCB(fswtchParamT[i], func, cfn.param);
      widgetsMask |= CUSTOM_FUNCTION_SOURCE_PARAM;
    }
    else if (func == FuncPlaySound || func == FuncPlayHaptic || func == FuncPlayValue || func == FuncPlayPrompt || func == FuncPlayBoth || func == FuncBackgroundMusic || func == FuncSetScreen) {
      if (func != FuncBackgroundMusic) {
        if (modified)
          cfn.repeatParam = fswtchRepeat[i]->currentData().toInt();
        widgetsMask |= CUSTOM_FUNCTION_REPEAT;
        fswtchRepeat[i]->setModel(tabModelFactory->getItemModel(repeatId));
        fswtchRepeat[i]->setCurrentIndex(fswtchRepeat[i]->findData(cfn.repeatParam));
      }
      if (func == FuncPlayValue) {
        if (modified)
          cfn.param = fswtchParamT[i]->currentData().toInt();
        populateFuncParamCB(fswtchParamT[i], func, cfn.param);
        widgetsMask |= CUSTOM_FUNCTION_SOURCE_PARAM | CUSTOM_FUNCTION_REPEAT;
      }
      else if (func == FuncPlayPrompt || func == FuncPlayBoth) {
        if (firmware->getCapability(VoicesAsNumbers)) {
          fswtchParam[i]->setDecimals(0);
          fswtchParam[i]->setSingleStep(1);
          fswtchParam[i]->setMinimum(0);
          if (func == FuncPlayPrompt) {
            widgetsMask |= CUSTOM_FUNCTION_NUMERIC_PARAM | CUSTOM_FUNCTION_REPEAT | CUSTOM_FUNCTION_GV_TOOGLE;
          }
          else {
            widgetsMask |= CUSTOM_FUNCTION_NUMERIC_PARAM | CUSTOM_FUNCTION_REPEAT;
            fswtchParamGV[i]->setChecked(false);
          }
          fswtchParam[i]->setMaximum(func == FuncPlayBoth ? 254 : 255);
          if (modified) {
            if (fswtchParamGV[i]->isChecked()) {
              fswtchParam[i]->setMinimum(1);
              cfn.param = std::min(fswtchParam[i]->value(), 5.0) + (fswtchParamGV[i]->isChecked() ? 250 : 0);
            }
            else {
              cfn.param = fswtchParam[i]->value();
            }
          }
          if (cfn.param > 250 && (func != FuncPlayBoth)) {
            fswtchParamGV[i]->setChecked(true);
            fswtchParam[i]->setValue(cfn.param - 250);
            fswtchParam[i]->setMaximum(5);
          }
          else {
            fswtchParamGV[i]->setChecked(false);
            fswtchParam[i]->setValue(cfn.param);
          }
          if (cfn.param < 251)
            widgetsMask |= CUSTOM_FUNCTION_PLAY;
        }
        else {
          widgetsMask |= CUSTOM_FUNCTION_FILE_PARAM;
          if (modified) {
            Helpers::getFileComboBoxValue(fswtchParamArmT[i], cfn.paramarm, firmware->getCapability(VoicesMaxLength));
          }
          Helpers::populateFileComboBox(fswtchParamArmT[i], tracksSet, cfn.paramarm);
          if (fswtchParamArmT[i]->currentText() != CPN_STR_NONE_ITEM) {
            widgetsMask |= CUSTOM_FUNCTION_PLAY;
          }
        }
      }
      else if (func == FuncBackgroundMusic) {
        widgetsMask |= CUSTOM_FUNCTION_FILE_PARAM;
        if (modified) {
          Helpers::getFileComboBoxValue(fswtchParamArmT[i], cfn.paramarm, firmware->getCapability(VoicesMaxLength));
        }
        Helpers::populateFileComboBox(fswtchParamArmT[i], tracksSet, cfn.paramarm);
        if (fswtchParamArmT[i]->currentText() != CPN_STR_NONE_ITEM) {
          widgetsMask |= CUSTOM_FUNCTION_PLAY;
        }
      }
      else if (func == FuncPlaySound) {
        if (modified)
          cfn.param = (uint8_t)fswtchParamT[i]->currentIndex();
        populateFuncParamCB(fswtchParamT[i], func, cfn.param);
        widgetsMask |= CUSTOM_FUNCTION_SOURCE_PARAM | CUSTOM_FUNCTION_PLAY;
      }
      else if (func == FuncPlayHaptic) {
        if (modified)
          cfn.param = (uint8_t)fswtchParamT[i]->currentIndex();
        populateFuncParamCB(fswtchParamT[i], func, cfn.param);
        widgetsMask |= CUSTOM_FUNCTION_SOURCE_PARAM;
      }
      else if (func == FuncSetScreen) {
        if (modified) {
          cfn.param = (uint8_t)fswtchParam[i]->value();
          cfn.repeatParam = fswtchRepeat[i]->currentData().toInt();
        }
        fswtchParam[i]->setDecimals(0);
        if (Boards::getCapability(firmware->getBoard(), Board::HasColorLcd)) {
          fswtchParam[i]->setMinimum(1);
          if(model)
            fswtchParam[i]->setMaximum(model->getCustomScreensCount());
          else
            fswtchParam[i]->setMaximum(1);
        } else {
          fswtchParam[i]->setMinimum(0);
          fswtchParam[i]->setMaximum(4);
          widgetsMask |= CUSTOM_FUNCTION_REPEAT;
          fswtchRepeat[i]->setModel(tabModelFactory->getItemModel(repeatSetScreenId));
          fswtchRepeat[i]->setCurrentIndex(fswtchRepeat[i]->findData(cfn.repeatParam));
        }
        fswtchParam[i]->setSingleStep(1);
        fswtchParam[i]->setValue(cfn.param);
        widgetsMask |= CUSTOM_FUNCTION_NUMERIC_PARAM;
      }
    }
    else if (func == FuncPlayScript || func == FuncRGBLed) {
      widgetsMask |= CUSTOM_FUNCTION_FILE_PARAM | CUSTOM_FUNCTION_REPEAT;
      if (modified) {
        Helpers::getFileComboBoxValue(fswtchParamArmT[i], cfn.paramarm, 8);
        cfn.repeatParam = fswtchRepeat[i]->currentData().toInt();
      }
      Helpers::populateFileComboBox(fswtchParamArmT[i], scriptsSet, cfn.paramarm);
      fswtchRepeat[i]->setModel(tabModelFactory->getItemModel(repeatLuaId));
      fswtchRepeat[i]->setCurrentIndex(fswtchRepeat[i]->findData(cfn.repeatParam));
    }
    else {
      if (modified)
        cfn.param = fswtchParam[i]->value();
      fswtchParam[i]->setDecimals(0);
      fswtchParam[i]->setSingleStep(1);
      fswtchParam[i]->setValue(cfn.param);
      widgetsMask |= CUSTOM_FUNCTION_ENABLE;
    }
  }
  else {
    cfn.enabled = true;
  }

  fswtchFunc[i]->setVisible(widgetsMask & CUSTOM_FUNCTION_SHOW_FUNC);
  fswtchParam[i]->setVisible(widgetsMask & CUSTOM_FUNCTION_NUMERIC_PARAM);
  fswtchParamTime[i]->setVisible(widgetsMask & CUSTOM_FUNCTION_TIME_PARAM);
  fswtchParamGV[i]->setVisible(widgetsMask & CUSTOM_FUNCTION_GV_TOOGLE);
  fswtchParamT[i]->setVisible(widgetsMask & CUSTOM_FUNCTION_SOURCE_PARAM);
  fswtchParamArmT[i]->setVisible(widgetsMask & CUSTOM_FUNCTION_FILE_PARAM);
  fswtchEnable[i]->setVisible(widgetsMask & CUSTOM_FUNCTION_ENABLE);
  fswtchRepeat[i]->setVisible(widgetsMask & CUSTOM_FUNCTION_REPEAT);
  fswtchGVmode[i]->setVisible(widgetsMask & CUSTOM_FUNCTION_GV_MODE);
  playBT[i]->setVisible(widgetsMask & CUSTOM_FUNCTION_PLAY);
}

void CustomFunctionsPanel::update()
{
  lock = true;
  for (int i = 0; i < fswCapability; i++) {
    refreshCustomFunction(i);
  }
  lock = false;
}

void CustomFunctionsPanel::cmPaste()
{
  QByteArray data;
  if (hasClipboardData(&data)) {
    memcpy(&functions[selectedIndex], data.constData(), sizeof(CustomFunctionData));
    resetCBsAndRefresh(selectedIndex);
    emit modified();
  }
}

void CustomFunctionsPanel::cmDelete()
{
  if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Delete Function. Are you sure?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    return;

  memmove(&functions[selectedIndex], &functions[selectedIndex + 1], (CPN_MAX_SPECIAL_FUNCTIONS - (selectedIndex + 1)) * sizeof(CustomFunctionData));
  functions[fswCapability - 1].clear();

  for (int i = selectedIndex; i < fswCapability; i++) {
    resetCBsAndRefresh(i);
  }
  emit modified();
}

void CustomFunctionsPanel::cmCopy()
{
  QByteArray data;
  data.append((char*)&functions[selectedIndex], sizeof(CustomFunctionData));
  QMimeData *mimeData = new QMimeData;
  mimeData->setData(MIMETYPE_CUSTOM_FUNCTION, data);
  QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);
}

void CustomFunctionsPanel::cmCut()
{
  if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Cut Special Function. Are you sure?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    return;
  cmCopy();
  cmClear(false);
}

void CustomFunctionsPanel::onCustomContextMenuRequested(QPoint pos)
{
  QLabel *label = (QLabel *)sender();
  selectedIndex = label->property("index").toInt();
  QPoint globalPos = label->mapToGlobal(pos);

  QMenu contextMenu;
  contextMenu.addAction(CompanionIcon("copy.png"), tr("Copy"),this,SLOT(cmCopy()));
  contextMenu.addAction(CompanionIcon("cut.png"), tr("Cut"),this,SLOT(cmCut()));
  contextMenu.addAction(CompanionIcon("paste.png"), tr("Paste"),this,SLOT(cmPaste()))->setEnabled(hasClipboardData());
  contextMenu.addAction(CompanionIcon("clear.png"), tr("Clear"),this,SLOT(cmClear()));
  contextMenu.addSeparator();
  contextMenu.addAction(CompanionIcon("arrow-right.png"), tr("Insert"),this,SLOT(cmInsert()))->setEnabled(insertAllowed());
  contextMenu.addAction(CompanionIcon("arrow-left.png"), tr("Delete"),this,SLOT(cmDelete()));
  contextMenu.addAction(CompanionIcon("moveup.png"), tr("Move Up"),this,SLOT(cmMoveUp()))->setEnabled(moveUpAllowed());
  contextMenu.addAction(CompanionIcon("movedown.png"), tr("Move Down"),this,SLOT(cmMoveDown()))->setEnabled(moveDownAllowed());
  contextMenu.addSeparator();
  contextMenu.addAction(CompanionIcon("clear.png"), tr("Clear All"),this,SLOT(cmClearAll()));

  contextMenu.exec(globalPos);
}

void CustomFunctionsPanel::populateFuncParamCB(QComboBox *b, uint function, unsigned int value, unsigned int adjustmode)
{
  QStringList qs;
  b->setModel(new QStandardItemModel(b));  // clear combo box but not any shared item model

  if (function == FuncPlaySound) {
    b->setModel(tabModelFactory->getItemModel(playSoundId));
    b->setCurrentIndex(b->findData(value));
    if (b->currentIndex() < 0)
      b->setCurrentIndex(0);
  }
  else if (function == FuncPlayHaptic) {
    b->setModel(tabModelFactory->getItemModel(harpicId));
    b->setCurrentIndex(b->findData(value));
    if (b->currentIndex() < 0)
      b->setCurrentIndex(0);
  }
  else if (function == FuncReset) {
    b->setModel(tabFilterFactory->getItemModel(funcResetParamId));
    b->setCurrentIndex(b->findData(value));
    if (b->currentIndex() < 0)
      b->setCurrentIndex(0);
  }
  else if (function == FuncVolume || function == FuncBacklight) {
    b->setModel(tabFilterFactory->getItemModel(rawSourceInputsId));
    b->setCurrentIndex(b->findData(value));
    if (b->currentIndex() < 0 && value == 0)
      b->setCurrentIndex(b->count() / 2); // '----' not in list so set to first positive value
  }
  else if (function == FuncPlayValue) {
    b->setModel(tabFilterFactory->getItemModel(rawSourceAllId));
    b->setCurrentIndex(b->findData(value));
    if (b->currentIndex() < 0 && value == 0)
      b->setCurrentIndex(b->count() / 2); // '----' not in list so set to first positive value
  }
  else if (function >= FuncAdjustGV1 && function <= FuncAdjustGVLast) {
    switch (adjustmode) {
      case FUNC_ADJUST_GVAR_SOURCE:
      case FUNC_ADJUST_GVAR_SOURCERAW:
        b->setModel(tabFilterFactory->getItemModel(rawSourceInputsId));
        b->setCurrentIndex(b->findData(value));
        if (b->currentIndex() < 0 && value == 0)
          b->setCurrentIndex(b->count() / 2); // '----' not in list so set to first positive value
        break;
      case FUNC_ADJUST_GVAR_GVAR:
        b->setModel(tabFilterFactory->getItemModel(rawSourceGVarsId));
        b->setCurrentIndex(b->findData(value));
        if (b->currentIndex() < 0 && value == 0)
          b->setCurrentIndex(b->count() / 2); // '----' not in list so set to first positive value
        break;
    }
  }
  else {
    b->hide();
  }
}

void CustomFunctionsPanel::cmMoveUp()
{
  swapData(selectedIndex, selectedIndex - 1);
}

void CustomFunctionsPanel::cmMoveDown()
{
  swapData(selectedIndex, selectedIndex + 1);
}

void CustomFunctionsPanel::cmClear(bool prompt)
{
  if (prompt) {
    if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Clear Function. Are you sure?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
      return;
  }

  functions[selectedIndex].clear();
  resetCBsAndRefresh(selectedIndex);
  emit modified();
}

void CustomFunctionsPanel::cmClearAll()
{
  if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Clear all Functions. Are you sure?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    return;

  for (int i = 0; i < fswCapability; i++) {
    functions[i].clear();
    resetCBsAndRefresh(i);
  }
  emit modified();
}

void CustomFunctionsPanel::cmInsert()
{
  memmove(&functions[selectedIndex + 1], &functions[selectedIndex], (CPN_MAX_SPECIAL_FUNCTIONS - (selectedIndex + 1)) * sizeof(CustomFunctionData));
  functions[selectedIndex].clear();

  for (int i = selectedIndex; i < fswCapability; i++) {
    resetCBsAndRefresh(i);
  }
}

void CustomFunctionsPanel::swapData(int idx1, int idx2)
{
  if ((idx1 != idx2) && (!functions[idx1].isEmpty() || !functions[idx2].isEmpty())) {
    CustomFunctionData fswtmp = functions[idx2];
    CustomFunctionData *fsw1 = &functions[idx1];
    CustomFunctionData *fsw2 = &functions[idx2];
    memcpy(fsw2, fsw1, sizeof(CustomFunctionData));
    memcpy(fsw1, &fswtmp, sizeof(CustomFunctionData));
    resetCBsAndRefresh(idx1);
    resetCBsAndRefresh(idx2);
    emit modified();
  }
}

void CustomFunctionsPanel::resetCBsAndRefresh(int idx)
{
  lock = true;
  fswtchSwtch[idx]->setCurrentIndex(fswtchSwtch[idx]->findData(functions[idx].swtch.toValue()));
  fswtchFunc[idx]->setCurrentIndex(fswtchFunc[idx]->findData(functions[idx].func));
  fswtchGVmode[idx]->setCurrentIndex(functions[idx].adjustMode);
  populateFuncParamCB(fswtchParamT[idx], functions[idx].func, functions[idx].param, functions[idx].adjustMode);
  refreshCustomFunction(idx);
  lock = false;
}

bool CustomFunctionsPanel::hasClipboardData(QByteArray * data) const
{
  const QClipboard * clipboard = QApplication::clipboard();
  const QMimeData * mimeData = clipboard->mimeData();
  if (mimeData->hasFormat(MIMETYPE_CUSTOM_FUNCTION)) {
    if (data)
      data->append(mimeData->data(MIMETYPE_CUSTOM_FUNCTION));
    return true;
  }
  return false;
}

bool CustomFunctionsPanel::insertAllowed() const
{
  return ((selectedIndex < fswCapability - 1) && (functions[fswCapability - 1].isEmpty()));
}

bool CustomFunctionsPanel::moveDownAllowed() const
{
  return selectedIndex < fswCapability - 1;
}

bool CustomFunctionsPanel::moveUpAllowed() const
{
  return selectedIndex > 0;
}

void CustomFunctionsPanel::connectItemModelEvents(FilteredItemModel * itemModel)
{
  connect(itemModel, &FilteredItemModel::aboutToBeUpdated, this, &CustomFunctionsPanel::onItemModelAboutToBeUpdated);
  connect(itemModel, &FilteredItemModel::updateComplete, this, &CustomFunctionsPanel::onItemModelUpdateComplete);
}

void CustomFunctionsPanel::onItemModelAboutToBeUpdated()
{
  lock = true;
  modelsUpdateCnt++;
}

void CustomFunctionsPanel::onItemModelUpdateComplete()
{
  modelsUpdateCnt--;
  if (modelsUpdateCnt < 1) {
    lock = true;
    for (int i = 0; i < fswCapability; i++) {
      fswtchSwtch[i]->setCurrentIndex(fswtchSwtch[i]->findData(functions[i].swtch.toValue()));
      fswtchFunc[i]->setCurrentIndex(fswtchFunc[i]->findData(functions[i].func));
    }
    update();
    lock = false;
  }
}
