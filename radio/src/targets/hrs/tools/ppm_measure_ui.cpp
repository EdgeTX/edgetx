/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * ColorLCD dashboard UI for V15 PPM / CPPM measure.
 */

#include "ppm_measure.h"
#include "auto_measure.h"

#include "edgetx.h"
#include "hrs_tool.h"

namespace {

static const char TITLE_PPM[] = "PPM Measure";

class PpmMeasureDialog : public HRSMeasureTool
{
 public:
  PpmMeasureDialog() : HRSMeasureTool()
  {
    setCloseHandler([=]() {
      const bool fromAuto = v15AutoMeasureIsHandoff();
      v15PpmMeasureStop();
      if (fromAuto) v15AutoMeasureStop();
    });

    if (!v15PpmMeasureIsActive())
      v15PpmMeasureStart();

    buildHeader(TITLE_PPM);
    buildLeftPanel();
    buildRightPanel();
    buildFooter();
    refreshAll(true);
  }

 protected:
  HRSText* lblChCount = nullptr;
  HRSText* lblFrame = nullptr;
  HRSText* lblPower = nullptr;
  HRSText* lblCurrent = nullptr;
  HRSText* lblSignal = nullptr;

  uint16_t dispCh[PPM_MEASURE_MAX_CHANNELS] = {};
  uint8_t dispCount = 0xFF;
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

    new HRSText(leftCard, {0, y, 0, 0}, "CPPM frame", HRS_COLOR_MUTED, FONT(XS));
    y += H1;

    lblChCount = new HRSText(leftCard, {0, y, 0, 0}, "Channels  --", HRS_COLOR_NEON_GREEN, FONT(STD));
    y += H2;

    lblFrame = new HRSText(leftCard, {0, y, 0, 0}, "Frame  ---- Hz", HRS_COLOR_ELEC_BLUE, FONT(XS));
    y += H3;

    lblPower = new HRSText(leftCard, {0, y, 0, 0}, "PWR ---", HRS_COLOR_NEON_GREEN, FONT(STD));
    y += H1;

    lblCurrent = new HRSText(leftCard, {0, y, 0, 0}, "I --- mA", HRS_COLOR_NEON_GREEN, FONT(STD));
    y += H2;

    lblSignal = new HRSText(leftCard, {0, y, 0, 0}, "SIGNAL LOST", HRS_COLOR_WARN, FONT(XS));
    y += H2;

    new HRSText(leftCard, {0, y, 0, 0}, "Ext:\n1=GND 2=+5V 3=PPM IN", HRS_COLOR_MUTED, FONT(XS));
  }

  void buildRightPanel()
  {
    waveformPanel(PPM_MEASURE_MAX_CHANNELS, "PPM wave - swipe L/R", "Connect CPPM/PPM to Ext SIG");
  }

