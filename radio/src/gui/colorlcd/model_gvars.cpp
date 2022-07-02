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

#include "opentx.h"
#include "model_gvars.h"
#include "libopenui.h"

#define SET_DIRTY()     storageDirty(EE_MODEL)

#define TEXT_LEFT_MARGIN 2
#define GVAR_NAME_SIZE 45

GVarButton::GVarButton(FormGroup * parent, const rect_t &rect, uint8_t gvar) :
  Button(parent, rect),
  gvarIdx(gvar)
{
  int perRow = (width() - GVAR_NAME_SIZE + TEXT_LEFT_MARGIN * 2) / GVAR_NAME_SIZE;
  lines = MAX_FLIGHT_MODES / perRow;
  if (MAX_FLIGHT_MODES % perRow != 0) lines++;
  setHeight(PAGE_LINE_HEIGHT * (lines << 1) + FIELD_PADDING_TOP * 2);
}

void GVarButton::checkEvents()
{
  Button::checkEvents();
  if (currentFlightMode != getFlightMode()) {
    invalidate();
  }
  else {
    int32_t sum = 0;
    for (int flightMode = 0; flightMode < MAX_FLIGHT_MODES; flightMode++) {
      FlightModeData * fmData = &g_model.flightModeData[flightMode];
      sum += fmData->gvars[gvarIdx];
    }
    if (sum != gvarSum) invalidate();
  }
}

void GVarButton::paint(BitmapBuffer * dc)
{
  GVarData * gvar = &g_model.gvars[gvarIdx];

  // The bounding rect
  int nameRectW = TEXT_LEFT_MARGIN + GVAR_NAME_SIZE;
  coord_t x = TEXT_LEFT_MARGIN;
  coord_t y = FIELD_PADDING_TOP;
  currentFlightMode = getFlightMode();
  gvarSum = 0;

  LcdFlags bgColor = COLOR_THEME_SECONDARY2;
  LcdFlags textColor = COLOR_THEME_PRIMARY1;
  if (hasFocus()) {
    bgColor   = COLOR_THEME_FOCUS;
    textColor = COLOR_THEME_PRIMARY2;
  }

  dc->drawSolidFilledRect(0, 0, nameRectW, rect.h, bgColor);
  dc->drawSolidFilledRect(nameRectW, 0, rect.w - nameRectW, rect.h, COLOR_THEME_PRIMARY2);

  dc->drawText(FIELD_PADDING_LEFT, FIELD_PADDING_TOP, getGVarString(gvarIdx), textColor);
  dc->drawSizedText(x, y, gvar->name, LEN_GVAR_NAME, textColor);

  // values are right aligned
  x += GVAR_NAME_SIZE;
  bool bgFilled = false;

  coord_t startX = x;
  for (int flightMode = 0; flightMode < MAX_FLIGHT_MODES; flightMode++) {
    FlightModeData * fmData = &g_model.flightModeData[flightMode];
    gvar_t v = fmData->gvars[gvarIdx];
    gvarSum += v;

    LcdFlags attr = COLOR_THEME_PRIMARY1 | RIGHT;
    if (flightMode == currentFlightMode)
      attr |= FONT(BOLD);

    x += GVAR_NAME_SIZE;
    if (x > width()) {
      bgFilled = false;
      x = nameRectW + GVAR_NAME_SIZE;
      y += PAGE_LINE_HEIGHT * 2;
      startX = nameRectW;
    }

    if (!bgFilled) {
      dc->drawSolidFilledRect(startX, y, width() - startX, PAGE_LINE_HEIGHT,
                              COLOR_THEME_SECONDARY2);
      bgFilled = true;
    }

    // Flight mode
    drawFlightMode(dc, x, y, flightMode, attr);

    coord_t yval = y + PAGE_LINE_HEIGHT;
    if (v <= GVAR_MAX && (gvar->prec > 0 || abs(v) >= 1000 || (abs(v) >= 100 && gvar->unit > 0))) {
      attr |= FONT(XS);
      attr &= ~FONT(BOLD);
      yval += 3;
    }

    if (v > GVAR_MAX) {
      uint8_t fm = v - GVAR_MAX - 1;
      if (fm >= flightMode)
        fm++;
      drawFlightMode(dc, x, yval, fm, attr);
    }
    else {
      drawGVarValue(dc, x, yval, gvarIdx, v, attr);
    }
  }

  if (hasFocus())
    dc->drawSolidRect(0, 0, rect.w, rect.h, 2, COLOR_THEME_FOCUS);
  else
    dc->drawSolidRect(0, 0, rect.w, rect.h, 2, COLOR_THEME_SECONDARY2);
}

