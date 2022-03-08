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

#include "curvereference.h"
#include "adjustmentreference.h"
#include "helpers.h"
#include "modeldata.h"
#include "filtereditemmodels.h"
#include "curveimagewidget.h"
#include "curvedialog.h"

const QString CurveReference::toString(const ModelData * model, bool verbose) const
{
  if (value == 0)
    return CPN_STR_NONE_ITEM;

  QString ret;
  unsigned idx = abs(value) - 1;

  switch(type) {
    case CURVE_REF_DIFF:
    case CURVE_REF_EXPO:
      ret = AdjustmentReference(value).toString(model);
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
        ret.prepend(CPN_STR_SW_INDICATOR_REV);
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
  return (type == CURVE_REF_DIFF || type == CURVE_REF_EXPO) && AdjustmentReference(value).type == AdjustmentReference::ADJUST_REF_VALUE;
}

const bool CurveReference::isAvailable() const
{
  return true;
}

//  static
int CurveReference::getDefaultValue(const CurveRefType type, const bool isGVar)
{
  if (isGVar && (type == CURVE_REF_DIFF || type == CURVE_REF_EXPO))
    return AdjustmentReference(AdjustmentReference::ADJUST_REF_GVAR, 1).toValue();
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

CurveReferenceUIManager::CurveReferenceUIManager(QComboBox * curveTypeCB, QCheckBox * curveGVarCB, QSpinBox * curveValueSB,
                                                 QComboBox * curveValueCB, QLabel * curveLabelImage,
                                                 CurveReference & curveRef, ModelData & model, CompoundItemModelFactory * sharedItemModels,
                                                 CurveRefFilteredFactory * curveRefFilteredFactory, QObject * parent) :
  QObject(parent),
  curveTypeCB(curveTypeCB),
  curveGVarCB(curveGVarCB),
  curveValueSB(curveValueSB),
  curveValueCB(curveValueCB),
  curveRef(curveRef),
  model(model),
  sharedItemModels(sharedItemModels),
  filteredModelFactory(curveRefFilteredFactory),
  lock(false),
  curveImageWidget(nullptr)
{
  connectItemModelEvents(filteredModelFactory->getItemModel(CurveRefFilteredFactory::CRFIM_CURVE));

  hasCapabilityGvars = getCurrentFirmware()->getCapability(Gvars);

  if (curveTypeCB) {
    curveTypeCB->setModel(filteredModelFactory->getItemModel(CurveRefFilteredFactory::CRFIM_TYPE));
    curveTypeCB->setCurrentIndex(curveTypeCB->findData((int)curveRef.type));
    connect(curveTypeCB, SIGNAL(currentIndexChanged(int)), this, SLOT(typeChanged(int)));
  }

  if (curveGVarCB)
    connect(curveGVarCB, SIGNAL(stateChanged(int)), this, SLOT(gvarCBChanged(int)));

  if (curveValueSB) {
    curveValueSB->setMinimum(-100);
    curveValueSB->setMaximum(100);
    connect(curveValueSB, SIGNAL(editingFinished()), this, SLOT(valueSBChanged()));
  }

  if (curveValueCB) {
    curveValueCB->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    curveValueCB->setMaxVisibleItems(10);
    connect(curveValueCB, SIGNAL(currentIndexChanged(int)), this, SLOT(valueCBChanged()));

    if (curveLabelImage) {
      curveImageWidget = qobject_cast<CurveImageWidget *>(curveLabelImage);
      if (curveImageWidget) {
        curveImageWidget->set(&model, getCurrentFirmware(), sharedItemModels, abs(curveRef.value) - 1, Qt::black, 3);
        curveImageWidget->setGrid(Qt::gray, 2);
        connect(curveImageWidget, &CurveImageWidget::doubleClicked, this, &CurveReferenceUIManager::curveImageDoubleClicked);
      }
    }
  }

  update();
}

CurveReferenceUIManager::~CurveReferenceUIManager()
{

}

#define CURVE_REF_UI_GVAR_SHOW  (1 << 0)
#define CURVE_REF_UI_VALUE_SHOW (1 << 1)
#define CURVE_REF_UI_REF_SHOW   (1 << 2)
#define CURVE_REF_UI_CURVE_SHOW (1 << 3)

void CurveReferenceUIManager::update()
{
  lock = true;
  int widgetsMask = 0;

  if (curveRef.type == CurveReference::CURVE_REF_DIFF || curveRef.type == CurveReference::CURVE_REF_EXPO) {
    if (hasCapabilityGvars)
      widgetsMask |= CURVE_REF_UI_GVAR_SHOW;
    if (curveRef.isValueNumber()) {
      curveGVarCB->setChecked(false);
      curveValueSB->setValue(curveRef.value);
      widgetsMask |= CURVE_REF_UI_VALUE_SHOW;
    }
    else {
      curveGVarCB->setChecked(true);
      widgetsMask |= CURVE_REF_UI_REF_SHOW;
    }
  }
  else {
    widgetsMask |= CURVE_REF_UI_REF_SHOW;
  }

  if (curveRef.type == CurveReference::CURVE_REF_CUSTOM)
    widgetsMask |= CURVE_REF_UI_CURVE_SHOW;


  if (curveTypeCB) {
    curveTypeCB->setCurrentIndex(curveTypeCB->findData(curveRef.type));
    curveTypeCB->show();
  }

  if (curveGVarCB)
    curveGVarCB->setVisible(widgetsMask & CURVE_REF_UI_GVAR_SHOW);

  if (curveValueSB)
    curveValueSB->setVisible(widgetsMask & CURVE_REF_UI_VALUE_SHOW);

  if (curveValueCB) {
    if (curveRef.isValueReference())
      populateValueCB(curveValueCB);
    curveValueCB->setVisible(widgetsMask & CURVE_REF_UI_REF_SHOW);
  }

  if (curveValueCB && curveImageWidget) {
    curveImageWidget->draw();
    curveImageWidget->setVisible(widgetsMask & CURVE_REF_UI_CURVE_SHOW);
  }

  emit resized();

  lock = false;
}

void CurveReferenceUIManager::gvarCBChanged(int state)
{
  if (!lock) {
    curveRef.value = CurveReference::getDefaultValue(curveRef.type, state);
    update();
  }
}

void CurveReferenceUIManager::typeChanged(int value)
{
  if (!lock) {
    CurveReference::CurveRefType type = (CurveReference::CurveRefType)curveTypeCB->itemData(curveTypeCB->currentIndex()).toInt();
    curveRef = CurveReference(type, CurveReference::getDefaultValue(type));
    if (curveImageWidget && curveRef.type == CurveReference::CURVE_REF_CUSTOM)
      curveImageWidget->setIndex(abs(curveRef.value) - 1);
    update();
  }
}

void CurveReferenceUIManager::valueSBChanged()
{
  if (!lock) {
    curveRef.value = curveValueSB->value();
    update();
  }
}

void CurveReferenceUIManager::valueCBChanged()
{
  if (!lock) {
    curveRef.value = curveValueCB->itemData(curveValueCB->currentIndex()).toInt();
    if (curveImageWidget && curveRef.type == CurveReference::CURVE_REF_CUSTOM)
      curveImageWidget->setIndex(abs(curveRef.value) - 1);
    update();
  }
}

void CurveReferenceUIManager::populateValueCB(QComboBox * cb)
{
  if (cb) {
    switch (curveRef.type) {
      case CurveReference::CURVE_REF_DIFF:
      case CurveReference::CURVE_REF_EXPO:
        cb->setModel(filteredModelFactory->getItemModel(CurveRefFilteredFactory::CRFIM_GVARREF));
        break;
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
    curveImageWidget->edit();
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
