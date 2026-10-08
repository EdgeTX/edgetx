/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * ColorLCD Pulse Scope UI for V15 EXT digital waveform.
 */

#include "logic_measure.h"

#include "edgetx.h"
#include "button.h"
#include "hrs_tool.h"

#include <stdio.h>

namespace {

static LAYOUT_VAL_SCALED(WAVE_H_MAX, 70)
static LAYOUT_VAL_SCALED(WAVE_H_MIN, 36)
static LAYOUT_VAL_SCALED(MODE_BTN_H, 24)
static constexpr int WAVE_MAX_PTS = 96;
static constexpr int EDGE_COPY_MAX = LOGIC_MEASURE_COPY_MAX;

static const char TITLE_LOGIC[] = "Pulse Scope";

static const char* tbLabel(uint8_t tb)
{
  switch (tb) {
    case LOGIC_MEASURE_TB_100US: return "100us";
    case LOGIC_MEASURE_TB_500US: return "500us";
    case LOGIC_MEASURE_TB_1MS: return "1ms";
    case LOGIC_MEASURE_TB_2MS: return "2ms";
    case LOGIC_MEASURE_TB_5MS: return "5ms";
    case LOGIC_MEASURE_TB_10MS: return "10ms";
    case LOGIC_MEASURE_TB_20MS: return "20ms";
    case LOGIC_MEASURE_TB_50MS: return "50ms";
    default: return "?";
  }
}

class LogicMeasureDialog : public HRSTool
{
 public:
  LogicMeasureDialog() : HRSTool()
  {
    onClosing([=]() { v15LogicMeasureStop(); });

    if (!v15LogicMeasureIsActive())
      v15LogicMeasureStart();

    buildHeader(TITLE_LOGIC);
    buildLeftPanel();
    buildRightPanel();
    buildFooter();
    refreshAll(true);
  }

 protected:
  HRSText* lblLevel = nullptr;
  HRSText* lblEdges = nullptr;
  HRSText* lblHigh = nullptr;
  HRSText* lblLow = nullptr;
  HRSText* lblDuty = nullptr;
  HRSText* lblPeriod = nullptr;
  HRSText* lblPower = nullptr;
  HRSText* lblCurrent = nullptr;
  HRSText* lblSignal = nullptr;
  HRSText* lblStatus = nullptr;
  HRSText* lblTbScale = nullptr;

  lv_obj_t* waveLine = nullptr;
  lv_point_t wavePts[WAVE_MAX_PTS] = {};
  uint16_t wavePtCount = 0;
  coord_t waveInnerW = 0;
  coord_t waveH = 0;

  HRSButton* btnRun = nullptr;
  HRSButton* btnStop = nullptr;
  HRSButton* btnSingle = nullptr;
  HRSButton* tbBtn[LOGIC_MEASURE_TB_COUNT] = {};

  uint32_t dispEdges = 0xFFFFFFFFu;
  uint16_t dispHigh = 0xFFFF;
  uint16_t dispLow = 0xFFFF;
  uint16_t dispPeriod = 0xFFFF;
  uint8_t dispDuty = 0xFF;
  uint8_t dispMode = 0xFF;
  uint8_t dispTb = 0xFF;
  int16_t dispCurrent = -1;
  bool dispLevel = false;
  bool dispSignal = false;
  bool dispFault = false;
  bool dispPower = false;
  bool dispRate = false;
  tmr10ms_t lastUiRefresh = 0;

