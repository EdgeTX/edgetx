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
#include "eeprominterface.h"

class CompoundItemModelFactory;
class FilteredItemModel;

constexpr char MIMETYPE_FLIGHTMODE[] = "application/x-companion-flightmode";
constexpr char MIMETYPE_GVAR_PARAMS[]  = "application/x-companion-gvar-params";
constexpr char MIMETYPE_GVAR_VALUE[] = "application/x-companion-gvar-value";
constexpr char MIMETYPE_GVAR_ALL_VALUES[] = "application/x-companion-gvar-all-values";

namespace Ui {
  class FlightMode;
}

class FlightModePanel : public ModelPanel
{
    Q_OBJECT

  public:
    FlightModePanel(QWidget *parent, ModelData &model, int modeIdx,
                    GeneralSettings & generalSettings,
                    Firmware * firmware,
                    FilteredItemModel * rawSwitchFilteredModel,
                    QString radioMode);
    virtual ~FlightModePanel();

    virtual void update();

  signals:
    void phaseDataChanged();
    void phaseNameChanged();
    void phaseSwitchChanged();

  public slots:
    void onThrottleReverseChanged();

  private slots:
    void phaseName_editingFinished();
    void phaseSwitch_currentIndexChanged(int index);
    void phaseFadeIn_editingFinished();
    void phaseFadeOut_editingFinished();
    void phaseTrimUse_currentIndexChanged(int index);
    void phaseTrim_valueChanged();
    void phaseTrimSlider_valueChanged();
    void onCustomContextMenuRequested(QPoint pos);
    void cmClear(bool prompt = true);
    void cmClearAll();
    void cmCopy();
    void cmCut();
    void cmDelete();
    void cmInsert();
    void cmPaste();
    void cmMoveDown();
    void cmMoveUp();
    void onItemModelAboutToBeUpdated();
    void onItemModelUpdateComplete();

  private:
    Ui::FlightMode *ui;
    int phaseIdx;
    FlightModeData & phase;
    int fmCount;
    int trimCount;
    int gvCount;
    QVector<QLabel *> trimsLabel;
    QVector<QComboBox *> trimsUse;
    QVector<QSpinBox *> trimsValue;
    QVector<QSlider *> trimsSlider;
    Board::Type board;
    QString radioMode;

    void trimUpdate(unsigned int trim);
    bool hasClipboardData(QByteArray * data = nullptr) const;
    bool insertAllowed() const;
    bool moveDownAllowed() const;
    bool moveUpAllowed() const;
    void swapData(int idx1, int idx2);
    void connectItemModelEvents(const FilteredItemModel * itemModel);
};

class FlightModesPanel : public ModelPanel
{
    Q_OBJECT

  public:
    FlightModesPanel(QWidget *parent, ModelData & model,
                     GeneralSettings & generalSettings,
                     Firmware * firmware,
                     CompoundItemModelFactory * sharedItemModels,
                     QString radioModes);
    virtual ~FlightModesPanel();

  public slots:
    virtual void update() override;
    void onThrottleReverseChanged();

  signals:
    void updated();
    void refreshThrottleTrim();

  private slots:
    void onPhaseNameChanged();
    void onTabIndexChanged(int index);
    void onItemModelAboutToBeUpdated();
    void onItemModelUpdateComplete();

  private:
    QString getTabName(int index);

    int modesCount;
    QTabWidget *tabWidget;
    CompoundItemModelFactory *sharedItemModels;
    FilteredItemModel *rawSwitchFilteredModel;
    QVector<GenericPanel *> panels;
    QString radioMode;

    void updateItemModels();
    void connectItemModelEvents(const FilteredItemModel * itemModel);
};
