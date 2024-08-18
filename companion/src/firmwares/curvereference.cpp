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

#include "curvereference.h"
#include "helpers.h"
#include "modeldata.h"
#include "generalsettings.h"
#include "filtereditemmodels.h"
#include "curveimagewidget.h"
#include "curvedialog.h"
#include "sourcenumref.h"

const QString CurveReference::toString(const ModelData * model, bool verbose, const GeneralSettings * const generalSettings,
                                       Board::Type board, bool prefixCustomName) const
{
  if (value == 0)
    return CPN_STR_NONE_ITEM;

  QString ret;
  unsigned idx = abs(value) - 1;

  switch(type) {
    case CURVE_REF_DIFF:
    case CURVE_REF_EXPO:
      ret = SourceNumRef(value).toString(model, generalSettings, board, prefixCustomName);
      break;
    case CURVE_REF_FUNC:
      ret = functionToString(value);
      break;
    case CURVE_REF_CUSTOM:
      if (model)
        ret = model->curves[idx].nameToString(idx);
      else
        ret = CurveData().nameToString(idx);
      if (value < 0)
        ret.prepend(CPN_STR_SRC_INDICATOR_NEG);
      break;
    default:
      return CPN_STR_UNKNOWN_ITEM;
  }

  if (verbose)
    ret = tr(qPrintable(QString(typeToString(type) + "(%1)").arg(ret)));

  return ret;
}

const bool CurveReference::isValueNumber() const
{
  return (type == CURVE_REF_DIFF || type == CURVE_REF_EXPO) && SourceNumRef(value).isNumber();
}

const bool CurveReference::isAvailable() const
{
  return true;
}

//  static
int CurveReference::getDefaultValue(const CurveRefType type, const bool isGVar)
{
  if (isGVar && (type == CURVE_REF_DIFF || type == CURVE_REF_EXPO))
    return SourceNumRef(SOURCE_TYPE_GVAR, 1).toValue();
  else if (type == CURVE_REF_FUNC)
    return 1;
  else
    return 0;
}

//  static
QString CurveReference::typeToString(const CurveRefType type)
{
  const QStringList strl = { tr("Diff"), tr("Expo") , tr("Func"), tr("Custom") };
  int idx = (int)type;

  if (idx < 0 || idx >= strl.count())
    return CPN_STR_UNKNOWN_ITEM;

  return strl.at(idx);
}

//  static
QString CurveReference::functionToString(const int value)
{
  const QStringList strl = { "x>0", "x<0", "|x|", "f>0", "f<0", "|f|" };
  int idx = value - 1;

  if (idx < 0 || idx >= strl.count())
    return CPN_STR_UNKNOWN_ITEM;

  return strl.at(idx);
}

//  static
bool CurveReference::isTypeAvailable(const CurveRefType type)
{
  bool ret = false;
  Firmware * fw = getCurrentFirmware();

  switch(type) {
    case CURVE_REF_DIFF:
      if (fw->getCapability(HasInputDiff))
        ret = true;
      break;
    case CURVE_REF_EXPO:
      if (fw->getCapability(HasMixerExpo))
        ret = true;
      break;
    case CURVE_REF_FUNC:
    case CURVE_REF_CUSTOM:
      ret = true;
      break;
  }

  return ret;
}

//  static
bool CurveReference::isFunctionAvailable(const int value)
{
  return true;
}

//  static
int CurveReference::functionCount()
{
  return 6;
}


/*
 * CurveReferenceUIManager
*/

CurveReferenceUIManager::CurveReferenceUIManager(QComboBox * cboType, QCheckBox * chkUseSource, QSpinBox * sbxValue,
                                                 QComboBox * cboSource, QComboBox * cboCurveFunc, CurveImageWidget * curveImage,
                                                 CurveReference & curveRef, ModelData & model, CompoundItemModelFactory * sharedItemModels,
                                                 CurveRefFilteredFactory * curveRefFilteredFactory, FilteredItemModel * sourceItemModel,
                                                 QObject * parent) :
  QObject(parent),
  cboType(cboType),
  chkUseSource(chkUseSource),
  sbxValue(sbxValue),
  cboSource(cboSource),
  cboCurveFunc(cboCurveFunc),
  curveImage(curveImage),
  curveRef(curveRef),
  model(model),
  filteredModelFactory(curveRefFilteredFactory),
  lock(false),
  srcNumRefEditor(nullptr)
{
  connectItemModelEvents(filteredModelFactory->getItemModel(CurveRefFilteredFactory::CRFIM_CURVE));

  if (chkUseSource || sbxValue || cboSource)
    srcNumRefEditor = new SourceNumRefEditor(curveRef.value, chkUseSource, sbxValue, cboSource, 0, -100, 100, 1, model, sourceItemModel);

  if (cboType) {
    cboType->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    cboType->setModel(filteredModelFactory->getItemModel(CurveRefFilteredFactory::CRFIM_TYPE));
    cboType->setCurrentIndex(cboType->findData((int)curveRef.type));
    connect(cboType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CurveReferenceUIManager::cboTypeChanged);
  }

  if (cboCurveFunc) {
    if (!cboType) {
      cboCurveFunc->setModel(filteredModelFactory->getItemModel(CurveRefFilteredFactory::CRFIM_CURVE));
      cboCurveFunc->setCurrentIndex(cboCurveFunc->findData(curveRef.value));
    }
    cboCurveFunc->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    cboCurveFunc->setMaxVisibleItems(10);
    connect(cboCurveFunc, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CurveReferenceUIManager::cboCurveFuncChanged);
  }

  if (curveImage) {
    curveImage->set(&model, getCurrentFirmware(), sharedItemModels, curveRef.value, Qt::black, 3);
    curveImage->setGrid(Qt::gray, 2);
    connect(curveImage, &CurveImageWidget::doubleClicked, this, &CurveReferenceUIManager::curveImageDoubleClicked);
  }

  update();
}

