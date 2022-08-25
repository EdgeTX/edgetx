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

#include "model_usbjoystick.h"
#include "usb_joystick.h"
#include "channel_bar.h"
#include "button_matrix.h"
#include "lvgl_widgets/input_mix_line.h"

#include "opentx.h"
#define SET_DIRTY() storageDirty(EE_MODEL)

#if LCD_W > LCD_H // Landscape

static const lv_coord_t line_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                     LV_GRID_FR(1), LV_GRID_FR(1),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t line_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

#define USBCH_EDIT_STATUS_BAR_WIDTH 250
#define USBCH_EDIT_STATUS_BAR_MARGIN 3
#define USBCH_EDIT_RIGHT_MARGIN 0
#define USBCH_BTNMX_COL 8
#define USBCH_COLLISION_Y PAGE_TITLE_TOP + PAGE_LINE_HEIGHT

static const lv_coord_t ch_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(2),
                                     LV_GRID_FR(1), LV_GRID_FR(2),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t ch_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

#else // Portrait

static const lv_coord_t line_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t line_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT,
                                     LV_GRID_TEMPLATE_LAST};

#define USBCH_EDIT_STATUS_BAR_WIDTH 180
#define USBCH_EDIT_STATUS_BAR_MARGIN 0
#define USBCH_EDIT_RIGHT_MARGIN 3
#define USBCH_BTNMX_COL 8
#define USBCH_COLLISION_Y PAGE_TITLE_TOP

static const lv_coord_t ch_col_dsc[] = {LV_GRID_FR(2), LV_GRID_FR(3),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t ch_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT,
                                     LV_GRID_TEMPLATE_LAST};

#endif

class USBChannelEditStatusBar : public Window
{
 public:
  USBChannelEditStatusBar(Window *parent, const rect_t &rect, int8_t channel) :
      Window(parent, rect), _channel(channel)
  {
    channelBar = new ComboChannelBar(this, {USBCH_EDIT_STATUS_BAR_MARGIN, 0, rect.w - (USBCH_EDIT_STATUS_BAR_MARGIN * 2), rect.h}, channel);
    channelBar->setLeftMargin(0);
    channelBar->setTextColor(COLOR_THEME_PRIMARY2);
    channelBar->setOutputChannelBarLimitColor(COLOR_THEME_EDIT);
  }

  void paint(BitmapBuffer *dc) override
  {
    // dc->clear(COLOR_THEME_SECONDARY2);
  }

 protected:
  ComboChannelBar *channelBar;
  int8_t _channel;
};

class USBChannelButtonSel : public ButtonMatrix
{
public:
  USBChannelButtonSel(Window* parent, const rect_t& rect, uint8_t channel,
                      std::function<int()> _getValue,
                      std::function<void(int)> _setValue = nullptr);

  void setChannel(uint8_t ch)
  {
    channel = ch;
    invalidate();
  }

  void setSetValueHandler(std::function<void(int)> handler)
  {
    _setValue = std::move(handler);
  }

  void setGetValueHandler(std::function<int()> handler)
  {
    _getValue = std::move(handler);
  }

  void setValue(int val);
  int getValue() const { return _getValue(); }

  void onPress(uint8_t btn_id) override;
  bool isActive(uint8_t btn_id) override { return false; }

  uint8_t getBtnState(uint8_t id)
  {
    if (id < USBJ_BUTTON_SIZE) return _btns[id];
    return 0;
  }

  void updateState();

protected:
  uint8_t channel = 0;
  std::function<int()> _getValue;
  std::function<void(int)> _setValue;
  uint8_t _btns[USBJ_BUTTON_SIZE];
};

