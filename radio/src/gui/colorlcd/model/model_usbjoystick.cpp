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

#include "button_matrix.h"
#include "channel_bar.h"
#include "list_line_button.h"
#include "edgetx.h"
#include "etx_lv_theme.h"
#include "usb_joystick.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

#define ETX_STATE_COLLISION_WARN LV_STATE_USER_1

#if !PORTRAIT_LCD  // Landscape

static const lv_coord_t line_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                          LV_GRID_FR(1), LV_GRID_FR(1),
                                          LV_GRID_TEMPLATE_LAST};

static const lv_coord_t ch_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(2),
                                        LV_GRID_FR(1), LV_GRID_FR(2),
                                        LV_GRID_TEMPLATE_LAST};

static const lv_coord_t b_col_dsc[] = {LV_GRID_FR(10),       20,
                                       LV_GRID_FR(10),       LV_GRID_FR(12),
                                       LV_GRID_FR(9),        LV_GRID_FR(9),
                                       LV_GRID_TEMPLATE_LAST};

static const lv_coord_t b_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

#else  // Portrait

static const lv_coord_t line_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                          LV_GRID_TEMPLATE_LAST};

static const lv_coord_t ch_col_dsc[] = {LV_GRID_FR(2), LV_GRID_FR(3),
                                        LV_GRID_TEMPLATE_LAST};

static const lv_coord_t b_col_dsc[] = {LV_GRID_FR(1), 20, LV_GRID_FR(1),
                                       LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};

static const lv_coord_t b_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT,
                                       LV_GRID_TEMPLATE_LAST};

#endif

#define USBCH_BTNMX_COL 8

static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

class USBChannelEditStatusBar : public Window
{
 public:
  USBChannelEditStatusBar(Window* parent, const rect_t& rect, int8_t channel) :
      Window(parent, rect), _channel(channel)
  {
    channelBar = new ComboChannelBar(
        this,
        {USBCH_EDIT_STATUS_BAR_MARGIN, 0,
         rect.w - (USBCH_EDIT_STATUS_BAR_MARGIN * 2), rect.h},
        channel, true);
  }

  static LAYOUT_VAL(USBCH_EDIT_STATUS_BAR_MARGIN, 3, 0)

 protected:
  ComboChannelBar* channelBar;
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
    bg_color[0] = makeLvColor(COLOR_THEME_PRIMARY2);  // Unused
    fg_color[0] = makeLvColor(COLOR_THEME_SECONDARY1);
    bg_color[1] =
        makeLvColor(COLOR_THEME_DISABLED);  // Used by other channel_bar
    fg_color[1] = makeLvColor(COLOR_THEME_PRIMARY1);
    bg_color[2] = makeLvColor(COLOR_THEME_ACTIVE);  // Used by this channel
    fg_color[2] = makeLvColor(COLOR_THEME_PRIMARY1);
    bg_color[3] = makeLvColor(COLOR_THEME_WARNING);  // Collision
    fg_color[3] = makeLvColor(COLOR_THEME_PRIMARY2);

    initBtnMap(USBCH_BTNMX_COL, USBJ_BUTTON_SIZE);
    char snum[5];
    for (uint8_t btn = 0; btn < USBJ_BUTTON_SIZE; btn++) {
      snprintf(snum, 5, "%u", btn);
      setText(btn, snum);
    }
    update();

    etx_obj_add_style(lvobj, styles->circle, LV_PART_ITEMS);

    lv_obj_add_event_cb(lvobj, btnsel_event_cb, LV_EVENT_DRAW_PART_BEGIN, this);

    memset(m_btns, 0, USBJ_BUTTON_SIZE);
    for (uint8_t ch = 0; ch < USBJ_MAX_JOYSTICK_CHANNELS; ch++) {
      if (ch != m_channel) {
        USBJoystickChData* cch = usbJChAddress(ch);

        if (cch->mode == USBJOYS_CH_BUTTON) {
          uint8_t last = cch->lastBtnNum();
          for (uint8_t b = cch->btn_num; b <= last; b++) {
            m_btns[b] = 1;
          }
        }
      }
    }
    updateState();
  }

  void onPress(uint8_t btn_id) override
  {
    if (m_setValue) {
      m_setValue(btn_id);
    }
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
    USBJoystickChData* cch = usbJChAddress(m_channel);
    uint8_t last = cch->lastBtnNum();
    uint8_t i;

    for (i = 0; i < USBJ_BUTTON_SIZE; i++) m_btns[i] &= 1;

    for (i = cch->btn_num; i <= last; i++) m_btns[i] |= 2;
  }

  void setColor(lv_obj_draw_part_dsc_t* dsc)
  {
    uint8_t state = getBtnState((uint8_t)dsc->id);
    dsc->rect_dsc->bg_color = bg_color[state];
    dsc->label_dsc->color = fg_color[state];
  }

 protected:
  uint8_t m_channel = 0;
  std::function<void(int)> m_setValue;
  uint8_t m_btns[USBJ_BUTTON_SIZE];
  lv_color_t bg_color[4];
  lv_color_t fg_color[4];
};

