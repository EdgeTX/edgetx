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
#include "switchchoice.h"

#include "dataconstants.h"
#include "draw_functions.h"
#include "menu.h"
#include "menutoolbar.h"
#include "opentx.h"
#include "strhelpers.h"
#include "switches.h"
#include "modal_window.h"
#include "button_matrix.h"

#if LCD_W > LCD_H
#define DLG_W (LCD_W - 96)
#define DLG_H (LCD_H - 32)
#define SW_INDENT 20
#define MAX_COLS 4
#define MAX_ROWS 3
#else
#define DLG_W (LCD_W - 20)
#define DLG_H (LCD_H - 96)
#define SW_INDENT 10
#define MAX_COLS 3
#define MAX_ROWS 5
#endif

// ButtonMatrix with one or more rows of switches
class SwitchButtons : public ButtonMatrix
{
  public:
    SwitchButtons(Window* parent, int firstIdx, int lastIdx,
                       std::function<int16_t()> getValue, std::function<void(int16_t, bool)> setValue,
                       bool invert, int cols, int rows, int btn_cnt, bool isSwitches) :
        ButtonMatrix(parent, rect_t{}),
        m_getValue(std::move(getValue)),
        m_setValue(std::move(setValue)),
        inverted(invert),
        firstIdx(firstIdx),
        lastIdx(lastIdx),
        btnCnt(btn_cnt),
        isSwitches(isSwitches)
    {
      int max_btns = rows * cols;

      lv_obj_set_width(lvobj, cols * 75);
      lv_obj_set_height(lvobj, rows * 38);

      lv_obj_set_style_bg_opa(lvobj, LV_OPA_0, LV_PART_MAIN);

      lv_obj_set_style_pad_all(lvobj, lv_dpx(4), LV_PART_MAIN);
      lv_obj_set_style_pad_row(lvobj, lv_dpx(4), LV_PART_MAIN);
      lv_obj_set_style_pad_column(lvobj, lv_dpx(4), LV_PART_MAIN);

      lv_obj_remove_style(lvobj, nullptr, LV_PART_MAIN | LV_STATE_FOCUSED);
      lv_obj_remove_style(lvobj, nullptr, LV_PART_MAIN | LV_STATE_EDITED);

      initBtnMap(cols, max_btns);

      update();

      for (int i = btn_cnt; i < max_btns; i += 1) {
        hide(i);
      }

      btnValues = new int16_t[btn_cnt];
      for (int i = 0; i < btn_cnt; i +=1)
        btnValues[i] = -1;

      refresh();
    }

    ~SwitchButtons()
    {
     if (btnValues) {
       delete btnValues;
       btnValues = nullptr;
     }
    }

    void updateBtns()
    {
      update();
    }

    void setInverted(bool state)
    {
      inverted = state;
      refresh();
    }

    bool hasActive()
    {
      for (int i = 0; i < btnCnt; i += 1) {
        if (isActive(i)) {
          return true;
        }
      }
      return false;
    }

  protected:
    int16_t* btnValues = nullptr;
    std::function<int16_t()> m_getValue;
    std::function<void(int16_t, bool)> m_setValue;
    bool inverted;
    int firstIdx;
    int lastIdx;
    int btnCnt;
    bool isSwitches;

    void hide(int n)
    {
      lv_btnmatrix_set_btn_ctrl(lvobj, n, LV_BTNMATRIX_CTRL_HIDDEN);
    }

    void refresh()
    {
      if (isSwitches) {
        setSwitchText(firstIdx, lastIdx);
      } else {
        setButtonText(firstIdx, lastIdx);
      }

      update();
    }

    void setBtnText(int b, int sw)
    {
      std::string s("");
      if (inverted)
        s = "!";
      s += getSwitchPositionName(sw);
      setText(b, s.c_str());
      btnValues[b] = sw;
    }

    void setButtonText(int firstIdx, int lastIdx, int b = 0)
    {
      // For entries other than physical switches, just skip ones that aren't available
      for (int i = firstIdx; i <= lastIdx; i += 1) {
        if (isSwitchAvailableInMixes(i)) {
          setBtnText(b, i);
          b += 1;
        }
      }
    }