static void btnsel_event_cb(lv_event_t* e)
{
  lv_event_code_t code = lv_event_get_code(e);

  if (code == LV_EVENT_DRAW_PART_BEGIN) {
    lv_obj_draw_part_dsc_t* dsc = lv_event_get_draw_part_dsc(e);

    if(dsc->class_p == &lv_btnmatrix_class && dsc->type == LV_BTNMATRIX_DRAW_PART_BTN) {
      dsc->rect_dsc->radius = LV_RADIUS_CIRCLE;
      auto btsel = (USBChannelButtonSel*)lv_event_get_user_data(e);

      uint8_t state = btsel->getBtnState((uint8_t)dsc->id);
      if (state == 1) {
        dsc->rect_dsc->bg_color = makeLvColor(COLOR_THEME_DISABLED);
        dsc->label_dsc->color = makeLvColor(COLOR_THEME_PRIMARY1);
      }
      else if (state == 2) {
        dsc->rect_dsc->bg_color = makeLvColor(COLOR_THEME_EDIT);
        dsc->label_dsc->color = makeLvColor(COLOR_THEME_PRIMARY2);
      }
      else if (state == 3) {
        dsc->rect_dsc->bg_color = makeLvColor(COLOR_THEME_WARNING);
        dsc->label_dsc->color = makeLvColor(COLOR_THEME_PRIMARY2);
      }
      else {
        dsc->rect_dsc->bg_color = makeLvColor(COLOR_THEME_PRIMARY2);
        dsc->label_dsc->color = makeLvColor(COLOR_THEME_SECONDARY1);
      }
    }
  }
}

USBChannelButtonSel::USBChannelButtonSel(Window* parent, const rect_t& rect, uint8_t channel,
                    std::function<int()> _getValue,
                    std::function<void(int)> _setValue) :
  ButtonMatrix(parent, rect),
  channel(channel),
  _getValue(std::move(_getValue)),
  _setValue(std::move(_setValue))
{
  initBtnMap(USBCH_BTNMX_COL, USBJ_BUTTON_SIZE);
  char snum[5];
  for (uint8_t btn = 0; btn < MAX_OUTPUT_CHANNELS; btn++) {
    snprintf(snum, 5, "%u", btn);
    setText(btn, snum);
  }
  update();

  lv_obj_add_event_cb(lvobj, btnsel_event_cb, LV_EVENT_DRAW_PART_BEGIN, this);

  memset(_btns, 0, USBJ_BUTTON_SIZE);
  for (uint8_t ch = 0; ch < MAX_OUTPUT_CHANNELS; ch++) {
    if(ch == channel) continue;
    USBJoystickChData * cch = usbJChAddress(ch);

    if(cch->mode != USBJOYS_CH_BUTTON) continue;

    if (cch->switch_npos == 0) {
      _btns[cch->btn_num] = 1;
    }
    else {
     uint8_t last = cch->btn_num + cch->switch_npos;
     if(last >= USBJ_BUTTON_SIZE) {
       last = USBJ_BUTTON_SIZE-1;
     }

     for(uint8_t b = cch->btn_num; b <= last; b++) _btns[b] = 1;
    }
  }
  updateState();
}

void USBChannelButtonSel::setValue(int val)
{
  if (_setValue) {
    _setValue(val);
  }
}

void USBChannelButtonSel::onPress(uint8_t btn_id)
{
  setValue(btn_id);
  updateState();
}

void USBChannelButtonSel::updateState()
{
  USBJoystickChData * cch = usbJChAddress(channel);

  for(uint8_t i = 0; i < USBJ_BUTTON_SIZE; i++) _btns[i] &= 1;
  if (cch->switch_npos == 0) {
    _btns[cch->btn_num] |= 2;
  }
  else {
   uint8_t last = cch->btn_num + cch->switch_npos;
   if(last >= USBJ_BUTTON_SIZE) {
     last = USBJ_BUTTON_SIZE-1;
   }

   for(uint8_t b = cch->btn_num; b <= last; b++) _btns[b] |= 2;
  }
}

class USBChannelEditWindow : public Page
{
 public:
  explicit USBChannelEditWindow(uint8_t channel);

