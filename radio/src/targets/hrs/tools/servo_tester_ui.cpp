/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * ColorLCD dashboard UI for V15 servo tester (Battery Meter visual language).
 */

#include "servo_tester.h"

#include "edgetx.h"
#include "hrs_tool.h"

#include <string>

// Layout (480x272) — same margins / palette family as Battery Meter
static LAYOUT_VAL_SCALED(PULSE_BAR_H, 46)
static LAYOUT_VAL_SCALED(KNOB_ROW_H, 36)
static LAYOUT_VAL_SCALED(MODE_BTN_H, 28)
static LAYOUT_VAL_SCALED(PRESET_BTN_H, 26)
static constexpr uint16_t WAVE_VIEW_US = 2500;  // scale 0..2500 us

static const char TITLE_SERVO[] = "Servo Tester";

class ServoTesterDialog : public HRSTool
{
 public:
  ServoTesterDialog() : HRSTool()
  {
    onClosing([=]() { v15ServoTesterStop(); });

    v15ServoTesterSetType(SERVO_TESTER_TYPE_STD);
    v15ServoTesterSetRate(SERVO_TESTER_RATE_50);
    v15ServoTesterSetMode(SERVO_TESTER_MODE_MANUAL);
    if (!v15ServoTesterIsActive())
      v15ServoTesterStart();

    buildHeader(TITLE_SERVO);
    buildLeftPanel();
    buildRightPanel();
    buildFooter();
    refreshAll(true);
  }

 protected:
  // Left summary card
  HRSText* lblPulseBig = nullptr;
  HRSText* lblRange = nullptr;
  HRSButton* typeBtn[SERVO_TESTER_TYPE_COUNT] = {};
  HRSButton* rateBtn[SERVO_TESTER_RATE_COUNT] = {};
  HRSText* lblPower = nullptr;
  HRSText* lblCurrent = nullptr;
  HRSText* lblPinout = nullptr;

  // Right controls
  HRSButton* modeBtn[3] = {nullptr, nullptr, nullptr};
  lv_obj_t* pulseWaveLine = nullptr;
  lv_point_t pulseWavePts[6] = {};
  lv_point_t tickPts[6][2];
  lv_point_t basePts[2];
  HRSSlider* pulseKnob = nullptr;
  HRSText* lblStatus = nullptr;
  HRSText* footLeft = nullptr;

  // Avoid fighting touch: only push display when values change
  uint16_t dispPulse = 0xFFFF;
  int16_t dispCurrent = -1;
  uint8_t dispMode = 0xFF;
  uint8_t dispType = 0xFF;
  uint8_t dispRate = 0xFF;
  uint16_t dispHz = 0xFFFF;
  bool dispFault = false;
  bool dispPower = false;
  tmr10ms_t lastUiRefresh = 0;
  bool knobDragging = false;

