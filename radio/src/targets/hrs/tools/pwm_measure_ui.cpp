/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * ColorLCD dashboard UI for V15 RX PWM measure (Servo Tester visual language).
 */

#include "pwm_measure.h"
#include "auto_measure.h"

#include "edgetx.h"
#include "button.h"
#include "hrs_tool.h"

#include <stdio.h>

namespace {

static LAYOUT_VAL_SCALED(PULSE_BAR_H, 72)
static constexpr uint16_t WAVE_VIEW_US = PWM_MEASURE_VIEW_US;

static const char TITLE_PWM[] = "PWM Measure";

static void styleDashBtn(TextButton* btn)
{
  if (!btn || !btn->getLvObj()) return;
  lv_obj_t* o = btn->getLvObj();
  lv_obj_set_style_bg_color(o, HRSTool::colBtnIdle(), LV_PART_MAIN);
  lv_obj_set_style_border_color(o, HRSTool::colElecBlue(), LV_PART_MAIN);
  lv_obj_set_style_border_width(o, 1, LV_PART_MAIN);
  lv_obj_set_style_radius(o, PAD_MEDIUM, LV_PART_MAIN);
  lv_obj_set_style_text_color(o, HRSTool::colElecBlue(), LV_PART_MAIN);
  lv_obj_set_style_pad_hor(o, PAD_LARGE, LV_PART_MAIN);
  lv_obj_set_style_pad_ver(o, PAD_TINY, LV_PART_MAIN);
}

class PwmMeasureDialog : public HRSTool
{
 public:
  PwmMeasureDialog() : HRSTool()
  {
    onClosing([=]() {
      const bool fromAuto = v15AutoMeasureIsHandoff();
      v15PwmMeasureStop();
      if (fromAuto) v15AutoMeasureStop();
    });

    if (!v15PwmMeasureIsActive())
      v15PwmMeasureStart();

    buildHeader(TITLE_PWM);
    buildLeftPanel();
    buildRightPanel();
    buildFooter();
    refreshAll(true);
  }

 protected:
  StaticText* hdrTitle = nullptr;

  HRSText* lblPulseBig = nullptr;
  HRSText* lblFreq = nullptr;
  HRSText* lblPeriod = nullptr;
  HRSText* lblMinMax = nullptr;
  HRSText* lblPower = nullptr;
  HRSText* lblCurrent = nullptr;
  HRSText* lblSignal = nullptr;

  Window* rightCard = nullptr;
  lv_obj_t* pulseBarBg = nullptr;
  lv_obj_t* pulseWaveLine = nullptr;
  lv_point_t pulseWavePts[6] = {};
  TextButton* btnReset = nullptr;
  HRSText* lblStatus = nullptr;

  uint16_t dispPulse = 0xFFFF;
  uint16_t dispPeriod = 0xFFFF;
  uint16_t dispFreq = 0xFFFF;
  uint16_t dispMin = 0xFFFF;
  uint16_t dispMax = 0xFFFF;
  int16_t dispCurrent = -1;
  bool dispSignal = false;
  bool dispFault = false;
  bool dispPower = false;
  tmr10ms_t lastUiRefresh = 0;