 protected:
  uint8_t channel;
  USBChannelEditStatusBar* statusBar = nullptr;
  Window* _BtnModeLine = nullptr;
  Window* _AxisModeLine = nullptr;
  Window* _SimModeLine = nullptr;
  Window* _BtnNumLine = nullptr;
  USBChannelButtonSel* _BtnNumSel = nullptr;
  StaticText* collisionText = nullptr;
  Choice* _BtnPosChoice = nullptr;

protected:
  void update();
  void buildHeader(Window *window);
  void buildBody(FormWindow *window);
};

USBChannelEditWindow::USBChannelEditWindow(uint8_t channel) :
    Page(ICON_MODEL_USB), channel(channel)
{
  auto form = new FormWindow(&body, rect_t{});
  form->padAll(lv_dpx(8));

  buildHeader(&header);
  buildBody(form);
}

void USBChannelEditWindow::buildHeader(Window *window)
{
  int title_w = window->getRect().w - USBCH_EDIT_STATUS_BAR_WIDTH -
      USBCH_EDIT_RIGHT_MARGIN - USBCH_EDIT_STATUS_BAR_MARGIN - PAGE_TITLE_LEFT;
  new StaticText(this,
                 {PAGE_TITLE_LEFT, PAGE_TITLE_TOP, title_w, PAGE_LINE_HEIGHT},
                 TR_USBJOYSTICK_LABEL, 0, COLOR_THEME_PRIMARY2);

  collisionText = new StaticText(this,
                  {PAGE_TITLE_LEFT, USBCH_COLLISION_Y, title_w, PAGE_LINE_HEIGHT},
                  "", OPAQUE, FONT(BOLD) | COLOR_THEME_PRIMARY2 | CENTERED);
  collisionText->setBackgroundColor(COLOR_THEME_WARNING);

  statusBar = new USBChannelEditStatusBar(
      window,
      {window->getRect().w - USBCH_EDIT_STATUS_BAR_WIDTH -
           USBCH_EDIT_RIGHT_MARGIN,
       0, USBCH_EDIT_STATUS_BAR_WIDTH, MENU_HEADER_HEIGHT + 3},
      channel);
}

#define SET_VALUE_WUPDATE(value)             [=](int32_t newValue) { value = newValue; SET_DIRTY(); this->update(); }
void USBChannelEditWindow::buildBody(FormWindow* form)
{
  FlexGridLayout grid(ch_col_dsc, ch_row_dsc, 2);
  form->setFlexLayout();

  USBJoystickChData * cch = usbJChAddress(channel);

  auto line = form->newLine(&grid);
  new StaticText(line, rect_t{}, STR_USBJOYSTICK_CH_MODE, 0, COLOR_THEME_PRIMARY1);
  new Choice(line, rect_t{}, STR_VUSBJOYSTICK_CH_MODE, 0, USBJOYS_CH_LAST,
                             GET_DEFAULT(cch->mode), SET_VALUE_WUPDATE(cch->mode));

  new StaticText(line, rect_t{}, STR_USBJOYSTICK_CH_INVERSION, 0, COLOR_THEME_PRIMARY1);
  new CheckBox(line, rect_t{}, GET_SET_DEFAULT(cch->inversion));

  _BtnModeLine = form->newLine(&grid);
  new StaticText(_BtnModeLine, rect_t{}, STR_USBJOYSTICK_CH_BTNMODE, 0, COLOR_THEME_PRIMARY1);
  new Choice(_BtnModeLine, rect_t{}, STR_VUSBJOYSTICK_CH_BTNMODE, 0, USBJOYS_BTN_MODE_LAST,
                             GET_DEFAULT(cch->btn_mode),
                             [=](int32_t newValue)
                             {
                                cch->btn_mode = newValue;
                                if(cch->btn_mode == USBJOYS_BTN_MODE_SW_EMU) _BtnPosChoice->setValue(0);
                                else if(cch->btn_mode == USBJOYS_BTN_MODE_DELTA) _BtnPosChoice->setValue(1);
                                SET_DIRTY();
                                this->update();
                             });
  new StaticText(_BtnModeLine, rect_t{}, STR_USBJOYSTICK_CH_SWPOS, 0, COLOR_THEME_PRIMARY1);
  _BtnPosChoice = new Choice(_BtnModeLine, rect_t{}, STR_VUSBJOYSTICK_CH_SWPOS, 0, 7,
                             GET_DEFAULT(cch->switch_npos), SET_VALUE_WUPDATE(cch->switch_npos));

  _AxisModeLine = form->newLine(&grid);
  new StaticText(_AxisModeLine, rect_t{}, STR_USBJOYSTICK_CH_AXIS, 0, COLOR_THEME_PRIMARY1);
  new Choice(_AxisModeLine, rect_t{}, STR_VUSBJOYSTICK_CH_AXIS, 0, USBJOYS_AXIS_LAST,
                             GET_DEFAULT(cch->axis), SET_VALUE_WUPDATE(cch->axis));

  _SimModeLine = form->newLine(&grid);
  new StaticText(_SimModeLine, rect_t{}, STR_USBJOYSTICK_CH_SIM, 0, COLOR_THEME_PRIMARY1);
  new Choice(_SimModeLine, rect_t{}, STR_VUSBJOYSTICK_CH_SIM, 0, USBJOYS_SIM_LAST,
                             GET_DEFAULT(cch->sim), SET_VALUE_WUPDATE(cch->sim));

  _BtnNumLine = form->newLine(&grid);
  new StaticText(_BtnNumLine, rect_t{}, STR_USBJOYSTICK_CH_BTNNUM, 0, COLOR_THEME_PRIMARY1);
#if LCD_W > LCD_H
  _BtnNumSel = new USBChannelButtonSel(_BtnNumLine, rect_t{},
                                       channel, GET_DEFAULT(cch->btn_num), SET_VALUE_WUPDATE(cch->btn_num));
#else
  line = form->newLine(&grid);
  _BtnNumSel = new USBChannelButtonSel(line, rect_t{},
                                       channel, GET_DEFAULT(cch->btn_num), SET_VALUE_WUPDATE(cch->btn_num));
#endif

  update();
}

