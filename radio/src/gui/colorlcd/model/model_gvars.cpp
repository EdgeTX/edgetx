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

#include "model_gvars.h"

#include "libopenui.h"
#include "list_line_button.h"
#include "numberedit.h"
#include "edgetx.h"
#include "page.h"
#include "etx_lv_theme.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

#define ETX_STATE_VALUE_SMALL_FONT LV_STATE_USER_1

void getFMExtName(char* dest, int8_t idx)
{
  getFlightModeString(dest, idx);

  FlightModeData* fmData = &g_model.flightModeData[idx - 1];
  int userNameLen = zlen(fmData->name, LEN_FLIGHT_MODE_NAME);

  if (userNameLen > 0) {
    char* s = strAppend(dest + strlen(dest), ":", 1);
    strAppend(s, fmData->name, LEN_FLIGHT_MODE_NAME);
  }
}

class GVarButton : public ListLineButton
{
 public:
  GVarButton(Window* parent, uint8_t gvar) :
      ListLineButton(parent, gvar)
  {
    padAll(PAD_ZERO);
    setHeight(BTN_H);
    if (!modelFMEnabled()) padLeft(PAD_LARGE);

    lv_obj_add_event_cb(lvobj, GVarButton::on_draw, LV_EVENT_DRAW_MAIN_BEGIN,
                        nullptr);
  }

  static void on_draw(lv_event_t* e)
  {
    lv_obj_t* target = lv_event_get_target(e);
    auto line = (GVarButton*)lv_obj_get_user_data(target);
    if (line) line->build();
  }

  static LAYOUT_VAL(GVAR_NAME_SIZE, 44, 44)
  static constexpr coord_t GVAR_VAL_H = EdgeTxStyles::PAGE_LINE_HEIGHT + 2;
  static LAYOUT_VAL(GVAR_VAL_W, 45, 50)
  static LAYOUT_VAL(GVAR_COLS, MAX_FLIGHT_MODES, 5)
  static LAYOUT_VAL(BTN_H, EdgeTxStyles::UI_ELEMENT_HEIGHT, 50)
  static LAYOUT_VAL(GVAR_NM_Y, 4, 13)
  static LAYOUT_VAL(GVAR_YO, 4, 2)
  static LAYOUT_VAL(HDR_H, EdgeTxStyles::PAGE_LINE_HEIGHT + 2, EdgeTxStyles::PAGE_LINE_HEIGHT * 2 + 2)

  static const lv_obj_class_t gv_label_class;
  static const lv_obj_class_t gv_value_class;

 protected:
  bool init = false;
  uint8_t currentFlightMode = 0;  // used for checking updates
  lv_obj_t* valueTexts[MAX_FLIGHT_MODES];
  gvar_t values[MAX_FLIGHT_MODES];

  int numFlightModes() { return modelFMEnabled() ? MAX_FLIGHT_MODES : 1; }

  void checkEvents() override
  {
    ListLineButton::checkEvents();
    if (init) {
      if (modelFMEnabled()) {
        uint8_t newFM = getFlightMode();
        if (currentFlightMode != newFM) {
          lv_obj_add_state(valueTexts[newFM], LV_STATE_CHECKED);
          lv_obj_clear_state(valueTexts[currentFlightMode], LV_STATE_CHECKED);

          currentFlightMode = newFM;
        }
      }

      for (int flightMode = 0; flightMode < numFlightModes(); flightMode++) {
        FlightModeData* fmData = &g_model.flightModeData[flightMode];
        if (values[flightMode] != fmData->gvars[index]) {
          updateValueText(flightMode);
        }
      }
    }
  }

