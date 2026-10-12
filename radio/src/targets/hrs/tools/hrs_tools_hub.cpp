/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * HRS Tools hub — pick among EXT measure / test tools.
 */

#include "hrs_tools_hub.h"

#include "edgetx.h"
#include "hrs_tool.h"
#include "servo_tester.h"
#include "dshot_tester.h"
#include "logic_measure.h"
#include "auto_measure.h"

namespace {

static const char TITLE_HUB[] = "HRS Tools";

struct HubItem {
  const char* label;
  const char* hint;
  void (*open)(void);
};

static const HubItem kItems[] = {
    {"Auto Measure", "RX protocol auto-detect", v15AutoMeasureOpen},
    {"Pulse Scope", "Logic / pulse waveform", v15LogicMeasureOpen},
    {"Servo Tester", "PWM output test", v15ServoTesterOpen},
    {"DShot Tester", "ESC DShot output test", v15DshotTesterOpen},
};

class HrsToolsHubDialog : public HRSTool
{
 public:
  HrsToolsHubDialog() : HRSTool()
  {
    buildHeader(TITLE_HUB);

    LAYOUT_VAL_SCALED(BTN_H, 56)
    LAYOUT_VAL_SCALED(GAP, 10)
    LAYOUT_VAL_SCALED(HINT_H, 14)

    const coord_t contentH = LCD_H - MAIN_TOP - FOOTER_H - PAD_MEDIUM;
    auto* card = createPanel({PAD_MEDIUM, MAIN_TOP, LCD_W - PAD_MEDIUM * 2, contentH});

    new HRSText(card, {PAD_SMALL, PAD_TINY, 0, 0}, "Select Ext tool", HRS_COLOR_MUTED,
                FONT(XS));

    const int cols = 2;
    const int n = (int)(sizeof(kItems) / sizeof(kItems[0]));
    const coord_t innerW = LCD_W - PAD_MEDIUM * 2 - PAD_TINY * 2;
    const coord_t btnW = (innerW - PAD_SMALL * 2 - GAP) / cols;
    coord_t y0 = PAD_SMALL + HINT_H + GAP;

    for (int i = 0; i < n; ++i) {
      const HubItem& it = kItems[i];
      const int row = i / cols;
      const int col = i % cols;
      const coord_t x = PAD_SMALL + col * (btnW + GAP);
      const coord_t y = y0 + row * (BTN_H + HINT_H + GAP);

      auto* btn = new HRSButton(card, {x, y, btnW, BTN_H}, it.label, [=]() {
        // Keep hub underneath so RTN returns here (one level at a time).
        it.open();
        return 0;
      });
      if (btn && btn->getLvObj()) {
        lv_obj_set_style_border_color(btn->getLvObj(), colElecBlue(), LV_PART_MAIN);
        lv_obj_set_style_text_color(btn->getLvObj(), colElecBlue(), LV_PART_MAIN);
      }
      new HRSText(card, {x, y + BTN_H + 2, btnW, 0}, it.hint, HRS_COLOR_MUTED,
                  CENTERED | FONT(XS));
    }

    auto* foot = footerPanel();
    new HRSText(foot, rect_t{}, "Ext  GND / +5V / SIG", HRS_COLOR_MUTED, FONT(XS));
    new HRSText(foot, {0, 0, LV_PCT(100), 0}, "RTN = Back", HRS_COLOR_ELEC_BLUE,
                RIGHT | FONT(XS));
  }
};

}  // namespace

void v15HrsToolsOpen(void)
{
  new HrsToolsHubDialog();
}
