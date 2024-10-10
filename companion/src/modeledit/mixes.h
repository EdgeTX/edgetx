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

#ifndef _MIXES_H_
#define _MIXES_H_

#include "modeledit.h"
#include "mixerslistwidget.h"
#include "mixerdialog.h"
#include "modelprinter.h"

class CompoundItemModelFactory;
class FilteredItemModel;

class MixesPanel : public ModelPanel
{
    Q_OBJECT

  public:
    MixesPanel(QWidget *parent, ModelData & model, GeneralSettings & generalSettings, Firmware * firmware, CompoundItemModelFactory * sharedItemModels);
    virtual ~MixesPanel();

    virtual void update();

  private slots:
    // TODO all slots?
    void clearMixes();
    void mixersDelete(bool prompt = true);
    void mixersCut();
    void mixersCopy();
    void mixersPaste();
    void mixersDuplicate();
    void mixerOpen();
    void mixerAdd();
    void moveMixUp();
    void moveMixDown();
    void mixerHighlight();

    void mixerlistWidget_customContextMenuRequested(QPoint pos);
    void mixerlistWidget_doubleClicked(QModelIndex index);
    void mixerlistWidget_KeyPress(QKeyEvent *event);

    void mimeMixerDropped(int index, const QMimeData *data, Qt::DropAction action);
    void pasteMixerMimeData(const QMimeData * mimeData, int destIdx);

    void onItemModelAboutToBeUpdated();
    void onItemModelUpdateComplete();

  private:
    MixersListWidget * mixersListWidget;
    bool mixInserted;
    unsigned int highlightedSource;
    ModelPrinter modelPrinter;
    CompoundItemModelFactory *sharedItemModels;
    int modelsUpdateCnt;

    int getMixerIndex(unsigned int dch);
    bool gm_insertMix(int idx);
    void gm_deleteMix(int index);
    void gm_openMix(int index);
    int gm_moveMix(int idx, bool dir);
    void mixersDeleteList(QList<int> list);
    QList<int> createMixListFromSelected();
    void setSelectedByMixList(QList<int> list);
    void AddMixerLine(int dest);
    QString getMixerText(int dest, bool newChannel);
    void connectItemModelEvents(const int id);
    void updateItemModels();
};

#endif // _MIXES_H_