#define USBCH_LINES 5
void USBChannelEditWindow::update()
{
  const uint8_t hiding[USBJOYS_CH_LAST + 1][USBCH_LINES] = {{ 0, 0, 0, 0, 0 }, { 1, 0, 0, 1, 1 }, { 0, 1, 0, 0, 0 }, { 0, 0, 1, 0, 0 }};
  Window* lines[USBCH_LINES] = { _BtnModeLine, _AxisModeLine, _SimModeLine, _BtnNumLine, _BtnNumSel };

  USBJoystickChData * cch = usbJChAddress(channel);

  for(int i = 0; i < USBCH_LINES; i++) {
    if(!lines[i]) continue;
    if(hiding[cch->mode][i]) lv_obj_clear_flag(lines[i]->getLvObj(), LV_OBJ_FLAG_HIDDEN);
    else lv_obj_add_flag(lines[i]->getLvObj(), LV_OBJ_FLAG_HIDDEN);
  }

  if(_BtnPosChoice) _BtnPosChoice->enable((cch->btn_mode != USBJOYS_BTN_MODE_SW_EMU) && (cch->btn_mode != USBJOYS_BTN_MODE_DELTA));

  if(collisionText) {
    collisionText->setText("");
    lv_obj_add_flag(collisionText->getLvObj(), LV_OBJ_FLAG_HIDDEN);
    if (cch->mode == USBJOYS_CH_BUTTON) {
      if (isUSBBtnNumCollision(channel)) {
        collisionText->setText(STR_USBJOYSTICK_BTN_COLLISION);
        lv_obj_clear_flag(collisionText->getLvObj(), LV_OBJ_FLAG_HIDDEN);
      }
    }
    else if (cch->mode == USBJOYS_CH_AXIS) {
      if (isUSBAxisCollision(channel)) {
        collisionText->setText(STR_USBJOYSTICK_AXIS_COLLISION);
        lv_obj_clear_flag(collisionText->getLvObj(), LV_OBJ_FLAG_HIDDEN);
      }
    }
    else if (cch->mode == USBJOYS_CH_SIM) {
      if (isUSBSimCollision(channel)) {
        collisionText->setText(STR_USBJOYSTICK_AXIS_COLLISION);
        lv_obj_clear_flag(collisionText->getLvObj(), LV_OBJ_FLAG_HIDDEN);
      }
    }
  }

  _BtnNumSel->updateState();
}

