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

#include "customfunctiondata.h"
#include "eeprominterface.h"
#include "radiodata.h"
#include "radiodataconversionstate.h"
#include "compounditemmodels.h"

void CustomFunctionData::convert(RadioDataConversionState & cstate)
{
  cstate.setComponent(tr("CFN"), 8);
  cstate.setSubComp(nameToString(cstate.subCompIdx, (cstate.toModel() ? false : true)));
  swtch.convert(cstate);
  if (func == FuncVolume || func == FuncBacklight || func == FuncPlayValue || (func >= FuncAdjustGV1 && func <= FuncAdjustGVLast && adjustMode == 1)) {
    param = RawSource(param).convert(cstate.withComponentField("PARAM")).toValue();
  }
}

void CustomFunctionData::clear()
{
  memset(reinterpret_cast<void *>(this), 0, sizeof(CustomFunctionData));
  if (!getCurrentFirmware()->getCapability(SafetyChannelCustomFunction)) {
    func = FuncTrainer;
  }
}

bool CustomFunctionData::isEmpty() const
{
  return (swtch.type == SWITCH_TYPE_NONE);
}

//  static
QString CustomFunctionData::nameToString(const int index, const bool globalContext)
{
  return RadioData::getElementName((globalContext ? tr("GF") : tr("SF")), index + 1, 0, true);
}

QString CustomFunctionData::funcToString(const ModelData * model) const
{
  return funcToString(func, model);
}

//  static
QString CustomFunctionData::funcToString(const AssignFunc func, const ModelData * model)
{
  if (func >= FuncOverrideCH1 && func <= FuncOverrideCHLast)
    return tr("Override %1").arg(RawSource(SOURCE_TYPE_CH, func).toString(model));
  else if (func == FuncTrainer)
    return tr("Trainer Sticks");
  else if (func == FuncTrainerRUD)
    return tr("Trainer RUD");
  else if (func == FuncTrainerELE)
    return tr("Trainer ELE");
  else if (func == FuncTrainerTHR)
    return tr("Trainer THR");
  else if (func == FuncTrainerAIL)
    return tr("Trainer AIL");
  else if (func == FuncTrainerChannels)
    return tr("Trainer Channels");
  else if (func == FuncInstantTrim)
    return tr("Instant Trim");
  else if (func == FuncPlaySound)
    return tr("Play Sound");
  else if (func == FuncPlayHaptic)
    return tr("Haptic");
  else if (func == FuncReset)
    return tr("Reset");
  else if (func >= FuncSetTimer1 && func <= FuncSetTimerLast)
    return tr("Set %1").arg(RawSource(SOURCE_TYPE_SPECIAL, SOURCE_TYPE_SPECIAL_FIRST_TIMER + func - FuncSetTimer1).toString(model));
  else if (func == FuncVario)
    return tr("Vario");
  else if (func == FuncPlayPrompt)
    return tr("Play Track");
  else if (func == FuncPlayBoth)
    return tr("Play Both");
  else if (func == FuncPlayValue)
    return tr("Play Value");
  else if (func == FuncPlayScript)
    return tr("Lua Script");
  else if (func == FuncLogs)
    return tr("SD Logs");
  else if (func == FuncVolume)
    return tr("Volume");
  else if (func == FuncBacklight)
    return tr("Backlight");
  else if (func == FuncScreenshot)
    return tr("Screenshot");
  else if (func == FuncBackgroundMusic)
    return tr("Background Music");
  else if (func == FuncBackgroundMusicPause)
    return tr("Background Music Pause");
  else if (func >= FuncAdjustGV1 && func <= FuncAdjustGVLast)
    return tr("Adjust %1").arg(RawSource(SOURCE_TYPE_GVAR, func - FuncAdjustGV1).toString(model));
  else if (func == FuncSetFailsafe)
    return tr("Set Failsafe");
  else if (func == FuncRangeCheckInternalModule)
    return tr("Range Check Int. Module");
  else if (func == FuncRangeCheckExternalModule)
    return tr("Range Check Ext. Module");
  else if (func == FuncBindInternalModule)
    return tr("Bind Int. Module");
  else if (func == FuncBindExternalModule)
    return tr("Bind Ext. Module");
  else if (func == FuncRacingMode)
    return tr("Racing Mode");
  else if (func == FuncDisableTouch)
    return tr("Disable Touch");
  else {
    return QString(CPN_STR_UNKNOWN_ITEM);
  }
}

