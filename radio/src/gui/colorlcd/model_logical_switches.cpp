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
#include "opentx.h"
#include "libopenui.h"
#include "switches.h"

#define SET_DIRTY()     storageDirty(EE_MODEL)

static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(2),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t col_dsc2[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT,
                                     LV_GRID_TEMPLATE_LAST};

class LogicalSwitchEditPage: public Page
{
  public:
    explicit LogicalSwitchEditPage(uint8_t index):
      Page(ICON_MODEL_LOGICAL_SWITCHES),
      index(index)
    {
      buildHeader(&header);
      buildBody(&body);
    }

  protected:
    uint8_t index;
    bool active = false;
    FormGroup * logicalSwitchOneWindow = nullptr;
    StaticText * headerSwitchName = nullptr;
    NumberEdit * v2Edit = nullptr;

    bool isActive() const
    {
      return getSwitch(SWSRC_FIRST_LOGICAL_SWITCH + index);
    }

    void checkEvents() override
    {
      Page::checkEvents();
      if (active != isActive()) {
        if(isActive()) {
          lv_obj_add_state(headerSwitchName->getLvObj(), LV_STATE_USER_1);
        } else {
          lv_obj_clear_state(headerSwitchName->getLvObj(), LV_STATE_USER_1);
        }
        active = isActive();
        invalidate();
      }
    }

    void buildHeader(Window * window)
    {
      header.setTitle(STR_MENULOGICALSWITCHES);
      headerSwitchName = new StaticText(
          window,
          {PAGE_TITLE_LEFT, PAGE_TITLE_TOP + PAGE_LINE_HEIGHT,
           LCD_W - PAGE_TITLE_LEFT, PAGE_LINE_HEIGHT},
          getSwitchPositionName(SWSRC_SW1 + index), 0, COLOR_THEME_PRIMARY2);

      lv_obj_set_style_text_color(headerSwitchName->getLvObj(), makeLvColor(COLOR_THEME_ACTIVE), LV_STATE_USER_1);
      lv_obj_set_style_text_font(headerSwitchName->getLvObj(), getFont(FONT(BOLD)), LV_STATE_USER_1);
    }

