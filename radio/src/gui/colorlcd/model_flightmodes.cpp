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

#include "model_flightmodes.h"
#include "opentx.h"
#include "libopenui.h"

#define SET_DIRTY()     storageDirty(EE_MODEL)

#define LABEL_RIGHT_MARGIN 10

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
      setFocusHandler([=] (bool focus) {
        setSubWindowColors(focus);
      });
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
      uint32_t fillColor = getFocus() != this ? COLOR_THEME_SECONDARY2 : COLOR_THEME_FOCUS;
      dc->drawSolidFilledRect(0, 0, width(), height(), fillColor);

      FormGroup::paint(dc);
    }

    void setSubWindowColors(bool focus)
    {

      int32_t color = focus ? COLOR_THEME_PRIMARY2 : COLOR_THEME_PRIMARY1;
      for (auto window: getChildren()) {
        if (dynamic_cast<StaticText *>(window) != nullptr) {
          LcdFlags flags = dynamic_cast<Subtitle *>(window) == nullptr ? RIGHT : 0;
          window->setTextFlags(flags | color);
        }
      }
    }

  protected:
    uint8_t index;
    bool active = false;
};

rect_t getRightAlignedLabelSlot(FormGridLayout &grid)
{
  auto rect = grid.getLabelSlot();
  rect.w -= LABEL_RIGHT_MARGIN;
  return rect;
}

rect_t getRightAlignedFieldSlot(FormGridLayout &grid, uint8_t count, uint8_t index)
{
  auto rect = grid.getFieldSlot(count, index);
  rect.w -= LABEL_RIGHT_MARGIN;
  return rect;
}


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
    getFlightModeString(label, i + 1);
    new Subtitle(group, grid.getLabelSlot(), label, 0, COLOR_THEME_PRIMARY1);

    FormField *a = nullptr;
    std::function<uint32_t (FormField *)> backgroundHandler = [=](FormField *field) {
      if (! field->isEditMode()  && !field->hasFocus())
        return i == getFlightMode() ? COLOR_THEME_ACTIVE : COLOR_THEME_PRIMARY2;
      else if (field->hasFocus() && ! field->isEditMode())
        return COLOR_THEME_FOCUS;
      else
        return COLOR_THEME_EDIT;
    };

    bool bNarrowScreen = LCD_W < LCD_H;

    // Flight mode name
    new StaticText(group, getRightAlignedLabelSlot(grid), STR_NAME, 0, RIGHT | COLOR_THEME_PRIMARY1);

    rect_t r = grid.getFieldSlot();;
    if (! bNarrowScreen) {
      r = grid.getFieldSlot(3,0);
      r.w += 53;
    }
    a = new ModelTextEdit(group, r, g_model.flightModeData[i].name, LEN_FLIGHT_MODE_NAME);
    a->setBackgroundHandler(backgroundHandler);

    // Flight mode switch
    if (i > 0) {
      if (! bNarrowScreen) {
        new StaticText(group, getRightAlignedFieldSlot(grid, 3, 1), STR_SWITCH, 0, RIGHT | COLOR_THEME_PRIMARY1);
        a = new SwitchChoice(group, grid.getFieldSlot(3, 2), SWSRC_FIRST_IN_MIXES, SWSRC_LAST_IN_MIXES, GET_SET_DEFAULT(g_model.flightModeData[i].swtch));
      } 
      else {
        grid.nextLine();
        new StaticText(group, getRightAlignedLabelSlot(grid), STR_SWITCH, 0, RIGHT | COLOR_THEME_PRIMARY1);
        a = new SwitchChoice(group, grid.getFieldSlot(), SWSRC_FIRST_IN_MIXES, SWSRC_LAST_IN_MIXES, GET_SET_DEFAULT(g_model.flightModeData[i].swtch));
      }
      a->setBackgroundHandler(backgroundHandler);
      grid.nextLine();
    }
    else {
      grid.nextLine();
    }

    // Flight mode trims
    new StaticText(group, getRightAlignedLabelSlot(grid), STR_TRIMS, 0, RIGHT | COLOR_THEME_PRIMARY1);

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
      a = new Choice(group, grid.getFieldSlot(NUM_TRIMS, t), STR_VTRIMS_MODES, -1, 2*MAX_FLIGHT_MODES-1,
                 GET_DEFAULT(g_model.flightModeData[i].trim[t].mode==TRIM_MODE_NONE ? -1 : g_model.flightModeData[i].trim[t].mode),
                 SET_DEFAULT(g_model.flightModeData[i].trim[t].mode));
      a->setBackgroundHandler(backgroundHandler);
    }
    grid.nextLine();

    // Flight mode fade in / out
    new StaticText(group, getRightAlignedLabelSlot(grid), "Fade in/out", 0, RIGHT | COLOR_THEME_PRIMARY1);
    a = new NumberEdit(group, grid.getFieldSlot(2, 0), 0, DELAY_MAX,
                          GET_DEFAULT(g_model.flightModeData[i].fadeIn),
                          SET_VALUE(g_model.flightModeData[i].fadeIn, newValue),
                          0, PREC1);
    a->setBackgroundHandler(backgroundHandler);
    a = new NumberEdit(group, grid.getFieldSlot(2, 1), 0, DELAY_MAX,
                          GET_DEFAULT(g_model.flightModeData[i].fadeOut),
                          SET_VALUE(g_model.flightModeData[i].fadeOut, newValue),
                          0, PREC1);
    a->setBackgroundHandler(backgroundHandler);

    grid.nextLine();

    grid.spacer();
    coord_t height = grid.getWindowHeight();
    group->setHeight(height);
    y += height + 2;

    if (i == 0)
      group->setSubWindowColors(true);
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
