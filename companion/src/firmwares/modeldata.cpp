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

#include "modeldata.h"

#include "eeprominterface.h"
#include "generalsettings.h"
#include "macros.h"
#include "radiodataconversionstate.h"
#include "helpers.h"
#include "adjustmentreference.h"
#include "compounditemmodels.h"

ModelData::ModelData()
{
  clear();
}

ModelData::ModelData(const ModelData & src)
{
  *this = src;
}

ModelData & ModelData::operator = (const ModelData & src)
{
  memcpy(reinterpret_cast<void *>(this), &src, sizeof(ModelData));
  return *this;
}

ExpoData * ModelData::insertInput(const int idx)
{
  memmove(&expoData[idx + 1], &expoData[idx], (CPN_MAX_EXPOS - (idx + 1)) * sizeof(ExpoData));
  expoData[idx].clear();
  return &expoData[idx];
}

bool ModelData::isInputValid(const unsigned int idx) const
{
  for (int i = 0; i < CPN_MAX_EXPOS; i++) {
    const ExpoData * expo = &expoData[i];
    if (expo->mode == INPUT_MODE_NONE) break;
    if (expo->chn == idx)
      return true;
  }
  return false;
}

bool ModelData::hasExpos(uint8_t inputIdx) const
{
  for (int i = 0; i < CPN_MAX_EXPOS; i++) {
    const ExpoData & expo = expoData[i];
    if (expo.chn == inputIdx && expo.mode != INPUT_MODE_NONE) {
      return true;
    }
  }
  return false;
}

bool ModelData::hasMixes(uint8_t channelIdx) const
{
  channelIdx += 1;
  for (int i = 0; i < CPN_MAX_MIXERS; i++) {
    if (mixData[i].destCh == channelIdx) {
      return true;
    }
  }
  return false;
}

QVector<const ExpoData *> ModelData::expos(int input) const
{
  QVector<const ExpoData *> result;
  for (int i = 0; i < CPN_MAX_EXPOS; i++) {
    const ExpoData * ed = &expoData[i];
    if ((int)ed->chn == input && ed->mode != INPUT_MODE_NONE) {
      result << ed;
    }
  }
  return result;
}

QVector<const MixData *> ModelData::mixes(int channel) const
{
  QVector<const MixData *> result;
  for (int i = 0; i < CPN_MAX_MIXERS; i++) {
    const MixData * md = &mixData[i];
    if ((int)md->destCh == channel + 1) {
      result << md;
    }
  }
  return result;
}

void ModelData::removeInput(const int idx, bool clearName)
{
  unsigned int chn = expoData[idx].chn;

  memmove(&expoData[idx], &expoData[idx + 1], (CPN_MAX_EXPOS - (idx + 1)) * sizeof(ExpoData));
  expoData[CPN_MAX_EXPOS - 1].clear();

  //also remove input name if removing last line for this input
  if (clearName && !expos(chn).size())
    inputNames[chn][0] = 0;
}

void ModelData::clearInputs()
{
  for (int i = 0; i < CPN_MAX_EXPOS; i++)
    expoData[i].clear();

  //clear all input names
  if (getCurrentFirmware()->getCapability(VirtualInputs)) {
    for (int i = 0; i < CPN_MAX_INPUTS; i++) {
      inputNames[i][0] = 0;
    }
  }
}

void ModelData::clearMixes()
{
  for (int i = 0; i < CPN_MAX_MIXERS; i++)
    mixData[i].clear();
}

void ModelData::clear()
{
  memset(reinterpret_cast<void *>(this), 0, sizeof(ModelData));
  modelIndex = -1;  // an invalid index, this is managed by the TreeView data model
  moduleData[0].protocol = PULSES_OFF;
  moduleData[1].protocol = PULSES_OFF;
  moduleData[0].channelsCount = 8;
  moduleData[1].channelsStart = 0;
  moduleData[1].channelsCount = 8;
  moduleData[0].ppm.delay = 300;
  moduleData[1].ppm.delay = 300;
  moduleData[2].ppm.delay = 300;  //Trainer PPM
  for (int i = 0; i < CPN_MAX_FLIGHT_MODES; i++) {
    flightModeData[i].clear(i);
  }
  for (int i = 0; i < CPN_MAX_GVARS; i++) {
    gvarData[i].clear();
  }
  clearInputs();
  clearMixes();
  for (int i = 0; i < CPN_MAX_CHNOUT; i++)
    limitData[i].clear();
  for (int i = 0; i < CPN_MAX_STICKS; i++)
    expoData[i].clear();
  for (int i = 0; i < CPN_MAX_LOGICAL_SWITCHES; i++)
    logicalSw[i].clear();
  for (int i = 0; i < CPN_MAX_SPECIAL_FUNCTIONS; i++)
    customFn[i].clear();
  for (int i = 0; i < CPN_MAX_CURVES; i++)
    curves[i].clear();
  for (int i = 0; i < CPN_MAX_TIMERS; i++)
    timers[i].clear();
  swashRingData.clear();
  frsky.clear();
  rssiAlarms.clear();
  for (unsigned i = 0; i < CPN_MAX_SENSORS; i++)
    sensorData[i].clear();

  trainerMode = TRAINER_MODE_OFF;

  const char * layoutId = "Layout2P1";  // currently all using same default though might change for NV14
  RadioLayout::init(layoutId, customScreens);

  hatsMode = GeneralSettings::HATSMODE_GLOBAL;
}

bool ModelData::isEmpty() const
{
  return !used;
}

void ModelData::setDefaultInputs(const GeneralSettings & settings)
{
  for (int i = 0; i < Boards::getCapability(getCurrentBoard(), Board::Sticks); i++) {
    ExpoData * expo = &expoData[i];
    expo->chn = i;
    expo->mode = INPUT_MODE_BOTH;
    expo->srcRaw = settings.getDefaultSource(i);
    expo->weight = 100;
    strncpy(inputNames[i], Helpers::removeAccents(expo->srcRaw.toString(this)).toLatin1().constData(), sizeof(inputNames[i]) - 1);
  }
}

void ModelData::setDefaultMixes(const GeneralSettings & settings)
{
  setDefaultInputs(settings);

  for (int i = 0; i < Boards::getCapability(getCurrentBoard(), Board::Sticks); i++) {
    MixData * mix = &mixData[i];
    mix->destCh = i + 1;
    mix->weight = 100;
    mix->srcRaw = RawSource(SOURCE_TYPE_VIRTUAL_INPUT, i + 1);
  }
}

void ModelData::setDefaultFunctionSwitches(int functionSwitchCount)
{
  if (functionSwitchCount == 0)
    return;

  for (int i = 0; i < functionSwitchCount; i++) {
    setFuncSwitchConfig(i, Board::SWITCH_2POS);
    setFuncSwitchGroup(i, 1);
  }
}

void ModelData::setDefaultValues(unsigned int id, const GeneralSettings & settings)
{
  clear();
  used = true;
  sprintf(name, "MODEL%02d", id+1);
  for (int i = 0; i < CPN_MAX_MODULES; i++) {
    moduleData[i].modelId = id + 1;
  }
  setDefaultMixes(settings);
  setDefaultFunctionSwitches(Boards::getCapability(getCurrentFirmware()->getBoard(), Board::FunctionSwitches));
}

int ModelData::getTrimValue(int phaseIdx, int trimIdx)
{
  int result = 0;
  for (int i = 0; i < CPN_MAX_FLIGHT_MODES; i++) {
    FlightModeData & phase = flightModeData[phaseIdx];
    if (phase.trimMode[trimIdx] < 0) {
      return result;
    }
    else {
      if (phase.trimRef[trimIdx] == phaseIdx || phaseIdx == 0) {
        return result + phase.trim[trimIdx];
      }
      else {
        phaseIdx = phase.trimRef[trimIdx];
        if (phase.trimMode[trimIdx] != 0)
          result += phase.trim[trimIdx];
      }
    }
  }
  return 0;
}

bool ModelData::isGVarLinked(int phaseIdx, int gvarIdx)
{
  return flightModeData[phaseIdx].gvars[gvarIdx] > GVAR_MAX_VALUE;
}

bool ModelData::isGVarLinkedCircular(int phaseIdx, int gvarIdx)
{
  for (int i = 0; i < CPN_MAX_FLIGHT_MODES; i++) {
    int val = flightModeData[phaseIdx].gvars[gvarIdx];
    if (phaseIdx == 0 || val <= GVAR_MAX_VALUE)
      return false;
    int nextPhase = val - (GVAR_MAX_VALUE + 1);
    if (nextPhase >= phaseIdx)
      nextPhase += 1;
    phaseIdx = nextPhase;
  }
  return true;
}

int ModelData::getGVarValue(int phaseIdx, int gvarIdx)
{
  for (int i = 0; i < CPN_MAX_FLIGHT_MODES; i++) {
    int val = flightModeData[phaseIdx].gvars[gvarIdx];
    if (phaseIdx == 0 || val <= GVAR_MAX_VALUE)
      return val;
    int nextPhase = val - (GVAR_MAX_VALUE + 1);
    if (nextPhase >= phaseIdx)
      nextPhase += 1;
    phaseIdx = nextPhase;
  }
  return flightModeData[0].gvars[gvarIdx];  // circular linked so return FM0 value
}

