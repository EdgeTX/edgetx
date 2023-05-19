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

#define USBCH_EDIT_STATUS_BAR_WIDTH 250
#define USBCH_EDIT_STATUS_BAR_MARGIN 3
#define USBCH_EDIT_RIGHT_MARGIN 0
#define USBCH_COLS 4

static const lv_coord_t ch_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(2),
                                     LV_GRID_FR(1), LV_GRID_FR(2),
                                     LV_GRID_TEMPLATE_LAST};

#define USBCH_CHN_ROWS 1
#define USBCH_BTN_MODE_COL 4
#define USBCH_BTN_MODE_ROW 0
#define USBCH_LINE_HEIGHT PAGE_LINE_HEIGHT + 14

static const lv_coord_t b_col_dsc[] = {LV_GRID_FR(10), 20, LV_GRID_FR(10), LV_GRID_FR(12),
                                       LV_GRID_FR(9), LV_GRID_FR(9), LV_GRID_TEMPLATE_LAST};

static const lv_coord_t b_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

#else // Portrait

static const lv_coord_t line_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                     LV_GRID_TEMPLATE_LAST};

#define USBCH_EDIT_STATUS_BAR_WIDTH 160
#define USBCH_EDIT_STATUS_BAR_MARGIN 0
#define USBCH_EDIT_RIGHT_MARGIN 3
#define USBCH_COLS 2

static const lv_coord_t ch_col_dsc[] = {LV_GRID_FR(2), LV_GRID_FR(3),
                                     LV_GRID_TEMPLATE_LAST};

#define USBCH_CHN_ROWS 2
#define USBCH_BTN_MODE_COL 2
#define USBCH_BTN_MODE_ROW 1
#define USBCH_LINE_HEIGHT 2 * PAGE_LINE_HEIGHT + 10

static const lv_coord_t b_col_dsc[] = {LV_GRID_FR(1), 20, LV_GRID_FR(1), LV_GRID_FR(1),
                                       LV_GRID_TEMPLATE_LAST};

static const lv_coord_t b_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

#endif

#define USBCH_BTNMX_COL 8

static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

class USBChannelEditStatusBar : public Window
{
  public:
    USBChannelEditStatusBar(Window *parent, const rect_t &rect, int8_t channel) :
        Window(parent, rect), _channel(channel)
    {
      channelBar = new ComboChannelBar(this, {USBCH_EDIT_STATUS_BAR_MARGIN, 0, rect.w - (USBCH_EDIT_STATUS_BAR_MARGIN * 2), rect.h}, channel);
      channelBar->setLeftMargin(15);
      channelBar->setTextColor(COLOR_THEME_PRIMARY2);
      channelBar->setOutputChannelBarLimitColor(COLOR_THEME_EDIT);
    }

  protected:
    ComboChannelBar *channelBar;
    int8_t _channel;
};

static void btnsel_event_cb(lv_event_t* e);

class USBChannelButtonSel : public ButtonMatrix
{
  public:
    USBChannelButtonSel(Window* parent, const rect_t& rect, uint8_t channel,
                    std::function<void(int)> _setValue) :
      ButtonMatrix(parent, rect),
      m_channel(channel),
      m_setValue(std::move(_setValue))
    {
      initBtnMap(USBCH_BTNMX_COL, USBJ_BUTTON_SIZE);
      char snum[5];
      for (uint8_t btn = 0; btn < USBJ_BUTTON_SIZE; btn++) {
        snprintf(snum, 5, "%u", btn);
        setText(btn, snum);
      }
      update();

      lv_obj_add_event_cb(lvobj, btnsel_event_cb, LV_EVENT_DRAW_PART_BEGIN, this);

      memset(m_btns, 0, USBJ_BUTTON_SIZE);
      for (uint8_t ch = 0; ch < USBJ_MAX_JOYSTICK_CHANNELS; ch++) {
        if (ch != m_channel) {
          USBJoystickChData * cch = usbJChAddress(ch);

          if (cch->mode == USBJOYS_CH_BUTTON) {
            uint8_t last = cch->lastBtnNum();
            for(uint8_t b = cch->btn_num; b <= last; b++) {
              m_btns[b] = 1;
            }
          }
        }
      }
      updateState();
    }

