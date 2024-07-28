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

#include "helpers.h"
#include "modeledit.h"

#include <QtCore>

class CompoundItemModelFactory;
class FilteredItemModelFactory;
class CurveImageWidget;

constexpr char MIMETYPE_CHANNEL[] = "application/x-companion-channel";

class GVarGroup;

class LimitsGroup
{
  Q_DECLARE_TR_FUNCTIONS(LimitsGroup)

  public:
    LimitsGroup(Firmware * firmware, TableLayout * tableLayout, int row, int col, int & value, const ModelData & model, GeneralSettings & generalSettings,
                int min, int max, int deflt, FilteredItemModel * gvarModel, ModelPanel * panel = nullptr);
    ~LimitsGroup();

    void setValue(int val);
    void updateMinMax(int max);

  protected:
    Firmware *firmware;
    QDoubleSpinBox *spinbox;
    GVarGroup *gvarGroup;
    int &value;
    double displayStep;
    QCheckBox *gv;
};

class ChannelsPanel : public ModelPanel
{
    Q_OBJECT

  public:
    ChannelsPanel(QWidget * parent, ModelData & model, GeneralSettings & generalSettings, Firmware * firmware,
                  CompoundItemModelFactory * sharedItemModels);
    virtual ~ChannelsPanel();

  public slots:
    void refreshExtendedLimits();

  private slots:
    void symlimitsEdited();
    void nameEdited();
    void invEdited();
    void ppmcenterEdited();
    void update();
    void updateLine(int index);
    void cmDelete();
    void cmCopy();
    void cmPaste();
    void cmCut();
    void cmMoveUp();
    void cmMoveDown();
    void cmInsert();
    void cmClear(bool prompt = true);
    void cmClearAll();
    void onCustomContextMenuRequested(QPoint pos);
    void onItemModelAboutToBeUpdated();
    void onItemModelUpdateComplete();

  private:
    QLineEdit *name[CPN_MAX_CHNOUT];
    LimitsGroup *chnOffset[CPN_MAX_CHNOUT];
    LimitsGroup *chnMin[CPN_MAX_CHNOUT];
    LimitsGroup *chnMax[CPN_MAX_CHNOUT];
    QComboBox *invCB[CPN_MAX_CHNOUT];
    QComboBox *curveCB[CPN_MAX_CHNOUT];
    CurveImageWidget *curveImage[CPN_MAX_CHNOUT];
    QSpinBox *centerSB[CPN_MAX_CHNOUT];
    QCheckBox *symlimitsChk[CPN_MAX_CHNOUT];
    int selectedIndex;
    int chnCapability;
    CompoundItemModelFactory *sharedItemModels;
    FilteredItemModelFactory *dialogFilteredItemModels;
    CurveRefFilteredFactory *curveRefFilteredItemModels;
    CurveReferenceUIManager *curveGroup[CPN_MAX_CHNOUT];

    bool hasClipboardData(QByteArray * data = nullptr) const;
    bool insertAllowed() const;
    bool moveDownAllowed() const;
    bool moveUpAllowed() const;

    void updateItemModels();
    void connectItemModelEvents(const FilteredItemModel * itemModel);
};