bool ModelData::isREncLinked(int phaseIdx, int reIdx)
{
  return flightModeData[phaseIdx].rotaryEncoders[reIdx] > RENC_MAX_VALUE;
}

bool ModelData::isREncLinkedCircular(int phaseIdx, int reIdx)
{
  for (int i = 0; i < CPN_MAX_FLIGHT_MODES; i++) {
    int val = flightModeData[phaseIdx].rotaryEncoders[reIdx];
    if (phaseIdx == 0 || val <= RENC_MAX_VALUE)
      return false;
    int nextPhase = val - (RENC_MAX_VALUE + 1);
    if (nextPhase >= phaseIdx)
      nextPhase += 1;
    phaseIdx = nextPhase;
  }
  return true;
}

int ModelData::getREncValue(int phaseIdx, int reIdx)
{
  for (int i = 0; i < CPN_MAX_FLIGHT_MODES; i++) {
    int val = flightModeData[phaseIdx].rotaryEncoders[reIdx];
    if (phaseIdx == 0 || val <= RENC_MAX_VALUE)
      return val;
    int nextPhase = val - (RENC_MAX_VALUE + 1);
    if (nextPhase >= phaseIdx)
      nextPhase += 1;
    phaseIdx = nextPhase;
  }
  return flightModeData[0].rotaryEncoders[reIdx];  // circular linked so return FM0 value
}

void ModelData::setTrimValue(int phaseIdx, int trimIdx, int value)
{
  for (uint8_t i = 0; i < CPN_MAX_FLIGHT_MODES; i++) {
    FlightModeData & phase = flightModeData[phaseIdx];
    int mode = phase.trimMode[trimIdx];
    int p = phase.trimRef[trimIdx];
    int & trim = phase.trim[trimIdx];
    if (mode < 0)
      return;
    if (p == phaseIdx || phaseIdx == 0) {
      trim = value;
      break;
    }
    else if (mode == 0) {
      phaseIdx = p;
    }
    else {
      trim = value - getTrimValue(p, trimIdx);
      if (trim < -500)
        trim = -500;
      if (trim > 500)
        trim = 500;
      break;
    }
  }
}

void ModelData::removeGlobalVar(int & var)
{
  if (var >= 126 && var <= 130)
    var = flightModeData[0].gvars[var - 126];
  else if (var <= -126 && var >= -130)
    var = - flightModeData[0].gvars[-126 - var];
}

ModelData ModelData::removeGlobalVars()
{
  ModelData result = *this;

  for (int i = 0; i < CPN_MAX_MIXERS; i++) {
    removeGlobalVar(mixData[i].weight);
    removeGlobalVar(mixData[i].curve.value);
    removeGlobalVar(mixData[i].sOffset);
  }

  for (int i  =0; i < CPN_MAX_EXPOS; i++) {
    removeGlobalVar(expoData[i].weight);
    removeGlobalVar(expoData[i].curve.value);
  }

  return result;
}

int ModelData::getChannelsMax(bool forceExtendedLimits) const
{
  if (forceExtendedLimits || extendedLimits)
    return IS_HORUS_OR_TARANIS(getCurrentBoard()) ? 150 : 125;
  else
    return 100;
}

bool ModelData::isFunctionSwitchPositionAvailable(int index) const
{
  if (index == 0)
    return true;

  div_t qr = div(abs(index) - 1, 3);
  int fs = getFuncSwitchConfig(qr.quot);

  if (qr.rem == 1) {
    return false;
  }
  else {
    return fs != Board::SWITCH_NOT_AVAILABLE;
  }
}

bool ModelData::isFunctionSwitchSourceAllowed(int index) const
{
  if (index >= 0 && index < Boards::getCapability(getCurrentBoard(), Board::FunctionSwitches))
    return (int)getFuncSwitchConfig(index) != Board::SWITCH_NOT_AVAILABLE;

  return false;
}


bool ModelData::isAvailable(const RawSwitch & swtch) const
{
  unsigned index = abs(swtch.index) - 1;

  if (swtch.type == SWITCH_TYPE_VIRTUAL) {
    return logicalSw[index].func != LS_FN_OFF;
  }
  else if (swtch.type == SWITCH_TYPE_FLIGHT_MODE) {
    return index == 0 || flightModeData[index].swtch.type != SWITCH_TYPE_NONE;
  }
  else if (swtch.type == SWITCH_TYPE_SENSOR) {
    return strlen(sensorData[index].label) > 0;
  }
  else {
    return true;
  }
}

float ModelData::getGVarValuePrec(int phaseIdx, int gvarIdx)
{
  return getGVarValue(phaseIdx, gvarIdx) * gvarData[gvarIdx].multiplierGet();
}

void ModelData::convert(RadioDataConversionState & cstate)
{
  // Here we can add explicit conversions when moving from one board to another

  QString origin = QString(name);
  if (origin.isEmpty())
    origin = QString::number(cstate.modelIdx+1);
  cstate.setOrigin(tr("Model: ") % origin);

  cstate.setComponent("Settings", 0);
  if (thrTraceSrc && (int)thrTraceSrc < cstate.fromBoard.getCapability(Board::Pots) + cstate.fromBoard.getCapability(Board::Sliders)) {
    cstate.setSubComp(tr("Throttle Source"));
    thrTraceSrc = RawSource(SOURCE_TYPE_INPUT, (int)thrTraceSrc + 3).convert(cstate).index - 3;
  }

  Firmware *fw = getCurrentFirmware();

  if (fw->getCapability(HasModelImage) && bitmap[0] != '\0') {
    QString filename = bitmap;
    QFileInfo file(filename);
    if (fw->getCapability(ModelImageKeepExtn) && file.suffix() == "")
      filename.append(".bmp");  // bmp is assumed default for radios that do not store file extension
    else if (!fw->getCapability(ModelImageKeepExtn) && file.suffix() != "") {
      int posn = filename.indexOf("." % file.suffix(), + 1);
      filename.remove(posn, filename.size() - posn + 1);
    }
    memset(bitmap, 0, CPN_MAX_BITMAP_LEN);
    strncpy(bitmap, filename.toLatin1().data(), fw->getCapability(ModelImageNameLen));
  }

  for (int i = 0; i < CPN_MAX_MODULES; i++) {
    moduleData[i].convert(cstate.withComponentIndex(i));
  }

  for (int i = 0; i < CPN_MAX_TIMERS; i++) {
    timers[i].convert(cstate.withComponentIndex(i));
  }

  for (int i = 0; i < CPN_MAX_MIXERS; i++) {
    mixData[i].convert(cstate.withComponentIndex(i));
  }

  for (int i = 0; i < CPN_MAX_EXPOS; i++) {
    expoData[i].convert(cstate.withComponentIndex(i));
  }

  for (int i = 0; i < CPN_MAX_LOGICAL_SWITCHES; i++) {
    logicalSw[i].convert(cstate.withComponentIndex(i));
  }

  for (int i = 0; i < CPN_MAX_SPECIAL_FUNCTIONS; i++) {
    customFn[i].convert(cstate.withComponentIndex(i));
  }

  for (int i = 0; i < CPN_MAX_FLIGHT_MODES; i++) {
    flightModeData[i].convert(cstate.withComponentIndex(i));
  }
}

#define MAX_REF_UPDATES 100

void ModelData::appendUpdateReferenceParams(const ReferenceUpdateType type, const ReferenceUpdateAction action, const int index1, const int index2, const int shift)
{
  if (updRefList) {
    //qDebug() << "Append parameters - type:" << type << " action:" << action << " index1:" << index1 << " index2:" << index2 << " shift:" << shift;
    if (updRefList->size() <= MAX_REF_UPDATES)
      updRefList->append(UpdateReferenceParams(type, action, index1, index2, shift));
    else
      qDebug() << "Warning: Update ignored as the list of updates exceeds " << MAX_REF_UPDATES;
  }
}

int ModelData::updateAllReferences(const ReferenceUpdateType type, const ReferenceUpdateAction action, const int index1, const int index2, const int shift)
{
  //Stopwatch s1("ModelData::updateAllReferences");
  //s1.report("Start");

  int loopcnt = 0;
  int updcnt = 0;
  updRefList = nullptr;
  QVector<UpdateReferenceParams> updRefParams;   //  declaring this variable in ModelData class crashes program on opening model file
  updRefList = &updRefParams;                    //  so declare pointer variable in ModelData class and pass it the address of the local array

  if (updRefList) {
    appendUpdateReferenceParams(type, action, index1, index2, shift);

    while (!updRefList->isEmpty())
      {
        if (++loopcnt > MAX_REF_UPDATES) {
          qDebug() << "Warning: Update iterations terminated early as the list exceeded " << MAX_REF_UPDATES;
          break;
        }
        //qDebug() << "Start of iteration:" << loopcnt;
        updcnt += updateReference();
        updRefList->removeFirst();
      }
  }

  qDebug() << "Iterations:" << loopcnt << " References updated:" << updcnt;
  //s1.report("Finish");

  return updcnt;
}