    void setValue(int val)
    {
      if (m_setValue) {
        m_setValue(val);
      }
    }

    void onPress(uint8_t btn_id) override
    {
      setValue(btn_id);
      updateState();
    }

    bool isActive(uint8_t btn_id) override { return false; }

    uint8_t getBtnState(uint8_t id)
    {
      if (id < USBJ_BUTTON_SIZE) return m_btns[id];
      return 0;
    }

    void updateState()
    {
      USBJoystickChData * cch = usbJChAddress(m_channel);

      for(uint8_t i = 0; i < USBJ_BUTTON_SIZE; i++) m_btns[i] &= 1;
      uint8_t last = cch->lastBtnNum();
      for(uint8_t b = cch->btn_num; b <= last; b++) m_btns[b] |= 2;
    }

  protected:
    uint8_t m_channel = 0;
    std::function<void(int)> m_setValue;
    uint8_t m_btns[USBJ_BUTTON_SIZE];
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

#define USBCH_LINES 3
#define SET_VALUE_WUPDATE(value) [=](int32_t newValue) { value = newValue; SET_DIRTY(); this->update(); }

class USBChannelEditWindow : public Page
{
  public:
    USBChannelEditWindow(uint8_t channel) :
        Page(ICON_MODEL_USB), channel(channel)
    {
      auto form = new FormWindow(&body, rect_t{});
      form->padAll(2);
      form->padLeft(8);
      form->padRight(8);

      buildHeader(&header);
      buildBody(form);
    }

  protected:
    uint8_t channel;
    USBChannelEditStatusBar* statusBar = nullptr;
    FormWindow* m_btnModeFrame = nullptr;
    Window* m_axisModeLine = nullptr;
    Window* m_simModeLine = nullptr;
    USBChannelButtonSel* _BtnNumSel = nullptr;
    StaticText* collisionText = nullptr;
    Choice* m_btnPosChoice = nullptr;