  void buildLeftPanel()
  {
    auto leftCard = leftPanel(LEFT_W, true);

    LAYOUT_VAL_SCALED(H1, 31)
    LAYOUT_VAL_SCALED(H2, 15)
    LAYOUT_VAL_SCALED(H3, 18)

    coord_t y = 0;

    new HRSText(leftCard, {0, y, 0, 0}, "Pulse width", HRS_COLOR_MUTED, FONT(XS));
    y += PAD_LARGE;

    lblPulseBig =
        new HRSText(leftCard, {0, y, 0, 0}, "1500 us", HRS_COLOR_NEON_GREEN, FONT(XL));
    y += H1;  // Range + Type block: 3px higher

    lblRange =
        new HRSText(leftCard, {0, y, 0, 0}, "1000-2000 us", HRS_COLOR_ELEC_BLUE, FONT(XS));
    y += H2;

    new HRSText(leftCard, {0, y, 0, 0}, "Type", HRS_COLOR_ELEC_BLUE, FONT(XS));
    y += H3;

    // 2×2: Std Wide / Digi Heli
    const coord_t tBtnW = (LEFT_W - PAD_MEDIUM * 2 - PAD_SMALL) / 2;
    static const char* const typeLabels[] = {"Std", "Wide", "Digi", "Heli"};
    for (int i = 0; i < SERVO_TESTER_TYPE_COUNT; ++i) {
      const int row = i / 2;
      const int col = i % 2;
      const coord_t bx = col * (tBtnW + PAD_SMALL);
      const coord_t by = y + row * (PRESET_BTN_H + PAD_THREE);
      typeBtn[i] = new HRSButton(
          leftCard, {bx, by, tBtnW, PRESET_BTN_H}, typeLabels[i],
          [=]() -> uint8_t {
            if (v15ServoTesterOvercurrentFault()) return 0;
            v15ServoTesterSetType((uint8_t)i);
            syncKnobRange();
            dispType = 0xFF;
            dispRate = 0xFF;
            refreshAll(true);
            updateRateButtons(0);
            return updateTypeButtons(i);
          });
    }
    y += 2 * (PRESET_BTN_H + PAD_THREE);  // Rate block: 3px higher vs Type
    updateTypeButtons(0);

    new HRSText(leftCard, {0, y, 0, 0}, "Rate", HRS_COLOR_ELEC_BLUE, FONT(XS));
    y += H3;

    // 2×2: 50 100 / 200 560
    static const char* const rateLabels[] = {"50Hz", "100", "200", "560"};
    for (int i = 0; i < SERVO_TESTER_RATE_COUNT; ++i) {
      const int row = i / 2;
      const int col = i % 2;
      const coord_t bx = col * (tBtnW + PAD_SMALL);
      const coord_t by = y + row * (PRESET_BTN_H + PAD_THREE);
      rateBtn[i] = new HRSButton(
          leftCard, {bx, by, tBtnW, PRESET_BTN_H}, rateLabels[i],
          [=]() -> uint8_t {
            if (v15ServoTesterOvercurrentFault()) return 0;
            v15ServoTesterSetRate((uint8_t)i);
            syncKnobRange();
            dispRate = 0xFF;
            refreshAll(true);
            updateTypeButtons(0);
            return updateRateButtons(i);
          });
    }
    y += 2 * (PRESET_BTN_H + PAD_THREE) + PAD_TINY;
    updateRateButtons(0);

    char ocBuf[32];
    snprintf(ocBuf, sizeof(ocBuf), "OC +%d mA", SERVO_TESTER_OC_DELTA_MA);

    lblPower = new HRSText(leftCard, {0, y, 0, 0}, "PWR ---", HRS_COLOR_NEON_GREEN, FONT(STD));
    y += H2;

    lblCurrent = new HRSText(leftCard, {0, y, 0, 0}, "I --- mA", HRS_COLOR_NEON_GREEN, FONT(STD));
    y += H2;

    new HRSText(leftCard, {0, y, 0, 0}, ocBuf, HRS_COLOR_WARN, FONT(XS));
    y += H2;

    lblPinout = new HRSText(leftCard, {0, y, 0, 0}, "1=GND 2=+5V 3=PWM", HRS_COLOR_MUTED, FONT(XS));
  }

