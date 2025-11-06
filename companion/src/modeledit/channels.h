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

#include <QtCore>

class CompoundItemModelFactory;
class FilteredItemModel;
class RawSourceWidget;

constexpr char MIMETYPE_CHANNEL[] = "application/x-companion-channel";

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
    void nameEdited();
    void invertedEdited();
    void ppmCenterEdited();
    void subTrimModeEdited();
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
    int selectedIndex;
    int chnCapability;
    CompoundItemModelFactory *sharedItemModels;
    QLineEdit *leName[CPN_MAX_CHNOUT];
    RawSourceWidget *rswSubTrim[CPN_MAX_CHNOUT];
    RawSourceWidget *rswMin[CPN_MAX_CHNOUT];
    RawSourceWidget *rswMax[CPN_MAX_CHNOUT];
    QCheckBox *chkInverted[CPN_MAX_CHNOUT];
    RawSourceWidget *crwCurve[CPN_MAX_CHNOUT];
    QSpinBox *sbxPPMCenter[CPN_MAX_CHNOUT];
    QComboBox *cboSubTrimMode[CPN_MAX_CHNOUT];

    bool hasClipboardData(QByteArray * data = nullptr) const;
    bool insertAllowed() const;
    bool moveDownAllowed() const;
    bool moveUpAllowed() const;

    void updateItemModels();
    void connectItemModelEvents(const FilteredItemModel * itemModel);
};