  void buildLeftPanel()
  {
    auto leftCard = leftPanel(LEFT_W);

    coord_t y = PAD_SMALL;

    static LAYOUT_VAL_SCALED(H1, 14);
    static LAYOUT_VAL_SCALED(H2, 38);
    static LAYOUT_VAL_SCALED(H3, 18);
    static LAYOUT_VAL_SCALED(H4, 20);

    new HRSText(leftCard, {0, y, 0, 0}, "Pulse width", HRS_COLOR_MUTED, FONT(XS));
    y += H1;

    lblPulseBig = new HRSText(leftCard, {0, y, 0, 0}, "---- us", HRS_COLOR_MUTED, FONT(XL));
    y += H2;

    lblFreq = new HRSText(leftCard, {0, y, 0, 0}, "Freq  ---- Hz", HRS_COLOR_NEON_GREEN, FONT(XS));
    y += H3;

    lblPeriod = new HRSText(leftCard, {0, y, 0, 0}, "Period  ---- us", HRS_COLOR_ELEC_BLUE, FONT(XS));
    y += H3;

    lblMinMax = new HRSText(leftCard, {0, y, 0, 0}, "Min/Max  ----/----", HRS_COLOR_ELEC_BLUE, FONT(XS));
    y += H4;

    lblPower = new HRSText(leftCard, {0, y, 0, 0}, "PWR ---", HRS_COLOR_NEON_GREEN, FONT(STD));
    y += H3;

    lblCurrent = new HRSText(leftCard, {0, y, 0, 0}, "I --- mA", HRS_COLOR_NEON_GREEN, FONT(STD));
    y += H4;

    lblSignal = new HRSText(leftCard, {0, y, 0, 0}, "SIGNAL LOST", HRS_COLOR_WARN, FONT(XS));
    y += H4;

    new HRSText(leftCard, {0, y, 0, 0}, "Ext:\n1=GND 2=+5V 3=PWM ", HRS_COLOR_MUTED, FONT(XS));
  }

  void buildRightPanel()
  {
    rightCard = createPanel({RIGHT_X, MAIN_TOP, RIGHT_W, LCD_H - MAIN_TOP - FOOTER_H - PAD_MEDIUM});

    coord_t y = PAD_LARGE;

    static LAYOUT_VAL_SCALED(H1, 18)
    static LAYOUT_VAL_SCALED(H2, 28)
    static LAYOUT_VAL_SCALED(H3, 34)

    new HRSText(rightCard, {PAD_SMALL, y, 0, 0}, "Pulse map", HRS_COLOR_ELEC_BLUE, FONT(XS));
    y += H1;

    const coord_t barW = RIGHT_W - PAD_MEDIUM * 2;
    auto barPanel = createPanel(rightCard, {PAD_SMALL, y, barW, PULSE_BAR_H});
    pulseBarBg = barPanel->getLvObj();

    static const uint16_t kScaleUs[6] = {0, 500, 1000, 1500, 2000, 2500};
    const lv_coord_t innerW = barW - PAD_TINY * 2 - 3;
    const lv_coord_t x0 = 0;
    const lv_coord_t xEnd = innerW;
    const lv_coord_t yTickTop = (lv_coord_t)(PULSE_BAR_H - 14);
    const lv_coord_t yTickBot = (lv_coord_t)(PULSE_BAR_H - 8);
    static lv_point_t tickPts[6][2];
    for (int i = 0; i < 6; ++i) {
      const lv_coord_t tx =
          x0 + (lv_coord_t)(((int32_t)kScaleUs[i] * innerW) / WAVE_VIEW_US);
      tickPts[i][0] = {tx, yTickTop};
      tickPts[i][1] = {tx, yTickBot};
      auto tick = lv_line_create(pulseBarBg);
      lv_line_set_points(tick, tickPts[i], 2);
      lv_obj_set_style_line_width(tick, 1, LV_PART_MAIN);
      lv_obj_set_style_line_color(tick, colCardBorder(), LV_PART_MAIN);
    }

    auto baseLine = lv_line_create(pulseBarBg);
    static lv_point_t basePts[2];
    basePts[0] = {x0, (lv_coord_t)(PULSE_BAR_H - PAD_LARGE)};
    basePts[1] = {xEnd, (lv_coord_t)(PULSE_BAR_H - PAD_LARGE)};
    lv_line_set_points(baseLine, basePts, 2);
    lv_obj_set_style_line_width(baseLine, 1, LV_PART_MAIN);
    lv_obj_set_style_line_color(baseLine, colCardBorder(), LV_PART_MAIN);
    lv_obj_set_style_line_opa(baseLine, LV_OPA_40, LV_PART_MAIN);

    pulseWaveLine = lv_line_create(pulseBarBg);
    lv_obj_set_style_line_width(pulseWaveLine, PAD_TINY, LV_PART_MAIN);
    lv_obj_set_style_line_color(pulseWaveLine, colMuted(), LV_PART_MAIN | HRS_COLOR_MUTED);
    lv_obj_set_style_line_color(pulseWaveLine, colNeonGreen(), LV_PART_MAIN | HRS_COLOR_NEON_GREEN);
    lv_obj_set_style_line_rounded(pulseWaveLine, true, LV_PART_MAIN);

    updatePulseWaveform(0, false);

    y += PULSE_BAR_H + PAD_TINY;

    static const char* const kScaleTxt[6] = {"0", "500", "1000",
                                            "1500", "2000", "2500"};
    static LAYOUT_VAL_SCALED(LAB_W, 36)
    for (int i = 0; i < 6; ++i) {
      const lv_coord_t tx =
          x0 + (lv_coord_t)(((int32_t)kScaleUs[i] * innerW) / WAVE_VIEW_US);
      coord_t lx = PAD_SMALL + tx - LAB_W / 2;
      if (i == 0) lx = PAD_SMALL;
      if (i == 5) lx = PAD_SMALL + barW - LAB_W;
      new HRSText(rightCard, {lx, y, LAB_W, 0}, kScaleTxt[i],
                  HRS_COLOR_MUTED,
                  (i == 0) ? FONT(XS)
                           : (i == 5 ? (RIGHT | FONT(XS))
                                     : (CENTERED | FONT(XS))));
    }
    y += H2;  // Reset and below shifted down 10px

    // Compact action row matching dashboard palette
    btnReset = new TextButton(
        rightCard, {PAD_SMALL, y, 96, 28}, "Reset", [=]() -> uint8_t {
          v15PwmMeasureResetStats();
          dispMin = 0xFFFF;
          dispMax = 0xFFFF;
          refreshAll(true);
          return 0;
        });
    styleDashBtn(btnReset);

    new HRSText(rightCard, {108, y + PAD_MEDIUM, RIGHT_W - 116, 14},
                "clear Min / Max", HRS_COLOR_MUTED, FONT(XS));
    y += H3;

    lblStatus = new HRSText(rightCard, {PAD_SMALL, y, RIGHT_W - 8, 32},
                            "Connect RX PWM to Ext SIG",
                            HRS_COLOR_MUTED, FONT(XS));
  }