QString CustomFunctionData::paramToString(const ModelData * model) const
{
  QStringList qs;
  if (func <= FuncInstantTrim) {
    return QString("%1").arg(param);
  }
  else if (func == FuncLogs) {
    return QString("%1").arg(param / 10.0) + tr("s");
  }
  else if (func == FuncPlaySound) {
    return playSoundToString(param);
  }
  else if (func == FuncPlayHaptic) {
    return harpicToString(param);
  }
  else if (func == FuncReset) {
    return resetToString(param, model);
  }
  else if (func == FuncVolume || func == FuncPlayValue || func == FuncBacklight) {
    return RawSource(param).toString(model);
  }
  else if (func == FuncPlayPrompt || func == FuncPlayBoth) {
    if ( getCurrentFirmware()->getCapability(VoicesAsNumbers)) {
      return QString("%1").arg(param);
    }
    else {
      return paramarm;
    }
  }
  else if (func >= FuncAdjustGV1 && func < FuncCount) {
    switch (adjustMode) {
      case FUNC_ADJUST_GVAR_CONSTANT:
        return gvarAdjustModeToString(adjustMode) + QString(" %1").arg(param);
      case FUNC_ADJUST_GVAR_SOURCE:
      case FUNC_ADJUST_GVAR_GVAR:
        return RawSource(param).toString();
      case FUNC_ADJUST_GVAR_INCDEC:
        const float val = param * model->gvarData[func - FuncAdjustGV1].multiplierGet();
        const QString unit = model->gvarData[func - FuncAdjustGV1].unitToString();
        return gvarAdjustModeToString(adjustMode) + QString(": %1%2").arg(val).arg(unit);
    }
  }
  return "";
}

QString CustomFunctionData::repeatToString() const
{
  return repeatToString(repeatParam);
}

//  static
QString CustomFunctionData::repeatToString(const int value)
{
  if (value == -1) {
    return tr("Played once, not during startup");
  }
  else if (value == 0) {
    return tr("No repeat");
  }
  else {
    return tr("Repeat %1s").arg(value);
  }
}

QString CustomFunctionData::enabledToString() const
{
  if ((func >= FuncOverrideCH1 && func <= FuncOverrideCHLast) ||
      (func >= FuncAdjustGV1 && func <= FuncAdjustGVLast) ||
      (func == FuncReset) ||
      (func >= FuncSetTimer1 && func <= FuncSetTimerLast) ||
      (func == FuncVolume) ||
      (func == FuncBacklight) ||
      (func <= FuncInstantTrim)) {
    if (!enabled) {
      return tr("DISABLED");
    }
  }
  return "";
}

//  static
bool CustomFunctionData::isFuncAvailable(const int index)
{
  Firmware * fw = getCurrentFirmware();

  bool ret = (((index >= FuncOverrideCH1 && index <= FuncOverrideCHLast) && !fw->getCapability(SafetyChannelCustomFunction)) ||
        ((index == FuncVolume || index == FuncBackgroundMusic || index == FuncBackgroundMusicPause) && !fw->getCapability(HasVolume)) ||
        ((index == FuncPlayScript && !IS_HORUS_OR_TARANIS(fw->getBoard()))) ||
        ((index == FuncPlayHaptic) && !fw->getCapability(Haptic)) ||
        ((index == FuncPlayBoth) && !fw->getCapability(HasBeeper)) ||
        ((index == FuncLogs) && !fw->getCapability(HasSDLogs)) ||
        ((index >= FuncSetTimer1 && index <= FuncSetTimerLast) && index > FuncSetTimer1 + fw->getCapability(Timers)) ||
        ((index == FuncScreenshot) && !IS_HORUS_OR_TARANIS(fw->getBoard())) ||
        ((index >= FuncRangeCheckInternalModule && index <= FuncBindExternalModule) && !fw->getCapability(DangerousFunctions)) ||
        ((index >= FuncAdjustGV1 && index <= FuncAdjustGVLast) && !fw->getCapability(Gvars)) ||
		((index == FuncDisableTouch) && !IS_HORUS_OR_TARANIS(fw->getBoard()))
        );
  return !ret;
}

//  static
int CustomFunctionData::funcContext(const int index)
{
  int ret = AllFunctionContexts;

  if ((index >= FuncOverrideCH1 && index <= FuncOverrideCHLast) ||
      (index >= FuncRangeCheckInternalModule && index <= FuncBindExternalModule) ||
      (index >= FuncAdjustGV1 && index <= FuncAdjustGVLast))
    ret &= ~GlobalFunctionsContext;

  return ret;
}