  void updateWaveform(bool hasSignal, bool fault)
  {
    if (!waveLine || !waveBg) return;

    const lv_coord_t x0 = 0;
    const lv_coord_t xEnd = (lv_coord_t)(waveCanvasW - 1);
    const lv_coord_t yHigh = PAD_MEDIUM;
    const lv_coord_t yLow = (lv_coord_t)(WAVE_H - PAD_LARGE);

    auto setFlat = [&](HRSColor c, lv_coord_t y) {
      wavePts[0] = {x0, y};
      wavePts[1] = {xEnd, y};
      wavePtCount = 2;
      lv_line_set_points(waveLine, wavePts, wavePtCount);
      setColor(waveLine, c);
    };

    if (fault || !hasSignal) {
      setFlat(fault ? HRS_COLOR_DANGER : HRS_COLOR_MUTED, yLow);
      return;
    }

    uint16_t chUs[PPM_MEASURE_MAX_CHANNELS];
    const uint8_t n =
        v15PpmMeasureCopyWaveChannels(chUs, PPM_MEASURE_MAX_CHANNELS);
    uint16_t frameUs = v15PpmMeasureGetWaveFrameUs();
    if (n == 0) {
      setFlat(HRS_COLOR_WARN, yLow);
      return;
    }

    // Reconstruct rising-edge CPPM: short HIGH sep + LOW remainder per CH,
    // then long LOW sync for the rest of the frame.
    uint32_t totalUs = 0;
    for (uint8_t i = 0; i < n; ++i) totalUs += chUs[i];
    if (frameUs < totalUs + PPM_MEASURE_SYNC_MIN_US) {
      frameUs = (uint16_t)(totalUs + PPM_MEASURE_SYNC_MIN_US);
    }
    const lv_coord_t span = (lv_coord_t)(xEnd - x0);
    if (frameUs == 0 || span <= 0) {
      setFlat(HRS_COLOR_MUTED, yLow);
      return;
    }

    auto usToX = [&](uint32_t us) -> lv_coord_t {
      if (us > frameUs) us = frameUs;
      return (lv_coord_t)(x0 + ((int32_t)us * span) / (int32_t)frameUs);
    };

    auto pushPt = [&](lv_coord_t x, lv_coord_t y) -> bool {
      if (wavePtCount + 1 >= WAVE_MAX_PTS) return false;
      if (wavePtCount > 0 && wavePts[wavePtCount - 1].x == x &&
          wavePts[wavePtCount - 1].y == y) {
        return true;
      }
      wavePts[wavePtCount++] = {x, y};
      return true;
    };

    wavePtCount = 0;
    uint32_t t = 0;
    if (!pushPt(x0, yLow)) {
      setFlat(HRS_COLOR_MUTED, yLow);
      return;
    }

    for (uint8_t i = 0; i < n; ++i) {
      uint16_t ch = chUs[i];
      if (ch == 0) continue;
      uint16_t sep = PPM_MEASURE_WAVE_SEP_US;
      if (sep >= ch) sep = (uint16_t)(ch / 3);
      if (sep < 1) sep = 1;

      const lv_coord_t xRise = usToX(t);
      if (!pushPt(xRise, yLow) || !pushPt(xRise, yHigh)) break;
      t += sep;
      const lv_coord_t xFall = usToX(t);
      if (!pushPt(xFall, yHigh) || !pushPt(xFall, yLow)) break;
      t += (uint32_t)(ch - sep);
      const lv_coord_t xNext = usToX(t);
      if (!pushPt(xNext, yLow)) break;
    }

    // Sync / blanking gap to end of frame
    if (!pushPt(xEnd, yLow)) {
      /* keep what we have */
    }

    if (wavePtCount < 2) {
      setFlat(HRS_COLOR_MUTED, yLow);
      return;
    }
    lv_line_set_points(waveLine, wavePts, wavePtCount);
    setColor(waveLine, HRS_COLOR_NEON_GREEN);
  }

  void buildFooter()
  {
    auto* foot = footerPanel();

    new HRSText(foot, rect_t{}, "Ext  CPPM IN", HRS_COLOR_MUTED, FONT(XS));
    new HRSText(foot, {0, 0, LV_PCT(100), 0}, "RTN = Exit", HRS_COLOR_ELEC_BLUE, RIGHT | FONT(XS));
  }

  void updateChannelBar(int i, uint16_t us, bool active)
  {
    if (!chBarBg[i] || !chBarFill[i]) return;
    const coord_t barW = RIGHT_W - CH_BAR_X - CH_VAL_W - PAD_SMALL * 2;
    if (!active || us < PPM_MEASURE_CH_MIN_US) {
      lv_obj_set_width(chBarFill[i], 0);
      lv_obj_set_style_bg_color(chBarFill[i], colMuted(), LV_PART_MAIN);
      return;
    }
    uint16_t clamped = us;
    if (clamped > PPM_MEASURE_CH_MAX_US) clamped = PPM_MEASURE_CH_MAX_US;
    const int32_t span = PPM_MEASURE_CH_MAX_US - PPM_MEASURE_CH_MIN_US;
    const coord_t w =
        (coord_t)(((int32_t)(clamped - PPM_MEASURE_CH_MIN_US) * barW) / span);
    lv_obj_set_width(chBarFill[i], w > 0 ? w : 1);
    lv_obj_set_style_bg_color(chBarFill[i],
                              active ? colNeonGreen() : colMuted(),
                              LV_PART_MAIN);
  }

