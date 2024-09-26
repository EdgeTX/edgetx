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

#include "compounditemmodels.h"
#include "generalsettings.h"
#include "eeprominterface.h"
#include "modeldata.h"
#include "adjustmentreference.h"

// static
QString AbstractItemModel::idToString(const int value)
{
  switch (value) {
    case IMID_Unknown:
      return "Unknown";
    case IMID_RawSource:
      return "RawSource";
    case IMID_RawSwitch:
      return "RawSwitch";
    case IMID_Curve:
      return "Curve";
    case IMID_GVarRef:
      return "GVarRef";
    case IMID_ThrSource:
      return "ThrSource";
    case IMID_CustomFuncAction:
      return "CustomFuncAction";
    case IMID_CustomFuncResetParam:
      return "CustomFuncResetParam";
    case IMID_TeleSource:
      return "TeleSource";
    case IMID_CurveRefType:
      return "CurveRefType";
    case IMID_CurveRefFunc:
      return "CurveRefFunc";
    case IMID_FlexSwitches:
      return "FlexSwitches";
    default:
      return "Custom";
  }
}

// static
void AbstractItemModel::dumpItemModelContents(AbstractItemModel * itemModel)
{
  if (itemModel) {
    qDebug() << "id:" << itemModel->getId() << "name:" << itemModel->getName();

    for (int i = 0; i < itemModel->rowCount(); ++i) {
      qDebug() << "row:"   << i
               << "text:"  << itemModel->data(itemModel->index(i, 0), Qt::DisplayRole).toString()
               << "id:"    << itemModel->data(itemModel->index(i, 0), AbstractItemModel::IMDR_Id).toInt()
               << "avail:" << itemModel->data(itemModel->index(i, 0), AbstractItemModel::IMDR_Available).toBool()
               << "flags:" << itemModel->data(itemModel->index(i, 0), AbstractItemModel::IMDR_Flags).toInt()
               << "type:"  << itemModel->data(itemModel->index(i, 0), AbstractItemModel::IMDR_Type).toInt();
    }
  }
  else
    qDebug() << "Error: model not of class AbstractItemModel";
}

//
// AbstractStaticItemModel
//

void AbstractStaticItemModel::loadItemList()
{
  foreach (const ListItem *item, itemList) {
    QStandardItem * modelItem = new QStandardItem();
    modelItem->setText(item->text);
    modelItem->setData(item->id, IMDR_Id);
    modelItem->setData(item->type, IMDR_Type);
    modelItem->setData(item->flags, IMDR_Flags);
    modelItem->setData(item->isAvailable, IMDR_Available);
    appendRow(modelItem);
  }

  itemList.clear();
};

//
// RawSourceItemModel
//