  void buildFooter()
  {
    auto* foot = footerPanel();

    new HRSText(foot, rect_t{}, "Ext  PWM IN", HRS_COLOR_MUTED, FONT(XS));

    new HRSText(foot, {0, 0, LV_PCT(100), 0}, "RTN = Exit", HRS_COLOR_ELEC_BLUE, RIGHT | FONT(XS));
  }

  void updatePulseWaveform(uint16_t pulse, bool hasSignal)
  {
    if (!pulseWaveLine) return;

    const coord_t innerW = RIGHT_W - PAD_MEDIUM * 2 - PAD_TINY * 2 - 3;
    const lv_coord_t x0 = 0;
    const lv_coord_t xEnd = innerW;
    const lv_coord_t yHigh = PAD_LARGE;
    const lv_coord_t yLow = (lv_coord_t)(PULSE_BAR_H - PAD_LARGE);

    auto usToX = [&](uint16_t us) -> lv_coord_t {
      if (us > WAVE_VIEW_US) us = WAVE_VIEW_US;
      return x0 + (lv_coord_t)(((int32_t)us * innerW) / WAVE_VIEW_US);
    };

    if (!hasSignal || pulse < 1) {
      // Flat idle low line when no signal
      pulseWavePts[0] = {x0, yLow};
      pulseWavePts[1] = {xEnd, yLow};
      pulseWavePts[2] = {xEnd, yLow};
      pulseWavePts[3] = {xEnd, yLow};
      pulseWavePts[4] = {xEnd, yLow};
      pulseWavePts[5] = {xEnd, yLow};
      lv_line_set_points(pulseWaveLine, pulseWavePts, 2);
      setColor(pulseWaveLine, HRS_COLOR_MUTED);
      return;
    }

    uint16_t pw = pulse;
    if (pw > WAVE_VIEW_US) pw = WAVE_VIEW_US;

    constexpr lv_coord_t LEAD_PX = PAD_MEDIUM;
    const lv_coord_t xRise = (lv_coord_t)(x0 + LEAD_PX);
    lv_coord_t xFall = usToX(pw);
    if (xFall < xRise + 2) xFall = xRise + 2;

    pulseWavePts[0] = {x0, yLow};
    pulseWavePts[1] = {xRise, yLow};
    pulseWavePts[2] = {xRise, yHigh};
    pulseWavePts[3] = {xFall, yHigh};
    pulseWavePts[4] = {xFall, yLow};
    pulseWavePts[5] = {xEnd, yLow};

    lv_line_set_points(pulseWaveLine, pulseWavePts, 6);
    setColor(pulseWaveLine, HRS_COLOR_NEON_GREEN);
  }