  void buildLeftPanel()
  {
    auto* leftCard = leftPanel(LEFT_W);

    LAYOUT_VAL_SCALED(H1, 14)
    LAYOUT_VAL_SCALED(H2, 20)
    LAYOUT_VAL_SCALED(H3, 22)

    coord_t y = PAD_SMALL;

    new HRSText(leftCard, {0, y, 0, 0}, "SIG level", HRS_COLOR_MUTED, FONT(XS));
    y += H1;
    lblLevel = new HRSText(leftCard, {0, y, 0, 0}, "----", HRS_COLOR_NEON_GREEN, FONT(XL));
    y += H3 + PAD_SMALL + 3;  // clear XL level glyph before Edges

    lblEdges = new HRSText(leftCard, {0, y, 0, 0}, "Edges  ---- /s", HRS_COLOR_ELEC_BLUE, FONT(XS));
    y += H2;
    lblHigh = new HRSText(leftCard, {0, y, 0, 0}, "High  ---- us", HRS_COLOR_MUTED, FONT(XS));
    y += H2;
    lblLow = new HRSText(leftCard, {0, y, 0, 0}, "Low   ---- us", HRS_COLOR_MUTED, FONT(XS));
    y += H2;
    lblDuty = new HRSText(leftCard, {0, y, 0, 0}, "Duty  ---- %", HRS_COLOR_MUTED, FONT(XS));
    y += H2;
    lblPeriod = new HRSText(leftCard, {0, y, 0, 0}, "Per   ---- us", HRS_COLOR_MUTED, FONT(XS));
    y += H3 - 2;  // PWR up 2px

    lblPower = new HRSText(leftCard, {0, y, 0, 0}, "PWR ---", HRS_COLOR_NEON_GREEN, FONT(STD));
    y += H2;
    lblCurrent = new HRSText(leftCard, {0, y, 0, 0}, "I --- mA", HRS_COLOR_NEON_GREEN, FONT(STD));
    y += H2;
    lblSignal = new HRSText(leftCard, {0, y, 0, 0}, "NO SIGNAL", HRS_COLOR_WARN, FONT(XS));
  }

