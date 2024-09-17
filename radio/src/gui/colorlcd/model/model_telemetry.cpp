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

#include "model_telemetry.h"

#include "fullscreen_dialog.h"
#include "libopenui.h"
#include "list_line_button.h"
#include "edgetx.h"
#include "page.h"
#include "sourcechoice.h"
#include "etx_lv_theme.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

#define ETX_STATE_VALUE_SMALL_FONT LV_STATE_USER_1
#define ETX_STATE_VALUE_STALE_WARN LV_STATE_USER_1

std::string getSensorCustomValue(uint8_t sensor, int32_t value, LcdFlags flags);

#if (PORTRAIT_LCD) || defined(TRANSLATIONS_CZ)
#define TWOCOLBUTTONS 1
#else
#define TWOCOLBUTTONS 0
#endif

// Overview grid variants
static const lv_coord_t col_dsc[] = {LV_GRID_FR(14), LV_GRID_FR(6),
                                     LV_GRID_TEMPLATE_LAST};
#if TWOCOLBUTTONS
static const lv_coord_t col_dsc4[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                      LV_GRID_TEMPLATE_LAST};
#else
static const lv_coord_t col_dsc4[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                      LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
#endif
static const lv_coord_t col_dsc5[] = {LV_GRID_FR(5), LV_GRID_FR(4),
                                      LV_GRID_FR(4), LV_GRID_FR(4),
                                      LV_GRID_TEMPLATE_LAST};

static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

class TSStyle
{
 public:
  TSStyle() {}

  void init()
  {
    if (!styleInitDone) {
      styleInitDone = true;

      lv_style_init(&tsFreshStyle);
      lv_style_set_img_recolor_opa(&tsFreshStyle, LV_OPA_COVER);
    }

    // Always update colors in case theme changes
    lv_style_set_img_recolor(&tsFreshStyle,
                             makeLvColor(COLOR_THEME_SECONDARY1));
  }

  lv_obj_t* newNum(lv_obj_t* parent, uint8_t index);
  lv_obj_t* newId(lv_obj_t* parent, const char* id);
  lv_obj_t* newName(lv_obj_t* parent, const char* name);
  lv_obj_t* newValue(lv_obj_t* parent);

  lv_style_t tsFreshStyle;

  static LAYOUT_VAL(NUM_W, 36, 36)
  static LAYOUT_VAL(NUM_H, 20, 20)
  static LAYOUT_VAL(NAME_W, 56, 56)
  static LAYOUT_VAL(ID_Y, 17, 17)
  static LAYOUT_VAL(ID_H, 11, 11)
  static LAYOUT_VAL(FRSH_Y, 10, 10)

 private:
  bool styleInitDone;
};

static TSStyle tsStyle;

static void ts_num_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  etx_obj_add_style(obj, styles->text_align_center, LV_PART_MAIN);
  etx_font(obj, FONT_XS_INDEX, ETX_STATE_VALUE_SMALL_FONT);
}

static const lv_obj_class_t ts_num_class = {
    .base_class = &lv_label_class,
    .constructor_cb = ts_num_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = TSStyle::NUM_W,
    .height_def = TSStyle::NUM_H,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_INHERIT,
    .instance_size = sizeof(lv_label_t),
};

lv_obj_t* TSStyle::newNum(lv_obj_t* parent, uint8_t index)
{
  auto obj = etx_create(&ts_num_class, parent);
  lv_label_set_text(obj, std::to_string(index + 1).c_str());

  return obj;
}

static void ts_id_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  etx_obj_add_style(obj, styles->text_align_center, LV_PART_MAIN);
  etx_font(obj, FONT_XXS_INDEX);
}

static const lv_obj_class_t ts_id_class = {
    .base_class = &lv_label_class,
    .constructor_cb = ts_id_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = TSStyle::NUM_W,
    .height_def = TSStyle::ID_H,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_INHERIT,
    .instance_size = sizeof(lv_label_t),
};

lv_obj_t* TSStyle::newId(lv_obj_t* parent, const char* id)
{
  auto obj = etx_create(&ts_id_class, parent);
  lv_label_set_text(obj, id);

  return obj;
}

static void ts_name_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  etx_obj_add_style(obj, styles->text_align_left, LV_PART_MAIN);
}

static const lv_obj_class_t ts_name_class = {
    .base_class = &lv_label_class,
    .constructor_cb = ts_name_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = TSStyle::NAME_W,
    .height_def = TSStyle::NUM_H,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_INHERIT,
    .instance_size = sizeof(lv_label_t),
};

lv_obj_t* TSStyle::newName(lv_obj_t* parent, const char* name)
{
  auto obj = etx_create(&ts_name_class, parent);
  lv_label_set_text(obj, name);

  return obj;
}

static void ts_value_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  etx_obj_add_style(obj, styles->text_align_left, LV_PART_MAIN);
  etx_txt_color(obj, COLOR_THEME_WARNING_INDEX, ETX_STATE_VALUE_STALE_WARN);
}

