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

#include "rawsource.h"

#include <QtCore>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QSpinBox>
#include <QStandardItemModel>

class GeneralSettings;
class ModelData;
class CurveRefFilteredFactory;
class CompoundItemModelFactory;
class FilteredItemModel;
class CurveImageWidget;
class SourceNumRefEditor;

class CurveReference {

  Q_DECLARE_TR_FUNCTIONS(CurveReference)

  public:

    enum CurveRefType {
      CURVE_REF_DIFF,
      CURVE_REF_EXPO,
      CURVE_REF_FUNC,
      CURVE_REF_CUSTOM,
      MAX_CURVE_REF_TYPE = CURVE_REF_CUSTOM
    };

    enum CurveRefGroups {
      NoneGroup      = 0x001,
      NegativeGroup  = 0x002,
      PositiveGroup  = 0x004,

      AllCurveRefGroups   = NoneGroup | NegativeGroup | PositiveGroup,
      PositiveCurveRefGroups = AllCurveRefGroups &~ NegativeGroup
    };

    CurveReference() { clear(); }
    CurveReference(CurveRefType type, int value) : type(type), value(value) { ; }

    void clear() { memset(this, 0, sizeof(CurveReference)); }
    const bool isEmpty() const { return type == CURVE_REF_DIFF && value == 0; }
    const bool isSet() const { return !isEmpty(); }
    const bool isValueNumber() const;
    const bool isValueReference() const { return !isValueNumber(); }
    const QString toString(const ModelData * model = nullptr, bool verbose = true, const GeneralSettings * const generalSettings = nullptr,
                           Board::Type board = Board::BOARD_UNKNOWN, bool prefixCustomName = true) const;
    const bool isAvailable() const;

    CurveRefType type;
    int value;

    bool operator == ( const CurveReference & other) const {
      return (this->type == other.type) && (this->value == other.value);
    }

    bool operator != ( const CurveReference & other) const {
      return (this->type != other.type) || (this->value != other.value);
    }

    static int getDefaultValue(const CurveRefType type, const bool isGVar = false);
    static QString typeToString(const CurveRefType type);
    static QString functionToString(const int value);
    static bool isTypeAvailable(const CurveRefType type);
    static bool isFunctionAvailable(const int value);
    static int functionCount();
};

class CurveReferenceUIManager : public QObject {

  Q_OBJECT

  public:
    explicit CurveReferenceUIManager(QComboBox * cboType, QCheckBox * chkUseSource, QSpinBox * sbxValue, QComboBox * cboSource,
                                     QComboBox * cboCurveFunc, CurveImageWidget * curveImage, CurveReference & curveRef,
                                     ModelData & model, CompoundItemModelFactory * sharedItemModels,
                                     CurveRefFilteredFactory * curveRefFilteredFactory, FilteredItemModel * sourceItemModel,
                                     QObject * parent = nullptr);

    explicit CurveReferenceUIManager(QComboBox *cboCurveFunc, CurveImageWidget * curveImage, CurveReference & curveRef, ModelData & model,
                                     CompoundItemModelFactory * sharedItemModels, CurveRefFilteredFactory * curveRefFilteredFactory,
                                     QObject * parent = nullptr) :
                CurveReferenceUIManager(nullptr, nullptr, nullptr, nullptr, cboCurveFunc, curveImage, curveRef, model, sharedItemModels,
                                        curveRefFilteredFactory, nullptr, parent) {}

    virtual ~CurveReferenceUIManager();

  signals:
    void resized();

  protected slots:
    void cboTypeChanged(int);
    void cboCurveFuncChanged(int);
    void update();
    void curveImageDoubleClicked();
    void onItemModelAboutToBeUpdated();
    void onItemModelUpdateComplete();

  private:
    QComboBox *cboType;
    QCheckBox *chkUseSource;
    QSpinBox *sbxValue;
    QComboBox *cboSource;
    QComboBox *cboCurveFunc;
    CurveImageWidget *curveImage;
    CurveReference &curveRef;
    ModelData &model;
    CurveRefFilteredFactory *filteredModelFactory;
    bool lock;
    SourceNumRefEditor *srcNumRefEditor;

    void connectItemModelEvents(const FilteredItemModel * itemModel);
    void populateValueCB(QComboBox * cb);
};