static void btnsel_event_cb(lv_event_t* e)
{
  lv_event_code_t code = lv_event_get_code(e);

  if (code == LV_EVENT_DRAW_PART_BEGIN) {
    lv_obj_draw_part_dsc_t* dsc = lv_event_get_draw_part_dsc(e);

    if (dsc->class_p == &lv_btnmatrix_class &&
        dsc->type == LV_BTNMATRIX_DRAW_PART_BTN) {
      auto btsel = (USBChannelButtonSel*)lv_event_get_user_data(e);
      btsel->setColor(dsc);
    }
  }
}

#define USBCH_LINES 3
#define SET_VALUE_WUPDATE(value) \
  [=](int32_t newValue) {        \
    value = newValue;            \
    SET_DIRTY();                 \
    this->update();              \
  }

class USBChannelEditWindow : public Page
{
 public:
  USBChannelEditWindow(uint8_t channel) : Page(ICON_MODEL_USB, PAD_TINY), channel(channel)
  {
    body->padLeft(PAD_MEDIUM);
    body->padRight(PAD_MEDIUM);

    buildHeader(header);
    buildBody(body);
  }

  static LAYOUT_VAL(USBCH_EDIT_STATUS_BAR_WIDTH, 250, 160)
  static LAYOUT_VAL(USBCH_EDIT_RIGHT_MARGIN, 0, 3)
  static LAYOUT_VAL(USBCH_COLS, 4, 2)

 protected:
  uint8_t channel;
  USBChannelEditStatusBar* statusBar = nullptr;
  Window* m_btnModeFrame = nullptr;
  Window* m_axisModeLine = nullptr;
  Window* m_simModeLine = nullptr;
  USBChannelButtonSel* _BtnNumSel = nullptr;
  StaticText* collisionText = nullptr;
  Choice* m_btnPosChoice = nullptr;

  void update()
  {
    USBJoystickChData* cch = usbJChAddress(channel);

    m_btnModeFrame->show(cch->mode == USBJOYS_CH_BUTTON);
    m_axisModeLine->show(cch->mode == USBJOYS_CH_AXIS);
    m_simModeLine->show(cch->mode == USBJOYS_CH_SIM);

    if (m_btnPosChoice)
      m_btnPosChoice->enable((cch->param != USBJOYS_BTN_MODE_SW_EMU) &&
                             (cch->param != USBJOYS_BTN_MODE_DELTA));

    if (collisionText) {
      collisionText->setText("");
      collisionText->hide();
      if (cch->mode == USBJOYS_CH_BUTTON) {
        if (isUSBBtnNumCollision(channel)) {
          collisionText->setText(STR_USBJOYSTICK_BTN_COLLISION);
          collisionText->show();
        }
      } else if (cch->mode == USBJOYS_CH_AXIS) {
        if (isUSBAxisCollision(channel)) {
          collisionText->setText(STR_USBJOYSTICK_AXIS_COLLISION);
          collisionText->show();
        }
      } else if (cch->mode == USBJOYS_CH_SIM) {
        if (isUSBSimCollision(channel)) {
          collisionText->setText(STR_USBJOYSTICK_AXIS_COLLISION);
          collisionText->show();
        }
      }
    }

    _BtnNumSel->updateState();
  }

  void buildHeader(Window* window)
  {
    header->setTitle(STR_USBJOYSTICK_LABEL);
    header->setTitle2(getSourceString(MIXSRC_FIRST_CH + channel));

    statusBar = new USBChannelEditStatusBar(
        window,
        {window->getRect().w - USBCH_EDIT_STATUS_BAR_WIDTH -
             USBCH_EDIT_RIGHT_MARGIN,
         0, USBCH_EDIT_STATUS_BAR_WIDTH, EdgeTxStyles::MENU_HEADER_HEIGHT},
        channel);
  }