  void buildRightPanel()
  {
    auto rightCard = createPanel({RIGHT_X, MAIN_TOP, RIGHT_W, LCD_H - MAIN_TOP - FOOTER_H - PAD_MEDIUM});
    rightCard->padLeft(PAD_SMALL);

    coord_t y = 0;

    LAYOUT_VAL_SCALED(H1, 15)
    LAYOUT_VAL_SCALED(H2, 14)

    // Mode title + 3 tap buttons
    new HRSText(rightCard, {0, y, 0, 0}, "Mode", HRS_COLOR_ELEC_BLUE, FONT(XS));
    y += H1;

    const coord_t btnW = (RIGHT_W - PAD_LARGE * 2 - PAD_SMALL * 2) / 3;
    static const char* const modeLabels[] = {"Manual", "Auto", "Stick"};
    for (int i = 0; i < 3; ++i) {
      const coord_t bx = i * (btnW + PAD_SMALL);
      modeBtn[i] = new HRSButton(
          rightCard, {bx, y, btnW, MODE_BTN_H}, modeLabels[i], [=]() -> uint8_t {
            if (v15ServoTesterOvercurrentFault()) return 0;
            v15ServoTesterSetMode((uint8_t)i);
            dispMode = 0xFF;
            refreshAll(false);
            return updateModeButtons(i);
          });
    }
    y += MODE_BTN_H + PAD_LARGE;
    updateModeButtons(0);

    // ---- Pulse map (dynamic PWM waveform) ----
    new HRSText(rightCard, {0, y, 0, 0}, "Pulse map", HRS_COLOR_ELEC_BLUE, FONT(XS));
    y += H1;

    const coord_t barW = RIGHT_W - PAD_MEDIUM * 2;
    auto pulsePanel = createPanel(rightCard, {0, y, barW, PULSE_BAR_H}, false, 0);
    auto pulseBarBg = pulsePanel->getLvObj();

    // Scale tick marks at exact 0/500/.../2500 positions
    static const uint16_t kScaleUs[6] = {0, 500, 1000, 1500, 2000, 2500};
    const lv_coord_t x0 = PAD_SMALL;
    const lv_coord_t xEnd = (lv_coord_t)(barW - PAD_MEDIUM);
    const lv_coord_t innerW = xEnd - x0;
    const lv_coord_t yTickTop = (lv_coord_t)(PULSE_BAR_H - PAD_LARGE - PAD_MEDIUM);
    const lv_coord_t yTickBot = (lv_coord_t)(PULSE_BAR_H - PAD_LARGE);
    for (int i = 0; i < 6; ++i) {
      const lv_coord_t tx =
          x0 + (lv_coord_t)(((int32_t)kScaleUs[i] * innerW) / WAVE_VIEW_US);
      tickPts[i][0] = {tx, yTickTop};
      tickPts[i][1] = {tx, yTickBot};
      auto line = lv_line_create(pulseBarBg);
      lv_line_set_points(line, tickPts[i], 2);
      lv_obj_set_style_line_width(line, 1, LV_PART_MAIN);
      lv_obj_set_style_line_color(line, colCardBorder(), LV_PART_MAIN);
    }

    // Idle baseline
    basePts[0] = {x0, (lv_coord_t)(PULSE_BAR_H - PAD_LARGE - PAD_TINY)};
    basePts[1] = {xEnd, (lv_coord_t)(PULSE_BAR_H - PAD_LARGE - PAD_TINY)};
    auto baseLine = lv_line_create(pulseBarBg);
    lv_line_set_points(baseLine, basePts, 2);
    lv_obj_set_style_line_width(baseLine, 1, LV_PART_MAIN);
    lv_obj_set_style_line_color(baseLine, colCardBorder(), LV_PART_MAIN);
    lv_obj_set_style_line_opa(baseLine, LV_OPA_40, LV_PART_MAIN);

    pulseWaveLine = lv_line_create(pulseBarBg);
    lv_obj_set_style_line_width(pulseWaveLine, PAD_TINY, LV_PART_MAIN);
    lv_obj_set_style_line_color(pulseWaveLine, colNeonGreen(), LV_PART_MAIN | HRS_COLOR_NEON_GREEN);
    lv_obj_set_style_line_color(pulseWaveLine, colDanger(), LV_PART_MAIN | HRS_COLOR_DANGER);
    lv_obj_set_style_line_rounded(pulseWaveLine, true, LV_PART_MAIN);
    lv_line_set_points(pulseWaveLine, pulseWavePts, 6);
    updatePulseWaveform(SERVO_TESTER_PULSE_DEFAULT_US, false);

    y += PULSE_BAR_H + 1;

    // Digit labels under ticks (pixel-aligned to waveform X)
    static const char* const kScaleTxt[6] = {"0", "500", "1000",
                                            "1500", "2000", "2500"};
    LAYOUT_VAL_SCALED(LAB_W, 36)
    for (int i = 0; i < 6; ++i) {
      const lv_coord_t tx =
          x0 + (lv_coord_t)(((int32_t)kScaleUs[i] * innerW) / WAVE_VIEW_US);
      coord_t lx = tx - LAB_W / 2;
      if (i == 0) lx = 0;
      if (i == 5) lx = barW - LAB_W;
      new HRSText(rightCard, {lx, y, LAB_W, 0}, kScaleTxt[i],
                  HRS_COLOR_MUTED,
                  (i == 0) ? FONT(XS)
                           : (i == 5 ? (RIGHT | FONT(XS))
                                     : (CENTERED | FONT(XS))));
    }
    y += H2;

    new HRSText(rightCard, {0, y, 0, 0}, "Knob", HRS_COLOR_ELEC_BLUE, FONT(XS));
    y += H2;

    // Manual slider
    pulseKnob = new HRSSlider(rightCard, 0, y, barW,
                        v15ServoTesterGetPulseMinUs(), v15ServoTesterGetPulseMaxUs(),
                        [=]() { return v15ServoTesterGetPulseUs(); },
                        [=](int v) {
                          v15ServoTesterSetPulseUs((uint16_t)v);
                          // Do NOT refreshAll here — redrawing the waveform/labels on every
                          // touch sample starves LVGL input and the knob jumps in large µs
                          // steps (servo feels stepped while moving; hold stays fine).
                          dispPulse = (uint16_t)v;
                          if (lblPulseBig) {
                            char buf[16];
                            snprintf(buf, sizeof(buf), "%u us", (unsigned)v);
                            lblPulseBig->setText(buf);
                          }
                          updatePulseWaveform(v15ServoTesterGetPulseUs(), v15ServoTesterOvercurrentFault());
                        });
    y += KNOB_ROW_H + PAD_MEDIUM;

    lblStatus = new HRSText(rightCard, {0, y, 0, 0}, "", HRS_COLOR_MUTED, FONT(XS));

    updateModeVisibility();
  }