  void refreshAll(bool force = false)
  {
    const uint8_t count = v15PpmMeasureGetChannelCount();
    const uint16_t frameHz = v15PpmMeasureGetFrameHz();
    const int16_t current = v15PpmMeasureGetCurrentMa();
    const bool hasSignal = v15PpmMeasureHasSignal();
    const bool fault = v15PpmMeasureOvercurrentFault();
    const bool power = v15PpmMeasureIsPowerOn();

    uint16_t ch[PPM_MEASURE_MAX_CHANNELS];
    bool chChanged = force;
    for (int i = 0; i < PPM_MEASURE_MAX_CHANNELS; ++i) {
      ch[i] = v15PpmMeasureGetChannelUs((uint8_t)i);
      if (ch[i] != dispCh[i]) chChanged = true;
    }

    const uint32_t waveSeq = v15PpmMeasureGetWaveSeq();
    const bool waveChanged =
        force || (waveSeq != dispWaveSeq) || (hasSignal != dispSignal) ||
        (fault != dispFault);

    const bool metaChanged =
        force || (count != dispCount) || (frameHz != dispFrameHz) ||
        (hasSignal != dispSignal) || (fault != dispFault) ||
        (power != dispPower) || (current != dispCurrent);

    if (!chChanged && !metaChanged && !waveChanged) return;

    char buf[48];

    if (metaChanged || force) {
      dispCount = count;
      dispFrameHz = frameHz;
      dispSignal = hasSignal;
      dispFault = fault;
      dispPower = power;
      dispCurrent = current;

      if (lblChCount) {
        if (fault) {
          lblChCount->setText("Channels  --");
          lblChCount->setColor(HRS_COLOR_DANGER);
        } else if (hasSignal) {
          snprintf(buf, sizeof(buf), "Channels  %u", (unsigned)count);
          lblChCount->setText(buf);
          lblChCount->setColor(HRS_COLOR_NEON_GREEN);
        } else {
          lblChCount->setText("Channels  --");
          lblChCount->setColor(HRS_COLOR_MUTED);
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
          lblSignal->setText("SIGNAL LOST");
          lblSignal->setColor(HRS_COLOR_WARN);
        }
      }

      if (lblStatus) {
        if (fault) {
          snprintf(buf, sizeof(buf),
                   "OVERCURRENT +%d mA — external power cut",
                   PPM_MEASURE_OC_DELTA_MA);
          lblStatus->setText(buf);
          lblStatus->setColor(HRS_COLOR_DANGER);
        } else if (hasSignal) {
          lblStatus->setText("Measuring CPPM on Ext (rising-edge)");
          lblStatus->setColor(HRS_COLOR_MUTED);
        } else {
          lblStatus->setText("Waiting for PPM frame on Ext SIG...");
          lblStatus->setColor(HRS_COLOR_WARN);
        }
      }
    }

    if (chChanged || force || fault || !hasSignal) {
      for (int i = 0; i < PPM_MEASURE_MAX_CHANNELS; ++i) {
        dispCh[i] = ch[i];
        const bool active = hasSignal && !fault && (i < (int)count) && ch[i] > 0;
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
      updateWaveform(hasSignal, fault);
    }
  }

  void onCancel() override
  {
    const bool fromAuto = v15AutoMeasureIsHandoff();
    v15PpmMeasureStop();
    if (fromAuto) v15AutoMeasureStop();
    deleteLater();
  }

  void checkEvents() override
  {
    Window::checkEvents();
    v15PpmMeasureTask();

    if (v15AutoMeasureWatchHandoffLoss(v15PpmMeasureHasSignal())) {
      v15AutoMeasureResumeSearch();
      v15PpmMeasureStop();
      deleteLater();
      v15AutoMeasureOpen();
      return;
    }

    const tmr10ms_t now = get_tmr10ms();
    if ((tmr10ms_t)(now - lastUiRefresh) >= 5) {
      lastUiRefresh = now;
      refreshAll(false);
    } else if (v15PpmMeasureOvercurrentFault() != dispFault) {
      refreshAll(false);
    }
  }
};

}  // namespace

void v15PpmMeasureOpen(void)
{
  if (v15PpmMeasureIsActive() && !v15AutoMeasureIsHandoff()) return;
  if (!v15PpmMeasureIsActive()) {
    v15PpmMeasureStart();
    if (!v15PpmMeasureIsActive()) {
      hrsShowExtPortUnsafeWarning();
      return;
    }
  }
  new PpmMeasureDialog();
}
