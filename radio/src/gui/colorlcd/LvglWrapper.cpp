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

#include "LvglWrapper.h"
#include "view_main.h"

LvglWrapper* LvglWrapper::_instance = nullptr;

static lv_indev_drv_t touchDriver;
static lv_indev_drv_t keyboard_drv;
static lv_indev_drv_t rotaryDriver;
static lv_indev_t* rotaryDevice = nullptr;

lv_group_t* inputGroup;

#if defined(HARDWARE_TOUCH)
static bool touchOccured = false;
bool getTouchOccured()
{
  return touchOccured;
}

static TouchState lastState;
TouchState getLastTochState()
{
  touchOccured = false;
  return lastState;
}
#endif

extern "C" void keyboardDriverRead(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
  // if there is a keyboard event then call checkevents
  if (isEvent()) {
    event_t evt = getWindowEvent();
    auto focusWindow = Window::getFocus();
    if (focusWindow) {
      focusWindow->onEvent(evt);
    }
  }
}

static void copy_ts_to_indev_data(const TouchState &st, lv_indev_data_t *data)
{
#if defined(LCD_VERTICAL_INVERT)
  data->point.x = LCD_W - st.x;
  data->point.y = LCD_H - st.y;
#else
  data->point.x = st.x;
  data->point.y = st.y;
#endif
}

static lv_indev_data_t indev_data_backup;

static void backup_indev_data(lv_indev_data_t* data)
{
  memcpy(&indev_data_backup, data, sizeof(lv_indev_data_t));
}

static void copy_indev_data_backup(lv_indev_data_t* data)
{
  memcpy(data, &indev_data_backup, sizeof(lv_indev_data_t));
}

extern "C" void touchDriverRead(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
#if defined(HARDWARE_TOUCH)
  if(!touchPanelEventOccured()) {
    copy_indev_data_backup(data);
    return;
  }
  touchOccured = true;

  TouchState st = touchPanelRead();
  lastState = st; // hack for now

  if(st.event == TE_NONE) {
    TRACE("TE_NONE");
  } else if(st.event == TE_DOWN || st.event == TE_SLIDE) {
    TRACE("INDEV_STATE_PRESSED");
    data->state = LV_INDEV_STATE_PRESSED;
    copy_ts_to_indev_data(st, data);
  } else {
    TRACE("INDEV_STATE_RELEASED");
    data->state = LV_INDEV_STATE_RELEASED;
    copy_ts_to_indev_data(st, data);
  }

  backup_indev_data(data);
#endif
}

event_t encoderEvent = 0;
bool newEncoderEvent = false;
lv_indev_state_t encoderState = LV_INDEV_STATE_RELEASED;

void lvglPushEncoderEvent(event_t& evt)
{
  if (evt != EVT_KEY_FIRST(KEY_ENTER) && evt != EVT_KEY_BREAK(KEY_ENTER) &&
      evt != EVT_ROTARY_LEFT && evt != EVT_ROTARY_RIGHT)
    return;

  encoderEvent = evt;
  evt = 0;
  newEncoderEvent = true;
}

extern "C" void rotaryDriverRead(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
  data->enc_diff = 0;
  if(newEncoderEvent)
  {
    switch(encoderEvent)
    {
    case EVT_KEY_FIRST(KEY_ENTER):
      encoderState = LV_INDEV_STATE_PRESSED;
      break;
    case EVT_KEY_BREAK(KEY_ENTER):
      encoderState = LV_INDEV_STATE_RELEASED;
      break;
    case EVT_KEY_FIRST(KEY_EXIT):
      encoderState = LV_INDEV_STATE_PRESSED;
      break;
    case EVT_KEY_BREAK(KEY_EXIT):
      encoderState = LV_INDEV_STATE_RELEASED;
      break;
    case EVT_ROTARY_LEFT:
      data->enc_diff = -1;
      break;
    case EVT_ROTARY_RIGHT:
      data->enc_diff = 1;
      break;
    }
  }
  newEncoderEvent = false;
  data->state = encoderState;
}

/**
 * Helper function to translate a colorFlags value to a lv_color_t suitable
 * for passing to an lv_obj function
 * @param colorFlags a textFlags value.  This value will contain the color shifted by 16 bits.
 */
lv_color_t makeLvColor(uint32_t colorFlags)
{
  auto color = COLOR_VAL(colorFlags);
  return lv_color_make(GET_RED(color), GET_GREEN(color), GET_BLUE(color));
}