  void buildFooter()
  {
    auto foot = footerPanel();

    footLeft = new HRSText(foot, rect_t{}, "Ext  PWM 50Hz", HRS_COLOR_MUTED, FONT(XS));

    new HRSText(foot, {0, 0, LV_PCT(100), 0}, "RTN = Exit", HRS_COLOR_ELEC_BLUE, RIGHT | FONT(XS));
  }

  void syncKnobRange()
  {
    if (pulseKnob) {
      pulseKnob->setRange(v15ServoTesterGetPulseMinUs(), v15ServoTesterGetPulseMaxUs());
    }
  }

  bool updateTypeButtons(int btn)
  {
    const uint8_t type = v15ServoTesterGetType();
    for (int i = 0; i < SERVO_TESTER_TYPE_COUNT; ++i) {
      if (typeBtn[i])
        typeBtn[i]->check(type == i);
    }
    return btn == type;
  }

  bool updateRateButtons(int btn)
  {
    const uint8_t rate = v15ServoTesterGetRate();
    for (int i = 0; i < SERVO_TESTER_RATE_COUNT; ++i) {
      if (rateBtn[i])
        rateBtn[i]->check(rate == i);
    }
    return btn == rate;
  }

  bool updateModeButtons(int btn)
  {
    const uint8_t mode = v15ServoTesterGetMode();
    for (int i = 0; i < 3; ++i) {
      if (modeBtn[i])
        modeBtn[i]->check(mode == i);
    }
    return btn == mode;
  }

  void updateModeVisibility()
  {
    // Knob only follows Manual
    if (pulseKnob) {
      pulseKnob->enable(v15ServoTesterGetMode() == SERVO_TESTER_MODE_MANUAL);
    }
  }

  void updatePulseWaveform(uint16_t pulse, bool fault)
  {
    if (!pulseWaveLine) return;

    LAYOUT_VAL_SCALED(LOW_YO, 10)

    const coord_t barW = RIGHT_W - PAD_MEDIUM * 2;
    const lv_coord_t x0 = PAD_MEDIUM;
    const lv_coord_t xEnd = (lv_coord_t)(barW - PAD_MEDIUM);
    const lv_coord_t innerW = xEnd - x0;
    const lv_coord_t yHigh = PAD_LARGE;
    const lv_coord_t yLow = (lv_coord_t)(PULSE_BAR_H - LOW_YO);

    // Same mapping as tick labels: 0..2500 us → x0..xEnd
    auto usToX = [&](uint16_t us) -> lv_coord_t {
      if (us > WAVE_VIEW_US) us = WAVE_VIEW_US;
      return x0 + (lv_coord_t)(((int32_t)us * innerW) / WAVE_VIEW_US);
    };

    uint16_t pw = pulse;
    if (pw < SERVO_TESTER_PULSE_ABS_MIN_US) pw = SERVO_TESTER_PULSE_ABS_MIN_US;
    if (pw > SERVO_TESTER_PULSE_ABS_MAX_US) pw = SERVO_TESTER_PULSE_ABS_MAX_US;

    // Short lead-in low, then rise near 0; fall edge locked to pulse us tick
    const lv_coord_t xRise = (lv_coord_t)(x0 + PAD_MEDIUM);
    lv_coord_t xFall = usToX(pw);
    if (xFall < xRise + PAD_TINY) xFall = xRise + PAD_TINY;

    pulseWavePts[0] = {x0, yLow};
    pulseWavePts[1] = {xRise, yLow};
    pulseWavePts[2] = {xRise, yHigh};
    pulseWavePts[3] = {xFall, yHigh};
    pulseWavePts[4] = {xFall, yLow};
    pulseWavePts[5] = {xEnd, yLow};

    setColor(pulseWaveLine, fault ? HRS_COLOR_DANGER : HRS_COLOR_NEON_GREEN);
  }

