/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * ColorLCD dashboard UI for V15 SUMD measure.
 */

#include "sumd_measure.h"
#include "auto_measure.h"

#include "edgetx.h"
#include "hrs_tool.h"

namespace {

static const char TITLE_SUMD[] = "SUMD Measure";

class SumdMeasureDialog : public HRSMeasureTool
{
 public:
  SumdMeasureDialog() : HRSMeasureTool()
  {
    setCloseHandler([=]() {
      const bool fromAuto = v15AutoMeasureIsHandoff();
      v15SumdMeasureStop();
      if (fromAuto) v15AutoMeasureStop();
    });

    if (!v15SumdMeasureIsActive())
      v15SumdMeasureStart();

    buildHeader(TITLE_SUMD);
    buildLeftPanel();
    buildRightPanel();
    buildFooter();
    refreshAll(true);
  }

 protected:
  HRSText* lblPolarity = nullptr;
  HRSText* lblFrame = nullptr;
  HRSText* lblStatLine = nullptr;
  HRSText* lblPower = nullptr;
  HRSText* lblCurrent = nullptr;
  HRSText* lblSignal = nullptr;

  uint16_t dispCh[SUMD_MEASURE_MAX_CHANNELS] = {};
  uint8_t dispPolarity = 0xFF;
  uint16_t dispFrameHz = 0xFFFF;
  uint8_t dispStatus = 0xFF;
  uint8_t dispChCount = 0;
  uint32_t dispWaveSeq = 0;
  int16_t dispCurrent = -1;
  bool dispSignal = false;
  bool dispFault = false;
  bool dispPower = false;
  tmr10ms_t lastUiRefresh = 0;
  tmr10ms_t lastWaveRefresh = 0;

  void buildLeftPanel()
  {
    auto leftCard = leftPanel(LEFT_W);

    static LAYOUT_VAL_SCALED(H1, 18)
    static LAYOUT_VAL_SCALED(H2, 22)
    static LAYOUT_VAL_SCALED(H3, 20)
    static LAYOUT_VAL_SCALED(H4, 24)

    coord_t y = PAD_MEDIUM;

    new HRSText(leftCard, {0, y, 0, 0}, "SUMD 115200 8N1", HRS_COLOR_MUTED, FONT(XS));
    y += H1;

    lblPolarity = new HRSText(leftCard, {0, y, 0, 0}, "Pol  AUTO...", HRS_COLOR_WARN, FONT(STD));
    y += H2;

    lblFrame = new HRSText(leftCard, {0, y, 0, 0}, "Frame  ---- Hz", HRS_COLOR_ELEC_BLUE, FONT(XS));
    y += H3;

    lblStatLine = new HRSText(leftCard, {0, y, 0, 0}, "Stat --  Ch --", HRS_COLOR_MUTED, FONT(XS));
    y += H4;

    lblPower = new HRSText(leftCard, {0, y, 0, 0}, "PWR ---", HRS_COLOR_NEON_GREEN, FONT(STD));
    y += H1;

    lblCurrent = new HRSText(leftCard, {0, y, 0, 0}, "I --- mA", HRS_COLOR_NEON_GREEN, FONT(STD));
    y += H2;

    lblSignal = new HRSText(leftCard, {0, y, 0, 0}, "SIGNAL LOST", HRS_COLOR_WARN, FONT(XS));
    y += H2;

    new HRSText(leftCard, {0, y, 0, 0}, "Ext:\n1=GND 2=+5V 3=SUMD", HRS_COLOR_MUTED, FONT(XS));
  }

  void buildRightPanel()
  {
    waveformPanel(SUMD_MEASURE_MAX_CHANNELS, "SUMD wave - swipe L/R",
                  "ELRS: display undoes CH5↔CH8 swap");
  }