int ModelData::updateReference()
{
  UpdateReferenceParams p = updRefList->first();

  if (p.action < REF_UPD_ACT_CLEAR || p.action > REF_UPD_ACT_SWAP || p.type < REF_UPD_TYPE_CHANNEL || p.type > REF_UPD_TYPE_TIMER) {
    qDebug() << "Error - invalid parameters" << " > type:" << p.type << " action:" << p.action << " index1:" << p.index1 << " index2:" << p.index2 << " shift:" << p.shift;
    return 0;
  }

  memset(&updRefInfo, 0, sizeof(updRefInfo));
  updRefInfo.type = p.type;
  updRefInfo.action = p.action;
  updRefInfo.index1 = abs(p.index1);
  updRefInfo.index2 = abs(p.index2);
  updRefInfo.shift = p.shift;

  if ((updRefInfo.action == REF_UPD_ACT_SWAP && updRefInfo.index1 == updRefInfo.index2) || (updRefInfo.action == REF_UPD_ACT_SHIFT && updRefInfo.shift == 0)) {
    qDebug() << "Warning - nothing to do" << " > type:" << updRefInfo.type << " action:" << updRefInfo.action << " index1:" << updRefInfo.index1 << " index2:" << updRefInfo.index2 << " shift:" << updRefInfo.shift;
    return 0;
  }

  //Stopwatch s1("ModelData::updateReference");
  //s1.report("Start");

  Firmware *fw = getCurrentFirmware();

  updRefInfo.occurences = 1;

  switch (updRefInfo.type)
  {
    case REF_UPD_TYPE_CHANNEL:
      updRefInfo.srcType = SOURCE_TYPE_CH;
      updRefInfo.maxindex = fw->getCapability(Outputs);
      break;
    case REF_UPD_TYPE_CURVE:
      updRefInfo.maxindex = fw->getCapability(NumCurves);
      break;
    case REF_UPD_TYPE_FLIGHT_MODE:
      updRefInfo.swtchType = SWITCH_TYPE_FLIGHT_MODE;
      updRefInfo.maxindex = fw->getCapability(FlightModes);
      break;
    case REF_UPD_TYPE_GLOBAL_VARIABLE:
      updRefInfo.srcType = SOURCE_TYPE_GVAR;
      updRefInfo.maxindex = fw->getCapability(Gvars);
      break;
    case REF_UPD_TYPE_INPUT:
      updRefInfo.srcType = SOURCE_TYPE_VIRTUAL_INPUT;
      updRefInfo.maxindex = fw->getCapability(VirtualInputs);
      break;
    case REF_UPD_TYPE_LOGICAL_SWITCH:
      updRefInfo.srcType = SOURCE_TYPE_CUSTOM_SWITCH;
      updRefInfo.swtchType = SWITCH_TYPE_VIRTUAL;
      updRefInfo.maxindex = fw->getCapability(LogicalSwitches);
      break;
    case REF_UPD_TYPE_SCRIPT:
      updRefInfo.srcType = SOURCE_TYPE_LUA_OUTPUT;
      updRefInfo.maxindex = fw->getCapability(LuaScripts);
      break;
    case REF_UPD_TYPE_SENSOR:
      updRefInfo.srcType = SOURCE_TYPE_TELEMETRY;
      updRefInfo.swtchType = SWITCH_TYPE_SENSOR;
      updRefInfo.maxindex = fw->getCapability(Sensors);
      updRefInfo.occurences = 3;
      break;
    case REF_UPD_TYPE_TIMER:
      updRefInfo.srcType = SOURCE_TYPE_TIMER;
      updRefInfo.maxindex = fw->getCapability(Timers);
      break;
    default:
      qDebug() << "Error - unhandled reference type:" << updRefInfo.type;
      return 0;
  }

  updRefInfo.maxindex--;  //  getCapabilities and constants are 1 based

  //qDebug() << "updRefInfo - type:" << updRefInfo.type << " action:" << updRefInfo.action << " index1:" << updRefInfo.index1 << " index2:" << updRefInfo.index2 << " shift:" << updRefInfo.shift;
  //qDebug() << "maxindex:" << updRefInfo.maxindex << "updRefInfo - srcType:" << updRefInfo.srcType << " swtchType:" << updRefInfo.swtchType;

  //s1.report("Initialise");

  for (int i = fw->getCapability(NumFirstUsableModule); i < fw->getCapability(NumModules); i++) {
    ModuleData *md = &moduleData[i];
    if (md->protocol != PULSES_OFF && md->failsafeMode == FAILSAFE_CUSTOM && md->hasFailsafes(fw))
      updateModuleFailsafes(md);
  }
  //s1.report("Modules");

  for (int i = 0; i < CPN_MAX_TIMERS; i++) {
    TimerData *td = &timers[i];
    if (!td->isModeOff()) {
      updateTimerMode(td->swtch);
      if (td->isModeOff())
        appendUpdateReferenceParams(REF_UPD_TYPE_TIMER, REF_UPD_ACT_CLEAR, i);
    }
  }
  //s1.report("Timers");

  for (int i = 1; i < CPN_MAX_FLIGHT_MODES; i++) {  //  skip FM0 as switch not used
    FlightModeData *fmd = &flightModeData[i];
    if (fmd->swtch.isSet()) {
      updateSwitchRef(fmd->swtch);
      if(!fmd->swtch.isSet())
        appendUpdateReferenceParams(REF_UPD_TYPE_FLIGHT_MODE, REF_UPD_ACT_CLEAR, i);
    }
  }
  //s1.report("Flight Modes");

  for (int i = 0; i < CPN_MAX_EXPOS; i++) {
    ExpoData *ed = &expoData[i];
    if (!ed->isEmpty()) {
      updateSourceRef(ed->srcRaw);
      if (ed->srcRaw.isSet()) {
        updateSwitchRef(ed->swtch);
        updateCurveRef(ed->curve);
        updateAdjustRef(ed->weight);
        updateAdjustRef(ed->offset);
        updateFlightModeFlags(ed->flightModes);
      }
      else {
        unsigned int chnsave = ed->chn;
        removeInput(i);
        i--;
        if (!hasExpos(chnsave))
          appendUpdateReferenceParams(REF_UPD_TYPE_INPUT, REF_UPD_ACT_CLEAR, chnsave);
      }
    }
  }
  //s1.report("Inputs");

  for (int i = 0; i < CPN_MAX_MIXERS; i++) {
    MixData *md = &mixData[i];
    if (!md->isEmpty()) {
      updateDestCh(md);
      if (!md->isEmpty()) {
        updateSourceRef(md->srcRaw);
        if (md->srcRaw.isSet()) {
          updateSwitchRef(md->swtch);
          updateCurveRef(md->curve);
          updateAdjustRef(md->weight);
          updateAdjustRef(md->sOffset);
          updateFlightModeFlags(md->flightModes);
        }
        else {
          removeMix(i);
          i--;
        }
      }
    }
  }
  if (updRefInfo.type == REF_UPD_TYPE_CHANNEL)
    sortMixes();
  //s1.report("Mixes");

  for (int i = 0; i < CPN_MAX_CHNOUT; i++) {
    LimitData *ld = &limitData[i];
    if (!ld->isEmpty()) {
      updateAdjustRef(ld->min);
      updateAdjustRef(ld->max);
      updateAdjustRef(ld->offset);
      updateLimitCurveRef(ld->curve);
    }
  }
  //s1.report("Outputs");

  for (int i = 0; i < CPN_MAX_LOGICAL_SWITCHES; i++) {
    LogicalSwitchData *lsd = &logicalSw[i];
    if (!lsd->isEmpty()) {
      bool clearlsd = false;
      int oldval1;
      int oldval2;
      CSFunctionFamily family = lsd->getFunctionFamily();
      switch(family) {
        case LS_FAMILY_VOFS:
          if (lsd->val1 != 0) {
            updateSourceIntRef(lsd->val1);
            if (lsd->val1 == 0)
              clearlsd = true;
          }
          break;
        case LS_FAMILY_STICKY:
        case LS_FAMILY_VBOOL:
          oldval1 = lsd->val1;
          oldval2 = lsd->val2;
          if (lsd->val1 != 0)
            updateSwitchIntRef(lsd->val1);
          if (lsd->val2 != 0)
            updateSwitchIntRef(lsd->val2);
          if (lsd->val1 == 0 && lsd->val2 == 0 && ((lsd->val1 != oldval1 && oldval2 == 0) || (lsd->val2 != oldval2 && oldval1 == 0)))
            clearlsd = true;
          break;
        case LS_FAMILY_EDGE:
          if (lsd->val1 != 0) {
            updateSwitchIntRef(lsd->val1);
            if (lsd->val1 == 0)
              clearlsd = true;
          }
          break;
        case LS_FAMILY_VCOMP:
          oldval1 = lsd->val1;
          oldval2 = lsd->val2;
          if (lsd->val1 != 0)
            updateSourceIntRef(lsd->val1);
          if (lsd->val2 != 0)
            updateSourceIntRef(lsd->val2);
          if (lsd->val1 == 0 && lsd->val2 == 0 && ((lsd->val1 != oldval1 && oldval2 == 0) || (lsd->val2 != oldval2 && oldval1 == 0)))
            clearlsd = true;
          break;
        default:
          break;
      }

      if (lsd->andsw != 0)
        updateSwitchIntRef(lsd->andsw);

      if (clearlsd && lsd->andsw == 0) {
        lsd->clear();
        appendUpdateReferenceParams(REF_UPD_TYPE_LOGICAL_SWITCH, REF_UPD_ACT_CLEAR, i);
      }
    }
  }
  //s1.report("Logical Switches");

  for (int i = 0; i < CPN_MAX_SPECIAL_FUNCTIONS; i++) {
    CustomFunctionData *cfd = &customFn[i];
    if (!cfd->isEmpty()) {
      updateAssignFunc(cfd);
      if (!cfd->isEmpty()) {
        updateSwitchRef(cfd->swtch);
        if (cfd->func == FuncVolume || cfd->func == FuncBacklight || cfd->func == FuncPlayValue ||
            (cfd->func >= FuncAdjustGV1 && cfd->func <= FuncAdjustGVLast && (cfd->adjustMode == FUNC_ADJUST_GVAR_GVAR || cfd->adjustMode == FUNC_ADJUST_GVAR_SOURCE || cfd->adjustMode == FUNC_ADJUST_GVAR_SOURCERAW))) {
          updateSourceIntRef(cfd->param);
          if (cfd->param == 0)
            cfd->clear();
        }
        else if (cfd->func == FuncReset) {
          updateResetParam(cfd);
        }
      }
    }
  }
  //s1.report("Special Functions");

  if (fw->getCapability(Heli)) {
    updateSourceRef(swashRingData.aileronSource);
    updateSourceRef(swashRingData.collectiveSource);
    updateSourceRef(swashRingData.elevatorSource);
    //s1.report("Heli");
  }

  if (fw->getCapability(Telemetry)) {
    updateTelemetryRef(frsky.voltsSource);
    updateTelemetryRef(frsky.altitudeSource);
    updateTelemetryRef(frsky.currentSource);
    updateTelemetryRef(frsky.varioSource);
    for (int i = 0; i < fw->getCapability(TelemetryCustomScreens); i++) {
      switch(frsky.screens[i].type) {
        case TELEMETRY_SCREEN_BARS:
          for (int j = 0; j < fw->getCapability(TelemetryCustomScreensBars); j++) {
            FrSkyBarData *fbd = &frsky.screens[i].body.bars[j];
            updateSourceRef(fbd->source);
            if (!fbd->source.isSet()) {
              fbd->barMin = 0;
              fbd->barMax = 0;
            }
          }
          break;
        case TELEMETRY_SCREEN_NUMBERS:
          for (int j = 0; j < fw->getCapability(TelemetryCustomScreensLines); j++) {
            FrSkyLineData *fld = &frsky.screens[i].body.lines[j];
            for (int k = 0; k < fw->getCapability(TelemetryCustomScreensFieldsPerLine); k++) {
              updateSourceRef(fld->source[k]);
            }
          }
          break;
        default:
          break;
      }
    }
    //s1.report("Telemetry");
  }

  for (int i = 0; i < CPN_MAX_SENSORS; i++) {
    SensorData *sd = &sensorData[i];
    if (!sd->isEmpty() && sd->type == SensorData::TELEM_TYPE_CALCULATED) {
      if (sd->formula == SensorData::TELEM_FORMULA_CELL) {
        updateTelemetryRef(sd->source);
        if (sd->source == 0) {
          sd->clear();
          appendUpdateReferenceParams(REF_UPD_TYPE_SENSOR, REF_UPD_ACT_CLEAR, i);
        }
      }
      else if (sd->formula == SensorData::TELEM_FORMULA_DIST) {
        updateTelemetryRef(sd->gps);
        updateTelemetryRef(sd->alt);
      }
      else if (sd->formula == SensorData::TELEM_FORMULA_CONSUMPTION || sd->formula == SensorData::TELEM_FORMULA_TOTALIZE) {
        updateTelemetryRef(sd->amps);
        if (sd->amps == 0) {
          sd->clear();
          appendUpdateReferenceParams(REF_UPD_TYPE_SENSOR, REF_UPD_ACT_CLEAR, i);
        }
      }
      else {
        for (unsigned int i = 0; i < 4; i++) {
          updateTelemetryRef(sd->sources[i]);
        }
      }
    }
  }
  //s1.report("Telemetry Sensors");

  //  TODO needs lua incorporated into Companion as script needs to be parsed to determine if input field is source or value
  /*
  for (int i=0; i < CPN_MAX_SCRIPTS; i++) {
    ScriptData *sd = &scriptData[i];
    if (sd->filename[0] != '\0') {
      for (int j = 0; j < CPN_MAX_SCRIPT_INPUTS; j++) {
        //  get input parameters and for each one where type is SOURCE
        if(inputtype = "SOURCE")
          updateSourceIntRef(sd->inputs[j]);
      }
    }
  }
  s1.report("Custom Scripts");
  */

  //  TODO Horus CustomScreenData and TopBarData will need checking for updates but Companion does not current handle ie just data blobs refer modeldata.h

  qDebug() << "References updated this iteration:" << updRefInfo.updcnt;
  //s1.report("Finish");

  return updRefInfo.updcnt;
}