static const lv_obj_class_t ts_value_class = {
    .base_class = &lv_label_class,
    .constructor_cb = ts_value_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = LV_SIZE_CONTENT,
    .height_def = TSStyle::NUM_H,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_INHERIT,
    .instance_size = sizeof(lv_label_t),
};

lv_obj_t* TSStyle::newValue(lv_obj_t* parent)
{
  auto obj = etx_create(&ts_value_class, parent);
  lv_label_set_text(obj, "");

  return obj;
}

static void ts_fresh_icon_constructor(const lv_obj_class_t* class_p,
                                      lv_obj_t* obj)
{
  static uint8_t const freshBitmap[] = {
      0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00,
  };

  etx_obj_add_style(obj, tsStyle.tsFreshStyle, LV_PART_MAIN);
  lv_canvas_set_buffer(obj, (void*)freshBitmap, PAD_LARGE, PAD_LARGE, LV_IMG_CF_ALPHA_8BIT);
  lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
}

static const lv_obj_class_t ts_fresh_icon_class = {
    .base_class = &lv_canvas_class,
    .constructor_cb = ts_fresh_icon_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = PAD_LARGE,
    .height_def = PAD_LARGE,
    .editable = LV_OBJ_CLASS_EDITABLE_FALSE,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_INHERIT,
    .instance_size = sizeof(lv_canvas_t),
};

class SensorButton : public ListLineButton
{
 public:
  SensorButton(Window* parent, const rect_t& rect, uint8_t index) :
      ListLineButton(parent, index)
  {
    padAll(PAD_ZERO);
    setHeight(EdgeTxStyles::UI_ELEMENT_HEIGHT);

    check(isActive());

    lv_obj_add_event_cb(lvobj, SensorButton::on_draw, LV_EVENT_DRAW_MAIN_BEGIN,
                        nullptr);
  }

 protected:
  bool init = false;
  bool showId = false;
  lv_obj_t* numLabel = nullptr;
  lv_obj_t* idLabel = nullptr;
  lv_obj_t* valLabel = nullptr;
  lv_obj_t* fresh = nullptr;
  uint32_t lastRefresh = 0;
  std::string valString;

  static void on_draw(lv_event_t* e)
  {
    lv_obj_t* target = lv_event_get_target(e);
    auto line = (SensorButton*)lv_obj_get_user_data(target);
    if (line) {
      if (!line->init)
        line->delayed_init();
    }
  }

  bool isActive() const override { return telemetryItems[index].isAvailable(); }

