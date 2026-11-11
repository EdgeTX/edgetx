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

// V15 AI chat module: terminal window and the "AI Mode" Radio Setup line

#include "board.h"
#include "button.h"
#include "choice.h"
#include "edgetx.h"
#include "modal_window.h"
#include "page.h"
#include "static.h"

namespace {

class V15AiTerminalWindow;
static V15AiTerminalWindow* s_aiTerminal = nullptr;

class V15AiTerminalWindow : public ModalWindow
{
 public:
  V15AiTerminalWindow() : ModalWindow(false)
  {
    setCloseHandler([]() { s_aiTerminal = nullptr; });

    auto panel = new Window(this, rect_t{0, 0, LCD_W * 8 / 10, LCD_H * 6 / 10});
    panel->setWindowFlag(OPAQUE);
    panel->padAll(PAD_SMALL);
    panel->setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_SMALL, LV_PCT(100), LV_PCT(100));
    etx_solid_bg(panel->getLvObj(), COLOR_THEME_SECONDARY1_INDEX);
    lv_obj_clear_flag(panel->getLvObj(), LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_center(panel->getLvObj());

    auto titleBar = new Window(panel, rect_t{0, 0, LV_PCT(100), 0});
    titleBar->setFlexLayout(LV_FLEX_FLOW_ROW, PAD_SMALL, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_align(titleBar->getLvObj(), LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(titleBar->getLvObj(), LV_OBJ_FLAG_SCROLLABLE);

    new StaticText(titleBar, rect_t{}, "AI Terminal", COLOR_THEME_PRIMARY2_INDEX, FONT(BOLD));
    new TextButton(titleBar, rect_t{0, 0, EdgeTxStyles::UI_ELEMENT_HEIGHT, 0}, "X", [=]() {
      deleteLater();
      return 0;
    });

    logBox = new Window(panel, rect_t{0, 0, LV_PCT(100), 0});
    logBox->setWindowFlag(OPAQUE);
    logBox->padAll(PAD_SMALL);
    logBox->setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_ZERO, LV_PCT(100), LV_SIZE_CONTENT);
    etx_solid_bg(logBox->getLvObj(), COLOR_THEME_PRIMARY3_INDEX);
    etx_scrollbar(logBox->getLvObj());
    lv_obj_add_flag(logBox->getLvObj(), LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_grow(logBox->getLvObj(), 1);

    logText = new StaticText(logBox, rect_t{0, 0, LV_PCT(100), LV_SIZE_CONTENT},
                             "AI pairing started...", COLOR_THEME_PRIMARY2_INDEX, LEFT);
    tailAnchor = new StaticText(logBox, rect_t{0, 0, LV_PCT(100), LV_SIZE_CONTENT}, " ",
                  COLOR_THEME_PRIMARY2_INDEX, LEFT);
  }

  void checkEvents() override
  {
    ModalWindow::checkEvents();

    // Cap drain per frame — pairing floods + full setText must not stall LVGL.
    constexpr uint8_t kMaxLinesPerTick = 8;
    char line[128];
    bool updated = false;
    uint8_t n = 0;
    while (n < kMaxLinesPerTick && v15AiTerminalFetchLine(line, sizeof(line))) {
      appendLine(line);
      updated = true;
      ++n;
    }

    if (updated && logText) {
      logText->setText(logContent);
      if (tailAnchor) {
        lv_obj_scroll_to_view(tailAnchor->getLvObj(), LV_ANIM_OFF);
      }
    }
  }

 private:
  Window* logBox = nullptr;
  StaticText* logText = nullptr;
  StaticText* tailAnchor = nullptr;
  std::string logContent;

  void appendLine(const char* line)
  {
    if (!line || !line[0]) {
      return;
    }

    if (!logContent.empty()) {
      logContent += "\n";
    }
    logContent += line;

    static constexpr size_t MAX_LOG_CHARS = 900;
    if (logContent.size() > MAX_LOG_CHARS) {
      logContent.erase(0, logContent.size() - MAX_LOG_CHARS);
      size_t firstNl = logContent.find('\n');
      if (firstNl != std::string::npos) {
        logContent.erase(0, firstNl + 1);
      }
    }
  }
};

void openAiTerminalImpl()
{
  if (s_aiTerminal && !s_aiTerminal->deleted()) {
    return;
  }
  s_aiTerminal = new V15AiTerminalWindow();
}

void closeAiTerminalImpl()
{
  if (s_aiTerminal && !s_aiTerminal->deleted()) {
    s_aiTerminal->deleteLater();
  }
  s_aiTerminal = nullptr;
}

}  // namespace

void v15AiTerminalUiClose()
{
  closeAiTerminalImpl();
}

void v15AiTerminalUiOpen()
{
  openAiTerminalImpl();
}

coord_t customRadioSetupLines(Window* window, coord_t y, PaddingSize padding)
{
  auto w = new SetupLine(window, y, SubPage::EDT_X, padding, "AI Mode",
    [](Window* parent, coord_t x, coord_t y) {
      auto choice = new Choice(parent, {x, y, 0, 0}, 0, 3,
                               [=]() -> int32_t { return v15AiModeGet(); },
                               [=](int32_t newValue) {
                                 v15AiModeApply(newValue);
                                 if (newValue >= 2) {
                                   v15AiTerminalUiOpen();
                                 }
                               });
      choice->setTextHandler([](uint8_t value) {
        switch (value) {
          case 0:
            return std::string("AI Off");
          case 1:
            return std::string("AI On");
          case 2:
            return std::string("AI Pairing");
          case 3:
            return std::string("AI Boot");
          default:
            return std::string("AI Off");
        }
      });
    });
  return y + w->height() + padding;
}
