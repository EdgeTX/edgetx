/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * ColorLCD dashboard UI for V15 CRSF measure.
 */

#include "crsf_measure.h"
#include "auto_measure.h"

#include "edgetx.h"
#include "hrs_tool.h"

namespace {

static const char TITLE_CRSF[] = "CRSF Measure";

static void formatBaud(char* buf, size_t n, uint32_t baud)
{
  if (baud == 0) {
    snprintf(buf, n, "Baud  AUTO...");
  } else if (baud >= 1000) {
    snprintf(buf, n, "Baud  %lu k", (unsigned long)(baud / 1000u));
  } else {
    snprintf(buf, n, "Baud  %lu", (unsigned long)baud);
  }
}

class CrsfMeasureDialog : public HRSMeasureTool
{
 public:
  CrsfMeasureDialog() : HRSMeasureTool()
  {
    setCloseHandler([=]() {
      const bool fromAuto = v15AutoMeasureIsHandoff();
      v15CrsfMeasureStop();
      if (fromAuto) v15AutoMeasureStop();
    });

    if (!v15CrsfMeasureIsActive())
      v15CrsfMeasureStart();

    buildHeader(TITLE_CRSF);
    buildLeftPanel();
    buildRightPanel();
    buildFooter();
    refreshAll(true);
  }

 protected:
  HRSText* lblBaud = nullptr;
  HRSText* lblPolarity = nullptr;
  HRSText* lblFrame = nullptr;
  HRSText* lblPower = nullptr;
  HRSText* lblCurrent = nullptr;
  HRSText* lblSignal = nullptr;
  StaticText* lblPinout = nullptr;

  uint16_t dispCh[CRSF_MEASURE_MAX_CHANNELS] = {};
  uint8_t dispPolarity = 0xFF;
  uint32_t dispBaud = 0xFFFFFFFFu;
  uint16_t dispFrameHz = 0xFFFF;
  uint32_t dispWaveSeq = 0;
  int16_t dispCurrent = -1;
  bool dispSignal = false;
  bool dispFault = false;
  bool dispPower = false;
  tmr10ms_t lastUiRefresh = 0;

  void buildLeftPanel()
  {
    auto leftCard = leftPanel(LEFT_W);

    static LAYOUT_VAL_SCALED(H1, 18)
    static LAYOUT_VAL_SCALED(H2, 22)
    static LAYOUT_VAL_SCALED(H3, 24)

    coord_t y = PAD_MEDIUM;

    new HRSText(leftCard, {0, y, 0, 0}, "CRSF 8N1", HRS_COLOR_MUTED, FONT(XS));
    y += H1;

    lblBaud = new HRSText(leftCard, {0, y, 0, 0}, "Baud  AUTO...", HRS_COLOR_WARN, FONT(STD));
    y += H2;

    lblPolarity = new HRSText(leftCard, {0, y, 0, 0}, "Pol  AUTO...", HRS_COLOR_WARN, FONT(STD));
    y += H2;

    lblFrame = new HRSText(leftCard, {0, y, 0, 0}, "Frame  ---- Hz", HRS_COLOR_ELEC_BLUE, FONT(XS));
    y += H3;

    lblPower = new HRSText(leftCard, {0, y, 0, 0}, "PWR ---", HRS_COLOR_NEON_GREEN, FONT(STD));
    y += H1;

    lblCurrent = new HRSText(leftCard, {0, y, 0, 0}, "I --- mA", HRS_COLOR_NEON_GREEN, FONT(STD));
    y += H2;

    lblSignal = new HRSText(leftCard, {0, y, 0, 0}, "SIGNAL LOST", HRS_COLOR_WARN, FONT(XS));
    y += H2;

    new HRSText(leftCard, {0, y, 0, 0}, "Ext:\n1=GND 2=+5V 3=CRSF\n400/420/115k", HRS_COLOR_MUTED, FONT(XS));
  }

  void buildRightPanel()
  {
    waveformPanel(CRSF_MEASURE_MAX_CHANNELS, "CRSF wave - swipe L/R", "Auto: baud + polarity search");
  }