    void setSwitchText(int firstIdx, int lastIdx)
    {
      // For physical switches skip entire row if none of the options (up, center, down) are available
      int b = 0;
      for (int i = firstIdx; i <= lastIdx; i += 3) {
        if (isSwitchAvailableInMixes(i) || isSwitchAvailableInMixes(i+1) || isSwitchAvailableInMixes(i+2)) {
          if (isSwitchAvailableInMixes(i)) {
            setBtnText(b, i);
          } else {
            hide(b);
          }
          b += 1;
          if (isSwitchAvailableInMixes(i+1)) {
            setBtnText(b, i+1);
          } else {
            hide(b);
          }
          b += 1;
          if (isSwitchAvailableInMixes(i+2)) {
            setBtnText(b, i+2);
          } else {
            hide(b);
          }
          b += 1;
        }
      }
    }

    void onPress(uint8_t btn_id)
    {
      m_setValue(inverted ? -btnValues[btn_id] : btnValues[btn_id], false);
    }

    bool isActive(uint8_t btn_id)
    {
      return btnValues[btn_id] == abs(m_getValue());
    }
};

// Matrix of buttons for a given switch type (physical, logical, trim, etc)
// Note: may be split into multiple ButtonMatrix rows due to issues with scrolling a large ButtonMatrix
class SwitchMatrix : public Window
{
  public:
    SwitchMatrix(Window* parent, int firstIdx, int lastIdx,
                 std::function<int16_t()> getValue, std::function<void(int16_t, bool)> setValue,
                 bool invert) :
        Window(parent, rect_t{}),
        m_getValue(std::move(getValue)),
        m_setValue(std::move(setValue))
    {
      setWidth(LV_SIZE_CONTENT);
      setHeight(LV_SIZE_CONTENT);
      padAll(0);
      padLeft(SW_INDENT);

      auto form = new FormWindow(this, rect_t{});
      form->setFlexLayout(LV_FLEX_FLOW_COLUMN, 0);
      form->padAll(0);
      form->setWidth(LV_SIZE_CONTENT);
      form->setHeight(LV_SIZE_CONTENT);

      isSwitches = (firstIdx == SWSRC_FIRST_SWITCH);

      // Calculate sizes
      int cols = MAX_COLS;
#if LCD_H > LCD_W
      if (firstIdx == SWSRC_FIRST_TRIM)
        cols = 2;
#endif
      int btn_cnt = 0;

      if (isSwitches) {
        cols = 3;
        btn_cnt = switchButtonCount(firstIdx, lastIdx);
      } else {
        btn_cnt = buttonCount(firstIdx, lastIdx);
      }

      int rows = (btn_cnt + cols - 1) / cols;

      if (isSwitches) {
        btnsCnt = rows;
#if NUM_XPOTS > 0
        btnsCnt += 1;
#endif
      } else {
        if (rows <= MAX_ROWS)
          btnsCnt = 1;
        else
          btnsCnt = rows;
      }

      // Allocate storage for number of ButtonMatrix objects required
      btns = new SwitchButtons*[btnsCnt];

      // Build out all the rows
      if (isSwitches) {
        int n = 0;
        for (int i = firstIdx; i <= lastIdx; i += 3) {
          if (isSwitchAvailableInMixes(i) || isSwitchAvailableInMixes(i+1) || isSwitchAvailableInMixes(i+2)) {
            btns[n] = new SwitchButtons(form, i, i+2, m_getValue, m_setValue, invert, cols, 1, cols, true);
            lv_obj_add_event_cb(btns[n]->getLvObj(), longPressHandler, LV_EVENT_LONG_PRESSED, this);
            n += 1;
          }
        }
#if NUM_XPOTS > 0
        // Add multi-position switch
        int r = (buttonCount(SWSRC_FIRST_MULTIPOS_SWITCH, SWSRC_LAST_MULTIPOS_SWITCH) + cols - 1) / cols;
        btns[n] = new SwitchButtons(form, SWSRC_FIRST_MULTIPOS_SWITCH, SWSRC_LAST_MULTIPOS_SWITCH, m_getValue, m_setValue, invert, cols, r, r * cols, false);
        lv_obj_add_event_cb(btns[n]->getLvObj(), longPressHandler, LV_EVENT_LONG_PRESSED, this);
#endif
      } else {
        if (btnsCnt == 1) {
          btns[0] = new SwitchButtons(form, firstIdx, lastIdx, m_getValue, m_setValue, invert, cols, rows, btn_cnt, false);
          lv_obj_add_event_cb(btns[0]->getLvObj(), longPressHandler, LV_EVENT_LONG_PRESSED, this);
        } else {
          int sw = firstIdx;
          for (int i = 0; i <= rows; i += 1) {
            int lw = sw + cols - 1;
            if (lw > lastIdx) lw = lastIdx;
            btns[i] = new SwitchButtons(form, sw, lw, m_getValue, m_setValue, invert, cols, 1, cols, false);
            lv_obj_add_event_cb(btns[i]->getLvObj(), longPressHandler, LV_EVENT_LONG_PRESSED, this);
          }
        }
      }

      for (int i = 0; i < btnsCnt; i += 1) {
        if (btns[i]->hasActive()) {
          setFocus(i);
          break;
        }
      }
    }