  void buildBody(Window* form)
  {
    FlexGridLayout grid(ch_col_dsc, row_dsc, PAD_TINY);
    form->setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_ZERO);

    USBJoystickChData* cch = usbJChAddress(channel);

    auto line = form->newLine(grid);

    new StaticText(line, rect_t{}, STR_USBJOYSTICK_CH_MODE);
    new Choice(line, rect_t{}, STR_VUSBJOYSTICK_CH_MODE, 0, USBJOYS_CH_LAST,
               GET_DEFAULT(cch->mode), SET_VALUE_WUPDATE(cch->mode));

#if PORTRAIT_LCD
    line = form->newLine(grid);
#endif

    new StaticText(line, rect_t{}, STR_USBJOYSTICK_CH_INVERSION);
    new ToggleSwitch(line, rect_t{}, GET_SET_DEFAULT(cch->inversion));

    line = form->newLine(grid);
    m_btnModeFrame = new Window(line, rect_t{});
    m_btnModeFrame->setFlexLayout();

    line = m_btnModeFrame->newLine(grid);
    new StaticText(line, rect_t{}, STR_USBJOYSTICK_CH_BTNMODE);
    new Choice(line, rect_t{}, STR_VUSBJOYSTICK_CH_BTNMODE, 0,
               USBJOYS_BTN_MODE_LAST, GET_DEFAULT(cch->param),
               [=](int32_t newValue) {
                 cch->param = newValue;
                 if (cch->param == USBJOYS_BTN_MODE_SW_EMU)
                   m_btnPosChoice->setValue(0);
                 else if (cch->param == USBJOYS_BTN_MODE_DELTA)
                   m_btnPosChoice->setValue(1);
                 SET_DIRTY();
                 this->update();
               });

#if PORTRAIT_LCD
    line = m_btnModeFrame->newLine(grid);
#endif

    new StaticText(line, rect_t{}, STR_USBJOYSTICK_CH_SWPOS);
    m_btnPosChoice = new Choice(line, rect_t{}, STR_VUSBJOYSTICK_CH_SWPOS, 0, 7,
                                GET_DEFAULT(cch->switch_npos),
                                SET_VALUE_WUPDATE(cch->switch_npos));

    line = m_btnModeFrame->newLine(grid);
    new StaticText(line, rect_t{}, STR_USBJOYSTICK_CH_BTNNUM);
#if PORTRAIT_LCD
    line = m_btnModeFrame->newLine(grid);
#endif
    _BtnNumSel = new USBChannelButtonSel(line, rect_t{}, channel,
                                         SET_VALUE_WUPDATE(cch->btn_num));

    m_axisModeLine = form->newLine(grid);
    new StaticText(m_axisModeLine, rect_t{}, STR_USBJOYSTICK_CH_AXIS);
    new Choice(m_axisModeLine, rect_t{}, STR_VUSBJOYSTICK_CH_AXIS, 0,
               USBJOYS_AXIS_LAST, GET_DEFAULT(cch->param),
               SET_VALUE_WUPDATE(cch->param));

    m_simModeLine = form->newLine(grid);
    new StaticText(m_simModeLine, rect_t{}, STR_USBJOYSTICK_CH_SIM);
    new Choice(m_simModeLine, rect_t{}, STR_VUSBJOYSTICK_CH_SIM, 0,
               USBJOYS_SIM_LAST, GET_DEFAULT(cch->param),
               SET_VALUE_WUPDATE(cch->param));

    line = form->newLine(grid);
    line->padTop(PAD_ZERO);
    line->padBottom(PAD_ZERO);
    collisionText =
        new StaticText(line, rect_t{}, "",
                       COLOR_THEME_PRIMARY2_INDEX, FONT(BOLD) | CENTERED);
    etx_bg_color(collisionText->getLvObj(), COLOR_THEME_WARNING_INDEX);
    lv_obj_set_grid_cell(collisionText->getLvObj(), LV_GRID_ALIGN_STRETCH, 0,
                         USBCH_COLS, LV_GRID_ALIGN_CENTER, 0, 1);

    update();
  }
};