void GVarButton::drawFlightMode(BitmapBuffer * dc, coord_t x, coord_t y, int fm, LcdFlags attr)
{
  char label[16];
  getFlightModeString(label, fm + 1);
  dc->drawSizedText(x, y, label, strlen(label), attr);
}

void GVarRenderer::paint(BitmapBuffer * dc)
{
  lastFlightMode = getFlightMode();
  FlightModeData * fmData = &g_model.flightModeData[lastFlightMode];
  lastGVar = fmData->gvars[index];
  coord_t x = drawStringWithIndex(dc, 0, FIELD_PADDING_TOP, STR_GV, index + 1,
                                  COLOR_THEME_PRIMARY2, nullptr, "=");
  if (lastGVar > GVAR_MAX) {
    uint8_t fm = lastGVar - GVAR_MAX - 1;
    if (fm >= lastFlightMode)
      fm++;
    char label[16];
    getFlightModeString(label, fm + 1);
    dc->drawSizedText(x, FIELD_PADDING_TOP, label, strlen(label), COLOR_THEME_PRIMARY2);
  } else {
    drawGVarValue(dc, x, FIELD_PADDING_TOP, index, lastGVar, COLOR_THEME_PRIMARY2);
  }
}

void GVarRenderer::checkEvents()
{
  if (lastFlightMode != getFlightMode()) {
    invalidate();
    updated = true;
  }
  if (lastGVar != g_model.flightModeData[getFlightMode()].gvars[index]) {
    invalidate();
    updated = true;
  }
}

bool GVarRenderer::isUpdated()
{
  if (!updated)
    return false;
  updated = false;
  return true;
}

void GVarEditWindow::buildHeader(Window * window)
{
  new StaticText(window,
                 {PAGE_TITLE_LEFT, PAGE_TITLE_TOP, LCD_W - PAGE_TITLE_LEFT,
                  PAGE_LINE_HEIGHT},
                 STR_GLOBAL_VAR, 0, COLOR_THEME_PRIMARY2);
  gVarInHeader =
      new GVarRenderer(window,
                       {PAGE_TITLE_LEFT, PAGE_TITLE_TOP + PAGE_LINE_HEIGHT,
                        LCD_W - PAGE_TITLE_LEFT, PAGE_LINE_HEIGHT},
                       index);
}

void GVarEditWindow::checkEvents()
{
  Page::checkEvents();
  if (gVarInHeader && gVarInHeader->isUpdated()) {
    for (auto & value : values) {
      value->invalidate();
    }
  }
}

void GVarEditWindow::setProperties(int onlyForFlightMode)
{
  GVarData * gvar = &g_model.gvars[index];
  int32_t minValue = GVAR_MIN + gvar->min;
  int32_t maxValue = GVAR_MAX - gvar->max;
  const char * suffix = gvar->unit ? "%" : "";

  // TODO needed? LcdFlags prec = gvar->prec ? PREC1 : 0;
  if (min && max) {
    min->setMax(maxValue);
    max->setMin(minValue);

    min->setSuffix(suffix);
    max->setSuffix(suffix);

    // TODO min->setLcdFlags(prec);
    // TODO max->setLcdFlags(prec);

    min->invalidate();
    max->invalidate();
  }
  FlightModeData * fmData;
  for (int fm = 0; fm < MAX_FLIGHT_MODES; fm++) {
    if (values[fm] == nullptr)  // KLK: the order of calls has changed and this might not be initialized yet.
      continue;

    if (onlyForFlightMode >= 0 && fm != onlyForFlightMode)
      continue;
    fmData = &g_model.flightModeData[fm];

    // custom value
    if (fmData->gvars[index] <= GVAR_MAX || fm == 0) {
      values[fm]->setMin(GVAR_MIN + gvar->min);
      values[fm]->setMax(GVAR_MAX - gvar->max);
      values[fm]->setDisplayHandler(nullptr);
    }
    else {
      values[fm]->setMin(GVAR_MAX + 1);
      values[fm]->setMax(GVAR_MAX + MAX_FLIGHT_MODES - 1);
      values[fm]->setDisplayHandler([=](int32_t value) {
          uint8_t targetFlightMode = value - GVAR_MAX - 1;
          if (targetFlightMode >= fm)
            targetFlightMode++;
          char label[16];
          getFlightModeString(label, targetFlightMode + 1);
          return std::string(label);
      });
    }

    values[fm]->setSuffix(suffix);
    // TODO values[fm]->setLcdFlags(prec);
    values[fm]->invalidate();
  }
  if (gVarInHeader) gVarInHeader->invalidate();
}