  void updateWaveform(bool hasSignal, bool fault, uint8_t polarity)
  {
    if (!waveLine || !waveBg) return;

    const lv_coord_t x0 = 0;
    const lv_coord_t xEnd = (lv_coord_t)(waveCanvasW - 1);
    const lv_coord_t yHigh = PAD_MEDIUM;
    const lv_coord_t yLow = (lv_coord_t)(WAVE_H - PAD_LARGE);
    const lv_coord_t yMid = (lv_coord_t)(WAVE_H / 2);
    // Line bits are after UART decode; invert display so scope matches wire
    // when RX polarity is inverted (idle-low / INV CRSF).
    const bool invertWave = (polarity == CRSF_MEASURE_POL_INVERTED);

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

    uint8_t bytes[CRSF_MEASURE_WAVE_BYTES];
    const uint8_t n = v15CrsfMeasureCopyWaveBytes(bytes, sizeof(bytes));
    if (n == 0) {
      setFlat(HRS_COLOR_WARN);
      return;
    }

    // Map full frame across zoomed canvas; swipe to see rest
    const int totalBits = (int)n * 8;
    const lv_coord_t span = (lv_coord_t)(xEnd - x0);
    if (totalBits <= 0 || span <= 0) {
      setFlat(HRS_COLOR_MUTED);
      return;
    }

    uint16_t pi = 0;
    lv_coord_t x = x0;
    lv_coord_t yPrev = yMid;
    for (int bi = 0; bi < totalBits; ++bi) {
      if (pi + 3 >= WAVE_MAX_PTS) break;

      const uint8_t b = bytes[bi / 8];
      bool one = (b >> (7 - (bi % 8))) & 1;
      if (invertWave) one = !one;
      const lv_coord_t y = one ? yHigh : yLow;
      const lv_coord_t x1 =
          (lv_coord_t)(x0 + ((int32_t)(bi + 1) * span) / totalBits);

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
    auto foot = footerPanel();

    new HRSText(foot, rect_t{}, "Ext  CRSF IN", HRS_COLOR_MUTED, FONT(XS));
    new HRSText(foot, {0, 0, LV_PCT(100), 0}, "RTN = Exit", HRS_COLOR_ELEC_BLUE, RIGHT | FONT(XS));
  }

  void updateChannelBar(int i, uint16_t us, bool active)
  {
    if (!chBarBg[i] || !chBarFill[i]) return;
    const coord_t barW = RIGHT_W - CH_BAR_X - CH_VAL_W - PAD_SMALL * 2;
    if (!active || us < CRSF_MEASURE_CH_MIN_US) {
      lv_obj_set_width(chBarFill[i], 0);
      lv_obj_set_style_bg_color(chBarFill[i], colMuted(), LV_PART_MAIN);
      return;
    }
    uint16_t clamped = us;
    if (clamped > CRSF_MEASURE_CH_MAX_US) clamped = CRSF_MEASURE_CH_MAX_US;
    const int32_t span = CRSF_MEASURE_CH_MAX_US - CRSF_MEASURE_CH_MIN_US;
    const coord_t w =
        (coord_t)(((int32_t)(clamped - CRSF_MEASURE_CH_MIN_US) * barW) / span);
    lv_obj_set_width(chBarFill[i], w > 0 ? w : 1);
    lv_obj_set_style_bg_color(chBarFill[i],
                              active ? colNeonGreen() : colMuted(),
                              LV_PART_MAIN);
  }

  void refreshAll(bool force = false)
  {
    const uint8_t polarity = v15CrsfMeasureGetPolarity();
    const uint32_t baud = v15CrsfMeasureGetBaudrate();
    const uint16_t frameHz = v15CrsfMeasureGetFrameHz();
    const int16_t current = v15CrsfMeasureGetCurrentMa();
    const bool hasSignal = v15CrsfMeasureHasSignal();
    const bool fault = v15CrsfMeasureOvercurrentFault();
    const bool power = v15CrsfMeasureIsPowerOn();

    uint16_t ch[CRSF_MEASURE_MAX_CHANNELS];
    bool chChanged = force;
    for (int i = 0; i < CRSF_MEASURE_MAX_CHANNELS; ++i) {
      ch[i] = v15CrsfMeasureGetChannelUs((uint8_t)i);
      if (ch[i] != dispCh[i]) chChanged = true;
    }

    const uint32_t waveSeq = v15CrsfMeasureGetWaveSeq();
    const bool waveChanged =
        force || (waveSeq != dispWaveSeq) || (hasSignal != dispSignal) ||
        (fault != dispFault) || (polarity != dispPolarity);

    const bool metaChanged =
        force || (polarity != dispPolarity) || (baud != dispBaud) ||
        (frameHz != dispFrameHz) || (hasSignal != dispSignal) ||
        (fault != dispFault) || (power != dispPower) ||
        (current != dispCurrent);

    if (!chChanged && !metaChanged && !waveChanged) return;

    char buf[64];

    if (metaChanged || force) {
      dispPolarity = polarity;
      dispBaud = baud;
      dispFrameHz = frameHz;
      dispSignal = hasSignal;
      dispFault = fault;
      dispPower = power;
      dispCurrent = current;

      if (lblBaud) {
        if (fault) {
          lblBaud->setText("Baud  --");
          lblBaud->setColor(HRS_COLOR_DANGER);
        } else {
          formatBaud(buf, sizeof(buf), hasSignal ? baud : 0);
          lblBaud->setText(buf);
          lblBaud->setColor(hasSignal ? HRS_COLOR_NEON_GREEN : HRS_COLOR_WARN);
        }
      }

      if (lblPolarity) {
        if (fault) {
          lblPolarity->setText("Pol  --");
          lblPolarity->setColor(HRS_COLOR_DANGER);
        } else if (polarity == CRSF_MEASURE_POL_INVERTED) {
          lblPolarity->setText("Pol  INVERTED");
          lblPolarity->setColor(HRS_COLOR_NEON_GREEN);
        } else if (polarity == CRSF_MEASURE_POL_NORMAL) {
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
                   CRSF_MEASURE_OC_DELTA_MA);
          lblStatus->setText(buf);
          lblStatus->setColor(HRS_COLOR_DANGER);
        } else if (hasSignal) {
          lblStatus->setText("Measuring CRSF on Ext (CHANNELS 0x16)");
          lblStatus->setColor(HRS_COLOR_MUTED);
        } else {
          lblStatus->setText("Trying 400/420/115k × NORM/INV...");
          lblStatus->setColor(HRS_COLOR_WARN);
        }
      }
    }

    if (chChanged || force || fault || !hasSignal) {
      for (int i = 0; i < CRSF_MEASURE_MAX_CHANNELS; ++i) {
        dispCh[i] = ch[i];
        const bool active = hasSignal && !fault && ch[i] > 0;
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
      updateWaveform(hasSignal, fault, polarity);
    }
  }

  void onCancel() override
  {
    const bool fromAuto = v15AutoMeasureIsHandoff();
    v15CrsfMeasureStop();
    if (fromAuto) v15AutoMeasureStop();
    deleteLater();
  }

  void checkEvents() override
  {
    Window::checkEvents();
    v15CrsfMeasureTask();

    if (v15AutoMeasureWatchHandoffLoss(v15CrsfMeasureHasSignal())) {
      v15AutoMeasureResumeSearch();
      v15CrsfMeasureStop();
      deleteLater();
      v15AutoMeasureOpen();
      return;
    }

    const tmr10ms_t now = get_tmr10ms();
    if ((tmr10ms_t)(now - lastUiRefresh) >= 5) {
      lastUiRefresh = now;
      refreshAll(false);
    } else if (v15CrsfMeasureOvercurrentFault() != dispFault) {
      refreshAll(false);
    }
  }
};

}  // namespace

void v15CrsfMeasureOpen(void)
{
  if (v15CrsfMeasureIsActive() && !v15AutoMeasureIsHandoff()) return;
  if (!v15CrsfMeasureIsActive()) {
    v15CrsfMeasureStart();
    if (!v15CrsfMeasureIsActive()) {
      hrsShowExtPortUnsafeWarning();
      return;
    }
  }
  new CrsfMeasureDialog();
}