RawSourceItemModel::RawSourceItemModel(const GeneralSettings * const generalSettings, const ModelData * const modelData,
                                       Firmware * firmware, const Boards * const board, const Board::Type boardType) :
  AbstractDynamicItemModel(generalSettings, modelData, firmware, board, boardType)
{
  setId(IMID_RawSource);
  setUpdateMask(IMUE_All &~ (IMUE_Curves | IMUE_Scripts));

  // Descending source direction: inverted (!) sources
  addItems(SOURCE_TYPE_TELEMETRY,      RawSource::TelemGroup,    -firmware->getCapability(Sensors) * 3);
  addItems(SOURCE_TYPE_TIMER,          RawSource::TelemGroup,    -firmware->getCapability(Timers));
  addItems(SOURCE_TYPE_SPECIAL,        RawSource::TelemGroup,    -(SOURCE_TYPE_SPECIAL_COUNT - 1));
  addItems(SOURCE_TYPE_GVAR,           RawSource::GVarsGroup,    -firmware->getCapability(Gvars));
  addItems(SOURCE_TYPE_CH,             RawSource::SourcesGroup,  -firmware->getCapability(Outputs));
  addItems(SOURCE_TYPE_PPM,            RawSource::SourcesGroup,  -firmware->getCapability(TrainerInputs));
  addItems(SOURCE_TYPE_CYC,            RawSource::SourcesGroup,  -CPN_MAX_CYC);
  addItems(SOURCE_TYPE_CUSTOM_SWITCH,  RawSource::SwitchesGroup, -firmware->getCapability(LogicalSwitches));
  if (modelData)
    addItems(SOURCE_TYPE_FUNCTIONSWITCH_GROUP,  RawSource::SourcesGroup,  -CPN_MAX_FUNCTIONSWITCH_GROUP);
  addItems(SOURCE_TYPE_SWITCH,         RawSource::SwitchesGroup, -board->getCapability(Board::Switches));
  addItems(SOURCE_TYPE_MAX,            RawSource::SourcesGroup,  -1);
  addItems(SOURCE_TYPE_MIN,            RawSource::SourcesGroup,  -1);
  addItems(SOURCE_TYPE_SPACEMOUSE,     RawSource::SourcesGroup,  -CPN_MAX_SPACEMOUSE);
  addItems(SOURCE_TYPE_TRIM,           RawSource::TrimsGroup,    -board->getCapability(Board::NumTrims));
  addItems(SOURCE_TYPE_INPUT,          RawSource::SourcesGroup,  -board->getCapability(Board::Inputs));
  addItems(SOURCE_TYPE_VIRTUAL_INPUT,  RawSource::InputsGroup,   -firmware->getCapability(VirtualInputs));
  for (int i = firmware->getCapability(LuaScripts) - 1; i >= 0; i--)
    addItems(SOURCE_TYPE_LUA_OUTPUT,   RawSource::ScriptsGroup,  -firmware->getCapability(LuaOutputsPerScript), -i * 16);

  // Ascending source direction (including zero)
  addItems(SOURCE_TYPE_NONE,           RawSource::NoneGroup,     1);
  for (int i = 0; i < firmware->getCapability(LuaScripts); i++)
    addItems(SOURCE_TYPE_LUA_OUTPUT,   RawSource::ScriptsGroup,  firmware->getCapability(LuaOutputsPerScript), i * 16);
  addItems(SOURCE_TYPE_VIRTUAL_INPUT,  RawSource::InputsGroup,   firmware->getCapability(VirtualInputs));
  addItems(SOURCE_TYPE_INPUT,          RawSource::SourcesGroup,  board->getCapability(Board::Inputs));
  addItems(SOURCE_TYPE_TRIM,           RawSource::TrimsGroup,    board->getCapability(Board::NumTrims));
  addItems(SOURCE_TYPE_SPACEMOUSE,     RawSource::SourcesGroup,  CPN_MAX_SPACEMOUSE);
  addItems(SOURCE_TYPE_MIN,            RawSource::SourcesGroup,  1);
  addItems(SOURCE_TYPE_MAX,            RawSource::SourcesGroup,  1);
  addItems(SOURCE_TYPE_SWITCH,         RawSource::SwitchesGroup, board->getCapability(Board::Switches));
  if (modelData)
    addItems(SOURCE_TYPE_FUNCTIONSWITCH_GROUP,  RawSource::SourcesGroup,  CPN_MAX_FUNCTIONSWITCH_GROUP);
  addItems(SOURCE_TYPE_CUSTOM_SWITCH,  RawSource::SwitchesGroup, firmware->getCapability(LogicalSwitches));
  addItems(SOURCE_TYPE_CYC,            RawSource::SourcesGroup,  CPN_MAX_CYC);
  addItems(SOURCE_TYPE_PPM,            RawSource::SourcesGroup,  firmware->getCapability(TrainerInputs));
  addItems(SOURCE_TYPE_CH,             RawSource::SourcesGroup,  firmware->getCapability(Outputs));
  addItems(SOURCE_TYPE_GVAR,           RawSource::GVarsGroup,    firmware->getCapability(Gvars));
  addItems(SOURCE_TYPE_SPECIAL,        RawSource::TelemGroup,    SOURCE_TYPE_SPECIAL_COUNT - 1);
  addItems(SOURCE_TYPE_TIMER,          RawSource::TelemGroup,    firmware->getCapability(Timers));
  addItems(SOURCE_TYPE_TELEMETRY,      RawSource::TelemGroup,    firmware->getCapability(Sensors) * 3);
}

