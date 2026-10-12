/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * ColorLCD dashboard UI for V15 DShot ESC tester.
 */

#include "dshot_tester.h"

#include "edgetx.h"
#include "hrs_tool.h"

namespace {

static LAYOUT_VAL_SCALED(KNOB_ROW_H, 36)
static LAYOUT_VAL_SCALED(MODE_BTN_H, 28)

static const char TITLE_DSHOT[] = "DShot Tester";

static const char* protoName(uint8_t p)
{
  switch (p) {
    case DSHOT_TESTER_PROTO_150: return "DShot150";
    case DSHOT_TESTER_PROTO_300: return "DShot300";
    case DSHOT_TESTER_PROTO_600: return "DShot600";
    case DSHOT_TESTER_PROTO_1200: return "DShot1200";
    default: return "DShot???";
  }
}

class DshotTesterDialog : public HRSTool
{
 public:
  DshotTesterDialog() : HRSTool()
  {
    setCloseHandler([=]() { v15DshotTesterStop(); });

    v15DshotTesterSetProtocol(DSHOT_TESTER_PROTO_300);
    v15DshotTesterSetMode(DSHOT_TESTER_MODE_MANUAL);
    v15DshotTesterSetArmed(false);
    v15DshotTesterSetThrottlePercent(0);
    if (!v15DshotTesterIsActive())
      v15DshotTesterStart();

    buildHeader(TITLE_DSHOT);
    buildLeftPanel();
    buildRightPanel();
    buildFooter();
    refreshAll(true);
  }

 protected:
  HRSText* lblThrottleBig = nullptr;
  HRSText* lblValue = nullptr;
  HRSText* lblProto = nullptr;
  HRSText* lblPower = nullptr;
  HRSText* lblCurrent = nullptr;
  HRSText* lblArm = nullptr;
  HRSText* lblKnobTitle = nullptr;

  HRSButton* protoBtn[DSHOT_TESTER_PROTO_COUNT] = {};
  HRSButton* modeBtn[3] = {};
  HRSButton* armBtn = nullptr;
  HRSButton* beepBtn[5] = {};
  HRSSlider* thrKnob = nullptr;

  uint8_t dispPct = 0xFF;
  uint16_t dispValue = 0xFFFF;
  uint8_t dispProto = 0xFF;
  uint8_t dispMode = 0xFF;
  int16_t dispCurrent = -1;
  bool dispArmed = false;
  bool dispFault = false;
  bool dispPower = false;
  bool knobDragging = false;
  tmr10ms_t lastUiRefresh = 0;

  void applyThrottlePercent(uint8_t pct)
  {
    if (v15DshotTesterOvercurrentFault()) return;
    if (v15DshotTesterGetMode() != DSHOT_TESTER_MODE_MANUAL) return;
    if (!v15DshotTesterIsArmed()) {
      pct = 0;
    }
    v15DshotTesterSetThrottlePercent(pct);
    dispPct = pct;
    if (lblThrottleBig) {
      char buf[16];
      snprintf(buf, sizeof(buf), "%u %%", (unsigned)pct);
      lblThrottleBig->setText(buf);
    }
  }

  void buildLeftPanel()
  {
    auto* leftCard = leftPanel(LEFT_W);

    LAYOUT_VAL_SCALED(H1, 12)
    LAYOUT_VAL_SCALED(H2, 34)
    LAYOUT_VAL_SCALED(H3, 18)
    LAYOUT_VAL_SCALED(H4, 20)
    LAYOUT_VAL_SCALED(H5, 16)

    coord_t y = PAD_SMALL;

    new HRSText(leftCard, {0, y, 0, 0}, "Throttle", HRS_COLOR_MUTED, FONT(XS));
    y += H1;

    lblThrottleBig = new HRSText(leftCard, {0, y, 0, 0}, "0 %", HRS_COLOR_NEON_GREEN, FONT(XL));
    y += H2;

    lblValue = new HRSText(leftCard, {0, y, 0, 0}, "Value  0", HRS_COLOR_ELEC_BLUE, FONT(XS));
    y += H3;

    lblProto = new HRSText(leftCard, {0, y, 0, 0}, "DShot300", HRS_COLOR_MUTED, FONT(XS));
    y += H3;

    lblArm = new HRSText(leftCard, {0, y, 0, 0}, "DISARMED", HRS_COLOR_WARN, FONT(STD));
    y += H4;

    lblPower = new HRSText(leftCard, {0, y, 0, 0}, "PWR ON", HRS_COLOR_NEON_GREEN, FONT(XS));
    y += H5;

    lblCurrent = new HRSText(leftCard, {0, y, 0, 0}, "I 0 mA", HRS_COLOR_NEON_GREEN, FONT(XS));
    y += H4;

    new HRSText(leftCard, {0, y, 0, 0}, "Ext:\n1=GND 2=+5V 3=DShot\nESC needs own batt", HRS_COLOR_MUTED, FONT(XS));
  }