template <class R, typename T>
void ModelData::updateTypeIndexRef(R & curRef, const T type, const int idxAdj, const bool defClear, const int defType, const int defIndex)
{
  //qDebug() << "Raw value: " << curRef.toValue() << " String:" << curRef.toString() << " Type: " << curRef.type << " Index:" << curRef.index << " idxAdj:" << idxAdj << " defClear:" << defClear << " defType:" << defType << " defIndex:" << defIndex;
  if (!curRef.isSet() || curRef.type != type)
    return;

  R newRef;
  newRef.type = curRef.type;
  newRef.index = abs(curRef.index);

  div_t idx = div(newRef.index, updRefInfo.occurences);
  div_t newidx;

  switch (updRefInfo.action)
  {
    case REF_UPD_ACT_CLEAR:
      if (idx.quot != (updRefInfo.index1 + idxAdj))
        return;
      if (defClear)
        newRef.clear();
      else {
        newRef.type = (T)defType;
        newRef.index = defIndex;
      }
      break;
    case REF_UPD_ACT_SHIFT:
      if (idx.quot < (updRefInfo.index1 + idxAdj))
        return;

      newRef.index = ((idx.quot + updRefInfo.shift) * updRefInfo.occurences) + idx.rem;
      newidx = div(newRef.index, updRefInfo.occurences);

      if (newidx.quot < (updRefInfo.index1 + idxAdj) || newidx.quot > (updRefInfo.maxindex + idxAdj)) {
        if (defClear)
          newRef.clear();
        else {
          newRef.type = (T)defType;
          newRef.index = defIndex + idxAdj;
        }
      }
      break;
    case REF_UPD_ACT_SWAP:
      if (idx.quot == updRefInfo.index1 + idxAdj)
        newRef.index = ((updRefInfo.index2 + idxAdj) * updRefInfo.occurences) + idx.rem;
      else if (idx.quot == updRefInfo.index2 + idxAdj)
        newRef.index = ((updRefInfo.index1 + idxAdj) * updRefInfo.occurences) + idx.rem;
      break;
    default:
      qDebug() << "Error - unhandled action:" << updRefInfo.action;
      return;
  }

  if (curRef.type != newRef.type || abs(curRef.index) != newRef.index) {
    newRef.index = curRef.index < 0 ? -newRef.index : newRef.index;
    //qDebug() << "Updated reference: " << curRef.toString() << " -> " << newRef.toString();
    curRef = newRef;
    updRefInfo.updcnt++;
  }
}