void RawSourceItemModel::setDynamicItemData(QStandardItem * item, const RawSource & src) const
{
  item->setText(src.toString(modelData, generalSettings, boardType));
  item->setData(src.isAvailable(modelData, generalSettings, boardType), IMDR_Available);
}

void RawSourceItemModel::addItems(const RawSourceType & type, const int group, int count, const int start)
{
  const int idxAdj = (type == SOURCE_TYPE_NONE ? -1 : 0);

  int first = start + count < 0 ? start + count : start + 1;
  int last = start + count < 0 ? start : start + count + 1;

  for (int i = first; i < last; ++i) {
    const RawSource src = RawSource(type, i + idxAdj);
    QStandardItem * modelItem = new QStandardItem();
    modelItem->setData(src.toValue(), IMDR_Id);
    modelItem->setData(type, IMDR_Type);
    modelItem->setData((group | (i + idxAdj < 0 ? RawSource::NegativeGroup : i > 0 ? RawSource::PositiveGroup : 0)), IMDR_Flags);
    setDynamicItemData(modelItem, src);
    appendRow(modelItem);
  }
}

void RawSourceItemModel::update(const int event)
{
  if (doUpdate(event)) {
    emit aboutToBeUpdated();

    for (int i = 0; i < rowCount(); ++i)
      setDynamicItemData(item(i), RawSource(item(i)->data(IMDR_Id).toInt()));

    emit updateComplete();
  }
}

//
// RawSwitchItemModel
//

RawSwitchItemModel::RawSwitchItemModel(const GeneralSettings * const generalSettings, const ModelData * const modelData,
                                       Firmware * firmware, const Boards * const board, const Board::Type boardType) :
    AbstractDynamicItemModel(generalSettings, modelData, firmware, board, boardType)
{
  setId(IMID_RawSwitch);
  setUpdateMask(IMUE_FlightModes | IMUE_LogicalSwitches | IMUE_TeleSensors | IMUE_FunctionSwitches);

  // Descending switch direction: NOT (!) switches
  addItems(SWITCH_TYPE_TRAINER,        -1);
  addItems(SWITCH_TYPE_ACT,            -1);
  addItems(SWITCH_TYPE_SENSOR,         -firmware->getCapability(Sensors));
  addItems(SWITCH_TYPE_TELEMETRY,      -1);
  addItems(SWITCH_TYPE_FLIGHT_MODE,    -firmware->getCapability(FlightModes));
  addItems(SWITCH_TYPE_VIRTUAL,        -firmware->getCapability(LogicalSwitches));
  addItems(SWITCH_TYPE_TRIM,           -board->getCapability(Board::NumTrimSwitches));
  addItems(SWITCH_TYPE_MULTIPOS_POT,   -(board->getCapability(Board::MultiposPots) * board->getCapability(Board::MultiposPotsPositions)));
  addItems(SWITCH_TYPE_SWITCH,         -board->getCapability(Board::SwitchesPositions));

  // Ascending switch direction (including zero)
  addItems(SWITCH_TYPE_NONE, 1);
  addItems(SWITCH_TYPE_SWITCH,         board->getCapability(Board::SwitchesPositions));
  addItems(SWITCH_TYPE_MULTIPOS_POT,   board->getCapability(Board::MultiposPots) * board->getCapability(Board::MultiposPotsPositions));
  addItems(SWITCH_TYPE_TRIM,           board->getCapability(Board::NumTrimSwitches));
  addItems(SWITCH_TYPE_VIRTUAL,        firmware->getCapability(LogicalSwitches));
  addItems(SWITCH_TYPE_FLIGHT_MODE,    firmware->getCapability(FlightModes));
  addItems(SWITCH_TYPE_TELEMETRY,      1);
  addItems(SWITCH_TYPE_SENSOR,         firmware->getCapability(Sensors));
  addItems(SWITCH_TYPE_ON,             1);
  addItems(SWITCH_TYPE_ONE,            1);
  addItems(SWITCH_TYPE_ACT,            1);
  addItems(SWITCH_TYPE_TRAINER,        1);
}