  void refreshAll(bool force = false)
  {
    const uint16_t pulse = v15ServoTesterGetPulseUs();
    const uint16_t pmin = v15ServoTesterGetPulseMinUs();
    const uint16_t pmax = v15ServoTesterGetPulseMaxUs();
    const uint16_t hz = v15ServoTesterGetFrameHz();
    const int16_t current = v15ServoTesterGetCurrentMa();
    const uint8_t mode = v15ServoTesterGetMode();
    const uint8_t type = v15ServoTesterGetType();
    const uint8_t rate = v15ServoTesterGetRate();
    const bool fault = v15ServoTesterOvercurrentFault();
    const bool power = v15ServoTesterIsPowerOn();

    const bool pulseChanged = force || (pulse != dispPulse);
    const bool currentChanged = force || (current != dispCurrent);
    const bool modeChanged = force || (mode != dispMode);
    const bool typeChanged = force || (type != dispType);
    const bool rateChanged = force || (rate != dispRate);
    const bool hzChanged = force || (hz != dispHz);
    const bool faultChanged = force || (fault != dispFault);
    const bool powerChanged = force || (power != dispPower);

    if (!pulseChanged && !currentChanged && !modeChanged && !typeChanged &&
        !rateChanged && !hzChanged && !faultChanged && !powerChanged) {
      return;
    }

    char buf[48];

    if (typeChanged || rateChanged || pulseChanged || force) {
      dispType = type;
      dispRate = rate;
      if (lblRange) {
        snprintf(buf, sizeof(buf), "%u-%u us", (unsigned)pmin, (unsigned)pmax);
        lblRange->setText(buf);
      }
      if (lblPinout) {
        if (type == SERVO_TESTER_TYPE_HELI760) {
          lblPinout->setText("Sig=Ext SIG\nPWR=8-13V Ext");
          lblPinout->setColor(HRS_COLOR_WARN);
        } else {
          lblPinout->setText("1=GND 2=+5V 3=PWM");
          lblPinout->setColor(HRS_COLOR_MUTED);
        }
      }
    }

    if (hzChanged || force) {
      dispHz = hz;
      if (footLeft) {
        snprintf(buf, sizeof(buf), "Ext  PWM %uHz", (unsigned)hz);
        footLeft->setText(buf);
      }
    }

    if (pulseChanged || faultChanged) {
      dispPulse = pulse;
      snprintf(buf, sizeof(buf), "%u us", (unsigned)pulse);
      if (lblPulseBig) {
        lblPulseBig->setText(buf);
        lblPulseBig->setColor(fault ? HRS_COLOR_DANGER : HRS_COLOR_NEON_GREEN);
      }
      updatePulseWaveform(pulse, fault);
    }

    if (powerChanged || faultChanged) {
      dispPower = power;
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
    }

    if (currentChanged || faultChanged) {
      dispCurrent = current;
      if (lblCurrent) {
        snprintf(buf, sizeof(buf), "I %d mA", (int)current);
        lblCurrent->setText(buf);
        lblCurrent->setColor(fault ? HRS_COLOR_DANGER : HRS_COLOR_NEON_GREEN);
      }
    }

    if (modeChanged || faultChanged || typeChanged) {
      dispMode = mode;
      dispFault = fault;
      if (lblStatus) {
        if (fault) {
          snprintf(buf, sizeof(buf),
                   "OVERCURRENT  +%d mA  — RTN to exit",
                   SERVO_TESTER_OC_DELTA_MA);
          lblStatus->setText(buf);
          lblStatus->setColor(HRS_COLOR_DANGER);
          // Focus dialog root so RTN/ESC is not eaten by a disabled child
          if (lv_group_t* g = lv_group_get_default()) {
            lv_group_add_obj(g, lvobj);
            lv_group_focus_obj(lvobj);
          }
        } else if (mode == SERVO_TESTER_MODE_AUTO) {
          lblStatus->setText("AUTO sweep active");
          lblStatus->setColor(HRS_COLOR_MUTED);
        } else if (mode == SERVO_TESTER_MODE_STICK) {
          lblStatus->setText("STICK  Ail+Ele+Rud  (mixer-timed)");
          lblStatus->setColor(HRS_COLOR_MUTED);
        } else if (type == SERVO_TESTER_TYPE_HELI760) {
          lblStatus->setText("HELI 760us/560Hz — use 8-13V EXT power!");
          lblStatus->setColor(HRS_COLOR_WARN);
        } else {
          lblStatus->setText("MANUAL  — drag Knob");
          lblStatus->setColor(HRS_COLOR_MUTED);
        }
      }
      updateModeVisibility();
    }
  }

