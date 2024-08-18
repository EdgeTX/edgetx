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

#include "constants.h"
#include "modeledit.h"

class CompoundItemModelFactory;

constexpr char MIMETYPE_CURVE[] = { "application/x-companion-curve" };

class CurveImageWidget;

class CurvesPanel : public ModelPanel
{
    Q_OBJECT

  public:
    CurvesPanel(QWidget * parent, ModelData & model, GeneralSettings & generalSettings, Firmware * firmware,
                CompoundItemModelFactory * sharedItemModels);
    virtual ~CurvesPanel();

  private slots:
    void update();
    void update(int index);
    void on_curveImageDoubleClicked();

    void onCustomContextMenuRequested(QPoint pos);
    void cmClear(bool prompt = true);
    void cmClearAll();
    void cmCopy();
    void cmCut();
    void cmDelete();
    void cmEdit();
    void cmInsert();
    void cmMoveDown();
    void cmMoveUp();
    void cmPaste();

  private:
    void editCurve(int index);

    int maxCurves;

    void swapData(int idx1, int idx2);
    void updateItemModels();

    // context menu
    bool hasClipboardData(QByteArray * data = nullptr) const;
    bool curveExists() const;
    bool insertAllowed() const;
    bool moveDownAllowed() const;
    bool moveUpAllowed() const;
    int selectedIndex;

    // keep constructor pointer to enable triggering update of item models when curves change
    CompoundItemModelFactory *sharedItemModels;

    // ui widgets
    QWidget *grp[CPN_MAX_CURVES];
    QGridLayout *grid[CPN_MAX_CURVES];
    CurveImageWidget *image[CPN_MAX_CURVES];
    QLabel *name[CPN_MAX_CURVES];
    QLabel *type[CPN_MAX_CURVES];
    QLabel *numpoints[CPN_MAX_CURVES];
    QLabel *points[CPN_MAX_CURVES];
    QLabel *smooth[CPN_MAX_CURVES];
};