static void init_lvgl_drivers()
{
  // Register the driver and save the created display object
  lcdInitDisplayDriver();
 
  // add all lvgl object automatically to a input handling group
  inputGroup = lv_group_create();
  lv_group_set_default(inputGroup);

  // Register the driver in LVGL and save the created input device object
  lv_indev_drv_init(&touchDriver);          /*Basic initialization*/
  touchDriver.type = LV_INDEV_TYPE_POINTER; /*See below.*/
  touchDriver.read_cb = touchDriverRead;      /*See below.*/
  lv_indev_drv_register(&touchDriver);

  lv_indev_drv_init(&keyboard_drv);
  keyboard_drv.type = LV_INDEV_TYPE_KEYPAD;
  keyboard_drv.read_cb = keyboardDriverRead;
  lv_indev_drv_register(&keyboard_drv);

  lv_indev_drv_init(&rotaryDriver);
  rotaryDriver.type = LV_INDEV_TYPE_ENCODER;
  rotaryDriver.read_cb = rotaryDriverRead;
  rotaryDevice = lv_indev_drv_register(&rotaryDriver);

  lv_indev_set_group(rotaryDevice, inputGroup);
}

// The theme code needs to go somewhere else (gui/colorlcd/themes/default.cpp?)
static lv_style_t generic_style;
static lv_style_t btn_style;
static lv_style_t btn_checked_style;
static lv_style_t btn_focused_style;
static lv_style_t menu_line_style;
static lv_style_t menu_checked_style;
static lv_style_t textedit_style_main;
static lv_style_t textedit_style_focused;
static lv_style_t textedit_style_cursor;
static lv_style_t textedit_style_cursor_edit;
static lv_style_t numberedit_style_main;
static lv_style_t numberedit_style_focused;
static lv_style_t numberedit_style_cursor;
static lv_style_t numberedit_style_cursor_edit;

static lv_style_t focus_key_style;