  void build()
  {
    if (init) return;

    init =true;

    lv_obj_enable_style_refresh(false);

    currentFlightMode = getFlightMode();

    auto nm = lv_label_create(lvobj);
    lv_label_set_text(nm, getGVarString(index));
    lv_obj_set_pos(nm, PAD_TINY, GVAR_NM_Y);
    lv_obj_set_size(nm, GVAR_NAME_SIZE, EdgeTxStyles::PAGE_LINE_HEIGHT);

    if (modelFMEnabled()) {
      for (int flightMode = 0; flightMode < MAX_FLIGHT_MODES; flightMode++) {
        valueTexts[flightMode] = etx_create(&gv_value_class, lvobj);
        lv_obj_set_pos(valueTexts[flightMode], (flightMode % GVAR_COLS) * GVAR_VAL_W + GVAR_NAME_SIZE + 4,
                       (flightMode / GVAR_COLS) * GVAR_VAL_H + GVAR_YO);

        if (flightMode == currentFlightMode) {
          lv_obj_add_state(valueTexts[flightMode], LV_STATE_CHECKED);
        }

        updateValueText(flightMode);
      }
    } else {
      valueTexts[0] = lv_label_create(lvobj);
      lv_obj_set_pos(valueTexts[0], GVAR_NAME_SIZE + PAD_MEDIUM, (BTN_H - EdgeTxStyles::PAGE_LINE_HEIGHT - PAD_SMALL) / 2);

      updateValueText(0);
    }

    lv_obj_update_layout(lvobj);
  
    lv_obj_enable_style_refresh(true);
    lv_obj_refresh_style(lvobj, LV_PART_ANY, LV_STYLE_PROP_ANY);
  }

  void updateValueText(uint8_t flightMode)
  {
    lv_obj_t* field = valueTexts[flightMode];
    gvar_t value = g_model.flightModeData[flightMode].gvars[index];
    values[flightMode] = value;

    if (value > GVAR_MAX) {
      uint8_t fm = value - GVAR_MAX - 1;
      if (fm >= flightMode) fm += 1;
      char label[16] = {};
      getFlightModeString(label, fm + 1);

      lv_label_set_text(field, label);
    } else {
      uint8_t unit = g_model.gvars[index].unit;
      const char* suffix = (unit == 1) ? "%" : "";
      uint8_t prec = g_model.gvars[index].prec;
      if (prec)
        lv_label_set_text_fmt(field, "%d.%01u%s", value / 10,
                              (value < 0) ? (-value) % 10 : value % 10, suffix);
      else
        lv_label_set_text_fmt(field, "%d%s", value, suffix);
      if (unit) {
        if (value <= -1000 || value >= 1000 || (prec && (value <= -100))) {
          lv_obj_add_state(field, ETX_STATE_VALUE_SMALL_FONT);
        } else {
          lv_obj_clear_state(field, ETX_STATE_VALUE_SMALL_FONT);
        }
      }
    }
  }

  bool isActive() const override { return false; }
  void refresh() override {}
};

static void gv_label_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  etx_obj_add_style(obj, styles->text_align_center, LV_PART_MAIN);
  etx_font(obj, FONT_XS_INDEX);
  etx_solid_bg(obj, COLOR_THEME_ACTIVE_INDEX, LV_STATE_CHECKED);
}

const lv_obj_class_t GVarButton::gv_label_class = {
    .base_class = &lv_label_class,
    .constructor_cb = gv_label_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = GVarButton::GVAR_VAL_W,
    .height_def = EdgeTxStyles::PAGE_LINE_HEIGHT - PAD_MEDIUM,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_INHERIT,
    .instance_size = sizeof(lv_label_t),
};

static void gv_value_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  etx_obj_add_style(obj, styles->text_align_center, LV_PART_MAIN);
  etx_font(obj, FONT_XS_INDEX, LV_PART_MAIN | ETX_STATE_VALUE_SMALL_FONT);
  etx_solid_bg(obj, COLOR_THEME_ACTIVE_INDEX, LV_STATE_CHECKED);
}

const lv_obj_class_t GVarButton::gv_value_class = {
    .base_class = &lv_label_class,
    .constructor_cb = gv_value_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = GVarButton::GVAR_VAL_W,
    .height_def = EdgeTxStyles::PAGE_LINE_HEIGHT,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_INHERIT,
    .instance_size = sizeof(lv_label_t),
};