    ~SwitchMatrix()
    {
      if (btns)
        delete btns;
      btns = nullptr;
    }

    void updateBtns()
    {
      for (int i = 0; i < btnsCnt; i += 1)
        btns[i]->updateBtns();
    }

    void setInverted(bool state)
    {
      for (int i = 0; i < btnsCnt; i += 1)
        btns[i]->setInverted(state);
    }

    void setFocus(int n)
    {
      lv_group_focus_obj(btns[n]->getLvObj());
    }

  protected:
    SwitchButtons** btns = nullptr;
    int btnsCnt = 0;
    bool isSwitches = false;
    std::function<int16_t()> m_getValue;
    std::function<void(int16_t, bool)> m_setValue;

    static void longPressHandler(lv_event_t* e);

    int buttonCount(int firstIdx, int lastIdx)
    {
      int n = 0;
      for (int i = firstIdx; i <= lastIdx; i += 1) {
        if (isSwitchAvailableInMixes(i)) {
          n += 1;
        }
      }
      return n;
    }

    int switchButtonCount(int firstIdx, int lastIdx)
    {
      int n = 0;
      for (int i = firstIdx; i <= lastIdx; i += 3) {
        if (isSwitchAvailableInMixes(i) || isSwitchAvailableInMixes(i+1) || isSwitchAvailableInMixes(i+2)) {
          n += 3;
        }
      }
      return n;
    }
};

void SwitchMatrix::longPressHandler(lv_event_t* e)
{
  SwitchMatrix* sm = (SwitchMatrix*)lv_event_get_user_data(e);
  sm->m_setValue(sm->m_getValue(), true);
}

