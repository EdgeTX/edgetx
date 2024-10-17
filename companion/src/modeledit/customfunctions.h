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
#include "compounditemmodels.h"
#include "filtereditemmodels.h"

#include <QMediaPlayer>

class TimerEdit;

constexpr char MIMETYPE_CUSTOM_FUNCTION[] = "application/x-companion-custom-function";

class CustomFunctionsPanel : public GenericPanel
{
  Q_OBJECT

  public:
    CustomFunctionsPanel(QWidget *parent, ModelData * model, GeneralSettings & generalSettings, Firmware * firmware, CompoundItemModelFactory * sharedItemModels);
    virtual ~CustomFunctionsPanel();

    virtual void update();

  protected:
    CustomFunctionData * functions;

  private slots:
    void customFunctionEdited();
    void functionEdited();
    void onCustomContextMenuRequested(QPoint pos);
    void refreshCustomFunction(int index, bool modified=false);
    bool playSound(int index);
    void stopSound(int index);
    void toggleSound(bool play);
    void onMediaPlayerStateChanged(QMediaPlayer::State state);
    void onMediaPlayerError(QMediaPlayer::Error error);
    void cmDelete();
    void cmCopy();
    void cmPaste();
    void cmCut();
    void cmMoveUp();
    void cmMoveDown();
    void cmInsert();
    void cmClear(bool prompt = true);
    void cmClearAll();
    void onItemModelAboutToBeUpdated();
    void onItemModelUpdateComplete();

  private:
    void populateFuncParamCB(QComboBox *b, uint function, unsigned int value, unsigned int adjustmode=0);
    bool hasClipboardData(QByteArray * data = nullptr) const;
    bool insertAllowed() const;
    bool moveDownAllowed() const;
    bool moveUpAllowed() const;
    void swapData(int idx1, int idx2);
    void resetCBsAndRefresh(int idx);
    void connectItemModelEvents(FilteredItemModel * itemModel);

    CompoundItemModelFactory * tabModelFactory;
    FilteredItemModelFactory * tabFilterFactory;
    int funcActionsId;
    int funcResetParamId;
    int rawSwitchId;
    int rawSourceAllId;
    int rawSourceInputsId;
    int rawSourceGVarsId;
    int playSoundId;
    int harpicId;
    int repeatId;
    int repeatLuaId;
    int repeatSetScreenId;
    int gvarAdjustModeId;

    QSet<QString> tracksSet;
    QSet<QString> scriptsSet;
    int mediaPlayerCurrent;
    QComboBox * fswtchSwtch[CPN_MAX_SPECIAL_FUNCTIONS];
    QComboBox * fswtchFunc[CPN_MAX_SPECIAL_FUNCTIONS];
    QCheckBox * fswtchParamGV[CPN_MAX_SPECIAL_FUNCTIONS];
    QDoubleSpinBox * fswtchParam[CPN_MAX_SPECIAL_FUNCTIONS];
    TimerEdit * fswtchParamTime[CPN_MAX_SPECIAL_FUNCTIONS];
    QToolButton * playBT[CPN_MAX_SPECIAL_FUNCTIONS];
    QComboBox * fswtchParamT[CPN_MAX_SPECIAL_FUNCTIONS];
    QComboBox * fswtchParamArmT[CPN_MAX_SPECIAL_FUNCTIONS];
    QCheckBox * fswtchEnable[CPN_MAX_SPECIAL_FUNCTIONS];
    QComboBox * fswtchRepeat[CPN_MAX_SPECIAL_FUNCTIONS];
    QComboBox * fswtchGVmode[CPN_MAX_SPECIAL_FUNCTIONS];
    QMediaPlayer * mediaPlayer;

    int selectedIndex;
    int fswCapability;
    int modelsUpdateCnt;
};