class GVarHeader : public Window
{
 public:
  GVarHeader(Window* parent) :
      Window(parent, {0, 0, LCD_W, GVarButton::HDR_H})
  {
    padAll(PAD_ZERO);
    etx_solid_bg(lvobj, COLOR_THEME_SECONDARY3_INDEX);

    lv_obj_add_event_cb(lvobj, GVarHeader::on_draw, LV_EVENT_DRAW_MAIN_BEGIN,
                        nullptr);
  }

  static void on_draw(lv_event_t* e)
  {
    lv_obj_t* target = lv_event_get_target(e);
    auto line = (GVarHeader*)lv_obj_get_user_data(target);
    if (line) line->build();
  }

 protected:
  bool init = false;
  uint8_t currentFlightMode = 0;  // used for checking updates
  lv_obj_t* labelTexts[MAX_FLIGHT_MODES];

  int numFlightModes() { return modelFMEnabled() ? MAX_FLIGHT_MODES : 1; }

  void checkEvents() override
  {
    Window::checkEvents();
    if (init) {
      uint8_t newFM = getFlightMode();
      if (currentFlightMode != newFM) {
        lv_obj_add_state(labelTexts[newFM], LV_STATE_CHECKED);
        lv_obj_clear_state(labelTexts[currentFlightMode], LV_STATE_CHECKED);

        currentFlightMode = newFM;
      }
    }
  }

  void build()
  {
    if (init) return;

    init =true;

    currentFlightMode = getFlightMode();

    char label[16] = {};

    for (int flightMode = 0; flightMode < MAX_FLIGHT_MODES; flightMode++) {
      getFlightModeString(label, flightMode + 1);

      labelTexts[flightMode] = etx_create(&GVarButton::gv_value_class, lvobj);
      lv_label_set_text(labelTexts[flightMode], label);
      lv_obj_set_pos(labelTexts[flightMode], (flightMode % GVarButton::GVAR_COLS) * GVarButton::GVAR_VAL_W + GVarButton::GVAR_NAME_SIZE + 12,
                      (flightMode / GVarButton::GVAR_COLS) * EdgeTxStyles::PAGE_LINE_HEIGHT + 1);

      if (flightMode == currentFlightMode) {
        lv_obj_add_state(labelTexts[flightMode], LV_STATE_CHECKED);
      }
    }

    lv_obj_update_layout(lvobj);
  }
};

class GVarEditWindow : public Page
{
 public:
  explicit GVarEditWindow(uint8_t gvarIndex) :
      Page(ICON_MODEL_GVARS), index(gvarIndex)
  {
    buildHeader(header);
    buildBody(body);
  }

 protected:
  uint8_t index;
  gvar_t lastGVar = 0;
  bool refreshTitle = true;
  uint8_t lastFlightMode = 255;  // Force initial setting of header title
  NumberEdit* min = nullptr;
  NumberEdit* max = nullptr;
  NumberEdit* values[MAX_FLIGHT_MODES] = {};
  StaticText* gVarInHeader = nullptr;

  int numFlightModes() { return modelFMEnabled() ? MAX_FLIGHT_MODES : 1; }

  void buildHeader(Window* window)
  {
    header->setTitle(STR_MENU_GLOBAL_VARS);
    gVarInHeader = header->setTitle2("");
  }

  void checkEvents()
  {
    Page::checkEvents();

    auto curFM = getFlightMode();
    auto fmData = &g_model.flightModeData[curFM];

    if (gVarInHeader && ((lastFlightMode != curFM) ||
                         (lastGVar != fmData->gvars[index]) || refreshTitle)) {
      char label[32];
      refreshTitle = false;
      lastFlightMode = curFM;
      lastGVar = fmData->gvars[index];
      sprintf(label, "%s=", getSourceString(index + MIXSRC_FIRST_GVAR));
      if (lastGVar > GVAR_MAX) {
        uint8_t fm = lastGVar - GVAR_MAX - 1;
        if (fm >= curFM) fm++;
        getFMExtName(label + strlen(label), fm + 1);
      } else {
        strcat(label, getGVarValue(index, lastGVar, 0).c_str());
      }
      gVarInHeader->setText(label);
    }
  }