  void updateWaveform(bool hasSignal, bool fault, uint8_t polarity)
  {
    if (!waveLine || !waveBg) return;

    const lv_coord_t x0 = 0;
    const lv_coord_t xEnd = (lv_coord_t)(waveCanvasW - 1);
    const lv_coord_t yHigh = PAD_MEDIUM;
    const lv_coord_t yLow = (lv_coord_t)(WAVE_H - PAD_LARGE);
    const lv_coord_t yMid = (lv_coord_t)(WAVE_H / 2);
    const bool invertWave = (polarity == SUMD_MEASURE_POL_INVERTED);

    auto setFlat = [&](HRSColor c) {
      wavePts[0] = {x0, yMid};
      wavePts[1] = {xEnd, yMid};
      wavePtCount = 2;
      lv_line_set_points(waveLine, wavePts, wavePtCount);
      setColor(waveLine, c);
    };

    if (fault || !hasSignal) {
      setFlat(fault ? HRS_COLOR_DANGER : HRS_COLOR_MUTED);
      return;
    }

    uint8_t bytes[SUMD_MEASURE_WAVE_BYTES];
    const uint8_t n = v15SumdMeasureCopyWaveBytes(bytes, sizeof(bytes));
    if (n == 0) {
      setFlat(HRS_COLOR_WARN);
      return;
    }

    const int totalBits = (int)n * 8;
    const lv_coord_t span = (lv_coord_t)(xEnd - x0);
    if (totalBits <= 0 || span <= 0) {
      setFlat(HRS_COLOR_MUTED);
      return;
    }

    // Step bits so a 37-byte SUMD frame stays within WAVE_MAX_PTS without
    // building ~300 LVGL vertices every refresh (was starving UART poll).
    const int bitStep = (totalBits > 160) ? 2 : 1;

    uint16_t pi = 0;
    lv_coord_t x = x0;
    lv_coord_t yPrev = yMid;
    for (int bi = 0; bi < totalBits; bi += bitStep) {
      if (pi + 3 >= WAVE_MAX_PTS) break;

      const uint8_t bb = bytes[bi / 8];
      bool one = (bb >> (7 - (bi % 8))) & 1;
      if (invertWave) one = !one;
      const lv_coord_t y = one ? yHigh : yLow;
      const int biEnd = bi + bitStep;
      const lv_coord_t x1 =
          (lv_coord_t)(x0 + ((int32_t)(biEnd > totalBits ? totalBits : biEnd) * span) /
                               totalBits);

      if (bi == 0) {
        wavePts[pi++] = {x, y};
      } else if (y != yPrev) {
        wavePts[pi++] = {x, yPrev};
        wavePts[pi++] = {x, y};
      }

      if (pi + 1 >= WAVE_MAX_PTS) break;
      if (pi > 0 && wavePts[pi - 1].y == y && y == yPrev && bi > 0) {
        wavePts[pi - 1].x = x1;
      } else {
        wavePts[pi++] = {x1, y};
      }
      yPrev = y;
      x = x1;
    }
    if (pi < 2) {
      setFlat(HRS_COLOR_MUTED);
      return;
    }
    wavePtCount = pi;
    lv_line_set_points(waveLine, wavePts, wavePtCount);
    setColor(waveLine, HRS_COLOR_NEON_GREEN);
  }

  void buildFooter()
  {
    auto* foot = footerPanel();

    new HRSText(foot, rect_t{}, "Ext  SUMD IN", HRS_COLOR_MUTED, FONT(XS));
    new HRSText(foot, {0, 0, LV_PCT(100), 0}, "RTN = Exit", HRS_COLOR_ELEC_BLUE, RIGHT | FONT(XS));
  }

  void updateChannelBar(int i, uint16_t us, bool active)
  {
    if (!chBarBg[i] || !chBarFill[i]) return;
    const coord_t barW = RIGHT_W - CH_BAR_X - CH_VAL_W - PAD_SMALL * 2;
    if (!active || us < SUMD_MEASURE_CH_MIN_US) {
      lv_obj_set_width(chBarFill[i], 0);
      lv_obj_set_style_bg_color(chBarFill[i], colMuted(), LV_PART_MAIN);
      return;
    }
    uint16_t clamped = us;
    if (clamped > SUMD_MEASURE_CH_MAX_US) clamped = SUMD_MEASURE_CH_MAX_US;
    const int32_t span = SUMD_MEASURE_CH_MAX_US - SUMD_MEASURE_CH_MIN_US;
    const coord_t w =
        (coord_t)(((int32_t)(clamped - SUMD_MEASURE_CH_MIN_US) * barW) / span);
    lv_obj_set_width(chBarFill[i], w > 0 ? w : 1);
    lv_obj_set_style_bg_color(chBarFill[i],
                              active ? colNeonGreen() : colMuted(),
                              LV_PART_MAIN);
  }