    void updateLogicalSwitchOneWindow()
    {
      SwitchChoice* choice;
      NumberEdit* timer;
      
      logicalSwitchOneWindow->clear();
      logicalSwitchOneWindow->setFlexLayout();
      FlexGridLayout grid(col_dsc, row_dsc, 2);
      FlexGridLayout grid2(col_dsc2, row_dsc, 2);

      LogicalSwitchData * cs = lswAddress(index);
      uint8_t cstate = lswFamily(cs->func);

      // V1
      auto line = logicalSwitchOneWindow->newLine(&grid);
      new StaticText(line, rect_t{}, STR_V1, 0, COLOR_THEME_PRIMARY1);
      switch (cstate) {
        case LS_FAMILY_BOOL:
        case LS_FAMILY_STICKY:
        case LS_FAMILY_EDGE:
          choice = new SwitchChoice(line, rect_t{}, SWSRC_FIRST_IN_LOGICAL_SWITCHES, SWSRC_LAST_IN_LOGICAL_SWITCHES, GET_SET_DEFAULT(cs->v1));
          choice->setAvailableHandler(isSwitchAvailableInLogicalSwitches);
          break;
        case LS_FAMILY_COMP:
          new SourceChoice(line, rect_t{}, 0, MIXSRC_LAST_TELEM, GET_SET_DEFAULT(cs->v1));
          break;
        case LS_FAMILY_TIMER:
          timer = new NumberEdit(line, rect_t{}, -128, 122, GET_SET_DEFAULT(cs->v1));
          timer->setDisplayHandler([](int32_t value) {
            return formatNumberAsString(lswTimerValue(value), PREC1);
          });
          break;
        default:
          new SourceChoice(line, rect_t{}, 0, MIXSRC_LAST_TELEM, GET_DEFAULT(cs->v1),
                           [=](int32_t newValue) {
                             cs->v1 = newValue;
                             if (v2Edit != nullptr)
                             {
                               int16_t v2_min = 0, v2_max = 0;
                               getMixSrcRange(cs->v1, v2_min, v2_max);
                               v2Edit->setMin(v2_min);
                               v2Edit->setMax(v2_max);
                               v2Edit->setValue(cs->v2);
                             }
                             SET_DIRTY();
                           });
          break;
      }

      // V2
      if (cstate == LS_FAMILY_EDGE) {
        line = logicalSwitchOneWindow->newLine(&grid2);
      } else {
        line = logicalSwitchOneWindow->newLine(&grid);
      }
      new StaticText(line, rect_t{}, STR_V2, 0, COLOR_THEME_PRIMARY1);
      switch (cstate) {
        case LS_FAMILY_BOOL:
        case LS_FAMILY_STICKY:
          choice = new SwitchChoice(line, rect_t{}, SWSRC_FIRST_IN_LOGICAL_SWITCHES, SWSRC_LAST_IN_LOGICAL_SWITCHES, GET_SET_DEFAULT(cs->v2));
          choice->setAvailableHandler(isSwitchAvailableInLogicalSwitches);
          break;
        case LS_FAMILY_EDGE:
          {
            auto edit1 = new NumberEdit(line, rect_t{}, -129, 122, GET_DEFAULT(cs->v2));
            auto edit2 = new NumberEdit(line, rect_t{}, -1, 222 - cs->v2, GET_SET_DEFAULT(cs->v3));
            edit1->setSetValueHandler([=](int32_t newValue) {
              cs->v2 = newValue;
              SET_DIRTY();
              edit2->setMax(222 - cs->v2);
              edit2->setValue(cs->v3);
            });
            edit1->setDisplayHandler([](int32_t value) {
              return formatNumberAsString(lswTimerValue(value), PREC1);
            });
            edit2->setDisplayHandler([cs](int32_t value) {
              if (value < 0)
                return std::string("<<");
              else if (value == 0)
                return std::string("--");
              else {
                return formatNumberAsString(lswTimerValue(cs->v2 + value), PREC1);
              }
            });
          }
          break;
        case LS_FAMILY_COMP:
          new SourceChoice(line, rect_t{}, 0, MIXSRC_LAST_TELEM, GET_SET_DEFAULT(cs->v2));
          break;
        case LS_FAMILY_TIMER:
          timer = new NumberEdit(line, rect_t{}, -128, 122, GET_SET_DEFAULT(cs->v2));
          timer->setDisplayHandler([](int32_t value) {
            return formatNumberAsString(lswTimerValue(value), PREC1);
          });
          break;
        default:
          int16_t v2_min = 0, v2_max = 0;
          getMixSrcRange(cs->v1, v2_min, v2_max);
          v2Edit = new NumberEdit(line, rect_t{},
                                  v2_min, v2_max, GET_SET_DEFAULT(cs->v2));

          v2Edit->setDisplayHandler([=](int value) -> std::string {
            if (cs->v1 <= MIXSRC_LAST_CH) value = calc100toRESX(value);
            std::string txt = getSourceCustomValueString(cs->v1, value, 0);
            return txt;
          });
          break;
      }

      // AND switch
      line = logicalSwitchOneWindow->newLine(&grid);
      new StaticText(line, rect_t{}, STR_AND_SWITCH, 0, COLOR_THEME_PRIMARY1);
      choice = new SwitchChoice(line, rect_t{}, -MAX_LS_ANDSW, MAX_LS_ANDSW, GET_SET_DEFAULT(cs->andsw));
      choice->setAvailableHandler(isSwitchAvailableInLogicalSwitches);

      // Duration
      line = logicalSwitchOneWindow->newLine(&grid);
      new StaticText(line, rect_t{}, STR_DURATION, 0, COLOR_THEME_PRIMARY1);
      auto edit = new NumberEdit(line, rect_t{}, 0, MAX_LS_DURATION, GET_SET_DEFAULT(cs->duration), 0, PREC1);
      edit->setZeroText("---");

      // Delay
      line = logicalSwitchOneWindow->newLine(&grid);
      new StaticText(line, rect_t{}, STR_DELAY, 0, COLOR_THEME_PRIMARY1);
      if (cstate == LS_FAMILY_EDGE) {
        new StaticText(line, rect_t{}, STR_NA, 0, COLOR_THEME_PRIMARY1);
      }
      else {
        auto edit = new NumberEdit(line, rect_t{}, 0, MAX_LS_DELAY, GET_SET_DEFAULT(cs->delay), 0, PREC1);
        edit->setZeroText("---");
      }
    }