void RawSwitchItemModel::setDynamicItemData(QStandardItem * item, const RawSwitch & rsw) const
{
  item->setText(rsw.toString(boardType, generalSettings, modelData));
  item->setData(rsw.isAvailable(modelData, generalSettings, boardType), IMDR_Available);
}

void RawSwitchItemModel::addItems(const RawSwitchType & type, int count)
{
  // Most RawSwitch() indices are one-based (vs. typical zero); these are exceptions to the rule:
  const static QVector<int> rawSwitchIndexBaseZeroTypes = QVector<int>() << SWITCH_TYPE_NONE << SWITCH_TYPE_ON << SWITCH_TYPE_OFF << SWITCH_TYPE_TIMER_MODE;
  // handle exceptions in RawSwitch() index values
  const short rawIdxAdj = rawSwitchIndexBaseZeroTypes.contains(type) ? -1 : 0;

  // determine context flags
  int context = RawSwitch::AllSwitchContexts;
  switch (type) {
    case SWITCH_TYPE_FLIGHT_MODE:
      context &= ~RawSwitch::MixesContext;
    // fallthrough
    case SWITCH_TYPE_VIRTUAL:
    case SWITCH_TYPE_SENSOR:
      context &= ~RawSwitch::GlobalFunctionsContext;
      break;

    case SWITCH_TYPE_ON:
    case SWITCH_TYPE_ONE:
      context = RawSwitch::SpecialFunctionsContext | RawSwitch::GlobalFunctionsContext;
      break;

    default:
      break;
  }

  int i = (count < 0 ? count : 1);
  count = (i < 0 ? 0 : count + i);
  for ( ; i < count; ++i) {
    RawSwitch rs(type, i + rawIdxAdj);
    QStandardItem * modelItem = new QStandardItem();
    modelItem->setData(rs.toValue(), IMDR_Id);
    modelItem->setData(type, IMDR_Type);
    modelItem->setData(context, IMDR_Flags);
    setDynamicItemData(modelItem, rs);
    appendRow(modelItem);
  }
}

void RawSwitchItemModel::update(const int event)
{
  if (doUpdate(event)) {
    emit aboutToBeUpdated();

    for (int i = 0; i < rowCount(); ++i)
      setDynamicItemData(item(i), RawSwitch(item(i)->data(IMDR_Id).toInt()));

    emit updateComplete();
  }
}

//
// CurveItemModel
//

CurveItemModel::CurveItemModel(const GeneralSettings * const generalSettings, const ModelData * const modelData,
                               Firmware * firmware, const Boards * const board, const Board::Type boardType) :
    AbstractDynamicItemModel(generalSettings, modelData, firmware, board, boardType)
{
  setId(IMID_Curve);

  if (!modelData)
    return;

  setUpdateMask(IMUE_Curves);

  const int count = firmware->getCapability(NumCurves);

  for (int i = -count ; i <= count; ++i) {
    QStandardItem * modelItem = new QStandardItem();
    modelItem->setData(i, IMDR_Id);
    modelItem->setData(i < 0 ? IMDG_Negative : i > 0 ? IMDG_Positive : IMDG_None, IMDR_Flags);
    setDynamicItemData(modelItem, i);
    appendRow(modelItem);
  }
}

void CurveItemModel::setDynamicItemData(QStandardItem * item, const int value) const
{
  CurveReference cr = CurveReference(CurveReference::CURVE_REF_CUSTOM, value);
  item->setText(cr.toString(modelData, false));
  item->setData(cr.isAvailable(), IMDR_Available);
}

void CurveItemModel::update(const int event)
{
  if (doUpdate(event)) {
    emit aboutToBeUpdated();

    for (int i = 0; i < rowCount(); ++i)
     setDynamicItemData(item(i), item(i)->data(IMDR_Id).toInt());

    emit updateComplete();
  }
}

//
// GVarReferenceItemModel
//

