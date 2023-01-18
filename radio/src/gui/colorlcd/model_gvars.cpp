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

static const lv_coord_t btn_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(2),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t btn_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT,
                                     LV_GRID_TEMPLATE_LAST};

GVarButton::GVarButton(FormGroup * parent, const rect_t &rect, uint8_t gvar) :
  Button(parent, rect, nullptr, 0, 0, lv_btn_create),
  gvarIdx(gvar)
{
  setHeight(LV_SIZE_CONTENT);
  setWidth(LV_PCT(100));

  build();
}

void GVarButton::checkEvents()
{
  Button::checkEvents();
  uint8_t newFM = getFlightMode();
  if (currentFlightMode != newFM) {
    uint8_t lastFM = currentFlightMode;
    currentFlightMode = newFM;

    labels[lastFM]->setFont(FONT(STD));
    labels[currentFlightMode]->setFont(FONT(BOLD));
    updateValueText(lastFM);
    updateValueText(currentFlightMode);
  }
  for (int flightMode = 0; flightMode < MAX_FLIGHT_MODES; flightMode++) {
    FlightModeData * fmData = &g_model.flightModeData[flightMode];
    if(values[flightMode] != fmData->gvars[gvarIdx])
    {
      updateValueText(flightMode);
    }
  }
}