    void buildBody(FormWindow * window)
    {
      window->setFlexLayout();
      FlexGridLayout grid(col_dsc, row_dsc, 2);
      lv_obj_set_style_pad_all(window->getLvObj(), lv_dpx(8), 0);

      LogicalSwitchData * cs = lswAddress(index);

      // LS Func
      auto line = window->newLine(&grid);
      new StaticText(line, rect_t{}, STR_FUNC, 0, COLOR_THEME_PRIMARY1);
      auto functionChoice = new Choice(line, rect_t{}, STR_VCSWFUNC, 0, LS_FUNC_MAX, GET_DEFAULT(cs->func));
      functionChoice->setSetValueHandler([=](int32_t newValue) {
          cs->func = newValue;
          if (lswFamily(cs->func) == LS_FAMILY_TIMER) {
            cs->v1 = cs->v2 = 0;
          }
          else if (lswFamily(cs->func) == LS_FAMILY_EDGE) {
            cs->v1 = 0;
            cs->v2 = -129;
            cs->v3 = 0;
          }
          else {
            cs->v1 = cs->v2 = 0;
          }
          SET_DIRTY();
          updateLogicalSwitchOneWindow();
      });
      functionChoice->setAvailableHandler(isLogicalSwitchFunctionAvailable);

      logicalSwitchOneWindow = new FormWindow(window, rect_t{});
      updateLogicalSwitchOneWindow();
    }
};

static constexpr coord_t line1 = FIELD_PADDING_TOP;
static constexpr coord_t line2 = line1 + PAGE_LINE_HEIGHT;
static constexpr coord_t col0w = 56;
static constexpr coord_t col1 = 10 + col0w;
static constexpr coord_t col2 = (LCD_W - 100) / 3 + col1;
static constexpr coord_t col3 = ((LCD_W - 100) / 3) * 2 + col1;

void putsEdgeDelayParam(BitmapBuffer * dc, coord_t x, coord_t y, LogicalSwitchData * ls, LcdFlags flags = 0)
{
  coord_t lcdNextPos = 0;
  lcdNextPos = dc->drawText(x, y, "[", flags);
  lcdNextPos = dc->drawNumber(lcdNextPos+2, y, lswTimerValue(ls->v2), LEFT | PREC1 | flags);
  lcdNextPos = dc->drawText(lcdNextPos, y, ":", flags);
  if (ls->v3 < 0)
    lcdNextPos = dc->drawText(lcdNextPos+3, y, "<<", flags);
  else if (ls->v3 == 0)
    lcdNextPos = dc->drawText(lcdNextPos+3, y, "--", flags);
  else
    lcdNextPos = dc->drawNumber(lcdNextPos+3, y, lswTimerValue(ls->v2+ls->v3), LEFT | PREC1 | flags);
  dc->drawText(lcdNextPos, y, "]", flags);
}

class LogicalSwitchButton : public Button
{
 public:
  LogicalSwitchButton(Window* parent, const rect_t& rect, int lsIndex) :
      Button(parent, rect, nullptr, 0, COLOR_THEME_PRIMARY1), lsIndex(lsIndex), active(isActive())
  {
  }

  bool isActive() const
  {
    return getSwitch(SWSRC_FIRST_LOGICAL_SWITCH + lsIndex);
  }

  void checkEvents() override
  {
    if (active != isActive()) {
      invalidate();
      active = !active;
    }

    Button::checkEvents();
  }