  void setProperties(int onlyForFlightMode = -1)
  {
    GVarData* gvar = &g_model.gvars[index];
    int32_t minValue = GVAR_MIN + gvar->min;
    int32_t maxValue = GVAR_MAX - gvar->max;
    const char* suffix = gvar->unit ? "%" : "";

    if (min && max) {
      min->setMax(maxValue);
      max->setMin(minValue);

      min->setSuffix(suffix);
      max->setSuffix(suffix);

      if (gvar->prec) {
        min->setTextFlag(PREC1);
        max->setTextFlag(PREC1);
      } else {
        min->clearTextFlag(PREC1);
        max->clearTextFlag(PREC1);
      }

      min->update();
      max->update();
    }
    FlightModeData* fmData;
    for (int fm = 0; fm < numFlightModes(); fm++) {
      if (values[fm] == nullptr)  // KLK: the order of calls has changed and
                                  // this might not be initialized yet.
        continue;

      if (onlyForFlightMode >= 0 && fm != onlyForFlightMode) continue;
      fmData = &g_model.flightModeData[fm];

      // custom value
      if (fmData->gvars[index] <= GVAR_MAX || fm == 0) {
        values[fm]->setMin(GVAR_MIN + gvar->min);
        values[fm]->setMax(GVAR_MAX - gvar->max);
        // Update value if outside min/max range
        values[fm]->setValue(values[fm]->getValue());

        if (gvar->prec)
          values[fm]->setTextFlag(PREC1);
        else
          values[fm]->clearTextFlag(PREC1);

        values[fm]->setDisplayHandler(nullptr);
      } else {
        values[fm]->setMin(GVAR_MAX + 1);
        values[fm]->setMax(GVAR_MAX + MAX_FLIGHT_MODES - 1);
        values[fm]->setDisplayHandler([=](int32_t value) {
          uint8_t targetFlightMode = value - GVAR_MAX - 1;
          if (targetFlightMode >= fm) targetFlightMode++;
          char label[16];
          getFlightModeString(label, targetFlightMode + 1);
          return std::string(label);
        });
      }

      values[fm]->setSuffix(suffix);
    }
  }