  void refreshAll(bool force = false)
  {
    const uint8_t polarity = v15SumdMeasureGetPolarity();
    const uint16_t frameHz = v15SumdMeasureGetFrameHz();
    const uint8_t status = v15SumdMeasureGetStatus();
    const uint8_t chCount = v15SumdMeasureGetChannelCount();
    const int16_t current = v15SumdMeasureGetCurrentMa();
    const bool hasSignal = v15SumdMeasureHasSignal();
    const bool fault = v15SumdMeasureOvercurrentFault();
    const bool power = v15SumdMeasureIsPowerOn();

    uint16_t ch[SUMD_MEASURE_MAX_CHANNELS];
    bool chChanged = force;
    for (int i = 0; i < SUMD_MEASURE_MAX_CHANNELS; ++i) {
      ch[i] = v15SumdMeasureGetChannelUs((uint8_t)i);
      if (ch[i] != dispCh[i]) chChanged = true;
    }

    const uint32_t waveSeq = v15SumdMeasureGetWaveSeq();
    const tmr10ms_t now = get_tmr10ms();
    // Wave redraw is expensive; limit to ~5 Hz so UART polling stays ahead of
    // the 512 B RX ring (ELRS SUMD ≈ 100 Hz / 37 B).
    const bool waveDue =
        force || (tmr10ms_t)(now - lastWaveRefresh) >= 20 ||
        (hasSignal != dispSignal) || (fault != dispFault) ||
        (polarity != dispPolarity);
    const bool waveChanged =
        waveDue && (force || (waveSeq != dispWaveSeq) || (hasSignal != dispSignal) ||
                    (fault != dispFault) || (polarity != dispPolarity));

    const bool metaChanged =
        force || (polarity != dispPolarity) || (frameHz != dispFrameHz) ||
        (status != dispStatus) || (chCount != dispChCount) ||
        (hasSignal != dispSignal) || (fault != dispFault) ||
        (power != dispPower) || (current != dispCurrent);

    if (!chChanged && !metaChanged && !waveChanged) return;

    char buf[64];

    if (metaChanged || force) {
      dispPolarity = polarity;
      dispFrameHz = frameHz;
      dispStatus = status;
      dispChCount = chCount;
      dispSignal = hasSignal;
      dispFault = fault;
      dispPower = power;
      dispCurrent = current;

      if (lblPolarity) {
        if (fault) {
          lblPolarity->setText("Pol  --");
          lblPolarity->setColor(HRS_COLOR_DANGER);
        } else if (polarity == SUMD_MEASURE_POL_INVERTED) {
          lblPolarity->setText("Pol  INVERTED");
          lblPolarity->setColor(HRS_COLOR_NEON_GREEN);
        } else if (polarity == SUMD_MEASURE_POL_NORMAL) {
          lblPolarity->setText("Pol  NORMAL");
          lblPolarity->setColor(HRS_COLOR_NEON_GREEN);
        } else {
          lblPolarity->setText("Pol  AUTO...");
          lblPolarity->setColor(HRS_COLOR_WARN);
        }
      }

      if (lblFrame) {
        if (!fault && hasSignal && frameHz > 0) {
          snprintf(buf, sizeof(buf), "Frame  %u Hz", (unsigned)frameHz);
          lblFrame->setColor(HRS_COLOR_ELEC_BLUE);
        } else {
          snprintf(buf, sizeof(buf), "Frame  ---- Hz");
          lblFrame->setColor(HRS_COLOR_MUTED);
        }
        lblFrame->setText(buf);
      }

      if (lblStatLine) {
        if (!fault && hasSignal) {
          snprintf(buf, sizeof(buf), "Stat %s  Ch %u",
                   status ? "OK" : "FS/HOLD", (unsigned)chCount);
          lblStatLine->setColor(status ? HRS_COLOR_MUTED : HRS_COLOR_WARN);
        } else {
          snprintf(buf, sizeof(buf), "Stat --  Ch --");
          lblStatLine->setColor(HRS_COLOR_MUTED);
        }
        lblStatLine->setText(buf);
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

      if (lblSignal) {
        if (fault) {
          lblSignal->setText("OVERCURRENT");
          lblSignal->setColor(HRS_COLOR_DANGER);
        } else if (hasSignal) {
          lblSignal->setText("SIGNAL LOCK");
          lblSignal->setColor(HRS_COLOR_NEON_GREEN);
        } else {
          lblSignal->setText("SEARCHING...");
          lblSignal->setColor(HRS_COLOR_WARN);
        }
      }

      if (lblStatus) {
        if (fault) {
          snprintf(buf, sizeof(buf),
                   "OVERCURRENT +%d mA — external power cut",
                   SUMD_MEASURE_OC_DELTA_MA);
          lblStatus->setText(buf);
          lblStatus->setColor(HRS_COLOR_DANGER);
        } else if (hasSignal) {
          lblStatus->setText("SUMD OK · bars = TX order (ELRS CH5↔CH8 fixed)");
          lblStatus->setColor(HRS_COLOR_MUTED);
        } else {
          lblStatus->setText("Trying normal / inverted polarity...");
          lblStatus->setColor(HRS_COLOR_WARN);
        }
      }
    }

    if (chChanged || force || fault || !hasSignal) {
      for (int i = 0; i < SUMD_MEASURE_MAX_CHANNELS; ++i) {
        dispCh[i] = ch[i];
        const bool active = hasSignal && !fault && (uint8_t)i < chCount && ch[i] > 0;
        if (lblChVal[i]) {
          if (active) {
            snprintf(buf, sizeof(buf), "%u", (unsigned)ch[i]);
            lblChVal[i]->setText(buf);
            lblChVal[i]->setColor(HRS_COLOR_NEON_GREEN);
          } else {
            lblChVal[i]->setText("----");
            lblChVal[i]->setColor(HRS_COLOR_MUTED);
          }
        }
        if (lblChName[i]) {
          lblChName[i]->setColor(active ? HRS_COLOR_ELEC_BLUE : HRS_COLOR_MUTED);
        }
        updateChannelBar(i, ch[i], active);
      }
    }

    if (waveChanged) {
      dispWaveSeq = waveSeq;
      lastWaveRefresh = now;
      updateWaveform(hasSignal, fault, polarity);
    }
  }

  void onCancel() override
  {
    const bool fromAuto = v15AutoMeasureIsHandoff();
    v15SumdMeasureStop();
    if (fromAuto) v15AutoMeasureStop();
    deleteLater();
  }

  void checkEvents() override
  {
    Window::checkEvents();
    v15SumdMeasureTask();

    if (v15AutoMeasureWatchHandoffLoss(v15SumdMeasureHasSignal())) {
      v15AutoMeasureResumeSearch();
      v15SumdMeasureStop();
      deleteLater();
      v15AutoMeasureOpen();
      return;
    }

    const tmr10ms_t now = get_tmr10ms();
    if ((tmr10ms_t)(now - lastUiRefresh) >= 5) {
      lastUiRefresh = now;
      refreshAll(false);
    } else if (v15SumdMeasureOvercurrentFault() != dispFault) {
      refreshAll(false);
    }
  }
};

}  // namespace

void v15SumdMeasureOpen(void)
{
  if (v15SumdMeasureIsActive() && !v15AutoMeasureIsHandoff()) return;
  if (!v15SumdMeasureIsActive()) {
    v15SumdMeasureStart();
    if (!v15SumdMeasureIsActive()) {
      hrsShowExtPortUnsafeWarning();
      return;
    }
  }
  new SumdMeasureDialog();
}