  void paintLogicalSwitchLine(BitmapBuffer* dc)
  {
    LogicalSwitchData* ls = lswAddress(lsIndex);
    uint8_t lsFamily = lswFamily(ls->func);

    // CSW func
    dc->drawTextAtIndex(col1, line1, STR_VCSWFUNC, ls->func, COLOR_THEME_SECONDARY1);

    // CSW params
    if (lsFamily == LS_FAMILY_BOOL || lsFamily == LS_FAMILY_STICKY) {
      drawSwitch(dc, col2, line1, ls->v1, COLOR_THEME_SECONDARY1);
      drawSwitch(dc, col3, line1, ls->v2, COLOR_THEME_SECONDARY1);
    } else if (lsFamily == LS_FAMILY_EDGE) {
      drawSwitch(dc, col2, line1, ls->v1, COLOR_THEME_SECONDARY1);
      putsEdgeDelayParam(dc, col3, line1, ls, COLOR_THEME_SECONDARY1);
    } else if (lsFamily == LS_FAMILY_COMP) {
      drawSource(dc, col2, line1, ls->v1, COLOR_THEME_SECONDARY1);
      drawSource(dc, col3, line1, ls->v2, COLOR_THEME_SECONDARY1);
    } else if (lsFamily == LS_FAMILY_TIMER) {
      dc->drawNumber(col2, line1, lswTimerValue(ls->v1), COLOR_THEME_SECONDARY1 | LEFT | PREC1);
      dc->drawNumber(col3, line1, lswTimerValue(ls->v2), COLOR_THEME_SECONDARY1 | LEFT | PREC1);
    } else {
      drawSource(dc, col2, line1, ls->v1, COLOR_THEME_SECONDARY1);
      drawSourceCustomValue(dc, col3, line1, ls->v1,
          (ls->v1 <= MIXSRC_LAST_CH ? calc100toRESX(ls->v2) : ls->v2), COLOR_THEME_SECONDARY1);
    }

    // AND switch
    drawSwitch(dc, col1, line2, ls->andsw, COLOR_THEME_SECONDARY1);

    // CSW duration
    if (ls->duration > 0) {
      dc->drawNumber(col2, line2, ls->duration, COLOR_THEME_SECONDARY1 | PREC1 | LEFT);
    }

    // CSW delay
    if (lsFamily != LS_FAMILY_EDGE && ls->delay > 0) {
      dc->drawNumber(col3, line2, ls->delay, COLOR_THEME_SECONDARY1 | PREC1 | LEFT);
    }
  }

  void paint(BitmapBuffer* dc) override
  {
    if (active) {
      dc->drawSolidFilledRect(0, 0, rect.w, rect.h, COLOR_THEME_ACTIVE);
      dc->drawSolidFilledRect(0, 0, col0w, rect.h, COLOR_THEME_FOCUS);
      dc->drawText(8, 12, getSwitchPositionName(SWSRC_SW1 + lsIndex), COLOR_THEME_PRIMARY2);
    } else {
      dc->drawSolidFilledRect(0, 0, rect.w, rect.h, COLOR_THEME_PRIMARY2);
      dc->drawSolidFilledRect(0, 0, col0w, rect.h, COLOR_THEME_SECONDARY2);
      dc->drawText(8, 12, getSwitchPositionName(SWSRC_SW1 + lsIndex), COLOR_THEME_PRIMARY1);
    }

    paintLogicalSwitchLine(dc);

    // The bounding rect
    if (hasFocus())
      dc->drawSolidRect(0, 0, rect.w, rect.h, 2, COLOR_THEME_FOCUS);
    else
      dc->drawSolidRect(0, 0, rect.w, rect.h, 1, COLOR_THEME_SECONDARY2);
  }

 protected:
  uint8_t lsIndex;
  bool active;
};

#define LS_BUTTON_H 45

ModelLogicalSwitchesPage::ModelLogicalSwitchesPage():
  PageTab(STR_MENULOGICALSWITCHES, ICON_MODEL_LOGICAL_SWITCHES)
{
}

void ModelLogicalSwitchesPage::rebuild(FormWindow * window)
{
  auto scroll_y = lv_obj_get_scroll_y(window->getLvObj());

  // When window.clear() is called the last button on screen is given focus (???)
  // This causes the page to jump to the end when rebuilt.
  // Set flag to bypass the button focus handler and reset focusIndex when rebuilding
  isRebuilding = true;
  window->clear();
  build(window);
  isRebuilding = false;

  lv_obj_scroll_to_y(window->getLvObj(), scroll_y, LV_ANIM_OFF);
}