  void setNumIdState()
  {
    if (idLabel) {
      showId = g_model.showInstanceIds;
      if (showId) {
        lv_obj_clear_flag(idLabel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_state(numLabel, ETX_STATE_VALUE_SMALL_FONT);
      } else {
        lv_obj_add_flag(idLabel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_state(numLabel, ETX_STATE_VALUE_SMALL_FONT);
      }
    }
  }

  void checkEvents() override
  {
    ListLineButton::checkEvents();
    refresh();
  }

  void delayed_init()
  {
    char s[20];

    init = true;

    lv_obj_enable_style_refresh(false);

    numLabel = tsStyle.newNum(lvobj, index);
    lv_obj_set_pos(numLabel, PAD_TINY, PAD_MEDIUM/2);

    TelemetrySensor* sensor = &g_model.telemetrySensors[index];
    if (sensor->type == TELEM_TYPE_CUSTOM) {
      sprintf(s, "ID: %d", sensor->instance);

      idLabel = tsStyle.newId(lvobj, s);
      lv_obj_set_pos(idLabel, PAD_TINY, TSStyle::ID_Y);
    }

    setNumIdState();

    strAppend(s, g_model.telemetrySensors[index].label, TELEM_LABEL_LEN);
    lv_obj_t* nm = tsStyle.newName(lvobj, s);
    lv_obj_set_pos(nm, TSStyle::NUM_W + PAD_SMALL, PAD_MEDIUM/2);

    fresh = etx_create(&ts_fresh_icon_class, lvobj);
    lv_obj_set_pos(fresh, TSStyle::NUM_W + TSStyle::NAME_W + PAD_MEDIUM, TSStyle::FRSH_Y);

    valLabel = tsStyle.newValue(lvobj);
    lv_obj_set_pos(valLabel, TSStyle::NUM_W + TSStyle::NAME_W + PAD_LARGE * 2, PAD_MEDIUM/2);

    lv_obj_update_layout(lvobj);
  
    lv_obj_enable_style_refresh(true);
    lv_obj_refresh_style(lvobj, LV_PART_ANY, LV_STYLE_PROP_ANY);
  }

  void refresh() override
  {
    if (!init) return;

    if (showId != g_model.showInstanceIds) setNumIdState();

    // Draw a 'fresh' marker
    if (telemetryItems[index].isFresh())
      lv_obj_clear_flag(fresh, LV_OBJ_FLAG_HIDDEN);
    else
      lv_obj_add_flag(fresh, LV_OBJ_FLAG_HIDDEN);

    uint32_t now = RTOS_GET_MS();
    TelemetryItem& telemetryItem = telemetryItems[index];

    // Update value
    if ((now - lastRefresh >= 200) || telemetryItem.isFresh()) {
      // update at least every 200ms
      lastRefresh = now;

      std::string s;
      bool isOld = false;

      if (telemetryItem.isAvailable()) {
        isOld = telemetryItem.isOld();
        s = getSensorCustomValue(
            index, getValue(MIXSRC_FIRST_TELEM + 3 * index), LEFT);
      } else {
        s = "---";
      }

      if (isOld)
        lv_obj_add_state(valLabel, ETX_STATE_VALUE_STALE_WARN);
      else
        lv_obj_clear_state(valLabel, ETX_STATE_VALUE_STALE_WARN);

      if (valString != s) {
        valString = s;
        lv_label_set_text(valLabel, s.c_str());
      }
    }
  }
};

class SensorSourceChoice : public SourceChoice
{
 public:
  SensorSourceChoice(Window* window, const rect_t& rect, uint8_t* source,
                     IsValueAvailable isValueAvailable) :
      SourceChoice(window, rect, MIXSRC_NONE, MIXSRC_LAST_TELEM,
                   GET_DEFAULT(*source ? MIXSRC_FIRST_TELEM + 3 * (*source - 1)
                                       : MIXSRC_NONE),
                   [=](uint16_t newValue) {
                     *source = newValue == MIXSRC_NONE
                                   ? 0
                                   : (newValue - MIXSRC_FIRST_TELEM) / 3 + 1;
                   })
  {
    setAvailableHandler([=](int16_t value) {
      if (value == MIXSRC_NONE) return true;
      if (value < MIXSRC_FIRST_TELEM) return false;
      auto qr = div(value - MIXSRC_FIRST_TELEM, 3);
      return qr.rem == 0 && isValueAvailable(qr.quot + 1);
    });
  }
};

class SensorEditWindow : public SubPage
{
 public:
  explicit SensorEditWindow(uint8_t index) :
      SubPage(ICON_MODEL_TELEMETRY, STR_MENUTELEMETRY, "", true), index(index)
  {
    buildHeader(header);
    buildBody(body);
    enableRefresh();
  }

 protected:
  uint8_t index;
  uint32_t lastRefresh = 0;
  StaticText* headerValue = nullptr;

  enum ParamTypes {
    P_FORMULA = 0,
    P_ID,
    P_UNIT,
    P_PREC,
    P_CELLSENSOR,
    P_GPSSENSOR,
    P_CURRENTSENSOR,
    P_CONSUMPTIONSOURCE,
    P_CALC0,
    P_BLADES,
    P_RATIO,
    P_CELLINDEX,
    P_ALTSENSOR,
    P_CALC1,
    P_MULT,
    P_OFFSET,
    P_CALC2,
    P_CALC3,
    P_AUTOOFFSET,
    P_ONLYPOS,
    P_FILTER,
    P_PERSISTENT,
    P_COUNT,
  };

  Window* paramLines[P_COUNT] = {};

  void buildHeader(Window* window)
  {
    std::string title2 =
        STR_SENSOR + std::to_string(index + 1) + " = " + STR_NA;

    headerValue = header->setTitle2(title2);

    etx_txt_color(headerValue->getLvObj(), COLOR_THEME_WARNING_INDEX,
                  ETX_STATE_VALUE_STALE_WARN);
  }

  void checkEvents() override
  {
    uint32_t now = RTOS_GET_MS();
    TelemetryItem& telemetryItem = telemetryItems[index];

    if ((now - lastRefresh >= 200) || telemetryItem.isFresh()) {
      // update at least every 200ms
      lastRefresh = now;

      lv_obj_clear_state(headerValue->getLvObj(), ETX_STATE_VALUE_STALE_WARN);

      if (telemetryItem.isAvailable()) {
        if (telemetryItem.isOld())
          lv_obj_add_state(headerValue->getLvObj(), ETX_STATE_VALUE_STALE_WARN);
        std::string title2 =
            STR_SENSOR + std::to_string(index + 1) + " = " +
            getSensorCustomValue(
                index, getValue(MIXSRC_FIRST_TELEM + 3 * index), LEFT);
        headerValue->setText(title2);
      } else {
        headerValue->setText(STR_SENSOR + std::to_string(index + 1) + " = " +
                             STR_NA);
      }
    }
  }

  void updateSensorParameters()
  {
    TelemetrySensor* sensor = &g_model.telemetrySensors[index];

    for (int i = P_FORMULA; i < P_COUNT; i += 1) {
      paramLines[i]->hide();
    }

    if (sensor->type == TELEM_TYPE_CALCULATED) {
      // Formula
      paramLines[P_FORMULA]->show();
    } else {
      paramLines[P_ID]->show();
    }

    // Unit
    if ((sensor->type == TELEM_TYPE_CALCULATED &&
         (sensor->formula == TELEM_FORMULA_DIST)) ||
        sensor->isConfigurable()) {
      paramLines[P_UNIT]->show();
    }

    // Precision
    if (sensor->isPrecConfigurable()) {
      paramLines[P_PREC]->show();
    }

    // Params
    if (sensor->unit < UNIT_FIRST_VIRTUAL) {
      if (sensor->type == TELEM_TYPE_CALCULATED) {
        if (sensor->formula == TELEM_FORMULA_CELL) {
          paramLines[P_CELLSENSOR]->show();
        } else if (sensor->formula == TELEM_FORMULA_DIST) {
          paramLines[P_GPSSENSOR]->show();
        } else if (sensor->formula == TELEM_FORMULA_CONSUMPTION) {
          paramLines[P_CURRENTSENSOR]->show();
        } else if (sensor->formula == TELEM_FORMULA_TOTALIZE) {
          paramLines[P_CONSUMPTIONSOURCE]->show();
        } else {
          paramLines[P_CALC0]->show();
        }
      } else {
        if (sensor->unit == UNIT_RPMS) {
          paramLines[P_BLADES]->show();
        } else {
          paramLines[P_RATIO]->show();
        }
      }
    }

    if (!(sensor->unit == UNIT_GPS || sensor->unit == UNIT_DATETIME ||
          sensor->unit == UNIT_CELLS ||
          (sensor->type == TELEM_TYPE_CALCULATED &&
           (sensor->formula == TELEM_FORMULA_CONSUMPTION ||
            sensor->formula == TELEM_FORMULA_TOTALIZE)))) {
      if (sensor->type == TELEM_TYPE_CALCULATED) {
        if (sensor->formula == TELEM_FORMULA_CELL) {
          paramLines[P_CELLINDEX]->show();
        } else if (sensor->formula == TELEM_FORMULA_DIST) {
          paramLines[P_ALTSENSOR]->show();
        } else {
          paramLines[P_CALC1]->show();
        }
      } else if (sensor->unit == UNIT_RPMS) {
        paramLines[P_MULT]->show();
      } else {
        paramLines[P_OFFSET]->show();
      }
    }

    if ((sensor->type == TELEM_TYPE_CALCULATED &&
         sensor->formula < TELEM_FORMULA_MULTIPLY)) {
      paramLines[P_CALC2]->show();
      paramLines[P_CALC3]->show();
    }

    // Auto Offset
    if (sensor->unit != UNIT_RPMS && sensor->isConfigurable()) {
      paramLines[P_AUTOOFFSET]->show();
    }

    if (sensor->isConfigurable()) {
      // Only positive
      paramLines[P_ONLYPOS]->show();
      // Filter
      paramLines[P_FILTER]->show();
    }

    if (sensor->type == TELEM_TYPE_CALCULATED) {
      paramLines[P_PERSISTENT]->show();
    }
  }

  void buildBody(Window* window)
  {
    window->setFlexLayout();

    TelemetrySensor* sensor = &g_model.telemetrySensors[index];

    // Sensor name
    setupLine(STR_NAME, [=](Window* parent, coord_t x, coord_t y) {
          new ModelTextEdit(parent, {x, y, 0, 0}, sensor->label, sizeof(sensor->label));
        });

    // Type
    setupLine(STR_TYPE, [=](Window* parent, coord_t x, coord_t y) {
          new Choice(parent, {x, y, 0, 0}, STR_VSENSORTYPES, 0, 1,
                    GET_DEFAULT(sensor->type), [=](uint8_t newValue) {
                      sensor->type = newValue;
                      sensor->instance = 0;
                      if (sensor->type == TELEM_TYPE_CALCULATED) {
                        sensor->param = 0;
                        sensor->filter = 0;
                        sensor->autoOffset = 0;
                      }
                      SET_DIRTY();
                      updateSensorParameters();
                    });
        });

    // Parameters
    paramLines[P_FORMULA] = setupLine(STR_FORMULA, [=](Window* parent, coord_t x, coord_t y) {
          new Choice(parent, {x, y, 0, 0}, STR_VFORMULAS, 0,
                    TELEM_FORMULA_LAST, GET_DEFAULT(sensor->formula),
                    [=](uint8_t newValue) {
                      sensor->formula = newValue;
                      sensor->param = 0;
                      if (sensor->formula == TELEM_FORMULA_CELL) {
                        sensor->unit = UNIT_VOLTS;
                        sensor->prec = 2;
                      } else if (sensor->formula == TELEM_FORMULA_DIST) {
                        sensor->unit = UNIT_DIST;
                        sensor->prec = 0;
                      } else if (sensor->formula == TELEM_FORMULA_CONSUMPTION) {
                        sensor->unit = UNIT_MAH;
                        sensor->prec = 0;
                      }
                      SET_DIRTY();
                      telemetryItems[index].clear();
                      updateSensorParameters();
                    });
        });

    paramLines[P_ID] = setupLine(STR_ID, [=](Window* parent, coord_t x, coord_t y) {
          auto num = new NumberEdit(parent, {x, y, NUM_EDIT_W, 0}, 0, 0xFFFF,
                                    GET_SET_DEFAULT(sensor->id));
          num->setDisplayHandler([](int32_t value) {
            char buf[4];
            buf[0] = hex2char((value & 0xf000) >> 12);
            buf[1] = hex2char((value & 0x0f00) >> 8);
            buf[2] = hex2char((value & 0x00f0) >> 4);
            buf[3] = hex2char((value & 0x000f) >> 0);
            return std::string(buf, sizeof(buf));
          });
          num = new NumberEdit(paramLines[P_ID], {x, y, NUM_EDIT_W, 0}, 0, 0xff,
                              GET_SET_DEFAULT(sensor->instance));
        });

    paramLines[P_UNIT] = setupLine(STR_UNIT, [=](Window* parent, coord_t x, coord_t y) {
          new Choice(parent, {x, y, 0, 0}, STR_VTELEMUNIT, 0, UNIT_MAX,
                    GET_DEFAULT(sensor->unit), [=](uint8_t newValue) {
                      sensor->unit = newValue;
                      if (sensor->unit == UNIT_FAHRENHEIT) {
                        sensor->prec = 0;
                      }
                      SET_DIRTY();
                      telemetryItems[index].clear();
                      updateSensorParameters();
                    });
        });

    paramLines[P_PREC] = setupLine(STR_PRECISION, [=](Window* parent, coord_t x, coord_t y) {
          new Choice(parent, {x, y, 0, 0}, STR_VPREC, 0, 2,
                    GET_DEFAULT(sensor->prec), [=](uint8_t newValue) {
                      sensor->prec = newValue;
                      SET_DIRTY();
                      telemetryItems[index].clear();
                      updateSensorParameters();
                    });
        });

    paramLines[P_CELLSENSOR] = setupLine(STR_CELLSENSOR, [=](Window* parent, coord_t x, coord_t y) {
          new SensorSourceChoice(parent, {x, y, 0, 0},
                                &sensor->cell.source, isCellsSensor);
        });

    paramLines[P_GPSSENSOR] = setupLine(STR_GPSSENSOR, [=](Window* parent, coord_t x, coord_t y) {
          new SensorSourceChoice(parent, {x, y, 0, 0}, &sensor->dist.gps,
                                isGPSSensor);
        });

    paramLines[P_CURRENTSENSOR] = setupLine(STR_CURRENTSENSOR, [=](Window* parent, coord_t x, coord_t y) {
          new SensorSourceChoice(parent, {x, y, 0, 0},
                                &sensor->consumption.source, isSensorAvailable);
        });

    paramLines[P_CONSUMPTIONSOURCE] = setupLine(STR_SOURCE, [=](Window* parent, coord_t x, coord_t y) {
          new SensorSourceChoice(parent, {x, y, 0, 0},
                                &sensor->consumption.source, isSensorAvailable);
        });

    std::string s(STR_SOURCE);

    paramLines[P_CALC0] = setupLine((s + std::to_string(1)).c_str(), [=](Window* parent, coord_t x, coord_t y) {
          new SensorSourceChoice(parent, {x, y, 0, 0},
                                (uint8_t*)&sensor->calc.sources[0],
                                isSensorAvailable);
        });

    paramLines[P_BLADES] = setupLine(STR_BLADES, [=](Window* parent, coord_t x, coord_t y) {
          new NumberEdit(parent, {x, y, NUM_EDIT_W, 0}, 1, 30000,
                        GET_SET_DEFAULT(sensor->custom.ratio));
        });

    paramLines[P_RATIO] = setupLine(STR_RATIO, [=](Window* parent, coord_t x, coord_t y) {
          auto pct = new StaticText(parent, {x + NUM_EDIT_W + PAD_MEDIUM, y + PAD_MEDIUM, 0, 0}, "");
          auto num = new NumberEdit(
              parent, {x, y, NUM_EDIT_W, 0}, 0, 30000,
              GET_DEFAULT(sensor->custom.ratio),
              [=](int32_t value) {
                sensor->custom.ratio = value;
                std::string s("");
                if (sensor->custom.ratio != 0)
                  s = formatNumberAsString((sensor->custom.ratio * 1000) / 255, PREC1, 0, "", "%");
                pct->setText(s);
              },
              PREC1);
          num->setZeroText("-");
          std::string s("");
          if (sensor->custom.ratio != 0)
            s = formatNumberAsString((sensor->custom.ratio * 1000) / 255, PREC1, 0, "", "%");
          pct->setText(s);
        });

    paramLines[P_CELLINDEX] = setupLine(STR_CELLINDEX, [=](Window* parent, coord_t x, coord_t y) {
          new Choice(parent, {x, y, 0, 0}, STR_VCELLINDEX,
                    TELEM_CELL_INDEX_LOWEST, TELEM_CELL_INDEX_LAST,
                    GET_SET_DEFAULT(sensor->cell.index));
        });

    paramLines[P_ALTSENSOR] = setupLine(STR_ALTSENSOR, [=](Window* parent, coord_t x, coord_t y) {
          new SensorSourceChoice(parent, {x, y, 0, 0}, &sensor->dist.alt,
                                isAltSensor);
        });

    paramLines[P_CALC1] = setupLine((s + std::to_string(2)).c_str(), [=](Window* parent, coord_t x, coord_t y) {
          new SensorSourceChoice(parent, {x, y, 0, 0},
                                (uint8_t*)&sensor->calc.sources[1],
                                isSensorAvailable);
        });

    paramLines[P_MULT] = setupLine(STR_MULTIPLIER, [=](Window* parent, coord_t x, coord_t y) {
          new NumberEdit(parent, {x, y, NUM_EDIT_W, 0}, 1, 30000,
                        GET_SET_DEFAULT(sensor->custom.offset));
        });

    paramLines[P_OFFSET] = setupLine(STR_OFFSET, [=](Window* parent, coord_t x, coord_t y) {
          new NumberEdit(
              parent, {x, y, NUM_EDIT_W, 0}, -30000, 30000,
              GET_SET_DEFAULT(sensor->custom.offset),
              (sensor->prec > 0) ? (sensor->prec == 2 ? PREC2 : PREC1) : 0);
        });

    paramLines[P_CALC2] = setupLine((s + std::to_string(3)).c_str(), [=](Window* parent, coord_t x, coord_t y) {
          new SensorSourceChoice(parent, {x, y, 0, 0},
                                (uint8_t*)&sensor->calc.sources[2],
                                isSensorAvailable);
        });

    paramLines[P_CALC3] = setupLine((s + std::to_string(4)).c_str(), [=](Window* parent, coord_t x, coord_t y) {
          new SensorSourceChoice(parent, {x, y, 0, 0},
                                (uint8_t*)&sensor->calc.sources[3],
                                isSensorAvailable);
        });

    paramLines[P_AUTOOFFSET] = setupLine(STR_AUTOOFFSET, [=](Window* parent, coord_t x, coord_t y) {
          new ToggleSwitch(parent, {x, y, 0, 0},
                          GET_SET_DEFAULT(sensor->autoOffset));
        });

    paramLines[P_ONLYPOS] = setupLine(STR_ONLYPOSITIVE, [=](Window* parent, coord_t x, coord_t y) {
          new ToggleSwitch(parent, {x, y, 0, 0},
                          GET_SET_DEFAULT(sensor->onlyPositive));
        });

    paramLines[P_FILTER] = setupLine(STR_FILTER, [=](Window* parent, coord_t x, coord_t y) {
          new ToggleSwitch(parent, {x, y, 0, 0},
                          GET_SET_DEFAULT(sensor->filter));
        });

    paramLines[P_PERSISTENT] = setupLine(STR_PERSISTENT, [=](Window* parent, coord_t x, coord_t y) {
          new ToggleSwitch(parent, {x, y, 0, 0},
                          GET_DEFAULT(sensor->persistent), [=](int32_t newValue) {
                            sensor->persistent = newValue;
                            if (!sensor->persistent) sensor->persistentValue = 0;
                            SET_DIRTY();
                          });
        });

    // Logs
    setupLine(STR_LOGS, [=](Window* parent, coord_t x, coord_t y) {
          new ToggleSwitch(parent, {x, y, 0, 0}, GET_DEFAULT(sensor->logs),
                          [=](int32_t newValue) {
                            sensor->logs = newValue;
                            logsClose();
                            SET_DIRTY();
                          });
        });

    updateSensorParameters();
  }

  static LAYOUT_VAL(NUM_EDIT_W, 100, 80)
};

ModelTelemetryPage::ModelTelemetryPage() :
    PageTab(STR_MENUTELEMETRY, ICON_MODEL_TELEMETRY)
{
  tsStyle.init();
}

void ModelTelemetryPage::checkEvents()
{
  int _lastKnownIndex = availableTelemetryIndex();

  if (lastKnownIndex >= 0 && lastKnownIndex != _lastKnownIndex) {
    rebuild(window);
    lastKnownIndex = _lastKnownIndex;
  }

  PageTab::checkEvents();
}

void ModelTelemetryPage::rebuild(Window* window, int8_t focusSensorIndex)
{
  buildSensorList(focusSensorIndex);
  lastKnownIndex = availableTelemetryIndex();
}

void ModelTelemetryPage::editSensor(Window* window, uint8_t index)
{
  lastKnownIndex = -1;
  Window* editWindow = new SensorEditWindow(index);
  editWindow->setCloseHandler([=]() { rebuild(window, index); });
}

void ModelTelemetryPage::buildSensorList(int8_t focusSensorIndex)
{
  bool didFocus = false;
  SensorButton* first = nullptr;

  sensorWindow->clear();

  for (uint8_t idx = 0; idx < MAX_TELEMETRY_SENSORS; idx++) {
    if (g_model.telemetrySensors[idx].isAvailable()) {
      auto button = new SensorButton(sensorWindow, rect_t{}, idx);

      if (!first) first = button;

      button->setPressHandler([=]() -> uint8_t {
        Menu* menu = new Menu();
        menu->addLine(STR_EDIT, [=]() { editSensor(window, idx); });
        menu->addLine(STR_COPY, [=]() {
          auto newIndex = availableTelemetryIndex();
          if (newIndex >= 0) {
            TelemetrySensor& sourceSensor = g_model.telemetrySensors[idx];
            TelemetrySensor& newSensor = g_model.telemetrySensors[newIndex];
            newSensor = sourceSensor;
            TelemetryItem& sourceItem = telemetryItems[idx];
            TelemetryItem& newItem = telemetryItems[newIndex];
            newItem = sourceItem;
            SET_DIRTY();
            rebuild(window, newIndex);
          } else {
            new FullScreenDialog(WARNING_TYPE_ALERT, "", STR_TELEMETRYFULL);
          }
        });
        menu->addLine(STR_DELETE, [=]() {
          delTelemetryIndex(idx);  // calls setDirty internally
          for (uint8_t i = idx + 1; i < MAX_TELEMETRY_SENSORS; i += 1) {
            if (g_model.telemetrySensors[i].isAvailable()) {
              rebuild(window, i);
              return;
            }
          }
          for (int8_t i = idx - 1; i >= 0; i -= 1) {
            if (g_model.telemetrySensors[i].isAvailable()) {
              rebuild(window, i);
              return;
            }
          }
          rebuild(window, -1);
        });
        return 0;
      });
      if (focusSensorIndex == idx) {
        lv_group_focus_obj(button->getLvObj());
        didFocus = true;
      }
    }
  }

  if (!didFocus) {
    if (first && !allowNewSensors)
      lv_group_focus_obj(first->getLvObj());
    else
      lv_group_focus_obj(discover->getLvObj());
  }

  uint8_t sensorsCount = getTelemetrySensorsCount();
  deleteAll->show(sensorsCount > 0);
}

void ModelTelemetryPage::build(Window* window)
{
  window->padAll(PAD_TINY);
  window->padBottom(PAD_LARGE);
  window->setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_ZERO);

  this->window = window;

  // Sensors
  new Subtitle(window, STR_TELEMETRY_SENSORS);

  sensorWindow = new Window(window, rect_t{});
  sensorWindow->padAll(PAD_TINY);
  sensorWindow->setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_TINY);