  void buildBody(Window* window)
  {
    static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                         LV_GRID_FR(2), LV_GRID_TEMPLATE_LAST};
    static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT,
                                         LV_GRID_TEMPLATE_LAST};

    window->setFlexLayout();
    FlexGridLayout grid(col_dsc, row_dsc, PAD_TINY);

    auto line = window->newLine(grid);

    GVarData* gvar = &g_model.gvars[index];

    new StaticText(line, rect_t{}, STR_NAME);
    grid.nextCell();
    new ModelTextEdit(line, rect_t{}, gvar->name, LEN_GVAR_NAME, [=]() { refreshTitle = true; });

    line = window->newLine(grid);

    static const char* const strUnits[] = { "-", "%" };
    new StaticText(line, rect_t{}, STR_UNIT);
    grid.nextCell();
    new Choice(line, rect_t{}, strUnits, 0, 1, GET_DEFAULT(gvar->unit),
               [=](int16_t newValue) {
                 refreshTitle = (gvar->unit != newValue);
                 gvar->unit = newValue;
                 SET_DIRTY();
                 setProperties();
               });

    line = window->newLine(grid);

    new StaticText(line, rect_t{}, STR_PRECISION);
    grid.nextCell();
    new Choice(line, rect_t{}, STR_VPREC, 0, 1, GET_DEFAULT(gvar->prec),
               [=](int16_t newValue) {
                 refreshTitle = (gvar->prec != newValue);
                 gvar->prec = newValue;
                 SET_DIRTY();
                 setProperties();
               });

    line = window->newLine(grid);

    new StaticText(line, rect_t{}, STR_MIN);
    grid.nextCell();
    min = new NumberEdit(
        line, rect_t{}, GVAR_MIN, GVAR_MAX - gvar->max,
        [=] { return gvar->min + GVAR_MIN; },
        [=](int32_t newValue) {
          gvar->min = newValue - GVAR_MIN;
          SET_DIRTY();
          setProperties();
        });
    min->setAccelFactor(16);

    line = window->newLine(grid);

    new StaticText(line, rect_t{}, STR_MAX);
    grid.nextCell();
    max = new NumberEdit(
        line, rect_t{}, GVAR_MIN + gvar->min, GVAR_MAX,
        [=] { return GVAR_MAX - gvar->max; },
        [=](int32_t newValue) {
          gvar->max = GVAR_MAX - newValue;
          SET_DIRTY();
          setProperties();
        });
    max->setAccelFactor(16);

    line = window->newLine(grid);
    new StaticText(line, rect_t{}, STR_POPUP);
    grid.nextCell();
    new ToggleSwitch(line, rect_t{}, GET_SET_DEFAULT(gvar->popup));

    line = window->newLine(grid);
    char flightModeName[16];
    FlightModeData* fmData;

    for (int flightMode = 0; flightMode < numFlightModes(); flightMode++) {
      fmData = &g_model.flightModeData[flightMode];

      if (modelFMEnabled()) {
        getFMExtName(flightModeName, flightMode + 1);
        new StaticText(line, rect_t{}, flightModeName);
      } else {
        new StaticText(line, rect_t{}, STR_VALUE);
      }

      if (flightMode > 0) {
        auto cb = new ToggleSwitch(
            line, rect_t{}, [=] { return fmData->gvars[index] <= GVAR_MAX; },
            [=](uint8_t checked) {
              fmData->gvars[index] = checked ? 0 : GVAR_MAX + 1;
              setProperties(flightMode);
            });
        lv_obj_set_style_grid_cell_x_align(cb->getLvObj(), LV_GRID_ALIGN_END,
                                           0);
        lv_obj_invalidate(cb->getLvObj());
      } else {
        grid.nextCell();
      }

      values[flightMode] = new NumberEdit(
          line, rect_t{}, GVAR_MIN + gvar->min, GVAR_MAX + MAX_FLIGHT_MODES - 1,
          GET_SET_DEFAULT(fmData->gvars[index]));
      values[flightMode]->setAccelFactor(16);
      line = window->newLine(grid);
    }

    setProperties();
    lv_obj_set_height(window->getLvObj(),
                      LCD_H - lv_obj_get_height(header->getLvObj()));
    lv_obj_set_height(lvobj, LCD_H);
  }
};

ModelGVarsPage::ModelGVarsPage() :
    PageTab(STR_MENU_GLOBAL_VARS, ICON_MODEL_GVARS)
{
}

void ModelGVarsPage::cleanup()
{
  if (hdr)
    hdr->deleteLater();
  hdr = nullptr;
}

void ModelGVarsPage::rebuild(Window* window)
{
  auto scroll_y = lv_obj_get_scroll_y(window->getLvObj());
  window->clear();
  cleanup();
  build(window);
  lv_obj_scroll_to_y(window->getLvObj(), scroll_y, LV_ANIM_OFF);
}

void ModelGVarsPage::build(Window* window)
{
  coord_t yo = 0;
  if (modelFMEnabled()) {
    hdr = new GVarHeader(window->getParent());
    lv_obj_set_pos(hdr->getLvObj(), 0, TabsGroup::MENU_TITLE_TOP + TabsGroup::MENU_TITLE_HEIGHT);
    yo = GVarButton::HDR_H - 2;
  }

  for (uint8_t index = 0; index < MAX_GVARS; index++) {
    auto button = new GVarButton(window, index);
    lv_obj_set_pos(button->getLvObj(), 0, yo + index * (GVarButton::BTN_H + PAD_TINY));
    button->setPressHandler([=]() {
      Menu* menu = new Menu();
      menu->addLine(STR_EDIT, [=]() {
        Window* editWindow = new GVarEditWindow(index);
        editWindow->setCloseHandler([=]() { rebuild(window); });
      });
      menu->addLine(STR_CLEAR, [=]() {
        for (auto& flightMode : g_model.flightModeData) {
          flightMode.gvars[index] = 0;
        }
        storageDirty(EE_MODEL);
      });
      return 0;
    });
  }
}