  protected:
    void update()
    {
      USBJoystickChData * cch = usbJChAddress(channel);

      lv_obj_add_flag(m_btnModeFrame->getLvObj(), LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(m_axisModeLine->getLvObj(), LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(m_simModeLine->getLvObj(), LV_OBJ_FLAG_HIDDEN);
      if (cch->mode == USBJOYS_CH_BUTTON)
        lv_obj_clear_flag(m_btnModeFrame->getLvObj(), LV_OBJ_FLAG_HIDDEN);
      else if (cch->mode == USBJOYS_CH_AXIS)
        lv_obj_clear_flag(m_axisModeLine->getLvObj(), LV_OBJ_FLAG_HIDDEN);
      else if (cch->mode == USBJOYS_CH_SIM)
        lv_obj_clear_flag(m_simModeLine->getLvObj(), LV_OBJ_FLAG_HIDDEN);

      if(m_btnPosChoice)
        m_btnPosChoice->enable((cch->param != USBJOYS_BTN_MODE_SW_EMU) && (cch->param != USBJOYS_BTN_MODE_DELTA));

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

    void buildHeader(Window *window)
    {
      header.setTitle(STR_USBJOYSTICK_LABEL);
      header.setTitle2(getSourceString(MIXSRC_CH1 + channel));

      statusBar = new USBChannelEditStatusBar(
          window,
          {window->getRect().w - USBCH_EDIT_STATUS_BAR_WIDTH - USBCH_EDIT_RIGHT_MARGIN,
           0, USBCH_EDIT_STATUS_BAR_WIDTH, MENU_HEADER_HEIGHT},
          channel);
    }

    void buildBody(FormWindow* form)
    {
      FlexGridLayout grid(ch_col_dsc, row_dsc, 2);
      form->setFlexLayout();

      USBJoystickChData * cch = usbJChAddress(channel);

      auto line = form->newLine(&grid);

      line->padTop(0);
      line->padBottom(0);
      collisionText = new StaticText(line, rect_t{}, "", OPAQUE, FONT(BOLD) | COLOR_THEME_PRIMARY2 | CENTERED);
      collisionText->setBackgroundColor(COLOR_THEME_WARNING);
      lv_obj_set_grid_cell(collisionText->getLvObj(), LV_GRID_ALIGN_STRETCH, 0, USBCH_COLS, LV_GRID_ALIGN_CENTER, 0, 1);

      line = form->newLine(&grid);
      new StaticText(line, rect_t{}, STR_USBJOYSTICK_CH_MODE, 0, COLOR_THEME_PRIMARY1);
      new Choice(line, rect_t{}, STR_VUSBJOYSTICK_CH_MODE, 0, USBJOYS_CH_LAST,
                                 GET_DEFAULT(cch->mode), SET_VALUE_WUPDATE(cch->mode));

#if LCD_H > LCD_W
      line = form->newLine(&grid);
#endif

      new StaticText(line, rect_t{}, STR_USBJOYSTICK_CH_INVERSION, 0, COLOR_THEME_PRIMARY1);
      new CheckBox(line, rect_t{}, GET_SET_DEFAULT(cch->inversion));

      line = form->newLine(&grid);
      m_btnModeFrame = new FormWindow(line, rect_t{});
      m_btnModeFrame->setFlexLayout();

      line = m_btnModeFrame->newLine(&grid);
      new StaticText(line, rect_t{}, STR_USBJOYSTICK_CH_BTNMODE, 0, COLOR_THEME_PRIMARY1);
      new Choice(line, rect_t{}, STR_VUSBJOYSTICK_CH_BTNMODE, 0, USBJOYS_BTN_MODE_LAST,
                                 GET_DEFAULT(cch->param),
                                 [=](int32_t newValue)
                                 {
                                    cch->param = newValue;
                                    if(cch->param == USBJOYS_BTN_MODE_SW_EMU) m_btnPosChoice->setValue(0);
                                    else if(cch->param == USBJOYS_BTN_MODE_DELTA) m_btnPosChoice->setValue(1);
                                    SET_DIRTY();
                                    this->update();
                                 });

#if LCD_H > LCD_W
      line = m_btnModeFrame->newLine(&grid);
#endif

      new StaticText(line, rect_t{}, STR_USBJOYSTICK_CH_SWPOS, 0, COLOR_THEME_PRIMARY1);
      m_btnPosChoice = new Choice(line, rect_t{}, STR_VUSBJOYSTICK_CH_SWPOS, 0, 7,
                                 GET_DEFAULT(cch->switch_npos), SET_VALUE_WUPDATE(cch->switch_npos));

      line = m_btnModeFrame->newLine(&grid);
      new StaticText(line, rect_t{}, STR_USBJOYSTICK_CH_BTNNUM, 0, COLOR_THEME_PRIMARY1);
#if LCD_H > LCD_W
      line = m_btnModeFrame->newLine(&grid);
#endif
      _BtnNumSel = new USBChannelButtonSel(line, rect_t{},
                                           channel, SET_VALUE_WUPDATE(cch->btn_num));

      m_axisModeLine = form->newLine(&grid);
      new StaticText(m_axisModeLine, rect_t{}, STR_USBJOYSTICK_CH_AXIS, 0, COLOR_THEME_PRIMARY1);
      new Choice(m_axisModeLine, rect_t{}, STR_VUSBJOYSTICK_CH_AXIS, 0, USBJOYS_AXIS_LAST,
                                 GET_DEFAULT(cch->param), SET_VALUE_WUPDATE(cch->param));

      m_simModeLine = form->newLine(&grid);
      new StaticText(m_simModeLine, rect_t{}, STR_USBJOYSTICK_CH_SIM, 0, COLOR_THEME_PRIMARY1);
      new Choice(m_simModeLine, rect_t{}, STR_VUSBJOYSTICK_CH_SIM, 0, USBJOYS_SIM_LAST,
                                 GET_DEFAULT(cch->param), SET_VALUE_WUPDATE(cch->param));

      update();
    }
};

class USBChannelLineButton : public Button
{
  public:
    USBChannelLineButton(Window* parent, uint8_t index) :
      Button(parent, rect_t{}, nullptr, 0, 0, input_mix_line_create), index(index)
    {
      setHeight(USBCH_LINE_HEIGHT);

      lv_obj_set_layout(lvobj, LV_LAYOUT_GRID);
      lv_obj_set_grid_dsc_array(lvobj, b_col_dsc, b_row_dsc);
      lv_obj_set_style_pad_row(lvobj, 0, 0);
      lv_obj_set_style_pad_column(lvobj, 4, 0);

      lv_obj_add_event_cb(lvobj, USBChannelLineButton::on_draw,
                          LV_EVENT_DRAW_MAIN_BEGIN, nullptr);
    }

    static void on_draw(lv_event_t* e)
    {
      lv_obj_t* target = lv_event_get_target(e);
      auto line = (USBChannelLineButton*)lv_obj_get_user_data(target);
      if (line) {
        if (!line->init)
          line->delayed_init(e);
        else
          line->refresh();
      }
    }

    void delayed_init(lv_event_t* e)
    {
      m_chn = lv_label_create(lvobj);
      lv_obj_set_grid_cell(m_chn, LV_GRID_ALIGN_START, 0, 1,
                           LV_GRID_ALIGN_CENTER, 0, USBCH_CHN_ROWS);

      m_inverse = new StaticBitmap(this, rect_t{0, 0, 11, 16}, chanMonInvertedBitmap, COLOR_THEME_SECONDARY1, false);
      lv_obj_set_grid_cell(m_inverse->getLvObj(), LV_GRID_ALIGN_START, 1, 1,
                           LV_GRID_ALIGN_CENTER, 0, 1);

      m_mode = lv_label_create(lvobj);
      lv_obj_set_grid_cell(m_mode, LV_GRID_ALIGN_START, 2, 1,
                           LV_GRID_ALIGN_CENTER, 0, 1);

      m_param = lv_label_create(lvobj);
      lv_obj_set_grid_cell(m_param, LV_GRID_ALIGN_START, 3, 1,
                           LV_GRID_ALIGN_CENTER, 0, 1);

      m_btn_mode = lv_label_create(lvobj);
      lv_obj_set_grid_cell(m_btn_mode, LV_GRID_ALIGN_START, USBCH_BTN_MODE_COL, 1,
                           LV_GRID_ALIGN_CENTER, USBCH_BTN_MODE_ROW, 1);

      m_btns = lv_label_create(lvobj);
      lv_obj_set_grid_cell(m_btns, LV_GRID_ALIGN_START, USBCH_BTN_MODE_COL+1, 1,
                           LV_GRID_ALIGN_CENTER, USBCH_BTN_MODE_ROW, 1);

      lv_label_set_text(m_chn, getSourceString(MIXSRC_CH1 + index));
      lv_label_set_text(m_mode, "");
      lv_label_set_text(m_param, "");
      lv_label_set_text(m_btn_mode, "");
      lv_label_set_text(m_btns, "");

      init = true;
      refresh();
      lv_obj_update_layout(lvobj);

      if (e) {
        auto param = lv_event_get_param(e);
        lv_event_send(lvobj, LV_EVENT_DRAW_MAIN, param);
      }
    }

    void refresh()
    {
      if (!init) return;

      USBJoystickChData * cch = usbJChAddress(index);

      lv_label_set_text(m_mode, STR_VUSBJOYSTICK_CH_MODE[cch->mode]);

      if (cch->inversion)
        lv_obj_clear_flag(m_inverse->getLvObj(), LV_OBJ_FLAG_HIDDEN);
      else
        lv_obj_add_flag(m_inverse->getLvObj(), LV_OBJ_FLAG_HIDDEN);

      LcdFlags warn = COLOR_THEME_SECONDARY1;
      LcdFlags font = FONT(STD);
      const char* param = "";

      if (cch->mode == USBJOYS_CH_BUTTON) {
        param = STR_VUSBJOYSTICK_CH_BTNMODE[cch->param];
      }
      else if (cch->mode == USBJOYS_CH_AXIS) {
        param = STR_VUSBJOYSTICK_CH_AXIS[cch->param];
        if (isUSBAxisCollision(index)) {
          warn = COLOR_THEME_WARNING;
          font = FONT(BOLD);
        }
      }
      else if (cch->mode == USBJOYS_CH_SIM) {
        param = STR_VUSBJOYSTICK_CH_SIM[cch->param];
        if (isUSBSimCollision(index)) {
          warn = COLOR_THEME_WARNING;
          font = FONT(BOLD);
        }
      }

      lv_label_set_text(m_param, param);
      lv_obj_set_style_text_color(m_param, makeLvColor(warn), 0);
      lv_obj_set_style_text_font(m_param, getFont(font), 0);

      if (cch->mode == USBJOYS_CH_BUTTON) {
        lv_label_set_text(m_btn_mode, STR_VUSBJOYSTICK_CH_SWPOS[cch->switch_npos]);

        char str[20];

        uint8_t last = cch->lastBtnNum();
        if (last > cch->btn_num)
          snprintf(str, 20, "%u..%u", cch->btn_num, last);
        else
          snprintf(str, 20, "%u", cch->btn_num);
        lv_label_set_text(m_btns, str);
        if (isUSBBtnNumCollision(index)) {
          warn = COLOR_THEME_WARNING;
          font = FONT(BOLD);
        }
        lv_obj_set_style_text_color(m_btns, makeLvColor(warn), 0);
        lv_obj_set_style_text_font(m_btns, getFont(font), 0);
      } else {
        lv_label_set_text(m_btn_mode, "");
        lv_label_set_text(m_btns, "");
      }
    }

  protected:
    bool init = false;
    uint8_t index;
    
    lv_obj_t* m_chn;
    lv_obj_t* m_mode;
    lv_obj_t* m_param;
    lv_obj_t* m_btn_mode;
    lv_obj_t* m_btns;
    StaticBitmap* m_inverse;
};

ModelUSBJoystickPage::ModelUSBJoystickPage() :
    Page(ICON_MODEL_USB)
{
  header.setTitle(STR_MENU_MODEL_SETUP);
  header.setTitle2(STR_USBJOYSTICK_LABEL);

  auto form = new FormWindow(&body, rect_t{});
  form->padAll(lv_dpx(8));
  form->setFlexLayout();
  FlexGridLayout grid(line_col_dsc, row_dsc, 2);

  // Extended mode
  auto line = form->newLine(&grid);
  new StaticText(line, rect_t{}, STR_USBJOYSTICK_EXTMODE, 0, COLOR_THEME_PRIMARY1);
  new Choice(line, rect_t{}, STR_VUSBJOYSTICK_EXTMODE, 0, 1,
             GET_DEFAULT(g_model.usbJoystickExtMode), SET_VALUE_WUPDATE(g_model.usbJoystickExtMode));

#if LCD_H > LCD_W
  line = form->newLine(&grid);
#endif

  _IfModeLabel = new StaticText(line, rect_t{}, STR_USBJOYSTICK_IF_MODE, 0, COLOR_THEME_PRIMARY1);
  _IfMode = new Choice(line, rect_t{}, STR_VUSBJOYSTICK_IF_MODE, 0, USBJOYS_LAST,
                       GET_DEFAULT(g_model.usbJoystickIfMode), SET_VALUE_WUPDATE(g_model.usbJoystickIfMode));

  line = form->newLine(&grid);

  _CircCoutoutLabel = new StaticText(line, rect_t{}, STR_USBJOYSTICK_CIRC_COUTOUT, 0, COLOR_THEME_PRIMARY1);
  _CircCoutout = new Choice(line, rect_t{}, STR_VUSBJOYSTICK_CIRC_COUTOUT, 0, USBJOYS_LAST,
                            GET_DEFAULT(g_model.usbJoystickCircularCut), SET_VALUE_WUPDATE(g_model.usbJoystickCircularCut));

#if LCD_H > LCD_W
  line = form->newLine(&grid);
#endif

  _ApplyBtn = new TextButton(line, rect_t{}, STR_USBJOYSTICK_APPLY_CHANGES,
                            [=]() { onUSBJoystickModelChanged(); this->update(); return 0; });

  auto btngrp = new FormGroup(form, rect_t{});
  _ChannelsGroup = btngrp;
  btngrp->setFlexLayout();
  btngrp->padRow(lv_dpx(4));
  for (uint8_t ch = 0; ch < USBJ_MAX_JOYSTICK_CHANNELS; ch++) {

    // Channel settings
    auto btn = new USBChannelLineButton(btngrp, ch);

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