void GVarButton::build()
{
  lv_obj_set_flex_flow(lvobj, LV_FLEX_FLOW_ROW);
  lv_obj_set_style_pad_all(lvobj, 0, LV_PART_MAIN);
  lv_obj_set_style_bg_color(lvobj, makeLvColor(COLOR_THEME_SECONDARY2), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(lvobj, LV_OPA_COVER, LV_PART_MAIN);

  auto t = new StaticText(this, rect_t{}, getGVarString(gvarIdx), 0, COLOR_THEME_PRIMARY1);
  t->setWidth(GVAR_NAME_SIZE);
  Window* container = new Window(this, rect_t{});
  lv_obj_set_style_pad_all(container->getLvObj(), 0, LV_PART_MAIN);
  lv_obj_set_style_flex_grow(container->getLvObj(), 2, LV_PART_MAIN);
  lv_obj_set_height(container->getLvObj(), LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(container->getLvObj(), LV_FLEX_FLOW_ROW_WRAP);

  for (int flightMode = 0; flightMode < MAX_FLIGHT_MODES; flightMode++) {
    Window* fmCont = new Window(container, rect_t{});

    lv_obj_set_width(fmCont->getLvObj(), GVAR_NAME_SIZE);
    lv_obj_set_height(fmCont->getLvObj(), PAGE_LINE_HEIGHT*2);
    lv_obj_set_style_pad_hor(fmCont->getLvObj(), 3, LV_PART_MAIN);

    char label[16] = {};
    getFlightModeString(label, flightMode + 1);
    lv_obj_set_flex_flow(fmCont->getLvObj(), LV_FLEX_FLOW_COLUMN);

    labels[flightMode] = new StaticText(fmCont, rect_t{}, label, 0, COLOR_THEME_PRIMARY1|CENTERED);
    labels[flightMode]->setHeight(PAGE_LINE_HEIGHT);
    labels[flightMode]->setWidth(GVAR_NAME_SIZE);

    valueTexts[flightMode]  = new StaticText(fmCont, rect_t{}, "", 0, COLOR_THEME_PRIMARY1|CENTERED);
    StaticText* valText = valueTexts[flightMode];
    if (flightMode == currentFlightMode)
    {
      labels[flightMode]->setFont(FONT(BOLD));
      valText->setFont(FONT(BOLD));
    }

    lv_obj_set_style_flex_track_place(valText->getLvObj(), LV_FLEX_ALIGN_END, LV_PART_MAIN);
    valText->setWidth(GVAR_NAME_SIZE);
    valText->setBackgroundColor(COLOR_THEME_PRIMARY2);
    valText->setBackgroudOpacity(LV_OPA_COVER);
    valText->setHeight(PAGE_LINE_HEIGHT);

    updateValueText(flightMode);
  }
}

void GVarButton::updateValueText(uint8_t flightMode)
{
  StaticText* field = valueTexts[flightMode];
  gvar_t value = g_model.flightModeData[flightMode].gvars[gvarIdx];
  values[flightMode] = value;

  if (value > GVAR_MAX) {
    uint8_t fm = value - GVAR_MAX - 1;
    char label[16] = {};
    getFlightModeString(label, fm + 1);

    lv_label_set_text(field->getLvObj(), label);

    if(flightMode == currentFlightMode)
      field->setFont(FONT(BOLD));
    else
      field->setFont(FONT(STD));
  } else {
    uint8_t unit = g_model.gvars[gvarIdx].unit;
    const char * suffix = (unit == 1) ? "%": "";
    uint8_t prec = g_model.gvars[gvarIdx].prec;
    if(prec)
      lv_label_set_text_fmt(field->getLvObj(), "%d.%01u%s", value/10, (value<0)?(-value)%10:value%10, suffix);
    else
      lv_label_set_text_fmt(field->getLvObj(), "%d%s", value, suffix);
    if(unit)
    {
      if (value <= -1000 || value >= 1000
          || (prec && (value <= -100)) ) {
        field->setFont(FONT(XS));
      } else if (value <= -100) {
        field->setFont(FONT(STD));
      } else {
        if(flightMode == currentFlightMode)
          field->setFont(FONT(BOLD));
        else
          field->setFont(FONT(STD));
      }
    } else {
      if(value > -1000 && flightMode == currentFlightMode)
        field->setFont(FONT(BOLD));
      else
        field->setFont(FONT(STD));
    }
  }
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
  header.setTitle(STR_GLOBAL_VAR);
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

  LcdFlags prec = gvar->prec ? PREC1 : 0;
  if (min && max) {
    min->setMax(maxValue);
    max->setMin(minValue);

    min->setSuffix(suffix);
    max->setSuffix(suffix);

    LcdFlags minFlags = min->getTextFlags();
    LcdFlags maxFlags = max->getTextFlags();
    if(gvar->prec) {
      minFlags |= PREC1;
      maxFlags |= PREC1;
    } else {
      minFlags &= ~PREC1;
      maxFlags &= ~PREC1;
    }
    min->setTextFlags(minFlags);
    max->setTextFlags(maxFlags);

    min->update();
    max->update();
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

      LcdFlags flags = values[fm]->getTextFlags();
      if(gvar->prec)
        flags |= PREC1;
      else
    	flags &= ~PREC1;
      values[fm]->setTextFlags(flags);

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
    values[fm]->invalidate();
  }
  if (gVarInHeader) gVarInHeader->invalidate();
}

static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(2),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT,
                                     LV_GRID_TEMPLATE_LAST};


void GVarEditWindow::buildBody(FormWindow * window)
{
  window->setFlexLayout();
  FlexGridLayout grid(col_dsc, row_dsc, 2);
  auto line = window->newLine(&grid);

  GVarData * gvar = &g_model.gvars[index];

  new StaticText(line, rect_t{}, STR_NAME, 0, COLOR_THEME_PRIMARY1);
  grid.nextCell();
  Window* input = new ModelTextEdit(line, rect_t{}, gvar->name, LEN_GVAR_NAME);
  lv_obj_set_style_grid_cell_x_align(input->getLvObj(), LV_GRID_ALIGN_STRETCH, 0);

  line = window->newLine(&grid);

  new StaticText(line, rect_t{}, STR_UNIT, 0, COLOR_THEME_PRIMARY1);
  grid.nextCell();
  input = new Choice(line, rect_t{}, "\001-%", 0, 1, GET_DEFAULT(gvar->unit),
             [=](int16_t newValue) {
                 gvar->unit = newValue;
                 SET_DIRTY();
                 setProperties();
             }
  );
  lv_obj_set_style_grid_cell_x_align(input->getLvObj(), LV_GRID_ALIGN_STRETCH, 0);

  line = window->newLine(&grid);

  new StaticText(line, rect_t{}, STR_PRECISION, 0, COLOR_THEME_PRIMARY1);
  grid.nextCell();
  input = new Choice(line, rect_t{}, STR_VPREC, 0, 1, GET_DEFAULT(gvar->prec),
             [=](int16_t newValue) {
                 gvar->prec = newValue;
                 SET_DIRTY();
                 setProperties();
             }
  );
  lv_obj_set_style_grid_cell_x_align(input->getLvObj(), LV_GRID_ALIGN_STRETCH, 0);

  line = window->newLine(&grid);

  new StaticText(line, rect_t{}, STR_MIN, 0, COLOR_THEME_PRIMARY1);
  grid.nextCell();
  min = new NumberEdit(line, rect_t{}, GVAR_MIN, GVAR_MAX - gvar->max,
                       [=] { return gvar->min + GVAR_MIN; },
                       [=](int32_t newValue) {
                           gvar->min = newValue - GVAR_MIN;
                           SET_DIRTY();
                           setProperties();
                       }
  );
  min->setAccelFactor(16);
  lv_obj_set_style_grid_cell_x_align(min->getLvObj(), LV_GRID_ALIGN_STRETCH, 0);

  line = window->newLine(&grid);

  new StaticText(line, rect_t{}, STR_MAX, 0, COLOR_THEME_PRIMARY1);
  grid.nextCell();
  max = new NumberEdit(line, rect_t{}, GVAR_MIN + gvar->min, GVAR_MAX,
                       [=] { return GVAR_MAX - gvar->max; },
                       [=](int32_t newValue) {
                           gvar->max = GVAR_MAX - newValue;
                           SET_DIRTY();
                           setProperties();
                       }
  );
  max->setAccelFactor(16);
  lv_obj_set_style_grid_cell_x_align(max->getLvObj(), LV_GRID_ALIGN_STRETCH, 0);

  line = window->newLine(&grid);
  char flightModeName[16];
  FlightModeData * fmData;

  for (int flightMode = 0; flightMode < MAX_FLIGHT_MODES; flightMode++) {
    fmData = &g_model.flightModeData[flightMode];
    getFlightModeString(flightModeName, flightMode + 1);

    int userNameLen = zlen(fmData->name, LEN_FLIGHT_MODE_NAME);

    if (userNameLen > 0) {
      strcpy(flightModeName, fmData->name);
    }
    new StaticText(line, rect_t{}, flightModeName, 0, COLOR_THEME_PRIMARY1);
    if (flightMode > 0) {
      auto cb = new CheckBox(line, rect_t{},
                             [=] { return fmData->gvars[index] <= GVAR_MAX; }, [=](uint8_t checked) {
            fmData->gvars[index] = checked ? 0 : GVAR_MAX + 1;
            setProperties(flightMode);
        });
      lv_obj_set_style_grid_cell_x_align(cb->getLvObj(), LV_GRID_ALIGN_END, 0);
      lv_obj_invalidate(cb->getLvObj());
      cb->setLabel(STR_OWN);

    } else {
      grid.nextCell();
    }

    values[flightMode] = new NumberEdit(line, rect_t{}, GVAR_MIN + gvar->min, GVAR_MAX + MAX_FLIGHT_MODES - 1,
                                        GET_SET_DEFAULT(fmData->gvars[index]));
    values[flightMode]->setAccelFactor(16);
    line = window->newLine(&grid);
  }

  setProperties();
  lv_obj_set_height(window->getLvObj(), LCD_H - lv_obj_get_height(header.getLvObj()));
  lv_obj_set_height(lvobj, LCD_H);
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
  window->setFlexLayout(LV_FLEX_FLOW_COLUMN, 1);

  for (uint8_t index = 0; index < MAX_GVARS; index++) {
    Button * button = new GVarButton(window, rect_t{}, index);
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

  }
}