template <class R, typename T>
void ModelData::updateTypeValueRef(R & curRef, const T type, const int idxAdj, const bool defClear, const int defType, const int defValue)
{
  //qDebug() << " String:" << curRef.toString() << " Type: " << curRef.type << " Value:" << curRef.value;
  if (!curRef.isSet() || curRef.type != type)
    return;

  R newRef;
  newRef.type = curRef.type;
  newRef.value = abs(curRef.value);

  switch (updRefInfo.action)
  {
    case REF_UPD_ACT_CLEAR:
      if (newRef.value != (updRefInfo.index1 + idxAdj))
        return;
      if (defClear)
        newRef.clear();
      else {
        newRef.type = (T)defType;
        newRef.value = defValue;
      }
      break;
    case REF_UPD_ACT_SHIFT:
      if (newRef.value < (updRefInfo.index1 + idxAdj))
        return;

      newRef.value += updRefInfo.shift;

      if (newRef.value < (updRefInfo.index1 + idxAdj) || newRef.value > (updRefInfo.maxindex + idxAdj)) {
        if (defClear)
          newRef.clear();
        else {
          newRef.type = (T)defType;
          newRef.value = defValue + idxAdj;
        }
      }
      break;
    case REF_UPD_ACT_SWAP:
      if (newRef.value == updRefInfo.index1 + idxAdj)
        newRef.value = updRefInfo.index2 + idxAdj;
      else if (newRef.value == updRefInfo.index2 + idxAdj)
        newRef.value = updRefInfo.index1 + idxAdj;
      break;
    default:
      qDebug() << "Error - unhandled action:" << updRefInfo.action;
      return;
  }

  if (curRef.type != newRef.type || abs(curRef.value) != newRef.value) {
    newRef.value = curRef.value < 0 ? -newRef.value : newRef.value;
    //qDebug() << "Updated reference: " << curRef.toString() << " -> " << newRef.toString();
    curRef = newRef;
    updRefInfo.updcnt++;
  }
}

void ModelData::updateCurveRef(CurveReference & crv)
{
  if (updRefInfo.type == REF_UPD_TYPE_GLOBAL_VARIABLE && (crv.type == CurveReference::CURVE_REF_DIFF || crv.type == CurveReference::CURVE_REF_EXPO))
    updateAdjustRef(crv.value);
  else if (updRefInfo.type == REF_UPD_TYPE_CURVE && crv.type == CurveReference::CURVE_REF_CUSTOM)
    updateTypeValueRef<CurveReference, CurveReference::CurveRefType>(crv, CurveReference::CURVE_REF_CUSTOM, 1);
}

void ModelData::updateLimitCurveRef(CurveReference & crv)
{
  CurveReference src = CurveReference(CurveReference::CURVE_REF_CUSTOM, crv.value);
  updateCurveRef(src);
  if (crv.value != src.value)
    crv.value = src.value;
}

void ModelData::updateAdjustRef(int & value)
{
  if (updRefInfo.type != REF_UPD_TYPE_GLOBAL_VARIABLE)
    return;

  AdjustmentReference adj = AdjustmentReference(value);
  updateTypeValueRef<AdjustmentReference, AdjustmentReference::AdjustRefType>(adj, AdjustmentReference::ADJUST_REF_GVAR, 1);
  if (value != adj.toValue())
    value = adj.toValue();
}

void ModelData::updateAssignFunc(CustomFunctionData * cfd)
{
  const int invalidateRef = -1;
  int newRef = (int)cfd->func;
  int idxAdj = 0;

  switch (updRefInfo.type)
  {
    case REF_UPD_TYPE_CHANNEL:
      if(cfd->func < FuncOverrideCH1 || cfd->func > FuncOverrideCHLast)
        return;
      idxAdj = FuncOverrideCH1;
      break;
    case REF_UPD_TYPE_GLOBAL_VARIABLE:
      if (cfd->func < FuncAdjustGV1 || cfd->func > FuncAdjustGVLast)
        return;
      idxAdj = FuncAdjustGV1;
      break;
    case REF_UPD_TYPE_TIMER:
      if (cfd->func < FuncSetTimer1 || cfd->func > FuncSetTimerLast)
        return;
      idxAdj = FuncSetTimer1;
      break;
    default:
      return;
  }

  switch (updRefInfo.action)
  {
    case REF_UPD_ACT_CLEAR:
      if (newRef != (updRefInfo.index1 + idxAdj))
        return;
      newRef = invalidateRef;
      break;
    case REF_UPD_ACT_SHIFT:
      if (newRef < (updRefInfo.index1 + idxAdj))
        return;

      newRef += updRefInfo.shift;

      if (newRef < (updRefInfo.index1 + idxAdj) || newRef > (updRefInfo.maxindex + idxAdj))
        newRef = invalidateRef;
      break;
    case REF_UPD_ACT_SWAP:
      if (newRef == updRefInfo.index1 + idxAdj)
        newRef = updRefInfo.index2 + idxAdj;
      else if (newRef == updRefInfo.index2 + idxAdj)
        newRef = updRefInfo.index1 + idxAdj;
      break;
    default:
      qDebug() << "Error - unhandled action:" << updRefInfo.action;
      return;
  }

  if (newRef == invalidateRef) {
    cfd->clear();
    //qDebug() << "Function cleared";
    updRefInfo.updcnt++;
  }
  else if (cfd->func != (AssignFunc)newRef) {
    //qDebug() << "Updated reference:" << cfd->func << " -> " << newRef;
    cfd->func = (AssignFunc)newRef;
    updRefInfo.updcnt++;
  }
}

void ModelData::updateDestCh(MixData * md)
{
  if (updRefInfo.type != REF_UPD_TYPE_CHANNEL)
    return;

  const int invalidateRef = -1;
  const int idxAdj = 1;
  int newRef = md->destCh;
  switch (updRefInfo.action)
  {
    case REF_UPD_ACT_CLEAR:
      if (newRef != (updRefInfo.index1 + idxAdj))
        return;
      newRef = invalidateRef;
      break;
    case REF_UPD_ACT_SHIFT:
      if (newRef < (updRefInfo.index1 + idxAdj))
        return;

      newRef += updRefInfo.shift;

      if (newRef < (updRefInfo.index1 + idxAdj) || newRef > (updRefInfo.maxindex + idxAdj))
        newRef = invalidateRef;
      break;
    case REF_UPD_ACT_SWAP:
      if (newRef == updRefInfo.index1 + idxAdj)
        newRef = updRefInfo.index2 + idxAdj;
      else if (newRef == updRefInfo.index2 + idxAdj)
        newRef = updRefInfo.index1 + idxAdj;
      break;
    default:
      qDebug() << "Error - unhandled action:" << updRefInfo.action;
      return;
  }

  if (newRef == invalidateRef) {
    md->clear();
    //qDebug() << "Mix cleared";
    updRefInfo.updcnt++;
  }
  else if (md->destCh != static_cast<unsigned int>(newRef)) {
    //qDebug() << "Updated reference:" << md->destCh << " -> " << newRef;
    md->destCh = newRef;
    updRefInfo.updcnt++;
  }
}

void ModelData::updateFlightModeFlags(unsigned int & curRef)
{
  if (updRefInfo.type != REF_UPD_TYPE_FLIGHT_MODE || curRef == 0 /*all selected*/ || curRef == 511 /*all deselected*/)
   return;

  if (updRefInfo.index1 > CPN_MAX_FLIGHT_MODES || updRefInfo.index2 > CPN_MAX_FLIGHT_MODES)
    return;

  unsigned int newRef = curRef;
  bool flag[CPN_MAX_FLIGHT_MODES];
  bool f;

  int mask = 1;
  for (int i = 0; i < CPN_MAX_FLIGHT_MODES; i++) {
    flag[i] = curRef & mask;
    mask <<= 1;
  }

  switch (updRefInfo.action)
  {
    case REF_UPD_ACT_CLEAR:
      flag[updRefInfo.index1] = true;
      break;
    case REF_UPD_ACT_SHIFT:
        if(updRefInfo.shift < 0) {
          for (int i = updRefInfo.index1; i < CPN_MAX_FLIGHT_MODES; i++) {
            if (i - updRefInfo.shift <= updRefInfo.maxindex)
              flag[i] = flag[i - updRefInfo.shift];
            else
              flag[i] = true;
          }
        }
        else {
          for (int i = CPN_MAX_FLIGHT_MODES - 1; i >= updRefInfo.index1; i--) {
            if (i - updRefInfo.shift >= updRefInfo.index1)
              flag[i] = flag[i - updRefInfo.shift];
            else
              flag[i] = true;
          }
        }
      break;
    case REF_UPD_ACT_SWAP:
      f = flag[updRefInfo.index1];
      flag[updRefInfo.index1] = flag[updRefInfo.index2];
      flag[updRefInfo.index2] = f;
      break;
    default:
      qDebug() << "Error - unhandled action:" << updRefInfo.action;
      return;
  }

  newRef = 0;
  for (int i = CPN_MAX_FLIGHT_MODES - 1; i >= 0 ; i--) {
    if (flag[i])
      newRef++;
    newRef <<= 1;
  }
  newRef >>= 1;

  if (curRef != newRef) {
    //qDebug() << "Updated reference:" << curRef << " -> " << newRef;
    curRef = newRef;
    updRefInfo.updcnt++;
  }
}

