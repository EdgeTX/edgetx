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

#include "model_flightmodes.h"
#include "opentx.h"
#include "libopenui.h"

#define SET_DIRTY()     storageDirty(EE_MODEL)

ModelFlightModesPage::ModelFlightModesPage():
  PageTab(STR_MENUFLIGHTMODES, ICON_MODEL_FLIGHT_MODES)
{
}

bool isTrimModeAvailable(int mode)
{
  return (mode < 0 || (mode%2) == 0 || (mode/2) != 0); //ToDo menuVerticalPosition
}

class FlightModeGroup: public FormGroup
{
  public:
    FlightModeGroup(Window * parent, uint8_t index, const rect_t & rect) :
      FormGroup(parent, rect),
      index(index)
    {
    }

    void checkEvents() override
    {
      FormGroup::checkEvents();
      bool newActive = (getFlightMode() == index);
      if (newActive != active) {
        active = newActive;
        invalidate();
      }
    }

    void paint(BitmapBuffer * dc) override
    {
      if (index == getFlightMode()) {
        dc->drawSolidFilledRect(0, 0, width(), height(), HIGHLIGHT_COLOR);
      }
      else {
        dc->drawSolidFilledRect(0, 0, width(), height(), FIELD_BGCOLOR);
      }
      FormGroup::paint(dc);
    }

  protected:
    uint8_t index;
    bool active = false;
};

void ModelFlightModesPage::build(FormWindow * window)
{
  coord_t y = 2;

  for (int i = 0; i < MAX_FLIGHT_MODES; i++) {
    auto group = new FlightModeGroup(window, i, { 2, y, LCD_W - 10, 0 });

    FormGridLayout grid;
    grid.setMarginRight(15);
#if LCD_W > LCD_H
    grid.setLabelWidth(140);
#else
    grid.setLabelWidth(110);
#endif
    grid.spacer();

    char label[16];
    getFlightModeString(label, i+1);
    new Subtitle(group, grid.getLabelSlot(), label);
    grid.nextLine();

    // Flight mode name
    new StaticText(group, grid.getLabelSlot(true), STR_NAME);
    new ModelTextEdit(group, grid.getFieldSlot(), g_model.flightModeData[i].name, LEN_FLIGHT_MODE_NAME);
    grid.nextLine();

    // Flight mode switch
    if (i > 0) {
      new StaticText(group, grid.getLabelSlot(true), STR_SWITCH);
      new SwitchChoice(group, grid.getFieldSlot(), SWSRC_FIRST_IN_MIXES, SWSRC_LAST_IN_MIXES, GET_SET_DEFAULT(g_model.flightModeData[i].swtch));
      grid.nextLine();
    }

    // Flight mode trims
    new StaticText(group, grid.getLabelSlot(true), STR_TRIMS);

    const char * STR_VTRIMS_MODES = "\002"
                                    "--"
                                    "=0"
                                    "+0"
                                    "=1"
                                    "+1"
                                    "=2"
                                    "+2"
                                    "=3"
                                    "+3"
                                    "=4"
                                    "+4"
                                    "=5"
                                    "+5"
                                    "=6"
                                    "+6"
                                    "=7"
                                    "+7"
                                    "=8"
                                    "+8";
    for (int t = 0; t < NUM_TRIMS; t++) {
      // TODO isTrimModeAvailable to avoid +{{CURRENT}}
      new Choice(group, grid.getFieldSlot(NUM_TRIMS, t), STR_VTRIMS_MODES, -1, 2*MAX_FLIGHT_MODES-1,
                 GET_DEFAULT(g_model.flightModeData[i].trim[t].mode==TRIM_MODE_NONE ? -1 : g_model.flightModeData[i].trim[t].mode),
                 SET_DEFAULT(g_model.flightModeData[i].trim[t].mode));
    }
    grid.nextLine();

    // Flight mode fade in / out
    new StaticText(group, grid.getLabelSlot(true), "Fade in/out");
    new NumberEdit(group, grid.getFieldSlot(2, 0), 0, DELAY_MAX,
                          GET_DEFAULT(g_model.flightModeData[i].fadeIn),
                          SET_VALUE(g_model.flightModeData[i].fadeIn, newValue),
                          0, PREC1);
    new NumberEdit(group, grid.getFieldSlot(2, 1), 0, DELAY_MAX,
                          GET_DEFAULT(g_model.flightModeData[i].fadeOut),
                          SET_VALUE(g_model.flightModeData[i].fadeOut, newValue),
                          0, PREC1);
    grid.nextLine();

    grid.spacer();
    coord_t height = grid.getWindowHeight();
    group->setHeight(height);
    y += height + 2;
  }

  char label[32];
  sprintf(label, "Check FM Trims");
  // TODO rather use a centered slot?
  // TODO dynamic text with FM index for the button text
  new TextButton(window, { 60, y + 5, LCD_W - 120, PAGE_LINE_HEIGHT }, label,
                 [&]() -> uint8_t {
                   if (trimsCheckTimer)
                     trimsCheckTimer = 0;
                   else
                     trimsCheckTimer = 200; // 2 seconds trims cancelled
                   return trimsCheckTimer;
                 });

  window->setInnerHeight(y + 40);
}