  void buildRightPanel()
  {
    auto rightCard = createPanel({RIGHT_X, MAIN_TOP, RIGHT_W, LCD_H - MAIN_TOP - FOOTER_H - PAD_MEDIUM});
    rightCard->padLeft(PAD_SMALL);

    coord_t y = -PAD_SMALL;
    coord_t buttonW = RIGHT_W - PAD_SMALL * 2 - PAD_TINY;

    LAYOUT_VAL_SCALED(H1, 14)

    new HRSText(rightCard, {0, y, 0, 0}, "Protocol", HRS_COLOR_ELEC_BLUE, FONT(XS));
    y += PAD_LARGE * 2;

    static const char* const kProtoLbl[DSHOT_TESTER_PROTO_COUNT] = {
        "150", "300", "600", "1200"};
    const coord_t protoW = (buttonW - PAD_TINY * (DSHOT_TESTER_PROTO_COUNT - 1)) / DSHOT_TESTER_PROTO_COUNT;
    for (int i = 0; i < DSHOT_TESTER_PROTO_COUNT; ++i) {
      protoBtn[i] = new HRSButton(
          rightCard, {i * (protoW + PAD_TINY), y, protoW, MODE_BTN_H}, kProtoLbl[i],
          [=]() -> uint8_t {
            if (v15DshotTesterOvercurrentFault()) return 0;
            v15DshotTesterSetProtocol((uint8_t)i);
            refreshAll(true);
            return updateProtoButtons(i);
          });
    }
    y += MODE_BTN_H + PAD_MEDIUM;
    updateProtoButtons(0);

    new HRSText(rightCard, {0, y, 0, 0}, "Mode", HRS_COLOR_ELEC_BLUE, FONT(XS));

    static const char* const kModeLbl[3] = {"Manual", "Auto", "Stick"};
    const coord_t modeW = (buttonW - PAD_LARGE * 5 - PAD_TINY * 2) / 3;
    for (int i = 0; i < 3; ++i) {
      modeBtn[i] = new HRSButton(
          rightCard, {PAD_LARGE * 5 + i * (modeW + PAD_TINY), y, modeW, MODE_BTN_H}, kModeLbl[i],
          [=]() -> uint8_t {
            if (v15DshotTesterOvercurrentFault()) return 0;
            v15DshotTesterSetMode((uint8_t)i);
            refreshAll(true);
            return updateModeButtons(i);
          });
    }
    y += MODE_BTN_H + PAD_SMALL;
    updateModeButtons(0);

    armBtn = new HRSButton(
        rightCard, {0, y, buttonW, MODE_BTN_H}, "ARM (props of  f!)",
        [=]() -> uint8_t {
          if (v15DshotTesterOvercurrentFault()) return 0;
          const bool next = !v15DshotTesterIsArmed();
          v15DshotTesterSetArmed(next);
          if (!next) applyThrottlePercent(0);
          refreshAll(true);
          return v15DshotTesterIsArmed();
        });
    y += MODE_BTN_H + PAD_OUTLINE;

    // Manual throttle knob
    lblKnobTitle = new HRSText(rightCard, {0, y, 0, 0}, "Knob  (Manual throttle)", HRS_COLOR_ELEC_BLUE, FONT(XS));
    y += H1;

    // Manual slider
    thrKnob = new HRSSlider(rightCard, 0, y, RIGHT_W - PAD_MEDIUM * 2, 0, 100,
                        [=]() { return v15DshotTesterGetThrottlePercent(); },
                        [=](int v) {
                          if (v15DshotTesterGetMode() != DSHOT_TESTER_MODE_MANUAL ||
                              v15DshotTesterOvercurrentFault()) {
                            return;
                          }
                          if (!v15DshotTesterIsArmed()) {
                            v15DshotTesterSetThrottlePercent(0);
                            dispPct = 0;
                            if (lblThrottleBig) lblThrottleBig->setText("0 %");
                            return;
                          }
                          v15DshotTesterSetThrottlePercent((uint8_t)v);
                          dispPct = (uint8_t)v;
                          if (lblThrottleBig) {
                            char buf[16];
                            snprintf(buf, sizeof(buf), "%u %%", (unsigned)v);
                            lblThrottleBig->setText(buf);
                          }
                        });
    y += KNOB_ROW_H - PAD_TINY;

    new HRSText(rightCard, {0, y, 0, 0}, "ESC beep", HRS_COLOR_ELEC_BLUE, FONT(XS));
    y += PAD_LARGE * 2;

    static const char* const kBeepLbl[5] = {"B1", "B2", "B3", "B4", "B5"};
    const coord_t beepW = (buttonW - PAD_TINY * 4) / 5;
    for (int i = 0; i < 5; ++i) {
      beepBtn[i] = new HRSButton(
          rightCard, {i * (beepW + PAD_TINY), y, beepW, MODE_BTN_H}, kBeepLbl[i],
          [=]() -> uint8_t {
            if (v15DshotTesterOvercurrentFault()) return 0;
            v15DshotTesterSendCommand((uint16_t)(DSHOT_CMD_BEEP1 + i));
            return 0;
          });
    }

    updateModeVisibility();
  }