  FlexGridLayout grid4(col_dsc4, row_dsc);

  // Autodiscover button
  auto line = window->newLine(grid4);
  discover = new TextButton(
      line, rect_t{},
      (allowNewSensors) ? STR_STOP_DISCOVER_SENSORS : STR_DISCOVER_SENSORS);
  discover->setPressHandler([=]() {
    allowNewSensors = !allowNewSensors;
    if (allowNewSensors) {
      discover->setText(STR_STOP_DISCOVER_SENSORS);
      return 1;
    } else {
      discover->setText(STR_DISCOVER_SENSORS);
      return 0;
    }
  });
  lv_obj_set_grid_cell(discover->getLvObj(), LV_GRID_ALIGN_STRETCH, 0, 1,
                       LV_GRID_ALIGN_CENTER, 0, 1);
  discover->check(allowNewSensors);

  // New sensor button
  auto b =
      new TextButton(line, rect_t{}, STR_TELEMETRY_NEWSENSOR, [=]() -> uint8_t {
        int idx = availableTelemetryIndex();
        if (idx >= 0)
          editSensor(window, idx);
        else
          new FullScreenDialog(WARNING_TYPE_ALERT, "", STR_TELEMETRYFULL);
        return 0;
      });
  lv_obj_set_grid_cell(b->getLvObj(), LV_GRID_ALIGN_STRETCH, 1, 1,
                       LV_GRID_ALIGN_CENTER, 0, 1);

#if TWOCOLBUTTONS
  line = window->newLine(grid4);
#endif
  // Delete all sensors button
  deleteAll =
      new TextButton(line, rect_t{}, STR_DELETE_ALL_SENSORS, [=]() -> uint8_t {
        new ConfirmDialog(STR_DELETE_ALL_SENSORS, STR_CONFIRMDELETE,
                          [=]() {
                            for (int i = 0; i < MAX_TELEMETRY_SENSORS; i++) {
                              delTelemetryIndex(i);
                            }
                          });
        return 0;
      });
#if TWOCOLBUTTONS
  deleteAll->setWidth((LCD_W - 16) / 2);
  lv_obj_set_grid_cell(deleteAll->getLvObj(), LV_GRID_ALIGN_CENTER, 0, 2,
                       LV_GRID_ALIGN_CENTER, 0, 1);
#else
  lv_obj_set_grid_cell(deleteAll->getLvObj(), LV_GRID_ALIGN_STRETCH, 2, 1,
                       LV_GRID_ALIGN_CENTER, 0, 1);
#endif