void ModelData::updateTelemetryRef(int & curRef)
{
  if (updRefInfo.type != REF_UPD_TYPE_SENSOR)
    return;

  const int idxAdj = 1;
  int newRef = curRef;

  switch (updRefInfo.action)
  {
    case REF_UPD_ACT_CLEAR:
      if (newRef != (updRefInfo.index1 + idxAdj))
        return;
      newRef = 0;
      break;
    case REF_UPD_ACT_SHIFT:
      if (newRef < (updRefInfo.index1 + idxAdj))
        return;

      newRef += updRefInfo.shift;

      if (newRef < (updRefInfo.index1 + idxAdj) || newRef > (updRefInfo.maxindex + idxAdj))
        newRef = 0;
      break;
    case REF_UPD_ACT_SWAP:
      if (newRef == updRefInfo.index1 + idxAdj)
        newRef = updRefInfo.index2 + idxAdj;
      else if (newRef == updRefInfo.index2 + idxAdj)
        newRef = updRefInfo.index1 + idxAdj;
      break;
    default:
      qDebug() << "Error - unhandled action:" << updRefInfo.action;
      return;
  }

  if (curRef != newRef) {
    //qDebug() << "Updated reference:" << curRef << " -> " << newRef;
    curRef = newRef;
    updRefInfo.updcnt++;
  }
}

void ModelData::updateTelemetryRef(unsigned int & curRef)
{
  int newRef = (int)curRef;
  updateTelemetryRef(newRef);

  if (curRef != static_cast<unsigned int>(newRef)) {
    curRef = (unsigned int)newRef;
  }
}

void ModelData::updateModuleFailsafes(ModuleData * md)
{
  if (updRefInfo.type != REF_UPD_TYPE_CHANNEL)
    return;

  bool updated = false;

  switch (updRefInfo.action) {
    case REF_UPD_ACT_CLEAR:
      break;
    case REF_UPD_ACT_SHIFT:
      if (updRefInfo.shift == 0 || updRefInfo.index1 < 0 || updRefInfo.index1 > CPN_MAX_CHNOUT - 1)
        return;

      if (updRefInfo.shift > 0) {
        for (int i = CPN_MAX_CHNOUT - 1; i > updRefInfo.index1; i--) {
          limitData[i].failsafe = limitData[i - 1].failsafe;
        }
        limitData[updRefInfo.index1].failsafe = 0;
      }
      else {
        for (int i = updRefInfo.index1 + 1; i < CPN_MAX_CHNOUT - 1; i++) {
          limitData[i - 1].failsafe = limitData[i].failsafe;
        }
        limitData[CPN_MAX_CHNOUT - 1].failsafe = 0;
      }
      updated = true;
      break;
    case REF_UPD_ACT_SWAP:
      int tmp;
      if (updRefInfo.index1 >= 0 && updRefInfo.index1 < CPN_MAX_CHNOUT) {
        updated = true;
        tmp = limitData[updRefInfo.index1].failsafe;
        if (updRefInfo.index2 >= 0 && updRefInfo.index2 < CPN_MAX_CHNOUT)
          limitData[updRefInfo.index1].failsafe = limitData[updRefInfo.index2].failsafe;
        else
          limitData[updRefInfo.index1].failsafe = 0;
      }
      else
        tmp = 0;
      if (updRefInfo.index2 >= 0 && updRefInfo.index2 < CPN_MAX_CHNOUT) {
        updated = true;
        limitData[updRefInfo.index2].failsafe = tmp;
      }
      break;
    default:
      qDebug() << "Error - unhandled action:" << updRefInfo.action;
      return;
  }

  if (updated) {
    //qDebug() << "Updated module failsafes";
    updRefInfo.updcnt++;
  }
}

int ModelData::linkedFlightModeIndexToValue(const int phaseIdx, const int useFmIdx, const int maxOwnValue)
{
  int val;

  if (phaseIdx == useFmIdx || phaseIdx < 0 || phaseIdx > (CPN_MAX_FLIGHT_MODES - 1) || useFmIdx < 0 || useFmIdx > (CPN_MAX_FLIGHT_MODES - 1))
    val = flightModeData[phaseIdx].linkedFlightModeZero(phaseIdx, maxOwnValue);
  else
    val = maxOwnValue + useFmIdx + (useFmIdx >= phaseIdx ? 0 : 1);

  return val;
}

int ModelData::linkedFlightModeValueToIndex(const int phaseIdx, const int val, const int maxOwnValue)
{
  int idx = val - maxOwnValue - 1;
  if (idx >= phaseIdx)
    idx += 1;
  return idx;
}

int ModelData::getGVarFlightModeIndex(const int phaseIdx, const int gvarIdx)
{
  if (!isGVarLinked(phaseIdx, gvarIdx))
    return -1;
  return (linkedFlightModeValueToIndex(phaseIdx, flightModeData[phaseIdx].gvars[gvarIdx], GVAR_MAX_VALUE));
}

void ModelData::setGVarFlightModeIndexToValue(const int phaseIdx, const int gvarIdx, const int useFmIdx)
{
  flightModeData[phaseIdx].gvars[gvarIdx] = linkedFlightModeIndexToValue(phaseIdx, useFmIdx, GVAR_MAX_VALUE);
}

int ModelData::getREncFlightModeIndex(const int phaseIdx, const int reIdx)
{
  if (!isREncLinked(phaseIdx, reIdx))
    return -1;
  return (linkedFlightModeValueToIndex(phaseIdx, flightModeData[phaseIdx].rotaryEncoders[reIdx], RENC_MAX_VALUE));
}

void ModelData::setREncFlightModeIndexToValue(const int phaseIdx, const int reIdx, const int useFmIdx)
{
  flightModeData[phaseIdx].rotaryEncoders[reIdx] = linkedFlightModeIndexToValue(phaseIdx, useFmIdx, RENC_MAX_VALUE);
}

bool ModelData::isExpoParent(const int index)
{
  const ExpoData &ed = expoData[index];
  const QVector<const ExpoData *> chexpos = expos(ed.chn);
  return chexpos.constFirst() == &ed;
}

bool ModelData::isExpoChild(const int index)
{
  const ExpoData &ed = expoData[index];
  const QVector<const ExpoData *> chexpos = expos(ed.chn);
  return chexpos.constFirst() != &ed;
}

bool ModelData::hasExpoChildren(const int index)
{
  const ExpoData &ed = expoData[index];
  const QVector<const ExpoData *> chexpos = expos(ed.chn);
  return chexpos.constFirst() == &ed && chexpos.constLast() != &ed;
}

bool ModelData::hasExpoSiblings(const int index)
{
  const ExpoData &ed = expoData[index];
  const QVector<const ExpoData *> chexpos = expos(ed.chn);
  return !isExpoParent(index) && chexpos.size() > 2;
}

void ModelData::removeMix(const int idx)
{
  memmove(&mixData[idx], &mixData[idx + 1], (CPN_MAX_MIXERS - (idx + 1)) * sizeof(MixData));
  mixData[CPN_MAX_MIXERS - 1].clear();
}

void ModelData::sortMixes()
{
  unsigned int lastchn = 0;
  bool sortreq = false;

  for (int i = 0; i < CPN_MAX_MIXERS; i++) {
    MixData *md = &mixData[i];
    if (!md->isEmpty()) {
      if (md->destCh < lastchn) {
        sortreq = true;
        break;
      }
      else
        lastchn = md->destCh;
    }
  }

  if (!sortreq)
    return;

  //  QMap automatically sorts based on key
  QMap<int, int> map;
  for (int i = 0; i < CPN_MAX_MIXERS; i++) {
    MixData *md = &mixData[i];
    if (!md->isEmpty()) {
      //  destCh may not be unique so build a compound sort key
      map.insert(md->destCh * (CPN_MAX_MIXERS + 1) + i, i);
    }
  }

  MixData sortedMixData[CPN_MAX_MIXERS];
  int destidx = 0;

  QMap<int, int>::const_iterator i;
  for (i = map.constBegin(); i != map.constEnd(); ++i) {
    memcpy(&sortedMixData[destidx], &mixData[i.value()], sizeof(MixData));
    destidx++;
  }

  memcpy(&mixData[0], &sortedMixData[0], CPN_MAX_MIXERS * sizeof(MixData));
}

void ModelData::updateResetParam(CustomFunctionData * cfd)
{
  if (cfd->func != FuncReset)
    return;

  const int invalidateRef = -1;
  int newRef = cfd->param;
  int idxAdj = 0;
  Firmware *firmware = getCurrentFirmware();

  switch (updRefInfo.type)
  {
    case REF_UPD_TYPE_TIMER:
      if (cfd->param < 0 || cfd->param > 2)
        return;
      idxAdj = -2;   //  reverse earlier offset required for rawsource
      break;
    case REF_UPD_TYPE_SENSOR:
      idxAdj = 5/*3 Timers + Flight + Telemetery*/ + firmware->getCapability(RotaryEncoders);
      if (cfd->param < idxAdj || cfd->param > (idxAdj + firmware->getCapability(Sensors)))
        return;
      break;
    default:
      return;
  }

  switch (updRefInfo.action)
  {
    case REF_UPD_ACT_CLEAR:
      if (newRef != (updRefInfo.index1 + idxAdj))
        return;
      newRef = invalidateRef;
      break;
    case REF_UPD_ACT_SHIFT:
      if (newRef < (updRefInfo.index1 + idxAdj))
        return;

      newRef += updRefInfo.shift;

      if (newRef < (updRefInfo.index1 + idxAdj) || newRef > (updRefInfo.maxindex + idxAdj))
        newRef = invalidateRef;
      break;
    case REF_UPD_ACT_SWAP:
      if (newRef == updRefInfo.index1 + idxAdj)
        newRef = updRefInfo.index2 + idxAdj;
      else if (newRef == updRefInfo.index2 + idxAdj)
        newRef = updRefInfo.index1 + idxAdj;
      break;
    default:
      qDebug() << "Error - unhandled action:" << updRefInfo.action;
      return;
  }

  if (newRef == invalidateRef) {
    cfd->clear();
    //qDebug() << "Function cleared";
    updRefInfo.updcnt++;
  }
  else if (cfd->param != newRef) {
    //qDebug() << "Updated reference:" << cfd->param << " -> " << newRef;
    cfd->param = newRef;
    updRefInfo.updcnt++;
  }
}

