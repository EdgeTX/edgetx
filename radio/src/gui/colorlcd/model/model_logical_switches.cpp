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

#include "model_logical_switches.h"

#include "libopenui.h"
#include "list_line_button.h"
#include "edgetx.h"
#include "page.h"
#include "sourcechoice.h"
#include "switchchoice.h"
#include "switches.h"
#include "etx_lv_theme.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

#define ETX_STATE_LS_ACTIVE LV_STATE_USER_1
#define ETX_STATE_V1_SMALL_FONT LV_STATE_USER_1

static const lv_coord_t col_dsc[] = {LV_GRID_FR(2), LV_GRID_FR(3),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t col_dsc2[] = {LV_GRID_FR(4), LV_GRID_FR(3),
                                      LV_GRID_FR(3), LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

class LogicalSwitchEditPage : public Page
{
 public:
  explicit LogicalSwitchEditPage(uint8_t index) :
      Page(ICON_MODEL_LOGICAL_SWITCHES, PAD_ZERO), index(index)
  {
    buildHeader(header);
    buildBody(body);
  }

 protected:
  uint8_t index;
  bool active = false;
  Window* logicalSwitchOneWindow = nullptr;
  StaticText* headerSwitchName = nullptr;
  NumberEdit* v2Edit = nullptr;

  bool isActive() const
  {
    return getSwitch(SWSRC_FIRST_LOGICAL_SWITCH + index);
  }

  void checkEvents() override
  {
    Page::checkEvents();
    if (active != isActive()) {
      if (isActive()) {
        lv_obj_add_state(headerSwitchName->getLvObj(), ETX_STATE_LS_ACTIVE);
      } else {
        lv_obj_clear_state(headerSwitchName->getLvObj(), ETX_STATE_LS_ACTIVE);
      }
      active = isActive();
    }
  }

  void buildHeader(Window* window)
  {
    header->setTitle(STR_MENULOGICALSWITCHES);
    headerSwitchName = header->setTitle2(
        getSwitchPositionName(SWSRC_FIRST_LOGICAL_SWITCH + index));

    etx_txt_color(headerSwitchName->getLvObj(), COLOR_THEME_ACTIVE_INDEX,
                  ETX_STATE_LS_ACTIVE);
    etx_font(headerSwitchName->getLvObj(), FONT_BOLD_INDEX, ETX_STATE_LS_ACTIVE);
  }

  void getV2Range(LogicalSwitchData* cs, int16_t& v2_min, int16_t& v2_max)
  {
    getMixSrcRange(cs->v1, v2_min, v2_max);
    if ((cs->func == LS_FUNC_APOS) || (cs->func == LS_FUNC_ANEG) ||
        (cs->func == LS_FUNC_ADIFFEGREATER))
      v2_min = 0;
  }

  void updateLogicalSwitchOneWindow()
  {
    SwitchChoice* choice;
    NumberEdit* timer;

    logicalSwitchOneWindow->clear();
    logicalSwitchOneWindow->setFlexLayout();
    FlexGridLayout grid(col_dsc, row_dsc, PAD_TINY);
    FlexGridLayout grid2(col_dsc2, row_dsc, PAD_TINY);

    LogicalSwitchData* cs = lswAddress(index);
    uint8_t cstate = lswFamily(cs->func);

    // V1
    auto line = logicalSwitchOneWindow->newLine(grid);
    new StaticText(line, rect_t{}, STR_V1);
    switch (cstate) {
      case LS_FAMILY_BOOL:
      case LS_FAMILY_STICKY:
      case LS_FAMILY_EDGE:
        choice = new SwitchChoice(
            line, rect_t{}, SWSRC_FIRST_IN_LOGICAL_SWITCHES,
            SWSRC_LAST_IN_LOGICAL_SWITCHES, GET_SET_DEFAULT(cs->v1));
        choice->setAvailableHandler(isSwitchAvailableInLogicalSwitches);
        break;
      case LS_FAMILY_COMP:
        new SourceChoice(line, rect_t{}, 0, MIXSRC_LAST_TELEM,
                         GET_SET_DEFAULT(cs->v1), true);
        break;
      case LS_FAMILY_TIMER:
        timer =
            new NumberEdit(line, rect_t{}, -128, 122, GET_SET_DEFAULT(cs->v1));
        timer->setDisplayHandler([](int32_t value) {
          return formatNumberAsString(lswTimerValue(value), PREC1, 0, nullptr,
                                      "s");
        });
        break;
      default:
        new SourceChoice(line, rect_t{}, 0, MIXSRC_LAST_TELEM,
                         GET_DEFAULT(cs->v1), [=](int32_t newValue) {
                           cs->v1 = newValue;
                           if (v2Edit != nullptr) {
                             int16_t v2_min = 0, v2_max = 0;
                             getV2Range(cs, v2_min, v2_max);
                             v2Edit->setMin(v2_min);
                             v2Edit->setMax(v2_max);
                             v2Edit->setValue(cs->v2);
                           }
                           SET_DIRTY();
                         }, true);
        break;
    }

    // V2
    if (cstate == LS_FAMILY_EDGE) {
      line = logicalSwitchOneWindow->newLine(grid2);
    } else {
      line = logicalSwitchOneWindow->newLine(grid);
    }
    new StaticText(line, rect_t{}, STR_V2);
    switch (cstate) {
      case LS_FAMILY_BOOL:
      case LS_FAMILY_STICKY:
        choice = new SwitchChoice(
            line, rect_t{}, SWSRC_FIRST_IN_LOGICAL_SWITCHES,
            SWSRC_LAST_IN_LOGICAL_SWITCHES, GET_SET_DEFAULT(cs->v2));
        choice->setAvailableHandler(isSwitchAvailableInLogicalSwitches);
        break;
      case LS_FAMILY_EDGE: {
        auto edit1 =
            new NumberEdit(line, rect_t{}, -129, 122, GET_DEFAULT(cs->v2));
        auto edit2 = new NumberEdit(line, rect_t{}, -1, 222 - cs->v2,
                                    GET_SET_DEFAULT(cs->v3));
        edit1->setSetValueHandler([=](int32_t newValue) {
          cs->v2 = newValue;
          SET_DIRTY();
          edit2->setMax(222 - cs->v2);
          edit2->setValue(cs->v3);
        });
        edit1->setDisplayHandler([](int32_t value) {
          return formatNumberAsString(lswTimerValue(value), PREC1, 0, nullptr,
                                      "s");
        });
        edit2->setDisplayHandler([cs](int32_t value) {
          if (value < 0)
            return std::string("<<");
          else if (value == 0)
            return std::string("--");
          else {
            return formatNumberAsString(lswTimerValue(cs->v2 + value), PREC1, 0,
                                        nullptr, "s");
          }
        });
      } break;
      case LS_FAMILY_COMP:
        new SourceChoice(line, rect_t{}, 0, MIXSRC_LAST_TELEM,
                         GET_SET_DEFAULT(cs->v2), true);
        break;
      case LS_FAMILY_TIMER:
        timer =
            new NumberEdit(line, rect_t{}, -128, 122, GET_SET_DEFAULT(cs->v2));
        timer->setDisplayHandler([](int32_t value) {
          return formatNumberAsString(lswTimerValue(value), PREC1, 0, nullptr,
                                      "s");
        });
        break;
      default:
        int16_t v2_min = 0, v2_max = 0;
        getV2Range(cs, v2_min, v2_max);
        v2Edit = new NumberEdit(line, rect_t{}, v2_min, v2_max,
                                GET_SET_DEFAULT(cs->v2));

        v2Edit->setDisplayHandler([=](int value) -> std::string {
          if (abs(cs->v1) <= MIXSRC_LAST_CH) value = calc100toRESX(value);
          std::string txt = getSourceCustomValueString(cs->v1, value, 0);
          return txt;
        });
        break;
    }

    // AND switch
    line = logicalSwitchOneWindow->newLine(grid);
    new StaticText(line, rect_t{}, STR_AND_SWITCH);
    choice = new SwitchChoice(line, rect_t{}, -MAX_LS_ANDSW, MAX_LS_ANDSW,
                              GET_SET_DEFAULT(cs->andsw));
    choice->setAvailableHandler(isSwitchAvailableInLogicalSwitches);

    // Duration
    line = logicalSwitchOneWindow->newLine(grid);
    new StaticText(line, rect_t{}, STR_DURATION);
    auto edit = new NumberEdit(line, rect_t{}, 0, MAX_LS_DURATION,
                               GET_SET_DEFAULT(cs->duration), PREC1);
    edit->setZeroText("---");
    edit->setDisplayHandler([](int32_t value) {
      return formatNumberAsString(value, PREC1, 0, nullptr, "s");
    });

    // Delay
    line = logicalSwitchOneWindow->newLine(grid);
    new StaticText(line, rect_t{}, STR_DELAY);
    if (cstate == LS_FAMILY_EDGE) {
      new StaticText(line, rect_t{}, STR_NA);
    } else {
      auto edit = new NumberEdit(line, rect_t{}, 0, MAX_LS_DELAY,
                                 GET_SET_DEFAULT(cs->delay), PREC1);
      edit->setDisplayHandler([](int32_t value) {
        if (value == 0) return std::string("---");
        return formatNumberAsString(value, PREC1, 0, nullptr, "s");
      });
    }

    // Sticky persist
    if (cstate == LS_FAMILY_STICKY) {
      line = logicalSwitchOneWindow->newLine(grid);
      new StaticText(line, rect_t{}, STR_PERSISTENT);
      new ToggleSwitch(line, rect_t{}, GET_SET_DEFAULT(cs->lsPersist));
    }
  }

  void buildBody(Window* window)
  {
    window->setFlexLayout();
    window->padLeft(PAD_SMALL);
    window->padRight(PAD_SMALL);

    FlexGridLayout grid(col_dsc, row_dsc, PAD_TINY);

    LogicalSwitchData* cs = lswAddress(index);

    // LS Func
    auto line = window->newLine(grid);
    new StaticText(line, rect_t{}, STR_FUNC);
    auto functionChoice = new Choice(line, rect_t{}, STR_VCSWFUNC, 0,
                                     LS_FUNC_MAX, GET_DEFAULT(cs->func));
    functionChoice->setSetValueHandler([=](int32_t newValue) {
      cs->func = newValue;
      if (lswFamily(cs->func) == LS_FAMILY_TIMER) {
        cs->v1 = cs->v2 = 0;
      } else if (lswFamily(cs->func) == LS_FAMILY_EDGE) {
        cs->v1 = 0;
        cs->v2 = -129;
        cs->v3 = 0;
      } else {
        cs->v1 = cs->v2 = 0;
      }
      SET_DIRTY();
      updateLogicalSwitchOneWindow();
    });

    logicalSwitchOneWindow = new Window(window, rect_t{});
    updateLogicalSwitchOneWindow();
  }
};

void getsEdgeDelayParam(char* s, LogicalSwitchData* ls)
{
  sprintf(s, "[%s:%s]",
          formatNumberAsString(lswTimerValue(ls->v2), PREC1, 0, nullptr, "s")
              .c_str(),
          (ls->v3 < 0)    ? "<<"
          : (ls->v3 == 0) ? "--"
                          : formatNumberAsString(lswTimerValue(ls->v2 + ls->v3),
                                                 PREC1, 0, nullptr, "s")
                                .c_str());
}

class LogicalSwitchButton : public ListLineButton
{
 public:
  LogicalSwitchButton(Window* parent, int lsIndex) :
      ListLineButton(parent, lsIndex)
  {
    setHeight(LS_BUTTON_H);
    padAll(PAD_ZERO);

    check(isActive());

    lv_obj_add_event_cb(lvobj, LogicalSwitchButton::on_draw,
                        LV_EVENT_DRAW_MAIN_BEGIN, nullptr);
  }

  static void on_draw(lv_event_t* e)
  {
    lv_obj_t* target = lv_event_get_target(e);
    auto line = (LogicalSwitchButton*)lv_obj_get_user_data(target);
    if (line) {
      if (!line->init)
        line->delayed_init();
      line->refresh();
    }
  }

  void delayed_init()
  {
    init = true;

    lv_obj_enable_style_refresh(false);

    lsName = lv_label_create(lvobj);
    etx_obj_add_style(lsName, styles->text_align_left, LV_PART_MAIN);
    lv_obj_set_pos(lsName, NM_X, NM_Y);
    lv_obj_set_size(lsName, NM_W, NM_H);

    lsFunc = lv_label_create(lvobj);
    etx_obj_add_style(lsFunc, styles->text_align_left, LV_PART_MAIN);
    lv_obj_set_pos(lsFunc, FN_X, FN_Y);
    lv_obj_set_size(lsFunc, FN_W, FN_H);
    lv_obj_set_style_text_font(lsFunc, getFont(FONT(BOLD)), LV_STATE_USER_1);

    lsV1 = lv_label_create(lvobj);
    etx_obj_add_style(lsV1, styles->text_align_center, LV_PART_MAIN);
    etx_font(lsV1, FONT_XS_INDEX, ETX_STATE_V1_SMALL_FONT);
    lv_obj_set_pos(lsV1, V1_X, V1_Y);
    lv_obj_set_size(lsV1, V1_W, V1_H);
    lv_obj_set_style_text_font(lsV1, getFont(FONT(BOLD)), LV_STATE_USER_1);

    lsV2 = lv_label_create(lvobj);
    etx_obj_add_style(lsV2, styles->text_align_center, LV_PART_MAIN);
    lv_obj_set_pos(lsV2, V2_X, V2_Y);
    lv_obj_set_size(lsV2, V2_W, V2_H);
    lv_obj_set_style_text_font(lsV2, getFont(FONT(BOLD)), LV_STATE_USER_1);

    lsAnd = lv_label_create(lvobj);
    etx_obj_add_style(lsAnd, styles->text_align_center, LV_PART_MAIN);
    lv_obj_set_pos(lsAnd, AND_X, AND_Y);
    lv_obj_set_size(lsAnd, AND_W, AND_H);
    lv_obj_set_style_text_font(lsAnd, getFont(FONT(BOLD)), LV_STATE_USER_1);

    lsDuration = lv_label_create(lvobj);
    etx_obj_add_style(lsDuration, styles->text_align_center, LV_PART_MAIN);
    lv_obj_set_pos(lsDuration, DUR_X, DUR_Y);
    lv_obj_set_size(lsDuration, DUR_W, DUR_H);

    lsDelay = lv_label_create(lvobj);
    etx_obj_add_style(lsDelay, styles->text_align_center, LV_PART_MAIN);
    lv_obj_set_pos(lsDelay, DEL_X, DEL_Y);
    lv_obj_set_size(lsDelay, DEL_W, DEL_H);

    lv_obj_update_layout(lvobj);

    lv_obj_enable_style_refresh(true);
    lv_obj_refresh_style(lvobj, LV_PART_ANY, LV_STYLE_PROP_ANY);
  }

  bool isActive() const override
  {
    return getSwitch(SWSRC_FIRST_LOGICAL_SWITCH + index);
  }

  void checkEvents() override
  {
    ListLineButton::checkEvents();
    check(isActive());

    LogicalSwitchData* ls = lswAddress(index);
    uint8_t lsFamily = lswFamily(ls->func);

    if (lsFamily == LS_FAMILY_STICKY && getLSStickyState(index))
      lv_obj_add_state(lsFunc, LV_STATE_USER_1);
    else
      lv_obj_clear_state(lsFunc, LV_STATE_USER_1);

    if ((lsFamily == LS_FAMILY_BOOL || lsFamily == LS_FAMILY_EDGE || lsFamily == LS_FAMILY_STICKY) && getSwitch(ls->v1))
      lv_obj_add_state(lsV1, LV_STATE_USER_1);
    else
      lv_obj_clear_state(lsV1, LV_STATE_USER_1);

    if ((lsFamily == LS_FAMILY_BOOL || lsFamily == LS_FAMILY_STICKY) && getSwitch(ls->v2))
      lv_obj_add_state(lsV2, LV_STATE_USER_1);
    else
      lv_obj_clear_state(lsV2, LV_STATE_USER_1);

    if (getSwitch(ls->andsw))
      lv_obj_add_state(lsAnd, LV_STATE_USER_1);
    else
      lv_obj_clear_state(lsAnd, LV_STATE_USER_1);
  }

  void refresh() override
  {
    if (!init) return;

    char s[20];

    LogicalSwitchData* ls = lswAddress(index);
    uint8_t lsFamily = lswFamily(ls->func);

    lv_label_set_text(
        lsName, getSwitchPositionName(SWSRC_FIRST_LOGICAL_SWITCH + index));
    lv_label_set_text(lsFunc, STR_VCSWFUNC[ls->func]);

    // CSW params - V1
    switch (lsFamily) {
      case LS_FAMILY_BOOL:
      case LS_FAMILY_STICKY:
      case LS_FAMILY_EDGE:
        lv_label_set_text(lsV1, getSwitchPositionName(ls->v1));
        break;
      case LS_FAMILY_TIMER:
        lv_label_set_text(lsV1, formatNumberAsString(lswTimerValue(ls->v1),
                                                     PREC1, 0, nullptr, "s")
                                    .c_str());
        break;
      default: {
        char* s = getSourceString(ls->v1);
        if (getTextWidth(s, 0, FONT(STD)) > 88)
          lv_obj_add_state(lsV1, ETX_STATE_V1_SMALL_FONT);
        else
          lv_obj_clear_state(lsV1, ETX_STATE_V1_SMALL_FONT);
        lv_label_set_text(lsV1, s);
      } break;
    }

    // CSW params - V2
    strcat(s, " ");
    switch (lsFamily) {
      case LS_FAMILY_BOOL:
      case LS_FAMILY_STICKY:
        lv_label_set_text(lsV2, getSwitchPositionName(ls->v2));
        break;
      case LS_FAMILY_EDGE:
        getsEdgeDelayParam(s, ls);
        lv_label_set_text(lsV2, s);
        break;
      case LS_FAMILY_TIMER:
        lv_label_set_text(lsV2, formatNumberAsString(lswTimerValue(ls->v2),
                                                     PREC1, 0, nullptr, "s")
                                    .c_str());
        break;
      case LS_FAMILY_COMP:
        lv_label_set_text(lsV2, getSourceString(ls->v2));
        break;
      default:
        lv_label_set_text(
            lsV2,
            getSourceCustomValueString(
                ls->v1,
                (ls->v1 <= MIXSRC_LAST_CH ? calc100toRESX(ls->v2) : ls->v2),
                0));
        break;
    }

    // AND switch
    lv_label_set_text(lsAnd, getSwitchPositionName(ls->andsw));

    // CSW duration
    if (ls->duration > 0) {
      lv_label_set_text(
          lsDuration,
          formatNumberAsString(ls->duration, PREC1, 0, nullptr, "s").c_str());
    } else {
      lv_label_set_text(lsDuration, "");
    }

    // CSW delay
    if (lsFamily != LS_FAMILY_EDGE && ls->delay > 0) {
      lv_label_set_text(
          lsDelay,
          formatNumberAsString(ls->delay, PREC1, 0, nullptr, "s").c_str());
    } else {
      lv_label_set_text(lsDelay, "");
    }
  }

  static LAYOUT_VAL(LS_BUTTON_H, 32, 44)

  static constexpr coord_t NM_X = PAD_TINY;
  static LAYOUT_VAL(NM_Y, 4, 10)
  static LAYOUT_VAL(NM_W, 30, 36)
  static LAYOUT_VAL(NM_H, 20, 20)
  static constexpr coord_t FN_X = NM_X + NM_W + PAD_TINY;
  static constexpr coord_t FN_Y = NM_Y;
  static LAYOUT_VAL(FN_W, 50, 58)
  static constexpr coord_t FN_H = NM_H;
  static constexpr coord_t V1_X = FN_X + FN_W + PAD_TINY;
  static LAYOUT_VAL(V1_Y, NM_Y, 0)
  static LAYOUT_VAL(V1_W, 88, 88)
  static constexpr coord_t V1_H = NM_H;
  static constexpr coord_t V2_X = V1_X + V1_W + PAD_TINY;
  static constexpr coord_t V2_Y = V1_Y;
  static LAYOUT_VAL(V2_W, 110, 110)
  static constexpr coord_t V2_H = NM_H;
  static LAYOUT_VAL(AND_X, V2_X + V2_W + PAD_TINY, FN_X + FN_W + PAD_TINY)
  static LAYOUT_VAL(AND_Y, NM_Y, 20)
  static constexpr coord_t AND_W = V1_W;
  static constexpr coord_t AND_H = NM_H;
  static constexpr coord_t DUR_X = AND_X + AND_W + PAD_TINY;
  static constexpr coord_t DUR_Y = AND_Y;
  static LAYOUT_VAL(DUR_W, 40, 54)
  static constexpr coord_t DUR_H = NM_H;
  static constexpr coord_t DEL_X = DUR_X + DUR_W + PAD_TINY;
  static constexpr coord_t DEL_Y = AND_Y;
  static constexpr coord_t DEL_H = NM_H;
  static constexpr coord_t DEL_W = DUR_W;

 protected:
  bool init = false;

  lv_obj_t* lsName = nullptr;
  lv_obj_t* lsFunc = nullptr;
  lv_obj_t* lsV1 = nullptr;
  lv_obj_t* lsV2 = nullptr;
  lv_obj_t* lsAnd = nullptr;
  lv_obj_t* lsDuration = nullptr;
  lv_obj_t* lsDelay = nullptr;
};

ModelLogicalSwitchesPage::ModelLogicalSwitchesPage() :
    PageTab(STR_MENULOGICALSWITCHES, ICON_MODEL_LOGICAL_SWITCHES)
{
}

void ModelLogicalSwitchesPage::rebuild(Window* window)
{
  // When window.clear() is called the last button on screen is given focus
  // (???) This causes the page to jump to the end when rebuilt. Set flag to
  // bypass the button focus handler and reset focusIndex when rebuilding
  isRebuilding = true;
  window->clear();
  build(window);
  isRebuilding = false;
}

void ModelLogicalSwitchesPage::newLS(Window* window, bool pasteLS)
{
  Menu* menu = new Menu();
  menu->setTitle(STR_MENU_LOGICAL_SWITCHES);

  // search for unused switches
  for (uint8_t i = 0; i < MAX_LOGICAL_SWITCHES; i++) {
    LogicalSwitchData* ls = lswAddress(i);
    if (ls->func == LS_FUNC_NONE) {
      std::string ch_name(
          getSwitchPositionName(SWSRC_FIRST_LOGICAL_SWITCH + i));
      menu->addLineBuffered(ch_name.c_str(), [=]() {
        if (pasteLS) {
          *ls = clipboard.data.csw;
          storageDirty(EE_MODEL);
          focusIndex = i;
          rebuild(window);
        } else {
          Window* lsWindow = new LogicalSwitchEditPage(i);
          lsWindow->setCloseHandler([=]() {
            if (ls->func != LS_FUNC_NONE) {
              focusIndex = i;
              rebuild(window);
            }
          });
        }
      });
    }
  }
  menu->updateLines();
}

void ModelLogicalSwitchesPage::plusPopup(Window* window)
{
  if (clipboard.type == CLIPBOARD_TYPE_CUSTOM_SWITCH) {
    Menu* menu = new Menu();
    menu->addLine(STR_NEW, [=]() { newLS(window, false); });
    menu->addLine(STR_PASTE, [=]() { newLS(window, true); });
  } else {
    newLS(window, false);
  }
}

void ModelLogicalSwitchesPage::build(Window* window)
{
  window->setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_TINY);

  bool hasEmptySwitch = false;

  // Reset focusIndex after switching tabs
  if (!isRebuilding) focusIndex = prevFocusIndex;

  for (uint8_t i = 0; i < MAX_LOGICAL_SWITCHES; i++) {
    LogicalSwitchData* ls = lswAddress(i);

    bool isActive = (ls->func != LS_FUNC_NONE);

    if (isActive) {
      auto button = new LogicalSwitchButton(window, i);

      button->setPressHandler([=]() {
        Menu* menu = new Menu();
        menu->addLine(STR_EDIT, [=]() {
          Window* lsWindow = new LogicalSwitchEditPage(i);
          lsWindow->setCloseHandler([=]() {
            if (!isActive)
              rebuild(window);
          });
        });
        if (isActive) {
          menu->addLine(STR_COPY, [=]() {
            clipboard.type = CLIPBOARD_TYPE_CUSTOM_SWITCH;
            clipboard.data.csw = *ls;
          });
        }
        if (clipboard.type == CLIPBOARD_TYPE_CUSTOM_SWITCH)
          menu->addLine(STR_PASTE, [=]() {
            *ls = clipboard.data.csw;
            storageDirty(EE_MODEL);
            rebuild(window);
          });
        if (isActive || ls->v1 || ls->v2 || ls->delay || ls->duration ||
            ls->andsw) {
          menu->addLine(STR_CLEAR, [=]() {
            memset(ls, 0, sizeof(LogicalSwitchData));
            storageDirty(EE_MODEL);
            rebuild(window);
          });
        }
        return 0;
      });

      if (focusIndex == i) {
        lv_group_focus_obj(button->getLvObj());
      }

      button->setLongPressHandler([=]() -> uint8_t {
        if (addButton) {
          lv_group_focus_obj(addButton->getLvObj());
          plusPopup(window);
        }
        return 0;
      });

      button->setFocusHandler([=](bool hasFocus) {
        if (hasFocus && !isRebuilding) {
          prevFocusIndex = focusIndex;
          focusIndex = i;
        }
      });
    } else {
      hasEmptySwitch = true;
    }
  }

  if (hasEmptySwitch) {
    addButton =
        new TextButton(window, rect_t{0, 0, window->width() - 12, LogicalSwitchButton::LS_BUTTON_H},
                       LV_SYMBOL_PLUS, [=]() {
                         plusPopup(window);
                         return 0;
                       });

    addButton->setLongPressHandler([=]() -> uint8_t {
      plusPopup(window);
      return 0;
    });

    addButton->setFocusHandler([=](bool hasFocus) {
      if (hasFocus && !isRebuilding) {
        prevFocusIndex = focusIndex;
      }
    });
  } else {
    addButton = nullptr;
  }
}