GVarReferenceItemModel::GVarReferenceItemModel(const GeneralSettings * const generalSettings, const ModelData * const modelData,
                                               Firmware * firmware, const Boards * const board, const Board::Type boardType) :
    AbstractDynamicItemModel(generalSettings, modelData, firmware, board, boardType)
{
  setId(IMID_GVarRef);

  if (!modelData)
    return;

  setUpdateMask(IMUE_GVars | IMUE_FlightModes | IMUE_LogicalSwitches);

  const int count = firmware->getCapability(Gvars);

  if (count > 0) {
    addItems(-count);
    addItems(count);
  }
}

void GVarReferenceItemModel::addItems(int count)
{
  int i = (count < 0 ? count : 1);
  count = (i < 0 ? 0 : count + i);
  for ( ; i < count; ++i) {
    QStandardItem * modelItem = new QStandardItem();
    AdjustmentReference ar(AdjustmentReference::ADJUST_REF_GVAR, i);
    modelItem->setData(ar.toValue(), IMDR_Id);
    modelItem->setData(i < 0 ? IMDG_Negative : i > 0 ? IMDG_Positive : IMDG_None, IMDR_Flags);
    setDynamicItemData(modelItem, ar);
    appendRow(modelItem);
  }
}

void GVarReferenceItemModel::setDynamicItemData(QStandardItem * item, const AdjustmentReference & ar) const
{
  item->setText(ar.toString(modelData, false));
  item->setData(ar.isAvailable(), IMDR_Available);
}

void GVarReferenceItemModel::update(const int event)
{
  if (doUpdate(event)) {
    emit aboutToBeUpdated();

    for (int i = 0; i < rowCount(); ++i)
     setDynamicItemData(item(i), AdjustmentReference(item(i)->data(IMDR_Id).toInt()));

    emit updateComplete();
  }
}

//
// ThrottleSourceItemModel
//

ThrottleSourceItemModel::ThrottleSourceItemModel(const GeneralSettings * const generalSettings, const ModelData * const modelData,
                                                 Firmware * firmware, const Boards * const board, const Board::Type boardType) :
    AbstractDynamicItemModel(generalSettings, modelData, firmware, board, boardType)
{
  setId(IMID_ThrSource);

  if (!modelData)
    return;

  setUpdateMask(IMUE_Channels);

  for (int i = 0; i < modelData->thrTraceSrcCount(); i++) {
    QStandardItem * modelItem = new QStandardItem();
    modelItem->setData(i, IMDR_Id);
    setDynamicItemData(modelItem, i);
    appendRow(modelItem);
  }
}

void ThrottleSourceItemModel::setDynamicItemData(QStandardItem * item, const int value) const
{
  item->setText(modelData->thrTraceSrcToString(generalSettings, value));
  item->setData(modelData->isThrTraceSrcAvailable(generalSettings, value), IMDR_Available);
}

void ThrottleSourceItemModel::update(const int event)
{
  if (doUpdate(event)) {
    emit aboutToBeUpdated();

    for (int i = 0; i < rowCount(); ++i)
      setDynamicItemData(item(i), item(i)->data(IMDR_Id).toInt());

    emit updateComplete();
  }
}

//
// CustomFuncActionItemModel
//

CustomFuncActionItemModel::CustomFuncActionItemModel(const GeneralSettings * const generalSettings, const ModelData * const modelData,
                                                     Firmware * firmware, const Boards * const board, const Board::Type boardType) :
    AbstractDynamicItemModel(generalSettings, modelData, firmware, board, boardType)
{
  setId(IMID_CustomFuncAction);
  setUpdateMask(IMUE_All &~ (IMUE_Curves | IMUE_Scripts));

  for (int i = 0; i < AssignFunc::FuncCount; i++) {
    QStandardItem * modelItem = new QStandardItem();
    modelItem->setData(i, IMDR_Id);
    modelItem->setData(CustomFunctionData::funcContext(i), IMDR_Flags);
    setDynamicItemData(modelItem, i);
    appendRow(modelItem);
  }

  sort(0);
}

void CustomFuncActionItemModel::setDynamicItemData(QStandardItem * item, const int value) const
{
  item->setText(CustomFunctionData::funcToString((AssignFunc)value, modelData));
  item->setData(CustomFunctionData::isFuncAvailable(value, modelData), IMDR_Available);
}