  FlexGridLayout grid(col_dsc, row_dsc, PAD_TINY);

  // Show instance IDs button
  line = window->newLine(grid);
  line->padLeft(PAD_LARGE);
  new StaticText(line, rect_t{}, STR_SHOW_INSTANCE_ID);
  new ToggleSwitch(line, rect_t{}, GET_SET_DEFAULT(g_model.showInstanceIds));

  // Ignore instance button
  line = window->newLine(grid);
  line->padLeft(PAD_LARGE);
  new StaticText(line, rect_t{}, STR_IGNORE_INSTANCE);
  new ToggleSwitch(line, rect_t{}, GET_SET_DEFAULT(g_model.ignoreSensorIds));

  // RX stat
  new Subtitle(window, getRxStatLabels()->label);

  line = window->newLine(grid);
  line->padLeft(PAD_LARGE);
  new StaticText(line, rect_t{}, STR_LOWALARM);
  new NumberEdit(line, {0, 0, NUM_EDIT_W, 0}, 0, 100,
                 GET_SET_DEFAULT(g_model.rfAlarms.warning));

  line = window->newLine(grid);
  line->padLeft(PAD_LARGE);
  new StaticText(line, rect_t{}, STR_CRITICALALARM);
  new NumberEdit(line, {0, 0, NUM_EDIT_W, 0}, 0, 100,
                 GET_SET_DEFAULT(g_model.rfAlarms.critical));