//  static
QString CustomFunctionData::resetToString(const int value, const ModelData * model)
{
  Firmware * firmware = getCurrentFirmware();
  int step = CPN_MAX_TIMERS;

  if (value < step) {
    if (value < firmware->getCapability(Timers))
      return RawSource(SOURCE_TYPE_SPECIAL, value + SOURCE_TYPE_SPECIAL_FIRST_TIMER).toString(model);
    else
      return QString(CPN_STR_UNKNOWN_ITEM);
  }

  if (value < ++step)
    return tr("Flight");

  if (value < ++step)
    return tr("Telemetry");

  if (value < step + firmware->getCapability(Sensors))
    return RawSource(SOURCE_TYPE_TELEMETRY, 3 * (value - step)).toString(model);

  return QString(CPN_STR_UNKNOWN_ITEM);
}

//  static
int CustomFunctionData::resetParamCount()
{
  Firmware * firmware = getCurrentFirmware();

  return CPN_MAX_TIMERS + 2 + firmware->getCapability(Sensors);
}

//  static
bool CustomFunctionData::isResetParamAvailable(const int index, const ModelData * model)
{
  Firmware * firmware = getCurrentFirmware();

  if (index < CPN_MAX_TIMERS) {
    if (index < firmware->getCapability(Timers))
      return true;
    else
      return false;
  }
  else if (index < CPN_MAX_TIMERS + 2)
    return true;
  else if (model && index < resetParamCount())
    return model->sensorData[index - (CPN_MAX_TIMERS + 2)].isAvailable();

  return false;
}

QString CustomFunctionData::harpicToString() const
{
  return harpicToString(param);
}

//  static
QString CustomFunctionData::harpicToString(const int value)
{
  return QString("%1").arg(value);
}

//  static
QStringList CustomFunctionData::playSoundStringList()
{
  return QStringList({ tr("Beep 1"), tr("Beep 2"), tr("Beep 3"), tr("Warn 1"), tr("Warn 2"), tr("Cheep"), tr("Ratata"), tr("Tick"),
                       tr("Siren"), tr("Ring"), tr("Sci Fi"), tr("Robot"), tr("Chirp"), tr("Tada"), tr("Cricket"), tr("Alarm Clock") });
}

QString CustomFunctionData::playSoundToString() const
{
  return playSoundToString(param);
}

//  static
QString CustomFunctionData::playSoundToString(const int value)
{
  const QStringList strl = playSoundStringList();
  if (value < strl.count())
    return strl.at(value);
  else
    return QString(CPN_STR_UNKNOWN_ITEM);
}

QString CustomFunctionData::gvarAdjustModeToString() const
{
  return gvarAdjustModeToString(adjustMode);
}

//  static
QString CustomFunctionData::gvarAdjustModeToString(const int value)
{
  switch (value) {
    case FUNC_ADJUST_GVAR_CONSTANT:
      return tr("Value");
    case FUNC_ADJUST_GVAR_SOURCE:
      return tr("Source");
    case FUNC_ADJUST_GVAR_GVAR:
      return tr("Global Variable");
    case FUNC_ADJUST_GVAR_INCDEC:
      return tr("Inc/Decrement");
    default:
      return QString(CPN_STR_UNKNOWN_ITEM);
  }
}

//  static
AbstractStaticItemModel * CustomFunctionData::repeatItemModel()
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName("customfunctiondata.repeat");

  for (int i = -1; i <= 60; i++) {
    mdl->appendToItemList(repeatToString(i), i);
  }

  mdl->loadItemList();
  return mdl;
}

//  static
AbstractStaticItemModel * CustomFunctionData::playSoundItemModel()
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName("customfunctiondata.playsound");

  for (int i = 0; i < playSoundStringList().count(); i++) {
    mdl->appendToItemList(playSoundToString(i), i);
  }

  mdl->loadItemList();
  return mdl;
}

//  static
AbstractStaticItemModel * CustomFunctionData::harpicItemModel()
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName("customfunctiondata.harpic");

  for (int i = 0; i <= 3; i++) {
    mdl->appendToItemList(harpicToString(i), i);
  }

  mdl->loadItemList();
  return mdl;
}

//  static
AbstractStaticItemModel * CustomFunctionData::gvarAdjustModeItemModel()
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName("customfunctiondata.gvaradjustmode");

  for (int i = 0; i < FUNC_ADJUST_GVAR_COUNT; i++) {
    mdl->appendToItemList(gvarAdjustModeToString(i), i);
  }

  mdl->loadItemList();
  return mdl;
}