  void buildFooter()
  {
    auto* foot = footerPanel();

    new HRSText(foot, rect_t{}, "Ext  DSHOT OUT", HRS_COLOR_MUTED, FONT(XS));
    new HRSText(foot, {0, 0, LV_PCT(100), 0}, "RTN = Exit", HRS_COLOR_ELEC_BLUE, RIGHT | FONT(XS));
  }

  void updateModeVisibility()
  {
    const bool manual = (v15DshotTesterGetMode() == DSHOT_TESTER_MODE_MANUAL) && v15DshotTesterIsArmed();

    if (lblKnobTitle) {
      lblKnobTitle->setColor(manual ? HRS_COLOR_ELEC_BLUE : HRS_COLOR_MUTED);
    }

    if (thrKnob) {
      thrKnob->enable(manual);
    }
  }

  void refreshAll(bool force = false)
  {
    const uint8_t pct = v15DshotTesterGetThrottlePercent();
    const uint16_t value = v15DshotTesterGetOutputValue();
    const uint8_t proto = v15DshotTesterGetProtocol();
    const uint8_t mode = v15DshotTesterGetMode();
    const int16_t current = v15DshotTesterGetCurrentMa();
    const bool armed = v15DshotTesterIsArmed();
    const bool fault = v15DshotTesterOvercurrentFault();
    const bool power = v15DshotTesterIsPowerOn();

    const bool changed =
        force || pct != dispPct || value != dispValue || proto != dispProto ||
        mode != dispMode || current != dispCurrent || armed != dispArmed ||
        fault != dispFault || power != dispPower;
    if (!changed) return;

    dispPct = pct;
    dispValue = value;
    dispProto = proto;
    dispMode = mode;
    dispCurrent = current;
    dispArmed = armed;
    dispFault = fault;
    dispPower = power;

    char buf[48];
    if (lblThrottleBig && !knobDragging) {
      snprintf(buf, sizeof(buf), "%u %%", (unsigned)pct);
      lblThrottleBig->setText(buf);
      lblThrottleBig->setColor(fault ? HRS_COLOR_DANGER
                                     : (armed ? HRS_COLOR_NEON_GREEN : HRS_COLOR_MUTED));
    }
    if (lblValue) {
      snprintf(buf, sizeof(buf), "Value  %u", (unsigned)value);
      lblValue->setText(buf);
    }
    if (lblProto) lblProto->setText(protoName(proto));

    if (lblArm) {
      if (fault) {
        lblArm->setText("FAULT");
        lblArm->setColor(HRS_COLOR_DANGER);
      } else if (armed) {
        lblArm->setText("ARMED");
        lblArm->setColor(HRS_COLOR_DANGER);
      } else {
        lblArm->setText("DISARMED");
        lblArm->setColor(HRS_COLOR_WARN);
      }
    }

    if (lblPower) {
      if (fault) {
        lblPower->setText("PWR CUT");
        lblPower->setColor(HRS_COLOR_DANGER);
      } else if (power) {
        lblPower->setText("PWR ON");
        lblPower->setColor(HRS_COLOR_NEON_GREEN);
      } else {
        lblPower->setText("PWR OFF");
        lblPower->setColor(HRS_COLOR_MUTED);
      }
    }
    if (lblCurrent) {
      snprintf(buf, sizeof(buf), "I %d mA", (int)current);
      lblCurrent->setText(buf);
      lblCurrent->setColor(fault ? HRS_COLOR_DANGER : HRS_COLOR_NEON_GREEN);
    }

    updateProtoButtons(0);
    updateModeButtons(0);

    if (armBtn) {
      armBtn->setText(armed ? "DISARM" : "ARM (props off!)");
    }

    updateModeVisibility();

    if (fault) {
      lv_group_focus_obj(lvobj);
    }
  }