class USBChannelLineButton : public ListLineButton
{
 public:
  USBChannelLineButton(Window* parent, uint8_t index) :
      ListLineButton(parent, index)
  {
    setHeight(USBCH_LINE_HEIGHT);
#if !PORTRAIT_LCD
    padTop(PAD_SMALL);
#endif

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
        line->delayed_init();
      line->refresh();
    }
  }

  void delayed_init()
  {
    init = true;

    m_chn = lv_label_create(lvobj);
    lv_obj_set_grid_cell(m_chn, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_CENTER,
                         0, USBCH_CHN_ROWS);

    m_inverse =
        new StaticIcon(this, 0, 0, ICON_CHAN_MONITOR_INVERTED,
                         COLOR_THEME_SECONDARY1_INDEX);
    lv_obj_set_grid_cell(m_inverse->getLvObj(), LV_GRID_ALIGN_START, 1, 1,
                         LV_GRID_ALIGN_CENTER, 0, 1);

    m_mode = lv_label_create(lvobj);
    lv_obj_set_grid_cell(m_mode, LV_GRID_ALIGN_START, 2, 1,
                         LV_GRID_ALIGN_CENTER, 0, 1);

    m_param = lv_label_create(lvobj);
    etx_txt_color(m_param, COLOR_THEME_WARNING_INDEX, ETX_STATE_COLLISION_WARN);
    etx_font(m_param, FONT_BOLD_INDEX, ETX_STATE_COLLISION_WARN);
    lv_obj_set_grid_cell(m_param, LV_GRID_ALIGN_START, 3, 1,
                         LV_GRID_ALIGN_CENTER, 0, 1);

    m_btn_mode = lv_label_create(lvobj);
    lv_obj_set_grid_cell(m_btn_mode, LV_GRID_ALIGN_START, USBCH_BTN_MODE_COL, 1,
                         LV_GRID_ALIGN_CENTER, USBCH_BTN_MODE_ROW, 1);

    m_btns = lv_label_create(lvobj);
    lv_obj_set_grid_cell(m_btns, LV_GRID_ALIGN_START, USBCH_BTN_MODE_COL + 1, 1,
                         LV_GRID_ALIGN_CENTER, USBCH_BTN_MODE_ROW, 1);

    lv_label_set_text(m_chn, getSourceString(MIXSRC_FIRST_CH + index));
    lv_label_set_text(m_mode, "");
    lv_label_set_text(m_param, "");
    lv_label_set_text(m_btn_mode, "");
    lv_label_set_text(m_btns, "");

    lv_obj_update_layout(lvobj);
  }

  void refresh() override
  {
    if (!init) return;

    USBJoystickChData* cch = usbJChAddress(index);

    lv_label_set_text(m_mode, STR_VUSBJOYSTICK_CH_MODE[cch->mode]);

    m_inverse->show(cch->inversion);

    const char* param = "";
    bool hasCollision = false;
  
    if (cch->mode == USBJOYS_CH_BUTTON) {
      param = STR_VUSBJOYSTICK_CH_BTNMODE[cch->param];
    } else if (cch->mode == USBJOYS_CH_AXIS) {
      param = STR_VUSBJOYSTICK_CH_AXIS[cch->param];
      if (isUSBAxisCollision(index)) {
        hasCollision = true;
      }
    } else if (cch->mode == USBJOYS_CH_SIM) {
      param = STR_VUSBJOYSTICK_CH_SIM[cch->param];
      if (isUSBSimCollision(index)) {
        hasCollision = true;
      }
    }

    lv_label_set_text(m_param, param);
    if (hasCollision)
      lv_obj_add_state(m_param, ETX_STATE_COLLISION_WARN);
    else
      lv_obj_clear_state(m_param, ETX_STATE_COLLISION_WARN);

    if (cch->mode == USBJOYS_CH_BUTTON) {
      lv_label_set_text(m_btn_mode,
                        STR_VUSBJOYSTICK_CH_SWPOS[cch->switch_npos]);

      char str[20];

      uint8_t last = cch->lastBtnNum();
      if (last > cch->btn_num)
        snprintf(str, 20, "%u..%u", cch->btn_num, last);
      else
        snprintf(str, 20, "%u", cch->btn_num);
      lv_label_set_text(m_btns, str);
      if (isUSBBtnNumCollision(index))
        lv_obj_add_state(m_param, ETX_STATE_COLLISION_WARN);
      else
        lv_obj_clear_state(m_param, ETX_STATE_COLLISION_WARN);
    } else {
      lv_label_set_text(m_btn_mode, "");
      lv_label_set_text(m_btns, "");
    }
  }

  bool isActive() const override { return false; }

  static LAYOUT_VAL(USBCH_LINE_HEIGHT, 32, 48)
  static LAYOUT_VAL(USBCH_CHN_ROWS, 1, 2)
  static LAYOUT_VAL(USBCH_BTN_MODE_COL, 4, 2)
  static LAYOUT_VAL(USBCH_BTN_MODE_ROW, 0, 1)

 protected:
  bool init = false;

  lv_obj_t* m_chn;
  lv_obj_t* m_mode;
  lv_obj_t* m_param;
  lv_obj_t* m_btn_mode;
  lv_obj_t* m_btns;
  StaticIcon* m_inverse;
};