void CustomFuncActionItemModel::update(const int event)
{
  if (doUpdate(event)) {
    emit aboutToBeUpdated();

    for (int i = 0; i < rowCount(); ++i)
      setDynamicItemData(item(i), item(i)->data(IMDR_Id).toInt());

    sort(0);

    emit updateComplete();
  }
}

//
// CustomFuncResetParamItemModel
//

CustomFuncResetParamItemModel::CustomFuncResetParamItemModel(const GeneralSettings * const generalSettings, const ModelData * const modelData,
                                                             Firmware * firmware, const Boards * const board, const Board::Type boardType) :
    AbstractDynamicItemModel(generalSettings, modelData, firmware, board, boardType)
{
  setId(IMID_CustomFuncResetParam);
  setUpdateMask(IMUE_TeleSensors | IMUE_Timers);

  for (int i = 0; i < CustomFunctionData::resetParamCount(); i++) {
    QStandardItem * modelItem = new QStandardItem();
    modelItem->setData(i, IMDR_Id);
    setDynamicItemData(modelItem, i);
    appendRow(modelItem);
  }
}

void CustomFuncResetParamItemModel::setDynamicItemData(QStandardItem * item, const int value) const
{
  item->setText(CustomFunctionData::resetToString(value, modelData));
  item->setData(CustomFunctionData::isResetParamAvailable(value, modelData), IMDR_Available);
}

void CustomFuncResetParamItemModel::update(const int event)
{
  if (doUpdate(event)) {
    emit aboutToBeUpdated();

    for (int i = 0; i < rowCount(); ++i)
      setDynamicItemData(item(i), item(i)->data(IMDR_Id).toInt());

    emit updateComplete();
  }
}

//
// TelemetrySourceItemModel
//

TelemetrySourceItemModel::TelemetrySourceItemModel(const GeneralSettings * const generalSettings, const ModelData * const modelData,
                                                   Firmware * firmware, const Boards * const board, const Board::Type boardType) :
    AbstractDynamicItemModel(generalSettings, modelData, firmware, board, boardType)
{
  setId(IMID_TeleSource);

  if (!modelData)
    return;

  setUpdateMask(IMUE_TeleSensors | IMUE_Modules);
  const int count = firmware->getCapability(Sensors);

  for (int i = -count; i <= count; ++i) {
    QStandardItem * modelItem = new QStandardItem();
    modelItem->setData(i, IMDR_Id);
    setDynamicItemData(modelItem, i);
    appendRow(modelItem);
  }
}

void TelemetrySourceItemModel::setDynamicItemData(QStandardItem * item, const int value) const
{
  item->setText(SensorData::sourceToString(modelData, value));
  item->setData(SensorData::isSourceAvailable(modelData, value), IMDR_Available);
  int flags = value < 0 ? SensorData::SensorTypeFlagNeg : value > 0 ? SensorData::SensorTypeFlagPos : SensorData::SensorTypeFlagNone;
  if (value > 0 && SensorData::isSourceVario(modelData, value))
    flags |= SensorData::SensorTypeFlagVario;
  item->setData(flags, IMDR_Flags);
}

void TelemetrySourceItemModel::update(const int event)
{
  if (doUpdate(event)) {
    emit aboutToBeUpdated();

    for (int i = 0; i < rowCount(); ++i) {
      setDynamicItemData(item(i), item(i)->data(IMDR_Id).toInt());
    }

    emit updateComplete();
  }
}

//
// CurveRefTypeItemModel
//

CurveRefTypeItemModel::CurveRefTypeItemModel(const GeneralSettings * const generalSettings, const ModelData * const modelData,
                                             Firmware * firmware, const Boards * const board, const Board::Type boardType) :
  AbstractStaticItemModel(generalSettings, modelData, firmware, board, boardType)
{
  setId(IMID_CurveRefType);

  for (int i = 0; i <= CurveReference::MAX_CURVE_REF_TYPE; i++) {
    QStandardItem * modelItem = new QStandardItem();
    modelItem->setText(CurveReference::typeToString((CurveReference::CurveRefType)i));
    modelItem->setData(i, IMDR_Id);
    modelItem->setData(CurveReference::isTypeAvailable((CurveReference::CurveRefType)i), IMDR_Available);
    appendRow(modelItem);
  }
}