  void refreshAll(bool force = false)
  {
    const uint16_t pulse = v15PwmMeasureGetPulseUs();
    const uint16_t period = v15PwmMeasureGetPeriodUs();
    const uint16_t freq = v15PwmMeasureGetFreqHz();
    const uint16_t pmin = v15PwmMeasureGetMinUs();
    const uint16_t pmax = v15PwmMeasureGetMaxUs();
    const int16_t current = v15PwmMeasureGetCurrentMa();
    const bool hasSignal = v15PwmMeasureHasSignal();
    const bool fault = v15PwmMeasureOvercurrentFault();
    const bool power = v15PwmMeasureIsPowerOn();

    const bool pulseChanged =
        force || (pulse != dispPulse) || (hasSignal != dispSignal) ||
        (fault != dispFault);
    const bool periodChanged = force || (period != dispPeriod);
    const bool freqChanged = force || (freq != dispFreq);
    const bool minMaxChanged =
        force || (pmin != dispMin) || (pmax != dispMax);
    const bool currentChanged = force || (current != dispCurrent);
    const bool powerChanged = force || (power != dispPower) || (fault != dispFault);

    if (!pulseChanged && !periodChanged && !freqChanged && !minMaxChanged &&
        !currentChanged && !powerChanged) {
      return;
    }

    char buf[48];

    if (pulseChanged) {
      dispPulse = pulse;
      dispSignal = hasSignal;
      dispFault = fault;
      if (lblPulseBig) {
        if (fault) {
          lblPulseBig->setText("OC CUT");
          lblPulseBig->setColor(HRS_COLOR_DANGER);
        } else if (hasSignal && pulse > 0) {
          snprintf(buf, sizeof(buf), "%u us", (unsigned)pulse);
          lblPulseBig->setText(buf);
          lblPulseBig->setColor(HRS_COLOR_NEON_GREEN);
        } else {
          lblPulseBig->setText("---- us");
          lblPulseBig->setColor(HRS_COLOR_MUTED);
        }
      }
      updatePulseWaveform(pulse, hasSignal && !fault);

      if (lblSignal) {
        if (fault) {
          lblSignal->setText("OVERCURRENT");
          lblSignal->setColor(HRS_COLOR_DANGER);
        } else if (hasSignal) {
          lblSignal->setText("SIGNAL LOCK");
          lblSignal->setColor(HRS_COLOR_NEON_GREEN);
        } else {
          lblSignal->setText("SIGNAL LOST");
          lblSignal->setColor(HRS_COLOR_WARN);
        }
      }

      if (lblStatus) {
        if (fault) {
          snprintf(buf, sizeof(buf),
                   "OVERCURRENT +%d mA — external power cut",
                   PWM_MEASURE_OC_DELTA_MA);
          lblStatus->setText(buf);
          lblStatus->setColor(HRS_COLOR_DANGER);
        } else if (hasSignal) {
          lblStatus->setText("Measuring RX PWM on Ext");
          lblStatus->setColor(HRS_COLOR_MUTED);
        } else {
          lblStatus->setText("Waiting for PWM...  connect RX signal to Ext SIG");
          lblStatus->setColor(HRS_COLOR_WARN);
        }
      }
    }

    if (freqChanged || pulseChanged) {
      dispFreq = freq;
      if (lblFreq) {
        if (!fault && hasSignal && freq > 0) {
          snprintf(buf, sizeof(buf), "Freq  %u Hz", (unsigned)freq);
          lblFreq->setColor(HRS_COLOR_NEON_GREEN);
        } else {
          snprintf(buf, sizeof(buf), "Freq  ---- Hz");
          lblFreq->setColor(HRS_COLOR_MUTED);
        }
        lblFreq->setText(buf);
      }
    }

    if (periodChanged || pulseChanged) {
      dispPeriod = period;
      if (lblPeriod) {
        if (!fault && hasSignal && period > 0) {
          snprintf(buf, sizeof(buf), "Period  %u us", (unsigned)period);
          lblPeriod->setColor(HRS_COLOR_ELEC_BLUE);
        } else {
          snprintf(buf, sizeof(buf), "Period  ---- us");
          lblPeriod->setColor(HRS_COLOR_MUTED);
        }
        lblPeriod->setText(buf);
      }
    }

    if (minMaxChanged) {
      dispMin = pmin;
      dispMax = pmax;
      if (lblMinMax) {
        if (pmin > 0 && pmax > 0) {
          snprintf(buf, sizeof(buf), "Min/Max  %u/%u", (unsigned)pmin,
                   (unsigned)pmax);
        } else {
          snprintf(buf, sizeof(buf), "Min/Max  ----/----");
        }
        lblMinMax->setText(buf);
      }
    }

    if (powerChanged) {
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

    if (currentChanged || fault != dispFault || force) {
      dispCurrent = current;
      if (lblCurrent) {
        snprintf(buf, sizeof(buf), "I %d mA", (int)current);
        lblCurrent->setText(buf);
        lblCurrent->setColor(fault ? HRS_COLOR_DANGER : HRS_COLOR_NEON_GREEN);
      }
    }
  }

  void onCancel() override
  {
    const bool fromAuto = v15AutoMeasureIsHandoff();
    v15PwmMeasureStop();
    if (fromAuto) v15AutoMeasureStop();
    closeWindow();
  }

  void checkEvents() override
  {
    Window::checkEvents();
    v15PwmMeasureTask();

    if (v15AutoMeasureWatchHandoffLoss(v15PwmMeasureHasSignal())) {
      v15AutoMeasureResumeSearch();
      v15PwmMeasureStop();
      closeWindow();
      v15AutoMeasureOpen();
      return;
    }

    const tmr10ms_t now = get_tmr10ms();
    // Cap display refresh at ~20 Hz (matches filter output)
    if ((tmr10ms_t)(now - lastUiRefresh) >= 5) {
      lastUiRefresh = now;
      refreshAll(false);
    } else if (v15PwmMeasureOvercurrentFault() != dispFault) {
      refreshAll(false);
    }
  }
};

}  // namespace

void v15PwmMeasureOpen(void)
{
  if (v15PwmMeasureIsActive() && !v15AutoMeasureIsHandoff()) return;
  if (!v15PwmMeasureIsActive()) {
    v15PwmMeasureStart();
    if (!v15PwmMeasureIsActive()) {
      hrsShowExtPortUnsafeWarning();
      return;
    }
  }
  new PwmMeasureDialog();
}