class USBChannelLineButton : public Button
{
public:
  USBChannelLineButton(Window* parent, uint8_t index);
  ~USBChannelLineButton() {}

  void paint(BitmapBuffer *dc) override;

protected:
  uint8_t index;
};

USBChannelLineButton::USBChannelLineButton(Window* parent, uint8_t index) :
  Button(parent, rect_t{}, nullptr, 0, 0, input_mix_line_create), index(index)
{
  setHeight(PAGE_LINE_HEIGHT + 14);
}

void USBChannelLineButton::paint(BitmapBuffer *dc)
{
  static constexpr coord_t line1 = FIELD_PADDING_TOP + 7;
  static constexpr coord_t colstep = (LCD_W - 40) / 5;

  coord_t col = 20;
  char str[20];

  USBJoystickChData * cch = usbJChAddress(index);
  dc->drawSolidFilledRect(0, 0, rect.w, rect.h, COLOR_THEME_PRIMARY2);

  // channel
  drawChn(dc, col, line1, index + 1, FONT(BOLD) | COLOR_THEME_SECONDARY1);

  // inversion
  if(cch->inversion) dc->drawMask(col + colstep - 25, line1+2, chanMonInvertedBitmap, COLOR_THEME_SECONDARY1);

  // mode
  col += colstep;
  dc->drawTextAtIndex(col, line1, STR_VUSBJOYSTICK_CH_MODE, cch->mode, COLOR_THEME_SECONDARY1);

  if (cch->mode == USBJOYS_CH_BUTTON) {
    col += colstep;
    dc->drawTextAtIndex(col, line1, STR_VUSBJOYSTICK_CH_SWPOS, cch->switch_npos, COLOR_THEME_SECONDARY1);
    col += colstep;
    dc->drawTextAtIndex(col, line1, STR_VUSBJOYSTICK_CH_BTNMODE, cch->btn_mode, COLOR_THEME_SECONDARY1);

    if (cch->switch_npos == 0) {
      col += colstep;
      LcdFlags warn = COLOR_THEME_SECONDARY1;
      if (isUSBBtnNumCollision(index)) warn = FONT(BOLD) | COLOR_THEME_WARNING;
      dc->drawNumber(col, line1, cch->btn_num, warn);
    }
    else {
      uint8_t last = cch->btn_num + cch->switch_npos;
      if(last >= USBJ_BUTTON_SIZE) {
        last = USBJ_BUTTON_SIZE-1;
      }
      snprintf(str, 20, "%u..%u", cch->btn_num, last);
      LcdFlags warn = COLOR_THEME_SECONDARY1;
      if (isUSBBtnNumCollision(index)) warn = FONT(BOLD) | COLOR_THEME_WARNING;
      col += colstep;
      dc->drawText(col, line1, str, warn);
    }
  }
  else if (cch->mode == USBJOYS_CH_AXIS) {
    LcdFlags warn = COLOR_THEME_SECONDARY1;
    if (isUSBAxisCollision(index)) warn = FONT(BOLD) | COLOR_THEME_WARNING;
    col += colstep;
    dc->drawTextAtIndex(col, line1, STR_VUSBJOYSTICK_CH_AXIS, cch->axis, warn);
  }
  else if (cch->mode == USBJOYS_CH_SIM) {
    LcdFlags warn = COLOR_THEME_SECONDARY1;
    if (isUSBSimCollision(index)) warn = FONT(BOLD) | COLOR_THEME_WARNING;
    col += colstep;
    dc->drawTextAtIndex(col, line1, STR_VUSBJOYSTICK_CH_SIM, cch->sim, warn);
  }
}

