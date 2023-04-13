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

class SwitchButtonMatrix : public ButtonMatrix
{
  public:
    SwitchButtonMatrix(Window* parent, int firstIdx, int lastIdx,
                       std::function<int16_t()> getValue, std::function<void(int16_t)> setValue,
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

    ~SwitchButtonMatrix()
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
    std::function<void(int16_t)> m_setValue;
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
      for (int i = firstIdx; i <= lastIdx; i += 1) {
        if (isSwitchAvailableInMixes(i)) {
          setBtnText(b, i);
          b += 1;
        }
      }
    }

    void setSwitchText(int firstIdx, int lastIdx)
    {
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
      m_setValue(inverted ? -btnValues[btn_id] : btnValues[btn_id]);
    }

    bool isActive(uint8_t btn_id)
    {
      return btnValues[btn_id] == abs(m_getValue());
    }
};

class SwitchMatrix : public Window
{
  public:
    SwitchMatrix(Window* parent, int firstIdx, int lastIdx,
                 std::function<int16_t()> getValue, std::function<void(int16_t)> setValue,
                 bool invert) :
        Window(parent, rect_t{})
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

      btns = new SwitchButtonMatrix*[btnsCnt];

      if (isSwitches) {
        int n = 0;
        for (int i = firstIdx; i <= lastIdx; i += 3) {
          if (isSwitchAvailableInMixes(i) || isSwitchAvailableInMixes(i+1) || isSwitchAvailableInMixes(i+2)) {
            btns[n] = new SwitchButtonMatrix(form, i, i+2, getValue, setValue, invert, cols, 1, cols, true);
            n += 1;
          }
        }
#if NUM_XPOTS > 0
        int r = (buttonCount(SWSRC_FIRST_MULTIPOS_SWITCH, SWSRC_LAST_MULTIPOS_SWITCH) + cols - 1) / cols;
        btns[n] = new SwitchButtonMatrix(form, SWSRC_FIRST_MULTIPOS_SWITCH, SWSRC_LAST_MULTIPOS_SWITCH, getValue, setValue, invert, cols, r, r * cols, false);
#endif
      } else {
        if (btnsCnt == 1) {
          btns[0] = new SwitchButtonMatrix(form, firstIdx, lastIdx, getValue, setValue, invert, cols, rows, btn_cnt, false);
        } else {
          int sw = firstIdx;
          for (int i = 0; i <= rows; i += 1) {
            int lw = sw + cols - 1;
            if (lw > lastIdx) lw = lastIdx;
            btns[i] = new SwitchButtonMatrix(form, sw, lw, getValue, setValue, invert, cols, 1, cols, false);
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
    SwitchButtonMatrix** btns = nullptr;
    int btnsCnt = 0;
    bool isSwitches = false;

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

class SwitchDialog : public ModalWindow
{
  protected:
    std::function<int16_t()> m_getValue;
    std::function<void(int16_t)> m_setValue;
    std::function<void()> m_onSave;
    std::function<void()> m_onCancel;
    StaticText* title = nullptr;
    FormWindow* tabsForm = nullptr;
    Window* switches = nullptr;
    FormWindow* switchesForm = nullptr;
    int filterSection = -1;
    int actionSel = -1;
    StaticText* sectionTitle[4] = {};
    SwitchMatrix* sectionBtn[4] = {};
    TextButton* sectionCtrl[4] = {};
    TextButton* actionBtns[4] = {};
    bool inverted = false;

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

    void setSwitchValue(int16_t newValue)
    {
      if (m_getValue() == newValue) {
        lv_group_focus_obj(actionBtns[3]->getLvObj());
//         m_onSave();
//         deleteLater();
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
    }

    void addSectionCtrl(const char* iconChar, int section)
    {
      sectionCtrl[section] = new TextButton(tabsForm, rect_t{0, 0, 24, 24}, iconChar, [=]() -> uint8_t {
                       if (filterSection == section)
                         applyFilter(-1);
                       else
                         applyFilter(section);
                       return filterSection == section;
                     });
    }

    void addSection(const char* title, int firstIdx, int lastIdx, int section)
    {
      sectionTitle[section] = new StaticText(switchesForm, rect_t{}, title, 0, COLOR_THEME_PRIMARY1);
      sectionBtn[section] = new SwitchMatrix(switchesForm, firstIdx, lastIdx, m_getValue, [=](int16_t n) { setSwitchValue(n); }, inverted);
    }

  public:
    SwitchDialog(Window* parent, int vmax,
                 std::function<int16_t()> getValue,
                 std::function<void(int16_t)> setValue,
                 std::function<void()> onSave,
                 std::function<void()> onCancel) :
        ModalWindow(parent, true),
        m_getValue(std::move(getValue)),
        m_setValue(std::move(setValue)),
        m_onSave(std::move(onSave)),
        m_onCancel(std::move(onCancel))
    {
      inverted = m_getValue() < 0;

      auto window = new Window(this, rect_t{(LCD_W-DLG_W)/2, (LCD_H-DLG_H)/2, DLG_W, DLG_H});
      window->padAll(0);
      lv_obj_set_style_bg_color(window->getLvObj(), makeLvColor(COLOR_THEME_SECONDARY3), 0);
      lv_obj_set_style_bg_opa(window->getLvObj(), LV_OPA_100, 0);
      lv_obj_set_style_border_width(window->getLvObj(), 1, 0);
      lv_obj_set_style_border_color(window->getLvObj(), makeLvColor(COLOR_THEME_SECONDARY2), 0);
      lv_obj_set_style_border_opa(window->getLvObj(), LV_OPA_100, 0);

      auto form = new FormWindow(window, rect_t{});
      form->setFlexLayout(LV_FLEX_FLOW_COLUMN, 0);
      form->padAll(0);

      auto hdr = new Window(form, rect_t{0, 0, DLG_W-2, 22});
      hdr->padAll(0);
      hdr->padLeft(4);
      lv_obj_set_style_bg_color(hdr->getLvObj(), makeLvColor(COLOR_THEME_SECONDARY1), 0);
      lv_obj_set_style_bg_opa(hdr->getLvObj(), LV_OPA_100, 0);
      lv_obj_set_style_border_width(hdr->getLvObj(), 1, 0);
      lv_obj_set_style_border_color(hdr->getLvObj(), makeLvColor(COLOR_THEME_SECONDARY2), 0);
      lv_obj_set_style_border_opa(hdr->getLvObj(), LV_OPA_100, 0);

      title = new StaticText(hdr, rect_t{}, STR_SWITCH, 0, COLOR_THEME_PRIMARY2);
      setTitle();

      auto content = new Window(form, rect_t{0, 0, DLG_W-2, DLG_H-64});
      content->padAll(0);

      auto contentForm = new FormWindow(content, rect_t{});
      contentForm->setFlexLayout(LV_FLEX_FLOW_ROW, 0);
      contentForm->padAll(0);

      auto tabs = new Window(contentForm, rect_t{0, 0, 40, DLG_H-64});
      tabs->padAll(0);
      lv_obj_set_style_border_width(tabs->getLvObj(), 1, 0);
      lv_obj_set_style_border_color(tabs->getLvObj(), makeLvColor(COLOR_THEME_SECONDARY2), 0);
      lv_obj_set_style_border_opa(tabs->getLvObj(), LV_OPA_100, 0);

      tabsForm = new FormWindow(tabs, rect_t{});
      tabsForm->setFlexLayout(LV_FLEX_FLOW_COLUMN, 4);
      tabsForm->padAll(0);
      tabsForm->padTop(4);
      lv_obj_set_flex_align(tabsForm->getLvObj(), LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_AROUND);

      addSectionCtrl(STR_CHAR_SWITCH, 0);

      if (vmax >= SWSRC_FIRST_TRIM) {
        addSectionCtrl(STR_CHAR_TRIM, 1);
      }

      if (vmax >= SWSRC_FIRST_LOGICAL_SWITCH) {
        addSectionCtrl(STR_CHAR_SWITCH, 2);
      }

      if (vmax >= SWSRC_TELEMETRY_STREAMING) {
        addSectionCtrl(STR_CHAR_TELEMETRY, 3);
      }

      auto body = new Window(contentForm, rect_t{0, 0, DLG_W-42, DLG_H-64});
      body->padAll(0);

      auto bodyForm = new FormWindow(body, rect_t{});
      bodyForm->setFlexLayout(LV_FLEX_FLOW_COLUMN, 0);
      bodyForm->padAll(0);

      switches = new Window(bodyForm, rect_t{0, 0, DLG_W-42, DLG_H-64});
      switches->padAll(0);
      lv_obj_set_style_border_width(switches->getLvObj(), 1, 0);
      lv_obj_set_style_border_color(switches->getLvObj(), makeLvColor(COLOR_THEME_SECONDARY2), 0);
      lv_obj_set_style_border_opa(switches->getLvObj(), LV_OPA_100, 0);
      lv_obj_set_scrollbar_mode(switches->getLvObj(), LV_SCROLLBAR_MODE_AUTO);
  
      switchesForm = new FormWindow(switches, rect_t{});
      switchesForm->setFlexLayout(LV_FLEX_FLOW_COLUMN, 4);
      switchesForm->padAll(4);
      lv_obj_set_flex_align(tabsForm->getLvObj(), LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_AROUND);

      addSection(STR_MENU_SWITCHES, SWSRC_FIRST_SWITCH, SWSRC_LAST_SWITCH, 0);

      if (vmax >= SWSRC_FIRST_TRIM) {
        addSection(STR_MENU_TRIMS, SWSRC_FIRST_TRIM, SWSRC_LAST_TRIM, 1);
      }

      if (vmax >= SWSRC_FIRST_LOGICAL_SWITCH) {
        addSection(STR_MENU_LOGICAL_SWITCHES, SWSRC_FIRST_LOGICAL_SWITCH, SWSRC_LAST_LOGICAL_SWITCH, 2);
      }

      if (vmax >= SWSRC_TELEMETRY_STREAMING) {
        addSection(STR_MENU_TELEMETRY, SWSRC_TELEMETRY_STREAMING, SWSRC_RADIO_ACTIVITY, 3);
      }

      auto ftr = new Window(form, rect_t{0, 0, DLG_W-2, 40});
      ftr->padAll(0);
      lv_obj_set_style_border_width(ftr->getLvObj(), 1, 0);
      lv_obj_set_style_border_color(ftr->getLvObj(), makeLvColor(COLOR_THEME_SECONDARY2), 0);
      lv_obj_set_style_border_opa(ftr->getLvObj(), LV_OPA_100, 0);

      auto ftrForm = new FormWindow(ftr, rect_t{});
      ftrForm->setFlexLayout(LV_FLEX_FLOW_ROW, 2);
      ftrForm->padAll(0);
      lv_obj_set_flex_align(ftrForm->getLvObj(), LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_AROUND);

      auto ftrL = new Window(ftrForm, rect_t{0, 0, (DLG_W-10)/2-20, 40});
      ftrL->padAll(0);

      auto ftrFormL = new FormWindow(ftrL, rect_t{});
      ftrFormL->setFlexLayout(LV_FLEX_FLOW_ROW, 2);
      ftrFormL->padAll(4);
      ftrFormL->padTop(6);
      lv_obj_set_flex_align(ftrFormL->getLvObj(), LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_AROUND);

      actionBtns[0] = new TextButton(ftrFormL, rect_t{0, 0, 50, 28}, "!",
                                     [=]() -> uint8_t {
                                       setInverted();
                                       return inverted;
                                     });
      actionBtns[0]->check(inverted);

      actionBtns[1] = new TextButton(ftrFormL, rect_t{0, 0, 50, 28}, "---",
                                     [=]() -> uint8_t {
                                       setSwitchValue(0);
                                       return 0;
                                     });

      auto ftrR = new Window(ftrForm, rect_t{0, 0, (DLG_W-10)/2+20, 40});
      ftrR->padAll(0);

      auto ftrFormR = new FormWindow(ftrR, rect_t{});
      ftrFormR->setFlexLayout(LV_FLEX_FLOW_ROW, 2);
      ftrFormR->padAll(4);
      ftrFormR->padTop(6);
      lv_obj_set_flex_align(ftrFormR->getLvObj(), LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_AROUND);

      actionBtns[2] = new TextButton(ftrFormR, rect_t{0, 0, 80, 28}, STR_CANCEL,
                                     [=]() -> uint8_t {
                                       m_onCancel();
                                       deleteLater();
                                       return 0;
                                     });

      actionBtns[3] = new TextButton(ftrFormR, rect_t{0, 0, 80, 28}, STR_SAVE,
                                     [=]() -> uint8_t {
                                       m_onSave();
                                       deleteLater();
                                       return 0;
                                     });

      sectionBtn[0]->setFocus(0);
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
        sectionBtn[filterSection >= 0 ? filterSection : 0]->setFocus(0);
      }
#if defined(KEYS_GPIO_REG_PGUP)
      else if (event == EVT_KEY_BREAK(KEY_PGUP)) {
#else
      else if (event == EVT_KEY_LONG(KEY_PGDN)) {
        killEvents(event);
#endif
        actionSel += 1;
        if (actionSel > 3)
          actionSel = -1;
        if (actionSel >= 0) {
          lv_group_focus_obj(actionBtns[actionSel]->getLvObj());
        } else {
          sectionBtn[filterSection >= 0 ? filterSection : 0]->setFocus(0);
        }
      }
    }
};

class SwitchChoiceMenuToolbar : public MenuToolbar
{
 public:
  SwitchChoiceMenuToolbar(SwitchChoice* choice, Menu* menu) :
      MenuToolbar(choice, menu)
  {
    addButton(STR_CHAR_SWITCH, SWSRC_FIRST_SWITCH, SWSRC_LAST_SWITCH);
    addButton(STR_CHAR_TRIM, SWSRC_FIRST_TRIM, SWSRC_LAST_TRIM);
    addButton(STR_CHAR_SWITCH, SWSRC_FIRST_LOGICAL_SWITCH,
              SWSRC_LAST_LOGICAL_SWITCH);
  }
};

void SwitchChoice::LongPressHandler(void* data)
{
  SwitchChoice* sc = (SwitchChoice*)data;
  sc->setValue(-sc->getIntValue());
}

SwitchChoice::SwitchChoice(Window* parent, const rect_t& rect, int vmin,
                           int vmax, std::function<int16_t()> getValue,
                           std::function<void(int16_t)> setValue) :
    Choice(parent, rect, vmin, vmax, getValue, setValue)
{
  switchValue = _getValue();

  setTextHandler([=](int value) {
    if (!isSwitchAvailableInMixes(value))
      return std::to_string(0);  // we will fix this later

    return std::string(getSwitchPositionName(value));
  });
 
  set_lv_LongPressHandler(LongPressHandler, this);
}

void SwitchChoice::openMenu()
{
  switchValue = _getValue();
  new SwitchDialog(this, vmax,
                   [=]() -> int16_t {
                     return switchValue;
                   },
                   [=](int16_t newValue) {
                     switchValue = newValue;
                   },
                   [=]() {
                     setValue(switchValue);
                   },
                   [=]() {
                     switchValue = _getValue();
                   }
                  );
}