class SwitchDialog : public ModalWindow
{
  public:
    SwitchDialog(Window* parent, int vmax,
                 std::function<int16_t()> getValue,
                 std::function<void(int16_t)> setValue,
                 std::function<void(bool)> onSave,
                 std::function<void()> onCancel) :
        ModalWindow(parent, true),
        m_getValue(std::move(getValue)),
        m_setValue(std::move(setValue)),
        m_onSave(std::move(onSave)),
        m_onCancel(std::move(onCancel))
    {
      inverted = m_getValue() < 0;

      // Layout windows

      // Main window
      auto window = new Window(this, rect_t{(LCD_W-DLG_W)/2, (LCD_H-DLG_H)/2, DLG_W, DLG_H}, OPAQUE);
      window->padAll(0);
      lv_obj_set_style_bg_color(window->getLvObj(), makeLvColor(COLOR_THEME_SECONDARY3), 0);
      lv_obj_set_style_bg_opa(window->getLvObj(), LV_OPA_100, 0);
      lv_obj_set_style_border_width(window->getLvObj(), 1, 0);
      lv_obj_set_style_border_color(window->getLvObj(), makeLvColor(COLOR_THEME_SECONDARY2), 0);
      lv_obj_set_style_border_opa(window->getLvObj(), LV_OPA_100, 0);

      // Header
      auto hdr = new Window(window, rect_t{0, 0, DLG_W-2, 22});
      hdr->padAll(0);
      hdr->padLeft(4);
      lv_obj_set_style_bg_color(hdr->getLvObj(), makeLvColor(COLOR_THEME_SECONDARY1), 0);
      lv_obj_set_style_bg_opa(hdr->getLvObj(), LV_OPA_100, 0);
      lv_obj_set_style_border_width(hdr->getLvObj(), 1, 0);
      lv_obj_set_style_border_color(hdr->getLvObj(), makeLvColor(COLOR_THEME_SECONDARY2), 0);
      lv_obj_set_style_border_opa(hdr->getLvObj(), LV_OPA_100, 0);

      // Left column for filter buttons
      auto tabs = new Window(window, rect_t{0, 22, 40, DLG_H-64});
      tabs->padAll(0);
      lv_obj_set_style_border_width(tabs->getLvObj(), 1, 0);
      lv_obj_set_style_border_color(tabs->getLvObj(), makeLvColor(COLOR_THEME_SECONDARY2), 0);
      lv_obj_set_style_border_opa(tabs->getLvObj(), LV_OPA_100, 0);

      auto tabsForm = new FormWindow(tabs, rect_t{});
      tabsForm->setFlexLayout(LV_FLEX_FLOW_COLUMN, 4);
      tabsForm->padAll(0);
      tabsForm->padTop(4);
      lv_obj_set_flex_align(tabsForm->getLvObj(), LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_AROUND);

      // Switch selection body
      auto switches = new Window(window, rect_t{40, 22, DLG_W-42, DLG_H-64});
      switches->padAll(0);
      lv_obj_set_style_border_width(switches->getLvObj(), 1, 0);
      lv_obj_set_style_border_color(switches->getLvObj(), makeLvColor(COLOR_THEME_SECONDARY2), 0);
      lv_obj_set_style_border_opa(switches->getLvObj(), LV_OPA_100, 0);
      lv_obj_set_scrollbar_mode(switches->getLvObj(), LV_SCROLLBAR_MODE_AUTO);
  
      auto switchesForm = new FormWindow(switches, rect_t{});
      switchesForm->setFlexLayout(LV_FLEX_FLOW_COLUMN, 4);
      switchesForm->padAll(4);
      lv_obj_set_flex_align(tabsForm->getLvObj(), LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_AROUND);

      // Footer for action buttons
      auto ftr = new Window(window, rect_t{0, DLG_H-42, DLG_W-2, 40});
      ftr->padAll(0);
      lv_obj_set_style_border_width(ftr->getLvObj(), 1, 0);
      lv_obj_set_style_border_color(ftr->getLvObj(), makeLvColor(COLOR_THEME_SECONDARY2), 0);
      lv_obj_set_style_border_opa(ftr->getLvObj(), LV_OPA_100, 0);

      auto ftrL = new Window(ftr, rect_t{0, 0, (DLG_W-4)/2-20, 38});
      ftrL->padAll(0);

      auto ftrFormL = new FormWindow(ftrL, rect_t{});
      ftrFormL->setFlexLayout(LV_FLEX_FLOW_ROW, 2);
      ftrFormL->padAll(4);
      ftrFormL->padTop(5);
      lv_obj_set_flex_align(ftrFormL->getLvObj(), LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_AROUND);

      auto ftrR = new Window(ftr, rect_t{(DLG_W-4)/2-20, 0, (DLG_W-4)/2+20, 38});
      ftrR->padAll(0);

      auto ftrFormR = new FormWindow(ftrR, rect_t{});
      ftrFormR->setFlexLayout(LV_FLEX_FLOW_ROW, 2);
      ftrFormR->padAll(4);
      ftrFormR->padTop(5);
      lv_obj_set_flex_align(ftrFormR->getLvObj(), LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_AROUND);

      // Add content

      // Header title
      title = new StaticText(hdr, rect_t{}, STR_SWITCH, 0, COLOR_THEME_PRIMARY2);
      setTitle();

      // Filter buttons
      addSectionCtrl(tabsForm, STR_CHAR_SWITCH, 0);

      if (vmax >= SWSRC_FIRST_TRIM) {
        addSectionCtrl(tabsForm, STR_CHAR_TRIM, 1);
      }

      if (vmax >= SWSRC_FIRST_LOGICAL_SWITCH) {
        addSectionCtrl(tabsForm, STR_CHAR_SWITCH, 2);
      }

      if (vmax >= SWSRC_TELEMETRY_STREAMING) {
        addSectionCtrl(tabsForm, STR_CHAR_TELEMETRY, 3);
      }

      // Switch sections
      addSection(switchesForm, STR_MENU_SWITCHES, SWSRC_FIRST_SWITCH, SWSRC_LAST_SWITCH, 0);

      if (vmax >= SWSRC_FIRST_TRIM) {
        addSection(switchesForm, STR_MENU_TRIMS, SWSRC_FIRST_TRIM, SWSRC_LAST_TRIM, 1);
      }

      if (vmax >= SWSRC_FIRST_LOGICAL_SWITCH) {
        addSection(switchesForm, STR_MENU_LOGICAL_SWITCHES, SWSRC_FIRST_LOGICAL_SWITCH, SWSRC_LAST_LOGICAL_SWITCH, 2);
      }

      if (vmax >= SWSRC_TELEMETRY_STREAMING) {
        addSection(switchesForm, STR_MENU_TELEMETRY, SWSRC_TELEMETRY_STREAMING, SWSRC_RADIO_ACTIVITY, 3);
      }

      // Action buttons
      actionBtns[0] = new TextButton(ftrFormL, rect_t{0, 0, 50, 28}, "!",
                                     [=]() -> uint8_t {
                                       setInverted();
                                       return inverted;
                                     });
      actionBtns[0]->check(inverted);

      actionBtns[1] = new TextButton(ftrFormL, rect_t{0, 0, 50, 28}, "---",
                                     [=]() -> uint8_t {
                                       setSwitchValue(0, false);
                                       return 0;
                                     });

      actionBtns[2] = new TextButton(ftrFormR, rect_t{0, 0, 80, 28}, STR_CANCEL,
                                     [=]() -> uint8_t {
                                       m_onCancel();
                                       deleteLater();
                                       return 0;
                                     });

      actionBtns[3] = new TextButton(ftrFormR, rect_t{0, 0, 80, 28}, STR_SAVE,
                                     [=]() -> uint8_t {
                                       m_onSave(false);
                                       deleteLater();
                                       return 0;
                                     });
    }