  void onCancel() override
  {
    v15ServoTesterStop();
    closeWindow();
  }

  void checkEvents() override
  {
    Window::checkEvents();
    v15ServoTesterTask();

    // Pulse path is independent of UI now — refresh ~20 Hz for snappy display
    const tmr10ms_t now = get_tmr10ms();
    const tmr10ms_t uiPeriod = knobDragging ? 3 : 5;  // 30 ms / 50 ms
    if ((tmr10ms_t)(now - lastUiRefresh) >= uiPeriod) {
      lastUiRefresh = now;
      refreshAll(false);
    } else if (v15ServoTesterOvercurrentFault() != dispFault) {
      refreshAll(false);
    }
  }
};

/** Full-screen red power warning shown on first entry. */
class ServoPowerWarnDialog : public HRSTool
{
 public:
  ServoPowerWarnDialog() : HRSTool()
  {
    lv_obj_set_style_bg_color(lvobj, colDashBg(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lvobj, LV_OPA_COVER, LV_PART_MAIN);

    // Top red bar
    auto* topBar = new Window(this, {0, 0, LCD_W, EdgeTxStyles::MENU_HEADER_HEIGHT});
    lv_obj_set_style_bg_color(topBar->getLvObj(), colDanger(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(topBar->getLvObj(), LV_OPA_COVER, LV_PART_MAIN);

    new HRSText(topBar, {0, PAD_LARGE, LCD_W, 0}, "WARNING", HRS_COLOR_WHITE, CENTERED | FONT(L));

    new HRSText(this, {0, EdgeTxStyles::UI_ELEMENT_HEIGHT * 2, LCD_W, 0}, TITLE_SERVO,
                       HRS_COLOR_DANGER, CENTERED | FONT(STD));

    new HRSText(this, {0, EdgeTxStyles::UI_ELEMENT_HEIGHT * 3, LCD_W, 0},
        "VCC provides 5V / 1A test power.\n"
        "Overcurrent may cut power off.\n"
        "Prefer an external supply for testing!",
        HRS_COLOR_MUTED, CENTERED | FONT(STD));

    LAYOUT_VAL_SCALED(BTN_W, 160)
    LAYOUT_VAL_SCALED(BTN_H, 40)

    new HRSButton(
        this, {(LCD_W - BTN_W) / 2, LCD_H - BTN_H - PAD_LARGE * 3, BTN_W, BTN_H}, "OK",
        [=]() {
          onCancel();
          v15ServoTesterStart();
          if (!v15ServoTesterIsActive()) {
            hrsShowExtPortUnsafeWarning();
            return 0;
          }
          new ServoTesterDialog();
          return 0;
        });
  }
};

void v15ServoTesterOpen(void)
{
  if (v15ServoTesterIsActive()) return;
  new ServoPowerWarnDialog();
}
