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

#include "modeledit.h"
#include "constants.h"
#include "boards.h"

class CompoundItemModelFactory;
class FilteredItemModel;
class TableLayout;

constexpr char MIMETYPE_GLOBAL_VARIABLE[] = "application/x-companion-global-variable";

class GlobalVariablesPanel : public ModelPanel
{
    Q_OBJECT

  public:
    GlobalVariablesPanel(QWidget *parent, ModelData & model,
                         GeneralSettings & generalSettings,
                         Firmware * firmware,
                         CompoundItemModelFactory * sharedItemModels);
    virtual ~GlobalVariablesPanel();

    virtual void update();

  private slots:
    void cmClear(bool prompt = true);
    void cmClearAll();
    void cmCopy();
    void cmCut();
    void cmDelete();
    void cmInsert();
    void cmMoveDown();
    void cmMoveUp();
    void cmPaste();
    void maxEditingFinished();
    void minEditingFinished();
    void modeCurrentIndexChanged(int index);
    void nameEditingFinished();
    void onCustomContextMenuRequested(QPoint pos);
    void onItemModelAboutToBeUpdated();
    void onItemModelUpdateComplete();
    void popupToggled(bool checked);
    void precCurrentIndexChanged(int index);
    void unitCurrentIndexChanged(int index);
    void useModeToggled(bool checked);
    void valueEditingFinished();

  private:
    CompoundItemModelFactory *sharedItemModels;
    int selectedIndex;
    int modelsUpdateCnt;
    int gvars;
    int modes;
    QString modeName;
    QString modePrefix;

    TableLayout *tableLayout;
    QLineEdit *leName[CPN_MAX_GVARS];
    QComboBox *cboUnit[CPN_MAX_GVARS];
    QComboBox *cboPrec[CPN_MAX_GVARS];
    QDoubleSpinBox *dsbMin[CPN_MAX_GVARS];
    QDoubleSpinBox *dsbMax[CPN_MAX_GVARS];
    QCheckBox *chkPopup[CPN_MAX_GVARS];
    QCheckBox *chkUseMode[CPN_MAX_GVARS][CPN_MAX_FLIGHT_MODES];
    QComboBox *cboMode[CPN_MAX_GVARS][CPN_MAX_FLIGHT_MODES];
    QDoubleSpinBox *dsbValue[CPN_MAX_GVARS][CPN_MAX_FLIGHT_MODES];

    void connectItemModelEvents(const FilteredItemModel * itemModel);
    bool deleteAllowed() const;
    bool getIndexes(QWidget * widget, int &gvarIdx, int &modeIdx);
    bool hasClipboardData(QByteArray * data = nullptr) const;
    bool moveDownAllowed() const;
    bool moveUpAllowed() const;
    void setIndexes(QWidget * widget, int gvarIdx, int modeIdx);
    void setValue(QDoubleSpinBox * spinBox, int val);
    void setMinMax(QDoubleSpinBox * spinBox, int min, int max, int val);
    void swapData(int index1, int index2);
    void updateLine(const int index);
    void updateItemModels();
};