ModelUSBJoystickPage::ModelUSBJoystickPage() :
    Page(ICON_MODEL_USB)
{
  header.setTitle(STR_USBJOYSTICK_LABEL);

  auto form = new FormWindow(&body, rect_t{});
  form->padAll(lv_dpx(8));
  form->setFlexLayout();
  FlexGridLayout grid(line_col_dsc, line_row_dsc, 2);

  // Extended mode
  auto line = form->newLine(&grid);
  new StaticText(line, rect_t{}, STR_USBJOYSTICK_EXTMODE, 0, COLOR_THEME_PRIMARY1);
  new Choice(line, rect_t{}, STR_VUSBJOYSTICK_EXTMODE, 0, 1,
             GET_DEFAULT(g_model.usbJoystickExtMode), SET_VALUE_WUPDATE(g_model.usbJoystickExtMode));

  _IfModeLabel = new StaticText(line, rect_t{}, STR_USBJOYSTICK_IF_MODE, 0, COLOR_THEME_PRIMARY1);
  _IfMode = new Choice(line, rect_t{}, STR_VUSBJOYSTICK_IF_MODE, 0, USBJOYS_LAST,
                       GET_DEFAULT(g_model.usbJoystickIfMode), SET_VALUE_WUPDATE(g_model.usbJoystickIfMode));

  line = form->newLine(&grid);
  _CircCoutoutLabel = new StaticText(line, rect_t{}, STR_USBJOYSTICK_CIRC_COUTOUT, 0, COLOR_THEME_PRIMARY1);
  _CircCoutout = new Choice(line, rect_t{}, STR_VUSBJOYSTICK_CIRC_COUTOUT, 0, USBJOYS_LAST,
                            GET_DEFAULT(g_model.usbJoystickCircularCut), SET_VALUE_WUPDATE(g_model.usbJoystickCircularCut));

  _ApplyBtn = new TextButton(line, rect_t{}, STR_USBJOYSTICK_APPLY_CHANGES,
                            [=]() { onUSBJoystickModelChanged(); this->update(); return 0; });

  auto btngrp = new FormGroup(form, rect_t{});
  _ChannelsGroup = btngrp;
  btngrp->setFlexLayout();
  btngrp->padRow(lv_dpx(4));
  for (uint8_t ch = 0; ch < MAX_OUTPUT_CHANNELS; ch++) {

    // Channel settings
    auto btn = new USBChannelLineButton(btngrp, ch);
    // btn->refresh();

    USBJoystickChData * cch = usbJChAddress(ch);
    btn->setPressHandler([=]() -> uint8_t {
      if (cch->mode == USBJOYS_CH_NONE) {
        editChannel(ch, btn);
      }
      else {
        Menu *menu = new Menu(parent);
        menu->addLine(STR_EDIT, [=]() {
          editChannel(ch, btn);
        });
        menu->addLine(STR_CLEAR, [=]() {
          memset(cch, 0, sizeof(USBJoystickChData));
          SET_DIRTY();
          btn->invalidate();
        });
      }
      return 0;
    });
  }

  update();
}

void ModelUSBJoystickPage::update()
{
  const uint8_t usbj_ctrls = 6;
  Window* ctrls[usbj_ctrls] = { _IfModeLabel, _IfMode, _CircCoutoutLabel, _CircCoutout, _ApplyBtn, _ChannelsGroup };

  for(uint8_t i = 0; i < usbj_ctrls; i++) {
    if(usbJoystickExtMode()) {
      lv_obj_clear_flag(ctrls[i]->getLvObj(), LV_OBJ_FLAG_HIDDEN);
      _ApplyBtn->enable(usbJoystickSettingsChanged());
    }
    else {
      lv_obj_add_flag(ctrls[i]->getLvObj(), LV_OBJ_FLAG_HIDDEN);
    }
  }
}

void ModelUSBJoystickPage::editChannel(uint8_t channel, USBChannelLineButton* btn)
{
  auto chedit = new USBChannelEditWindow(channel);
  chedit->setCloseHandler(
      [=]() { this->update(); });
}