  protected:
    std::function<int16_t()> m_getValue;
    std::function<void(int16_t)> m_setValue;
    std::function<void(bool)> m_onSave;
    std::function<void()> m_onCancel;
    StaticText* title = nullptr;
    int filterSection = -1;
    int actionSel = -1;
    StaticText* sectionTitle[4] = {};
    SwitchMatrix* sectionBtn[4] = {};
    TextButton* sectionCtrl[4] = {};
    TextButton* actionBtns[4] = {};
    bool inverted = false;
    bool doSave = false;
    bool isLongPressed = false;
    tmr10ms_t doSaveTime = 0;

    void checkEvents() override
    {
      if (doSave && (get_tmr10ms() > doSaveTime)) {
        doSave = false;
        m_onSave(isLongPressed);
        deleteLater();
      }
    }

    void setTitle()
    {
      int value = m_getValue();
      std::string s = std::string(STR_SWITCH) + ": ";

      if (value == 0)
        s += "---";
      else
        s += getSwitchPositionName(value);
      title->setText(s);
    }

    void setSwitchValue(int16_t newValue, bool isLongPress)
    {
      if (m_getValue() == newValue) {
        isLongPressed = isLongPress;
        doSaveTime = get_tmr10ms() + 10;
        doSave = true;
      } else {
        m_setValue(newValue);
        setTitle();
        for (int i = 0; i < 4; i += 1) {
          if (sectionBtn[i])
            sectionBtn[i]->updateBtns();
        }
      }
    }