  line = window->newLine(grid);
  line->padLeft(PAD_LARGE);
  new StaticText(line, rect_t{}, STR_DISABLE_ALARM);
  new ToggleSwitch(line, rect_t{},
                   GET_SET_DEFAULT(g_model.disableTelemetryWarning));

  // Vario
  new Subtitle(window, STR_VARIO);

  FlexGridLayout grid5(col_dsc5, row_dsc);

  line = window->newLine(grid5);
  line->padLeft(PAD_LARGE);
  new StaticText(line, rect_t{}, STR_SOURCE);
  auto choice = new SourceChoice(
      line, rect_t{}, MIXSRC_NONE, MIXSRC_LAST_TELEM,
      GET_DEFAULT(g_model.varioData.source
                      ? MIXSRC_FIRST_TELEM + 3 * (g_model.varioData.source - 1)
                      : MIXSRC_NONE),
      SET_VALUE(g_model.varioData.source,
                newValue == MIXSRC_NONE
                    ? 0
                    : (newValue - MIXSRC_FIRST_TELEM) / 3 + 1));
  choice->setAvailableHandler([=](int16_t value) {
    if (value == MIXSRC_NONE) return true;
    if (value < MIXSRC_FIRST_TELEM) return false;
    auto qr = div(value - MIXSRC_FIRST_TELEM, 3);
    return qr.rem == 0 && isVarioSensorAvailable(qr.quot + 1);
  });