  bool updateProtoButtons(int btn)
  {
    const uint8_t type = v15DshotTesterGetProtocol();
    for (int i = 0; i < DSHOT_TESTER_PROTO_COUNT; ++i) {
      if (protoBtn[i])
        protoBtn[i]->check(type == i);
    }
    return btn == type;
  }

  bool updateModeButtons(int btn)
  {
    const uint8_t mode = v15DshotTesterGetMode();
    for (int i = 0; i < 3; ++i) {
      if (modeBtn[i])
        modeBtn[i]->check(mode == i);
    }
    return btn == mode;
  }

  void onCancel() override
  {
    v15DshotTesterStop();
    deleteLater();
  }

  void checkEvents() override
  {
    Window::checkEvents();
    v15DshotTesterTask();

    const tmr10ms_t now = get_tmr10ms();
    if ((tmr10ms_t)(now - lastUiRefresh) >= 5) {
      lastUiRefresh = now;
      refreshAll(false);
    } else if (v15DshotTesterOvercurrentFault() != dispFault) {
      refreshAll(false);
    }
  }
};

/** Full-screen red warning dialog. */
class DshotSafetyDialog : public HRSTool
{
 public:
  DshotSafetyDialog() : HRSTool()
  {
    lv_obj_set_style_bg_color(lvobj, colDashBg(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lvobj, LV_OPA_COVER, LV_PART_MAIN);

    auto topBar = new Window(this, {0, 0, LCD_W, EdgeTxStyles::MENU_HEADER_HEIGHT});
    lv_obj_set_style_bg_color(topBar->getLvObj(), colDanger(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(topBar->getLvObj(), LV_OPA_COVER, LV_PART_MAIN);

    new HRSText(topBar, {0, PAD_LARGE, LCD_W, 0}, "WARNING", HRS_COLOR_WHITE, CENTERED | FONT(L));

    new HRSText(this, {0, EdgeTxStyles::UI_ELEMENT_HEIGHT * 2, LCD_W, 0}, TITLE_DSHOT,
                       HRS_COLOR_DANGER, CENTERED | FONT(STD));

    new HRSText(
        this, {0, EdgeTxStyles::UI_ELEMENT_HEIGHT * 3, LCD_W, 0},
        "REMOVE PROPS before testing!\n"
        "ESC needs its own motor battery.\n"
        "Ext +5V is 5V / 1A signal power only.\n"
        "Arm required before throttle > 0.",
        HRS_COLOR_MUTED, CENTERED | FONT(STD));

    LAYOUT_VAL_SCALED(BTN_W, 160)
    LAYOUT_VAL_SCALED(BTN_H, 40)

    new HRSButton(
        this, {(LCD_W - BTN_W) / 2, LCD_H - BTN_H - 24, BTN_W, BTN_H}, "OK",
        [=]() {
          onCancel();
          v15DshotTesterStart();
          if (!v15DshotTesterIsActive()) {
            hrsShowExtPortUnsafeWarning();
            return 0;
          }
          new DshotTesterDialog();
          return 0;
        });
  }
};

}  // namespace

void v15DshotTesterOpen(void)
{
  if (v15DshotTesterIsActive()) return;
  new DshotSafetyDialog();
}