//
// CurveRefFuncItemModel
//

CurveRefFuncItemModel::CurveRefFuncItemModel(const GeneralSettings * const generalSettings, const ModelData * const modelData,
                                             Firmware * firmware, const Boards * const board, const Board::Type boardType) :
  AbstractStaticItemModel(generalSettings, modelData, firmware, board, boardType)
{
  setId(IMID_CurveRefFunc);

  for (int i = 1; i <= CurveReference::functionCount(); i++) {
    QStandardItem * modelItem = new QStandardItem();
    modelItem->setText(CurveReference::functionToString(i));
    modelItem->setData(i, IMDR_Id);
    modelItem->setData(CurveReference::isFunctionAvailable(i), IMDR_Available);
    appendRow(modelItem);
  }
}

//
// PrecisionItemModel
//

PrecisionItemModel::PrecisionItemModel(const int minDecimals, const int maxDecimals, const QString suffix, const bool placeholders) :
  AbstractStaticItemModel()
{
  for (int i = minDecimals, j = maxDecimals; j >= 0; i++, j--) {
    QString str = QString("0.%1").arg(QString(i, '0'));
    if (placeholders)
      str.append(QString(j, '_'));
    if (!suffix.isEmpty())
      str.append(QString(" %1").arg(suffix));
    QStandardItem * modelItem = new QStandardItem();
    modelItem->setText(str);
    modelItem->setData(i, IMDR_Id);
    modelItem->setData(true, IMDR_Available);
    appendRow(modelItem);
  }
}

//
// FlexSwitchesItemModel
//

FlexSwitchesItemModel::FlexSwitchesItemModel(const GeneralSettings * const generalSettings, const ModelData * const modelData,
                                                   Firmware * firmware, const Boards * const board, const Board::Type boardType) :
    AbstractDynamicItemModel(generalSettings, modelData, firmware, board, boardType)
{
  setId(IMID_FlexSwitches);

  if (!generalSettings)
    return;

  setUpdateMask(IMUE_FunctionSwitches);
  const int count = Boards::getCapability(boardType, Board::Inputs);

  {
    QStandardItem * modelItem = new QStandardItem();
    modelItem->setData(-1, IMDR_Id);
    modelItem->setText(tr("None"));
    modelItem->setData(true, IMDR_Available);
    appendRow(modelItem);
  }

  for (int i = 0; i < count; ++i) {
    QStandardItem * modelItem = new QStandardItem();
    modelItem->setData(i, IMDR_Id);
    setDynamicItemData(modelItem, i);
    appendRow(modelItem);
  }
}

void FlexSwitchesItemModel::setDynamicItemData(QStandardItem * item, const int value) const
{
  item->setText(Boards::getInputName(value, boardType));
  item->setData(generalSettings->isInputFlexSwitchAvailable(value), IMDR_Available);
}

void FlexSwitchesItemModel::update(const int event)
{
  if (doUpdate(event)) {
    emit aboutToBeUpdated();

    for (int i = 1; i < rowCount(); ++i) {  // skip None
      setDynamicItemData(item(i), item(i)->data(IMDR_Id).toInt());
    }

    emit updateComplete();
  }
}

//
// CompoundItemModelFactory
//

CompoundItemModelFactory::CompoundItemModelFactory(const GeneralSettings * const generalSettings, const ModelData * const modelData) :
  generalSettings(generalSettings),
  modelData(modelData)
{
  firmware = getCurrentFirmware();
  board = new Boards(getCurrentBoard());
  boardType = getCurrentBoard();
}

CompoundItemModelFactory::~CompoundItemModelFactory()
{
  unregisterItemModels();
  delete board;
}