  line = window->newLine(grid5);
  line->padLeft(PAD_LARGE);
  new StaticText(line, rect_t{}, STR_RANGE);

  auto vMin = new NumberEdit(line, {0, 0, NUM_EDIT_W, 0}, -17, 17,
                             GET_SET_WITH_OFFSET(g_model.varioData.min, -10));
  vMin->setAvailableHandler(
      [](int val) { return val < g_model.varioData.max + 10; });

  auto vMax = new NumberEdit(line, {0, 0, NUM_EDIT_W, 0}, -17, 17,
                             GET_SET_WITH_OFFSET(g_model.varioData.max, 10));
  vMax->setAvailableHandler(
      [](int val) { return g_model.varioData.min - 10 < val; });

  line = window->newLine(grid5);
  line->padLeft(PAD_LARGE);
  new StaticText(line, rect_t{}, STR_CENTER);

  auto cMin = new NumberEdit(
      line, rect_t{0, 0, NUM_EDIT_W, 0}, -15, 15,
      GET_SET_WITH_OFFSET(g_model.varioData.centerMin, -5), PREC1);
  cMin->setAvailableHandler(
      [](int val) { return val < g_model.varioData.centerMax + 5; });

  auto cMax = new NumberEdit(
      line, rect_t{0, 0, NUM_EDIT_W, 0}, -15, 15,
      GET_SET_WITH_OFFSET(g_model.varioData.centerMax, 5), PREC1);
  cMax->setAvailableHandler(
      [](int val) { return g_model.varioData.centerMin - 5 < val; });