static void theme_apply_cb(lv_theme_t * th, lv_obj_t * obj)
{
  LV_UNUSED(th);

  if (lv_obj_check_type(obj, &lv_switch_class)) {
    return;
  }
  
  lv_obj_add_style(obj, &generic_style, LV_PART_MAIN);

  lv_obj_add_style(obj, &focus_key_style, LV_STATE_FOCUS_KEY);
  lv_obj_add_style(obj, &focus_key_style, LV_STATE_EDITED);

  lv_obj_t* parent = lv_obj_get_parent(obj);
  if (parent == NULL) {
    // main screen
    return;
  }

  if (lv_obj_check_type(obj, &lv_numberedit_class)) {
    lv_obj_add_style(obj, &numberedit_style_main, LV_PART_MAIN);
    lv_obj_add_style(obj, &numberedit_style_focused, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_add_style(obj, &numberedit_style_cursor, LV_PART_CURSOR);
    lv_obj_add_style(obj, &numberedit_style_cursor_edit, LV_PART_CURSOR | LV_STATE_EDITED);
  }

  if (lv_obj_check_type(obj, &lv_textarea_class)) {
    lv_obj_add_style(obj, &textedit_style_main, LV_PART_MAIN);
    lv_obj_add_style(obj, &textedit_style_focused, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_add_style(obj, &textedit_style_cursor, LV_PART_CURSOR);
    lv_obj_add_style(obj, &textedit_style_cursor_edit, LV_PART_CURSOR | LV_STATE_EDITED);
    return;
  }

  if (lv_obj_check_type(obj, &lv_btn_class)) {
    lv_obj_add_style(obj, &btn_style, 0);
    lv_obj_add_style(obj, &btn_checked_style, LV_STATE_CHECKED);
    lv_obj_add_style(obj, &btn_focused_style, LV_STATE_FOCUSED);
    return;
  }

  if (lv_obj_check_type(obj, &lv_list_btn_class)) {
    // menu items do not use the generic style
    lv_obj_remove_style(obj, &generic_style, LV_PART_MAIN);
    lv_obj_add_style(obj, &menu_line_style, 0);
    lv_obj_add_style(obj, &menu_checked_style, LV_STATE_CHECKED);
    return;
  }  
}

static void init_theme()
{
  /*Initialize the styles*/

  // generic focus outline style to overide default theme
  lv_style_init(&focus_key_style);
  lv_style_set_outline_width(&focus_key_style, 0);
  lv_style_set_outline_opa(&focus_key_style, LV_OPA_0);

  // Generic (applied to all)
  lv_style_init(&generic_style);
  lv_style_set_pad_all(&generic_style, 0);
  lv_style_set_bg_opa(&generic_style, LV_OPA_TRANSP);
  lv_style_set_border_width(&generic_style, 0);
  lv_style_set_radius(&generic_style, 0);

  // numberedit
  // LV_PART_MAIN
  lv_style_init(&numberedit_style_main);
  lv_style_init(&numberedit_style_focused);
  lv_style_init(&numberedit_style_cursor);
  lv_style_init(&numberedit_style_cursor_edit);

  lv_style_set_border_width(&numberedit_style_main, 1);
  lv_style_set_border_color(&numberedit_style_main, makeLvColor(COLOR_THEME_SECONDARY2));
  lv_style_set_bg_color(&numberedit_style_main, makeLvColor(COLOR_THEME_PRIMARY2));
  lv_style_set_bg_opa(&numberedit_style_main, LV_OPA_COVER);
  lv_style_set_text_color(&numberedit_style_main, makeLvColor(COLOR_THEME_SECONDARY1));
  lv_style_set_pad_left(&numberedit_style_main, FIELD_PADDING_LEFT);
  lv_style_set_pad_top(&numberedit_style_main, FIELD_PADDING_TOP);

  // LV_STATE_FOCUSED
  lv_style_set_bg_color(&numberedit_style_focused, makeLvColor(COLOR_THEME_FOCUS));
  lv_style_set_text_color(&numberedit_style_focused, makeLvColor(COLOR_THEME_PRIMARY2));

  // Hide cursor when not editing
  lv_style_set_opa(&numberedit_style_cursor, LV_OPA_0);

  // Show Cursor in "Edit" mode
  lv_style_set_opa(&numberedit_style_cursor_edit, LV_OPA_COVER);
  lv_style_set_bg_opa(&numberedit_style_cursor_edit, LV_OPA_50);

  // textedit
  lv_style_init(&textedit_style_main);
  lv_style_init(&textedit_style_focused);
  lv_style_init(&textedit_style_cursor);
  lv_style_init(&textedit_style_cursor_edit);
  
  // textedit style main
  lv_style_set_border_width(&textedit_style_main, 1);
  lv_style_set_border_color(&textedit_style_main, makeLvColor(COLOR_THEME_SECONDARY2));
  lv_style_set_bg_color(&textedit_style_main, makeLvColor(COLOR_THEME_PRIMARY2));
  lv_style_set_bg_opa(&textedit_style_main, LV_OPA_COVER);
  lv_style_set_text_color(&textedit_style_main, makeLvColor(COLOR_THEME_SECONDARY1));
  lv_style_set_pad_left(&textedit_style_main, FIELD_PADDING_LEFT);
  lv_style_set_pad_top(&textedit_style_main, FIELD_PADDING_TOP);

  //textedit style focused
  lv_style_set_bg_color(&textedit_style_focused, makeLvColor(COLOR_THEME_FOCUS));
  lv_style_set_text_color(&textedit_style_focused, makeLvColor(COLOR_THEME_PRIMARY2));

  // hide cursor when not editing
  lv_style_set_opa(&textedit_style_cursor, LV_OPA_0);

  // Show Cursor in "Edit" mode
  lv_style_set_opa(&textedit_style_cursor_edit, LV_OPA_COVER);
  lv_style_set_bg_opa(&textedit_style_cursor_edit, LV_OPA_50);

  // Buttons
  lv_style_init(&btn_style);
  lv_style_set_bg_opa(&btn_style, LV_OPA_100);
  lv_style_set_bg_color(&btn_style, makeLvColor(COLOR_THEME_SECONDARY2));
  lv_style_set_border_opa(&btn_style, LV_OPA_0);
  lv_style_set_border_width(&btn_style, 2);

  // LV_STATE_CHECKED
  lv_style_init(&btn_checked_style);
  lv_style_set_bg_color(&btn_checked_style, makeLvColor(COLOR_THEME_ACTIVE));

  // LV_STATE_FOCUSED
  lv_style_init(&btn_focused_style);
  lv_style_set_border_color(&btn_focused_style, makeLvColor(COLOR_THEME_FOCUS));
  lv_style_set_text_color(&btn_focused_style, makeLvColor(COLOR_THEME_PRIMARY2));
  lv_style_set_bg_color(&btn_focused_style, makeLvColor(COLOR_THEME_FOCUS));
  lv_style_set_border_opa(&btn_focused_style, LV_OPA_100);

  // Menus
  lv_style_init(&menu_line_style);
  lv_style_set_bg_opa(&menu_line_style, LV_OPA_100);
  lv_style_set_bg_color(&menu_line_style, makeLvColor(COLOR_THEME_PRIMARY2));

  lv_style_init(&menu_checked_style);
  lv_style_set_border_width(&menu_checked_style, 1);
  lv_style_set_border_color(&menu_checked_style, makeLvColor(COLOR_THEME_FOCUS));
  lv_style_set_text_color(&menu_checked_style, makeLvColor(COLOR_THEME_PRIMARY2));
  lv_style_set_bg_color(&menu_checked_style, makeLvColor(COLOR_THEME_FOCUS));

  /*Initialize the new theme from the current theme*/
  lv_theme_t * th_act = lv_disp_get_theme(NULL);
  static lv_theme_t th_new;
  th_new = *th_act;

  /*Set the parent theme and the style apply callback for the new theme*/
  lv_theme_set_parent(&th_new, th_act);
  lv_theme_set_apply_cb(&th_new, theme_apply_cb);

  /*Assign the new theme to the current display*/
  lv_disp_set_theme(NULL, &th_new);
}

LvglWrapper::LvglWrapper()
{
  init_lvgl_drivers();
  init_theme();

  // Create main window and load that screen
  auto window = MainWindow::instance();
  window->setActiveScreen();
}

void LvglWrapper::run()
{
  tmr10ms_t tick = get_tmr10ms();
  lv_tick_inc((tick - lastTick) * 10);
  lastTick = tick;
  lv_timer_handler();
}