  void buildRightPanel()
  {
    const coord_t cardH = LCD_H - MAIN_TOP - FOOTER_H - PAD_MEDIUM;
    auto rightCard = createPanel({RIGHT_X, MAIN_TOP, RIGHT_W, cardH});

    LAYOUT_VAL_SCALED(H1, 18)
    LAYOUT_VAL_SCALED(H2, 26)
    LAYOUT_VAL_SCALED(STATUS_H, 16)

    // Reserve space under the wave so mode + timebase rows are never clipped.
    // createPanel() also applies PAD_TINY on all sides.
    const coord_t panelPad = PAD_TINY * 2;
    const coord_t controlsH =
        H1 +                    // "Digital waveform"
        PAD_TINY + H1 +         // timebase scale text
        H2 +                    // Run/Stop/Single
        (MODE_BTN_H + PAD_TINY) * 2 +  // two timebase rows
        STATUS_H + PAD_SMALL;
    waveH = cardH - panelPad - PAD_SMALL - controlsH;
    if (waveH > WAVE_H_MAX) waveH = WAVE_H_MAX;
    if (waveH < WAVE_H_MIN) waveH = WAVE_H_MIN;

    coord_t y = 0;

    new HRSText(rightCard, {PAD_SMALL, y, 0, 0}, "Digital waveform", HRS_COLOR_MUTED, FONT(XS));
    y += H1;

    const coord_t barW = RIGHT_W - PAD_MEDIUM;
    auto wave = createPanel(rightCard, {PAD_TINY, y, barW, waveH});
    auto waveBg = wave->getLvObj();

    waveInnerW = barW - PAD_TINY * 2 - 2;

    // Mid reference
    auto mid = lv_line_create(waveBg);
    static lv_point_t midPts[2];
    midPts[0] = {0, (lv_coord_t)(waveH / 2)};
    midPts[1] = {(lv_coord_t)waveInnerW, (lv_coord_t)(waveH / 2)};
    lv_line_set_points(mid, midPts, 2);
    lv_obj_set_style_line_width(mid, 1, LV_PART_MAIN);
    lv_obj_set_style_line_color(mid, colCardBorder(), LV_PART_MAIN);
    lv_obj_set_style_line_opa(mid, LV_OPA_40, LV_PART_MAIN);

    waveLine = lv_line_create(waveBg);
    lv_obj_set_style_line_width(waveLine, PAD_TINY, LV_PART_MAIN);
    lv_obj_set_style_line_color(waveLine, colMuted(), LV_PART_MAIN | HRS_COLOR_MUTED);
    lv_obj_set_style_line_color(waveLine, colNeonGreen(), LV_PART_MAIN | HRS_COLOR_NEON_GREEN);
    lv_obj_set_style_line_color(waveLine, colDanger(), LV_PART_MAIN | HRS_COLOR_DANGER);
    lv_obj_set_style_line_color(waveLine, colWarn(), LV_PART_MAIN | HRS_COLOR_WARN);
    lv_obj_set_style_line_rounded(waveLine, false, LV_PART_MAIN);

    y += waveH + PAD_TINY;
    lblTbScale = new HRSText(rightCard, {PAD_SMALL, y - 3, barW, 0},
                             "div = 5 ms (full width)", HRS_COLOR_MUTED, FONT(XS));
    y += H1;

    // Run / Stop / Single
    const coord_t btnW = (barW - PAD_TINY * 2) / 3;
    btnRun = new HRSButton(rightCard, {PAD_SMALL, y, btnW, MODE_BTN_H}, "Run",
        [=]() -> uint8_t {
          v15LogicMeasureRun();
          refreshAll(true);
          return updateModeButtons(LOGIC_MEASURE_MODE_RUN);
        });
    btnStop = new HRSButton(rightCard, {PAD_SMALL + btnW + PAD_TINY, y, btnW, MODE_BTN_H}, "Stop",
        [=]() -> uint8_t {
          v15LogicMeasureStopCapture();
          refreshAll(true);
          return updateModeButtons(LOGIC_MEASURE_MODE_STOP);
        });
    btnSingle = new HRSButton(rightCard, {PAD_SMALL + (btnW + PAD_TINY) * 2, y, btnW, MODE_BTN_H}, "Single",
        [=]() -> uint8_t {
          v15LogicMeasureSingle();
          refreshAll(true);
          return updateModeButtons(LOGIC_MEASURE_MODE_SINGLE);
        });
    updateModeButtons(0);
    y += H2;

    // Timebase: 2 rows × 4 (last row fully filled)
    static const uint8_t kTbOrder[LOGIC_MEASURE_TB_COUNT] = {
        LOGIC_MEASURE_TB_100US, LOGIC_MEASURE_TB_500US, LOGIC_MEASURE_TB_1MS,
        LOGIC_MEASURE_TB_2MS,   LOGIC_MEASURE_TB_5MS,   LOGIC_MEASURE_TB_10MS,
        LOGIC_MEASURE_TB_20MS,  LOGIC_MEASURE_TB_50MS,
    };
    const int perRow = 4;
    const coord_t tbW = (barW - PAD_TINY * (perRow - 1)) / perRow;
    for (int i = 0; i < LOGIC_MEASURE_TB_COUNT; ++i) {
      const uint8_t tb = kTbOrder[i];
      const int row = i / perRow;
      const int col = i % perRow;
      const coord_t bx = PAD_SMALL + col * (tbW + PAD_TINY);
      const coord_t by = y + row * (MODE_BTN_H + PAD_TINY);
      tbBtn[tb] = new HRSButton(
          rightCard, {bx, by, tbW, MODE_BTN_H}, tbLabel(tb), [=]() -> uint8_t {
            v15LogicMeasureSetTimebase(tb);
            refreshAll(true);
            return updateTimeBaseButtons(i);
          });
    }
    updateTimeBaseButtons(0);
    y += (MODE_BTN_H + PAD_TINY) * 2;

    lblStatus = new HRSText(rightCard, {PAD_SMALL, y, barW, STATUS_H},
                            "Connect signal to Ext SIG", HRS_COLOR_MUTED, FONT(XS));
  }

  void buildFooter()
  {
    auto* foot = footerPanel();
    new HRSText(foot, rect_t{}, "Ext  LOGIC IN", HRS_COLOR_MUTED, FONT(XS));
    new HRSText(foot, {0, 0, LV_PCT(100), 0}, "RTN = Exit", HRS_COLOR_ELEC_BLUE,
                RIGHT | FONT(XS));
  }

  bool updateModeButtons(int btnMode)
  {
    const uint8_t mode = v15LogicMeasureGetMode();
    btnRun->check(mode == LOGIC_MEASURE_MODE_RUN);
    btnStop->check(mode == LOGIC_MEASURE_MODE_STOP);
    btnSingle->check(mode == LOGIC_MEASURE_MODE_SINGLE);

    return btnMode == mode;
  }

  bool updateTimeBaseButtons(int btn)
  {
    const uint8_t tb = v15LogicMeasureGetTimebase();
    for (int i = 0; i < LOGIC_MEASURE_TB_COUNT; ++i) {
      tbBtn[i]->check(tb == (uint8_t)i);
    }

    return btn == tb;
  }