void GVarEditWindow::buildBody(FormWindow * window)
{
  GVarData * gvar = &g_model.gvars[index];

  FormGridLayout grid;
  grid.spacer(8);

  new StaticText(window, grid.getLabelSlot(), STR_NAME, 0, COLOR_THEME_PRIMARY1);
  new ModelTextEdit(window, grid.getFieldSlot(), gvar->name, LEN_GVAR_NAME);

  grid.nextLine();

  new StaticText(window, grid.getLabelSlot(), STR_UNIT, 0, COLOR_THEME_PRIMARY1);
  new Choice(window, grid.getFieldSlot(), "\001-%", 0, 1, GET_DEFAULT(gvar->unit),
             [=](int16_t newValue) {
                 gvar->unit = newValue;
                 SET_DIRTY();
                 setProperties();
             }
  );

  grid.nextLine();

  new StaticText(window, grid.getLabelSlot(), STR_PRECISION, 0, COLOR_THEME_PRIMARY1);
  new Choice(window, grid.getFieldSlot(), STR_VPREC, 0, 1, GET_DEFAULT(gvar->prec),
             [=](int16_t newValue) {
                 gvar->prec = newValue;
                 SET_DIRTY();
                 setProperties();
             }
  );

  grid.nextLine();

  new StaticText(window, grid.getLabelSlot(), STR_MIN, 0, COLOR_THEME_PRIMARY1);
  min = new NumberEdit(window, grid.getFieldSlot(), GVAR_MIN, GVAR_MAX - gvar->max,
                       [=] { return gvar->min + GVAR_MIN; },
                       [=](int32_t newValue) {
                           gvar->min = newValue - GVAR_MIN;
                           SET_DIRTY();
                           setProperties();
                       }
  );
  grid.nextLine();

  new StaticText(window, grid.getLabelSlot(), STR_MAX, 0, COLOR_THEME_PRIMARY1);
  max = new NumberEdit(window, grid.getFieldSlot(), GVAR_MIN + gvar->min, GVAR_MAX,
                       [=] { return GVAR_MAX - gvar->max; },
                       [=](int32_t newValue) {
                           gvar->max = GVAR_MAX - newValue;
                           SET_DIRTY();
                           setProperties();
                       }
  );
  grid.nextLine();
  char flightModeName[16];
  FlightModeData * fmData;

  for (int flightMode = 0; flightMode < MAX_FLIGHT_MODES; flightMode++) {
    fmData = &g_model.flightModeData[flightMode];
    getFlightModeString(flightModeName, flightMode + 1);

    int userNameLen = zlen(fmData->name, LEN_FLIGHT_MODE_NAME);

    if (userNameLen > 0) {
      strcpy(flightModeName, fmData->name);
    }
    new StaticText(window, grid.getLabelSlot(), flightModeName, 0, COLOR_THEME_PRIMARY1);
    if (flightMode > 0) {
      auto cb = new CheckBox(window, grid.getFieldSlot(2, 0),
                             [=] { return fmData->gvars[index] <= GVAR_MAX; }, [=](uint8_t checked) {
            fmData->gvars[index] = checked ? 0 : GVAR_MAX + 1;
            setProperties(flightMode);
        });
      cb->setLabel(STR_OWN);
    }

    values[flightMode] = new NumberEdit(window, grid.getFieldSlot(2, 1), GVAR_MIN + gvar->min, GVAR_MAX + MAX_FLIGHT_MODES - 1,
                                        GET_SET_DEFAULT(fmData->gvars[index]));
    grid.nextLine();
  }

  setProperties();

}

void ModelGVarsPage::rebuild(FormWindow * window)
{
  auto scroll_y = lv_obj_get_scroll_y(window->getLvObj());  
  window->clear();
  build(window);
  lv_obj_scroll_to_y(window->getLvObj(), scroll_y, LV_ANIM_OFF);
}

void ModelGVarsPage::build(FormWindow * window)
{
  FormGridLayout grid;
  grid.spacer(PAGE_PADDING);
  grid.setLabelWidth(70);
  window->padAll(0);

  for (uint8_t index = 0; index < MAX_GVARS; index++) {
    Button * button = new GVarButton(window, grid.getLineSlot(), index);
    button->setPressHandler([=]() {
        Menu * menu = new Menu(window);
        menu->addLine(STR_EDIT, [=]() {
            Window * editWindow = new GVarEditWindow(index);
            editWindow->setCloseHandler([=]() {
                rebuild(window);
            });
        });
        menu->addLine(STR_CLEAR, [=]() {
            for (auto & flightMode : g_model.flightModeData) {
              flightMode.gvars[index] = 0;
            }
            storageDirty(EE_MODEL);
        });
        return 0;
    });

    grid.nextLine(button->height());
  }
}