ModelUSBJoystickPage::ModelUSBJoystickPage() : Page(ICON_MODEL_USB)
{
  header->setTitle(STR_MENU_MODEL_SETUP);
  header->setTitle2(STR_USBJOYSTICK_LABEL);

  body->setFlexLayout();
  FlexGridLayout grid(line_col_dsc, row_dsc, PAD_TINY);

  // Extended mode
  auto line = body->newLine(grid);
  new StaticText(line, rect_t{}, STR_USBJOYSTICK_EXTMODE);
  new Choice(line, rect_t{}, STR_VUSBJOYSTICK_EXTMODE, 0, 1,
             GET_DEFAULT(g_model.usbJoystickExtMode),
             SET_VALUE_WUPDATE(g_model.usbJoystickExtMode));

#if PORTRAIT_LCD
  line = body->newLine(grid);
#endif

  _IfModeLabel = new StaticText(line, rect_t{}, STR_USBJOYSTICK_IF_MODE);
  _IfMode = new Choice(line, rect_t{}, STR_VUSBJOYSTICK_IF_MODE, 0,
                       USBJOYS_LAST, GET_DEFAULT(g_model.usbJoystickIfMode),
                       SET_VALUE_WUPDATE(g_model.usbJoystickIfMode));

  line = body->newLine(grid);

  _CircCoutoutLabel = new StaticText(
      line, rect_t{}, STR_USBJOYSTICK_CIRC_COUTOUT);
  _CircCoutout =
      new Choice(line, rect_t{}, STR_VUSBJOYSTICK_CIRC_COUTOUT, 0, USBJOYS_LAST,
                 GET_DEFAULT(g_model.usbJoystickCircularCut),
                 SET_VALUE_WUPDATE(g_model.usbJoystickCircularCut));

#if PORTRAIT_LCD
  line = body->newLine(grid);
#endif

  _ApplyBtn =
      new TextButton(line, rect_t{}, STR_USBJOYSTICK_APPLY_CHANGES, [=]() {
        onUSBJoystickModelChanged();
        this->update();
        return 0;
      });

  auto btngrp = new Window(body, rect_t{});
  btngrp->padAll(PAD_TINY);
  _ChannelsGroup = btngrp;
  btngrp->setFlexLayout();
  btngrp->padRow(PAD_SMALL);
  for (uint8_t ch = 0; ch < USBJ_MAX_JOYSTICK_CHANNELS; ch++) {
    // Channel settings
    auto btn = new USBChannelLineButton(btngrp, ch);

    USBJoystickChData* cch = usbJChAddress(ch);
    btn->setPressHandler([=]() -> uint8_t {
      if (cch->mode == USBJOYS_CH_NONE) {
        editChannel(ch, btn);
      } else {
        Menu* menu = new Menu();
        menu->addLine(STR_EDIT, [=]() { editChannel(ch, btn); });
        menu->addLine(STR_CLEAR, [=]() {
          memset(cch, 0, sizeof(USBJoystickChData));
          SET_DIRTY();
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
  Window* ctrls[usbj_ctrls] = {_IfModeLabel, _IfMode,   _CircCoutoutLabel,
                               _CircCoutout, _ApplyBtn, _ChannelsGroup};

  for (uint8_t i = 0; i < usbj_ctrls; i++) {
    ctrls[i]->show(usbJoystickExtMode());
  }

  _ApplyBtn->enable(usbJoystickSettingsChanged());
}

void ModelUSBJoystickPage::editChannel(uint8_t channel,
                                       USBChannelLineButton* btn)
{
  auto chedit = new USBChannelEditWindow(channel);
  chedit->setCloseHandler([=]() { this->update(); });
}