QString ModelData::thrTraceSrcToString() const
{
  return thrTraceSrcToString(nullptr, (int)thrTraceSrc);
}

QString ModelData::thrTraceSrcToString(const GeneralSettings * generalSettings, const int index) const
{
  const Board::Type board = getCurrentBoard();
  const int pscnt = Boards::getCapability(board, Board::Pots) + Boards::getCapability(board, Board::Sliders);

  if (index == 0)
    return Boards::getCapability(board, Board::Air) ? tr("THR") : tr("TH");
  else if (index <= pscnt)
    //return Boards::getInputName(index + Boards::getCapability(board, Board::Sticks) - 1, board);
    return RawSource(SOURCE_TYPE_INPUT, index + Boards::getCapability(board, Board::Sticks)).toString(this, generalSettings, board);
  else if (index <= pscnt + getCurrentFirmware()->getCapability(Outputs))
    return RawSource(SOURCE_TYPE_CH, index - pscnt).toString(this);

  return QString(CPN_STR_UNKNOWN_ITEM);
}

int ModelData::thrTraceSrcCount() const
{
  Firmware * firmware = getCurrentFirmware();
  const Board::Type board = firmware->getBoard();

  return 1 + Boards::getCapability(board, Board::Pots) + Boards::getCapability(board, Board::Sliders) + firmware->getCapability(Outputs);
}

bool ModelData::isThrTraceSrcAvailable(const GeneralSettings * generalSettings, const int index) const
{
  const Board::Type board = getCurrentBoard();

  if (index == 0)
    return true;
  else if (index > 0 && index <= Boards::getCapability(board, Board::Pots) + Boards::getCapability(board, Board::Sliders))
    return RawSource(SOURCE_TYPE_INPUT, index + Boards::getCapability(board, Board::Sticks)).isAvailable(this, generalSettings, board);
  else
    return hasMixes(index - Boards::getCapability(board, Board::Pots) - Boards::getCapability(board, Board::Sliders) - 1);
}

void ModelData::limitsClear(const int index)
{
  if (index < 0 || index >= CPN_MAX_CHNOUT)
    return;

  if (!limitData[index].isEmpty()) {
    limitData[index].clear();
    updateAllReferences(REF_UPD_TYPE_CHANNEL, REF_UPD_ACT_CLEAR, index);
  }
}

void ModelData::limitsClearAll()
{
  for (int i = 0; i < CPN_MAX_CHNOUT; i++) {
    limitsClear(i);
  }
}

void ModelData::limitsDelete(const int index)
{
  if (index < 0 || index >= CPN_MAX_CHNOUT)
    return;

  memmove(&limitData[index], &limitData[index + 1], (CPN_MAX_CHNOUT - (index + 1)) * sizeof(LimitData));
  limitData[CPN_MAX_CHNOUT - 1].clear();
  updateAllReferences(REF_UPD_TYPE_CHANNEL, REF_UPD_ACT_SHIFT, index, 0, -1);
}

void ModelData::limitsGet(const int index, QByteArray & data)
{
  if (index < 0 || index >= CPN_MAX_CHNOUT)
    return;

  data.append((char*)&limitData[index], sizeof(LimitData));
}

void ModelData::limitsInsert(const int index)
{
  if (index < 0 || index >= CPN_MAX_CHNOUT)
    return;

  memmove(&limitData[index + 1], &limitData[index], (CPN_MAX_CHNOUT - (index + 1)) * sizeof(LimitData));
  limitData[index].clear();
  updateAllReferences(REF_UPD_TYPE_CHANNEL, REF_UPD_ACT_SHIFT, index, 0, 1);
}

void ModelData::limitsMove(const int index, const int offset)
{
  if (index + offset < 0 || index + offset >= CPN_MAX_CHNOUT)
    return;

  int idx1 = index;
  int idx2;
  const int direction = offset < 0 ? -1 : 1;
  const int cnt = abs(offset);

  for (int i = 1; i <= cnt; i++) {
    idx2 = idx1 + direction;
    LimitData tmp = limitData[idx2];
    LimitData *d1 = &limitData[idx1];
    LimitData *d2 = &limitData[idx2];
    memcpy(d2, d1, sizeof(LimitData));
    memcpy(d1, &tmp, sizeof(LimitData));
    updateAllReferences(REF_UPD_TYPE_CHANNEL, REF_UPD_ACT_SWAP, idx1, idx2);
    idx1 += direction;
  }
}

void ModelData::limitsSet(const int index, const QByteArray & data)
{
  if (index < 0 || index >= CPN_MAX_CHNOUT || data.size() < (int)sizeof(LimitData))
    return;

  memcpy(&limitData[index], data.constData(), sizeof(LimitData));
}

QString ModelData::trainerModeToString() const
{
  return trainerModeToString(trainerMode);
}

//  static
QString ModelData::trainerModeToString(int value)
{
  switch (value) {
    case TRAINER_MODE_OFF:
      return tr("OFF");
    case TRAINER_MODE_MASTER_JACK:
      return tr("Master/Jack");
    case TRAINER_MODE_SLAVE_JACK:
      return tr("Slave/Jack");
    case TRAINER_MODE_MASTER_SBUS_EXTERNAL_MODULE:
      return tr("Master/SBUS Module");
    case TRAINER_MODE_MASTER_CPPM_EXTERNAL_MODULE:
      return tr("Master/CPPM Module");
    case TRAINER_MODE_MASTER_SERIAL:
      return tr("Master/Serial");
    case TRAINER_MODE_MASTER_BLUETOOTH:
      return tr("Master/Bluetooth");
    case TRAINER_MODE_SLAVE_BLUETOOTH:
      return tr("Slave/Bluetooth");
    case TRAINER_MODE_MULTI:
      return tr("Master/Multi");
    default:
      return CPN_STR_UNKNOWN_ITEM;
  }
}

bool ModelData::isTrainerModeAvailable(const GeneralSettings & generalSettings, const Firmware * firmware, const int value)
{
  if (value < TRAINER_MODE_FIRST || value > TRAINER_MODE_LAST)
    return false;

  const Board::Type board = firmware->getBoard();

  if (value == TRAINER_MODE_MASTER_SBUS_EXTERNAL_MODULE &&
      IS_TARANIS_X9E(board) &&
      generalSettings.bluetoothMode)
    return false;

  if (value == TRAINER_MODE_MASTER_SBUS_EXTERNAL_MODULE &&
      !Boards::getCapability(board, Board::HasTrainerModuleSBUS))
    return false;

  if ((value == TRAINER_MODE_MASTER_SBUS_EXTERNAL_MODULE || value == TRAINER_MODE_MASTER_CPPM_EXTERNAL_MODULE) &&
      (Boards::getCapability(board, Board::HasTrainerModuleCPPM) ||
       Boards::getCapability(board, Board::HasTrainerModuleSBUS)) &&
      moduleData[1].protocol != PULSES_OFF)
    return false;

  if (value == TRAINER_MODE_MASTER_SERIAL &&
      (generalSettings.serialPort[GeneralSettings::SP_AUX1] != GeneralSettings::AUX_SERIAL_SBUS_TRAINER &&
       generalSettings.serialPort[GeneralSettings::SP_AUX2] != GeneralSettings::AUX_SERIAL_SBUS_TRAINER))
    return false;

  if ((value == TRAINER_MODE_MASTER_BLUETOOTH || value == TRAINER_MODE_SLAVE_BLUETOOTH) &&
      !IS_TARANIS_X9E(board) &&
      generalSettings.bluetoothMode != GeneralSettings::BLUETOOTH_MODE_TRAINER)
    return false;

  if ((value == TRAINER_MODE_MASTER_JACK || value == TRAINER_MODE_SLAVE_JACK) &&
      ((IS_TARANIS_XLITE(board) && !IS_TARANIS_X9LITES(board)) || IS_IFLIGHT_COMMANDO8(board)))
    return false;

  if (value == TRAINER_MODE_MASTER_CPPM_EXTERNAL_MODULE &&
      !Boards::getCapability(board, Board::HasTrainerModuleCPPM))
    return false;

  if (value == TRAINER_MODE_MULTI &&
      ((!Boards::getCapability(board, Board::HasInternalModuleSupport) &&
        !Boards::getCapability(board, Board::HasExternalModuleSupport)) ||
       (moduleData[0].protocol != PULSES_MULTIMODULE && moduleData[1].protocol != PULSES_MULTIMODULE)))
    return false;

  return true;
}