CurveReferenceUIManager::~CurveReferenceUIManager()
{
  delete srcNumRefEditor;
}

#define CURVE_REF_UI_SRC_SHOW   (1 << 0)
#define CURVE_REF_UI_VALUE_SHOW (1 << 1)
#define CURVE_REF_UI_REF_SHOW   (1 << 2)
#define CURVE_REF_UI_CURVE_SHOW (1 << 3)

void CurveReferenceUIManager::update()
{
  lock = true;

  if (srcNumRefEditor)
    srcNumRefEditor->setLock(true);

  int widgetsMask = 0;

  if (srcNumRefEditor) {
    if (curveRef.type == CurveReference::CURVE_REF_DIFF || curveRef.type == CurveReference::CURVE_REF_EXPO) {
      srcNumRefEditor->setVisible(true);
      srcNumRefEditor->update();
    }
    else {
      srcNumRefEditor->setVisible(false);
      widgetsMask |= CURVE_REF_UI_REF_SHOW;
    }
  }
  else
    widgetsMask |= CURVE_REF_UI_REF_SHOW;

  if (curveRef.type == CurveReference::CURVE_REF_CUSTOM && curveRef.value != 0)
    widgetsMask |= CURVE_REF_UI_CURVE_SHOW;

  if (cboType)
    cboType->setCurrentIndex(cboType->findData(curveRef.type));

  if (cboCurveFunc) {
    if (curveRef.isValueReference())
      populateValueCB(cboCurveFunc);
    cboCurveFunc->setVisible(widgetsMask & CURVE_REF_UI_REF_SHOW);
  }

  if (curveImage) {
    if (widgetsMask & CURVE_REF_UI_CURVE_SHOW) {
      curveImage->setIndex(curveRef.value);
      if (abs(curveRef.value) > 0 && abs(curveRef.value) <= CPN_MAX_CURVES)
        curveImage->setPen(colors[abs(curveRef.value) - 1], 3);
      else
        curveImage->setPen(Qt::black, 3);
    }
    curveImage->draw();
    curveImage->setVisible(widgetsMask & CURVE_REF_UI_CURVE_SHOW);
  }

  emit resized();

  if (srcNumRefEditor)
    srcNumRefEditor->setLock(false);

  lock = false;
}

void CurveReferenceUIManager::cboTypeChanged(int index)
{
  if (!lock) {
    CurveReference::CurveRefType type = (CurveReference::CurveRefType)cboType->itemData(index).toInt();
    curveRef = CurveReference(type, CurveReference::getDefaultValue(type));
    update();
  }
}

void CurveReferenceUIManager::cboCurveFuncChanged(int index)
{
  if (!lock) {
    if (!cboType)
      curveRef.type = CurveReference::CURVE_REF_CUSTOM;
    curveRef.value = cboCurveFunc->itemData(index).toInt();
    update();
  }
}

void CurveReferenceUIManager::populateValueCB(QComboBox * cb)
{
  if (cb) {
    switch (curveRef.type) {
      case CurveReference::CURVE_REF_FUNC:
        cb->setModel(filteredModelFactory->getItemModel(CurveRefFilteredFactory::CRFIM_FUNC));
        break;
      case CurveReference::CURVE_REF_CUSTOM:
        cb->setModel(filteredModelFactory->getItemModel(CurveRefFilteredFactory::CRFIM_CURVE));
        break;
      default:
        break;
    }

    cb->setCurrentIndex(cb->findData(curveRef.value));
  }
}

void CurveReferenceUIManager::curveImageDoubleClicked()
{
  if (curveRef.type == CurveReference::CURVE_REF_CUSTOM && abs(curveRef.value) > 0)
    curveImage->edit();
}

void CurveReferenceUIManager::connectItemModelEvents(const FilteredItemModel * itemModel)
{
  connect(itemModel, &FilteredItemModel::aboutToBeUpdated, this, &CurveReferenceUIManager::onItemModelAboutToBeUpdated);
  connect(itemModel, &FilteredItemModel::updateComplete, this, &CurveReferenceUIManager::onItemModelUpdateComplete);
}

void CurveReferenceUIManager::onItemModelAboutToBeUpdated()
{
  lock = true;
}

void CurveReferenceUIManager::onItemModelUpdateComplete()
{
  update();
}