    void applyFilter(int section)
    {
      filterSection = section;
      for (int i = 0; i < 4; i += 1) {
        if (sectionTitle[i]) {
          sectionCtrl[i]->check(filterSection == i);
          if ((filterSection == i) || (filterSection == -1)) {
            lv_obj_clear_flag(sectionTitle[i]->getLvObj(), LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(sectionBtn[i]->getLvObj(), LV_OBJ_FLAG_HIDDEN);
          } else {
            lv_obj_add_flag(sectionTitle[i]->getLvObj(), LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(sectionBtn[i]->getLvObj(), LV_OBJ_FLAG_HIDDEN);
          }
        }
      }
      sectionBtn[filterSection >= 0 ? filterSection : 0]->setFocus(0);
    }

    void addSectionCtrl(FormWindow* tabsForm, const char* iconChar, int section)
    {
      sectionCtrl[section] = new TextButton(tabsForm, rect_t{0, 0, 24, 24}, iconChar, [=]() -> uint8_t {
                       if (filterSection == section)
                         applyFilter(-1);
                       else
                         applyFilter(section);
                       return filterSection == section;
                     });
    }

    void addSection(FormWindow* switchesForm, const char* title, int firstIdx, int lastIdx, int section)
    {
      sectionTitle[section] = new StaticText(switchesForm, rect_t{}, title, 0, COLOR_THEME_PRIMARY1);
      sectionBtn[section] = new SwitchMatrix(switchesForm, firstIdx, lastIdx, m_getValue, [=](int16_t n, bool isLongPress) { setSwitchValue(n, isLongPress); }, inverted);
    }

    void setInverted()
    {
      inverted = !inverted;
      for (int i = 0; i < 4; i += 1) {
        if (sectionBtn[i])
          sectionBtn[i]->setInverted(inverted);
      }
      m_setValue(-m_getValue());
      setTitle();
    }

    void onCancel() override
    {
      m_onCancel();
      deleteLater();
    }

    void onEvent(event_t event) override
    {
      if (event == EVT_KEY_BREAK(KEY_PGDN)) {
        filterSection += 1;
        if (filterSection > 3)
          filterSection = -1;
        applyFilter(filterSection);
      }
#if defined(KEYS_GPIO_REG_PGUP)
      else if (event == EVT_KEY_BREAK(KEY_PGUP)) {
#else
      else if (event == EVT_KEY_LONG(KEY_PGDN)) {
        killEvents(event);
#endif
        actionSel -= 1;
        if (actionSel < -1)
          actionSel = 3;
        if (actionSel >= 0) {
          lv_group_focus_obj(actionBtns[actionSel]->getLvObj());
        } else {
          sectionBtn[filterSection >= 0 ? filterSection : 0]->setFocus(0);
        }
      }
    }
};

SwitchChoice::SwitchChoice(Window* parent, const rect_t& rect, int vmin,
                           int vmax, std::function<int16_t()> getValue,
                           std::function<void(int16_t)> setValue) :
    Choice(parent, rect, vmin, vmax, getValue, setValue)
{
  switchValue = getIntValue();

  setTextHandler([=](int value) {
    if (!isSwitchAvailableInMixes(value))
      return std::to_string(0);  // we will fix this later

    return std::string(getSwitchPositionName(value));
  });
 
  set_lv_LongPressHandler(longPressHandler, this);
}

void SwitchChoice::openMenu()
{
  switchValue = getIntValue();
  new SwitchDialog(this, vmax,
                   [=]() -> int16_t {
                     return switchValue;
                   },
                   [=](int16_t newValue) {
                     switchValue = newValue;
                   },
                   [=](bool isLongPress) {
                     longPressData.isLongPressed = isLongPress;
                     setValue(switchValue);
                   },
                   [=]() {
                     switchValue = _getValue();
                   }
                  );
}

void SwitchChoice::longPressHandler(void* data)
{
  SwitchChoice* sc = (SwitchChoice*)data;
  sc->setValue(-sc->getIntValue());
}