void CompoundItemModelFactory::addItemModel(const int id)
{
  switch (id) {
    case AbstractItemModel::IMID_RawSource:
      registerItemModel(new RawSourceItemModel(generalSettings, modelData, firmware, board, boardType));
      break;
    case AbstractItemModel::IMID_RawSwitch:
      registerItemModel(new RawSwitchItemModel(generalSettings, modelData, firmware, board, boardType));
      break;
    case AbstractItemModel::IMID_Curve:
      registerItemModel(new CurveItemModel(generalSettings, modelData, firmware, board, boardType));
      break;
    case AbstractItemModel::IMID_GVarRef:
      registerItemModel(new GVarReferenceItemModel(generalSettings, modelData, firmware, board, boardType));
      break;
    case AbstractItemModel::IMID_ThrSource:
      registerItemModel(new ThrottleSourceItemModel(generalSettings, modelData, firmware, board, boardType));
      break;
    case AbstractItemModel::IMID_CustomFuncAction:
      registerItemModel(new CustomFuncActionItemModel(generalSettings, modelData, firmware, board, boardType));
      break;
    case AbstractItemModel::IMID_CustomFuncResetParam:
      registerItemModel(new CustomFuncResetParamItemModel(generalSettings, modelData, firmware, board, boardType));
      break;
    case AbstractItemModel::IMID_TeleSource:
      registerItemModel(new TelemetrySourceItemModel(generalSettings, modelData, firmware, board, boardType));
      break;
    case AbstractItemModel::IMID_CurveRefType:
      registerItemModel(new CurveRefTypeItemModel(generalSettings, modelData, firmware, board, boardType));
      break;
    case AbstractItemModel::IMID_CurveRefFunc:
      registerItemModel(new CurveRefFuncItemModel(generalSettings, modelData, firmware, board, boardType));
      break;
    case AbstractItemModel::IMID_FlexSwitches:
      registerItemModel(new FlexSwitchesItemModel(generalSettings, modelData, firmware, board, boardType));
      break;
    default:
      qDebug() << "Error: unknown item model: id";
      break;
  }
}

int CompoundItemModelFactory::registerItemModel(AbstractItemModel * itemModel)
{
  if (itemModel) {
    if (!isItemModelRegistered(itemModel->getId())) {
      setSourceId(itemModel);
      registeredItemModels.append(itemModel);
      return itemModel->getId();
    }
    else
      return itemModel->getId();
  }
  return -1;
}

void CompoundItemModelFactory::unregisterItemModels()
{
  foreach (AbstractItemModel *itemModel, registeredItemModels) {
    delete itemModel;
  }
}

void CompoundItemModelFactory::unregisterItemModel(const int id)
{
  AbstractItemModel * itemModel =  getItemModel(id);
  if (itemModel)
    delete itemModel;
}

AbstractItemModel * CompoundItemModelFactory::getItemModel(const int id) const
{
  foreach (AbstractItemModel * itemModel, registeredItemModels) {
    if (itemModel->getId() == id)
      return itemModel;
  }

  return nullptr;
}

AbstractItemModel * CompoundItemModelFactory::getItemModel(const QString name) const
{
  foreach (AbstractItemModel * itemModel, registeredItemModels) {
    if (itemModel->getName() == name)
      return itemModel;
  }

  return nullptr;
}

bool CompoundItemModelFactory::isItemModelRegistered(const int id) const
{
  foreach (AbstractItemModel * itemModel, registeredItemModels) {
    if (itemModel->getId() == id)
      return true;
  }

  return false;
}

void CompoundItemModelFactory::update(const int event)
{
  foreach (AbstractItemModel * itemModel, registeredItemModels) {
    itemModel->update(event);
  }
}

void CompoundItemModelFactory::dumpAllItemModelContents() const
{
  foreach (AbstractItemModel * itemModel, registeredItemModels) {
    AbstractItemModel::dumpItemModelContents(itemModel);
  }
}

void CompoundItemModelFactory::setSourceId(AbstractItemModel * itemModel)
{
  if (itemModel && itemModel->getId() == AbstractItemModel::IMID_Unknown)
    itemModel->setId(AbstractItemModel::IMID_ReservedCount + registeredItemModels.count());
}