AbstractStaticItemModel * ModelData::trainerModeItemModel(const GeneralSettings & generalSettings, const Firmware * firmware)
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName(AIM_MODELDATA_TRAINERMODE);

  for (int i = TRAINER_MODE_FIRST; i <= TRAINER_MODE_LAST; i++) {
    mdl->appendToItemList(trainerModeToString(i), i, isTrainerModeAvailable(generalSettings, firmware, i));
  }

  mdl->loadItemList();
  return mdl;
}

unsigned int ModelData::getFuncSwitchConfig(unsigned int index) const
{
  if (index < CPN_MAX_SWITCHES_FUNCTION)
    return Helpers::getBitmappedValue(functionSwitchConfig, index, 2);
  else
    return FUNC_SWITCH_CONFIG_NONE;
}

void ModelData::setFuncSwitchConfig(unsigned int index, unsigned int value)
{
  if (index < CPN_MAX_SWITCHES_FUNCTION)
    Helpers::setBitmappedValue(functionSwitchConfig, value, index, 2);
}

//  static
QString ModelData::funcSwitchConfigToString(unsigned int value)
{
  switch (value) {
    case FUNC_SWITCH_CONFIG_NONE:
      return tr("NONE");
    case FUNC_SWITCH_CONFIG_TOGGLE:
      return tr("TOGGLE");
    case FUNC_SWITCH_CONFIG_2POS:
      return tr("2POS");
    default:
      return CPN_STR_UNKNOWN_ITEM;
  }
}

//  static
AbstractStaticItemModel * ModelData::funcSwitchConfigItemModel()
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName(AIM_MODELDATA_FUNCSWITCHCONFIG);
    for (unsigned int i = FUNC_SWITCH_CONFIG_FIRST; i <= FUNC_SWITCH_CONFIG_LAST; i++) {
      mdl->appendToItemList(funcSwitchConfigToString(i), i);
  }
  mdl->loadItemList();
  return mdl;
}

AbstractStaticItemModel * ModelData::funcSwitchGroupStartSwitchModel(int switchcnt)
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName(AIM_MODELDATA_FUNCSWITCHGROUPSTARTSWITCH);

  mdl->appendToItemList(tr("Restore"), 0);
  for (unsigned int i = 1; i <= switchcnt; i += 1) {
    mdl->appendToItemList(tr("SW") + QString::number(i), i);
  }
  mdl->appendToItemList(tr("Off"), switchcnt + 1);

  mdl->loadItemList();
  return mdl;
}

AbstractStaticItemModel * ModelData::funcSwitchGroupsModel()
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName(AIM_MODELDATA_FUNCSWITCHGROUPS);

  mdl->appendToItemList(tr("---"), 0);
  for (unsigned int i = 1; i <= 3; i += 1) {
    mdl->appendToItemList(tr("Group ") + QString::number(i), i);
  }

  mdl->loadItemList();
  return mdl;
}

unsigned int ModelData::getFuncSwitchGroup(unsigned int index) const
{
  if (index < CPN_MAX_SWITCHES_FUNCTION)
    return Helpers::getBitmappedValue(functionSwitchGroup, index, 2);
  else
    return 0;
}

void ModelData::setFuncSwitchGroup(unsigned int index, unsigned int value)
{
  if (index < CPN_MAX_SWITCHES_FUNCTION)
    Helpers::setBitmappedValue(functionSwitchGroup, value, index, 2);
}

unsigned int ModelData::getFuncSwitchAlwaysOnGroup(unsigned int group) const
{
  if (group > 0 && group < 4) {
    unsigned int switchcnt = Boards::getCapability(getCurrentFirmware()->getBoard(), Board::FunctionSwitches);
    return Helpers::getBitmappedValue(functionSwitchGroup, group, 1, 2 * switchcnt);
  }
  else
    return 0;
}

unsigned int ModelData::getFuncSwitchAlwaysOnGroupForSwitch(unsigned int index) const
{
  if (index < CPN_MAX_SWITCHES_FUNCTION)
    return getFuncSwitchAlwaysOnGroup(getFuncSwitchGroup(index));
  else
    return 0;
}

void ModelData::setFuncSwitchAlwaysOnGroup(unsigned int group, unsigned int value)
{
  if (group > 0 && group < 4) {
    unsigned int switchcnt = Boards::getCapability(getCurrentFirmware()->getBoard(), Board::FunctionSwitches);
    Helpers::setBitmappedValue(functionSwitchGroup, value, group, 1, 2 * switchcnt);
  }
}

unsigned int ModelData::getFuncSwitchStart(unsigned int index) const
{
  if (index < CPN_MAX_SWITCHES_FUNCTION)
    return Helpers::getBitmappedValue(functionSwitchStartConfig, index, 2);
  else
    return FUNC_SWITCH_START_OFF;
}

void ModelData::setFuncSwitchStart(unsigned int index, unsigned int value)
{
  if (index < CPN_MAX_SWITCHES_FUNCTION)
    Helpers::setBitmappedValue(functionSwitchStartConfig, value, index, 2);
}

int ModelData::getFuncGroupSwitchCount(unsigned int group, int switchcnt) const
{
  int grpSwitchCount = 0;
  for (int i = 0; i < switchcnt; i += 1) {
    if (getFuncSwitchGroup(i) == group) {
      grpSwitchCount += 1;
    }
  }
  return grpSwitchCount;
}

unsigned int ModelData::getFuncGroupSwitchStart(unsigned int group, int switchcnt) const
{
  bool allDown = true;
  for (int i = 0; i < switchcnt; i += 1) {
    if (getFuncSwitchGroup(i) == group) {
      if (getFuncSwitchStart(i) == FUNC_SWITCH_START_ON)
        return i + 1;
      if (getFuncSwitchStart(i) != FUNC_SWITCH_START_OFF)
        allDown = false;
    }
  }
  if (allDown && getFuncGroupSwitchCount(group, switchcnt) > 0) return switchcnt + 1;
  return 0;
}

void ModelData::setFuncGroupSwitchStart(unsigned int group, unsigned int value, int switchcnt)
{
  for (int i = 0; i < switchcnt; i += 1) {
    if (getFuncSwitchGroup(i) == group)
      setFuncSwitchStart(i, value ? ModelData::FUNC_SWITCH_START_OFF : ModelData::FUNC_SWITCH_START_PREVIOUS);
  }
  if (value > 0 && value <= switchcnt) {
    setFuncSwitchStart(value - 1, ModelData::FUNC_SWITCH_START_ON);
  }
}

void ModelData::setGroupSwitchState(uint8_t group, int switchcnt)
{
  // Check rules for always on group
  //  - Toggle switch type not valid, change all switches to 2POS
  //  - One switch must be turned on, turn on first switch if needed
  if (getFuncSwitchAlwaysOnGroup(group)) {
    for (int j = 0; j < switchcnt; j += 1) {
      if (getFuncSwitchGroup(j) == group) {
        setFuncSwitchConfig(j, FUNC_SWITCH_CONFIG_2POS); // Toggle not valid
      }
    }
    if (getFuncGroupSwitchStart(group, switchcnt) == switchcnt + 1) {
      // Start state for all switches is off - set all to 'last'
      for (int j = 0; j < switchcnt; j += 1)
        if (getFuncSwitchGroup(j) == group)
          setFuncSwitchStart(j, FUNC_SWITCH_START_PREVIOUS);
    }
  }
}

 //  static
 QString ModelData::funcSwitchStartToString(unsigned int value)
 {
   switch (value) {
     case FUNC_SWITCH_START_OFF:
       return tr("Off");
     case FUNC_SWITCH_START_ON:
       return tr("On");
     case FUNC_SWITCH_START_PREVIOUS:
       return tr("Restore");
     default:
       return CPN_STR_UNKNOWN_ITEM;
   }
 }

 //  static
 AbstractStaticItemModel * ModelData::funcSwitchStartItemModel()
 {
   AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
   mdl->setName(AIM_MODELDATA_FUNCSWITCHSTART);

   for (unsigned int i = FUNC_SWITCH_START_FIRST; i <= FUNC_SWITCH_START_LAST; i++) {
     mdl->appendToItemList(funcSwitchStartToString(i), i);
   }

   mdl->loadItemList();
   return mdl;
 }

int ModelData::getCustomScreensCount() const
{
  int cnt = 0;

  for (int i = 0; i < MAX_CUSTOM_SCREENS; i++) {
    if (customScreens.customScreenData[i].layoutId[0] != '\0')
      cnt++;
  }

  return cnt;
}