  void updateWaveform()
  {
    if (!waveLine) return;

    const lv_coord_t x0 = 0;
    const lv_coord_t xEnd = (lv_coord_t)waveInnerW;
    const lv_coord_t yHigh = PAD_TINY + 2;
    const lv_coord_t yLow = (lv_coord_t)(waveH - PAD_TINY - 4);
    if (yLow <= yHigh) return;
    const uint32_t winUs = v15LogicMeasureGetTimebaseUs();
    if (winUs == 0) return;

    LogicMeasureEdge edges[EDGE_COPY_MAX];
    uint8_t startLevel = 0;
    const uint16_t n =
        v15LogicMeasureCopyEdges(edges, EDGE_COPY_MAX, &startLevel);

    auto usToX = [&](uint32_t us) -> lv_coord_t {
      if (us >= winUs) return xEnd;
      return x0 + (lv_coord_t)((us * (uint32_t)waveInnerW) / winUs);
    };

    if (n == 0 && !v15LogicMeasureHasSignal()) {
      wavePts[0] = {x0, yLow};
      wavePts[1] = {xEnd, yLow};
      wavePtCount = 2;
      lv_line_set_points(waveLine, wavePts, wavePtCount);
      setColor(waveLine, HRS_COLOR_MUTED);
      return;
    }

    uint16_t p = 0;
    lv_coord_t y = startLevel ? yHigh : yLow;
    wavePts[p++] = {x0, y};

    uint16_t prevT = 0;
    for (uint16_t i = 0; i < n && p + 2 < WAVE_MAX_PTS; ++i) {
      const lv_coord_t x = usToX(edges[i].tUs);
      // horizontal to edge time
      if (x != wavePts[p - 1].x) {
        wavePts[p++] = {x, y};
      }
      y = edges[i].level ? yHigh : yLow;
      wavePts[p++] = {x, y};
      prevT = edges[i].tUs;
      (void)prevT;
    }

    if (p < WAVE_MAX_PTS) {
      wavePts[p++] = {xEnd, y};
    }
    if (p < 2) {
      wavePts[0] = {x0, y};
      wavePts[1] = {xEnd, y};
      p = 2;
    }
    wavePtCount = p;
    lv_line_set_points(waveLine, wavePts, wavePtCount);
    setColor(waveLine, HRS_COLOR_NEON_GREEN);
  }