  new Choice(line, rect_t{}, STR_VVARIOCENTER, 0, 1,
             GET_SET_DEFAULT(g_model.varioData.centerSilent));

  // Don't call this before the 'discover' button has been created
  buildSensorList(-1);
}

// Version of 'drawSensorCustomValue' and related functions that returns strings
// instead of drawing to screen
// TODO: should this be moved somewhere else?

std::string getSensorCustomValue(uint8_t sensor, int32_t value, LcdFlags flags)
{
  TelemetryItem& telemetryItem = telemetryItems[sensor];
  TelemetrySensor& telemetrySensor = g_model.telemetrySensors[sensor];

  if (telemetrySensor.unit == UNIT_DATETIME) {
    return getTelemDate(telemetryItem) + " " + getTelemTime(telemetryItem);
  } else if (telemetrySensor.unit == UNIT_GPS) {
    return getGPSSensorValue(telemetryItem, flags);
  } else if (telemetrySensor.unit == UNIT_TEXT) {
    return std::string(telemetryItem.text);
  } else {
    if (telemetrySensor.prec > 0) {
      flags |= (telemetrySensor.prec == 1 ? PREC1 : PREC2);
    }
    return getValueWithUnit(
        value,
        telemetrySensor.unit == UNIT_CELLS ? UNIT_VOLTS : telemetrySensor.unit,
        flags);
  }

  return std::string("");
}