void ModelLogicalSwitchesPage::editLogicalSwitch(FormWindow * window, uint8_t lsIndex)
{
  Window * lsWindow = new LogicalSwitchEditPage(lsIndex);
  lsWindow->setCloseHandler([=]() {
    rebuild(window);
  });
}

void ModelLogicalSwitchesPage::build(FormWindow* window)
{
#if LCD_W > LCD_H
  #define PER_ROW 6
  static const lv_coord_t l_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1),
                                         LV_GRID_TEMPLATE_LAST};
#else
  #define PER_ROW 4
  static const lv_coord_t l_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1),
                                         LV_GRID_TEMPLATE_LAST};
#endif

  window->padAll(4);

  auto form = new FormWindow(window, rect_t{});
  form->setFlexLayout();
  form->padAll(0);

  FlexGridLayout grid(l_col_dsc, row_dsc, 2);

  FormWindow::Line* line;
  firstActiveButton = nullptr;
  firstInactiveButton = nullptr;
  bool hasFocusButton = false;
  Button* button;

  // Reset focusIndex after switching tabs
  if (!isRebuilding)
    focusIndex = prevFocusIndex;

  uint8_t scol = 0;

  for (uint8_t i = 0; i < MAX_LOGICAL_SWITCHES; i++) {
    LogicalSwitchData* ls = lswAddress(i);

    bool isActive = (ls->func != LS_FUNC_NONE);

    if (!isActive) {
      if (scol == 0) {
        line = form->newLine(&grid);
        lv_obj_set_style_pad_column(line->getLvObj(), 4, LV_PART_MAIN);
      }

      button = new TextButton(line, rect_t{0, 0, 0, LS_BUTTON_H}, getSwitchPositionName(SWSRC_SW1 + i));

      if (firstInactiveButton == nullptr)
        firstInactiveButton = button;

      button->setLongPressHandler([=]() -> uint8_t {
        if (firstActiveButton)
          lv_group_focus_obj(firstActiveButton->getLvObj());
        return 0;
      });

      lv_obj_set_grid_cell(button->getLvObj(), LV_GRID_ALIGN_STRETCH, scol, 1, LV_GRID_ALIGN_CENTER, 0, 1);
      scol = (scol + 1) % PER_ROW;
    } else {
      line = form->newLine(&grid);
      scol = 0;

      button = new LogicalSwitchButton(line, rect_t{0, 0, window->width() - 12, LS_BUTTON_H}, i);

      if (firstActiveButton == nullptr)
        firstActiveButton = button;

      button->setLongPressHandler([=]() -> uint8_t {
        if (firstInactiveButton)
          lv_group_focus_obj(firstInactiveButton->getLvObj());
        return 0;
      });

      lv_obj_set_grid_cell(button->getLvObj(), LV_GRID_ALIGN_CENTER, 0, PER_ROW, LV_GRID_ALIGN_CENTER, 0, 1);
    }

    button->setPressHandler([=]() {
      Menu* menu = new Menu(window);
      menu->addLine(STR_EDIT, [=]() { editLogicalSwitch(window, i); });
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
      if (isActive || ls->v1 || ls->v2 || ls->delay || ls->duration || ls->andsw) {
        menu->addLine(STR_CLEAR, [=]() {
          memset(ls, 0, sizeof(LogicalSwitchData));
          storageDirty(EE_MODEL);
          rebuild(window);
        });
      }
      return 0;
    });

    if (focusIndex == i) {
      hasFocusButton = true;
      lv_group_focus_obj(button->getLvObj());
    }

    button->setFocusHandler([=](bool hasFocus) {
      if (hasFocus && !isRebuilding) {
        prevFocusIndex = focusIndex;
        focusIndex = i;
      }
    });
  }

  if (!hasFocusButton)
  {
    if (firstActiveButton)
      lv_group_focus_obj(firstActiveButton->getLvObj());
    else if (firstInactiveButton)
      lv_group_focus_obj(firstInactiveButton->getLvObj());
  }
}