  void refreshAll(bool force = false)
  {
    const bool level = v15LogicMeasureGetLevel();
    const uint32_t edges = v15LogicMeasureGetEdgesHz();
    const uint16_t hi = v15LogicMeasureGetLastHighUs();
    const uint16_t lo = v15LogicMeasureGetLastLowUs();
    const uint8_t duty = v15LogicMeasureGetDutyPercent();
    const uint16_t period = v15LogicMeasureGetPeriodUs();
    const uint8_t mode = v15LogicMeasureGetMode();
    const uint8_t tb = v15LogicMeasureGetTimebase();
    const int16_t current = v15LogicMeasureGetCurrentMa();
    const bool signal = v15LogicMeasureHasSignal();
    const bool fault = v15LogicMeasureOvercurrentFault();
    const bool power = v15LogicMeasureIsPowerOn();
    const bool rate = v15LogicMeasureIsRateLimited();

    const bool changed =
        force || level != dispLevel || edges != dispEdges || hi != dispHigh ||
        lo != dispLow || duty != dispDuty || period != dispPeriod ||
        mode != dispMode || tb != dispTb || current != dispCurrent ||
        signal != dispSignal || fault != dispFault || power != dispPower ||
        rate != dispRate;
    if (!changed && mode == LOGIC_MEASURE_MODE_STOP) return;

    dispLevel = level;
    dispEdges = edges;
    dispHigh = hi;
    dispLow = lo;
    dispDuty = duty;
    dispPeriod = period;
    dispMode = mode;
    dispTb = tb;
    dispCurrent = current;
    dispSignal = signal;
    dispFault = fault;
    dispPower = power;
    dispRate = rate;

    char buf[48];

    if (lblLevel) {
      if (fault) {
        lblLevel->setText("FAULT");
        lblLevel->setColor(HRS_COLOR_DANGER);
      } else {
        lblLevel->setText(level ? "HIGH" : "LOW");
        lblLevel->setColor(level ? HRS_COLOR_NEON_GREEN : HRS_COLOR_ELEC_BLUE);
      }
    }
    if (lblEdges) {
      snprintf(buf, sizeof(buf), "Edges  %lu /s", (unsigned long)edges);
      lblEdges->setText(buf);
    }
    if (lblHigh) {
      if (hi == 0) lblHigh->setText("High  ---- us");
      else {
        snprintf(buf, sizeof(buf), "High  %u us", (unsigned)hi);
        lblHigh->setText(buf);
      }
    }
    if (lblLow) {
      if (lo == 0) lblLow->setText("Low   ---- us");
      else {
        snprintf(buf, sizeof(buf), "Low   %u us", (unsigned)lo);
        lblLow->setText(buf);
      }
    }
    if (lblDuty) {
      if (period == 0) lblDuty->setText("Duty  ---- %");
      else {
        snprintf(buf, sizeof(buf), "Duty  %u %%", (unsigned)duty);
        lblDuty->setText(buf);
      }
    }
    if (lblPeriod) {
      if (period == 0) lblPeriod->setText("Per   ---- us");
      else {
        snprintf(buf, sizeof(buf), "Per   %u us", (unsigned)period);
        lblPeriod->setText(buf);
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
    if (lblSignal) {
      if (fault) {
        lblSignal->setText("FAULT");
        lblSignal->setColor(HRS_COLOR_DANGER);
      } else if (rate) {
        lblSignal->setText("RATE LIMITED");
        lblSignal->setColor(HRS_COLOR_WARN);
      } else if (signal) {
        lblSignal->setText("ACTIVITY");
        lblSignal->setColor(HRS_COLOR_NEON_GREEN);
      } else {
        lblSignal->setText("NO SIGNAL");
        lblSignal->setColor(HRS_COLOR_WARN);
      }
    }

    if (lblTbScale) {
      const uint32_t us = v15LogicMeasureGetTimebaseUs();
      if (us < 1000) {
        snprintf(buf, sizeof(buf), "full width = %lu us", (unsigned long)us);
      } else {
        snprintf(buf, sizeof(buf), "full width = %lu ms",
                 (unsigned long)(us / 1000u));
      }
      lblTbScale->setText(buf);
    }

    if (lblStatus) {
      if (fault) {
        lblStatus->setText("Overcurrent — check EXT wiring");
        lblStatus->setColor(HRS_COLOR_DANGER);
      } else if (rate) {
        lblStatus->setText("Too many edges — capture paused briefly");
        lblStatus->setColor(HRS_COLOR_WARN);
      } else if (mode == LOGIC_MEASURE_MODE_RUN) {
        lblStatus->setText("Live · Stop to freeze waveform");
        lblStatus->setColor(HRS_COLOR_NEON_GREEN);
      } else if (mode == LOGIC_MEASURE_MODE_SINGLE) {
        lblStatus->setText("Single · waiting / capturing one screen…");
        lblStatus->setColor(HRS_COLOR_WARN);
      } else {
        lblStatus->setText("Frozen · Run or Single to resume");
        lblStatus->setColor(HRS_COLOR_ELEC_BLUE);
      }
    }

    updateModeButtons(0);
    updateWaveform();
  }

  void onCancel() override
  {
    v15LogicMeasureStop();
    closeWindow();
  }

  void checkEvents() override
  {
    Window::checkEvents();
    v15LogicMeasureTask();

    const tmr10ms_t now = get_tmr10ms();
    if ((tmr10ms_t)(now - lastUiRefresh) >= 10) {  // ~100 ms
      lastUiRefresh = now;
      refreshAll(false);
    } else if (v15LogicMeasureOvercurrentFault() != dispFault) {
      refreshAll(true);
    }
  }
};

}  // namespace

void v15LogicMeasureOpen(void)
{
  if (v15LogicMeasureIsActive()) return;
  if (!v15LogicMeasureIsActive()) {
    v15LogicMeasureStart();
    if (!v15LogicMeasureIsActive()) {
      hrsShowExtPortUnsafeWarning();
      return;
    }
  }
  new LogicMeasureDialog();
}
