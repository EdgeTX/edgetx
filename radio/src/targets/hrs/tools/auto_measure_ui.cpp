/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * Auto Measure — left panel static; right panel shows search progress.
 */

#include "auto_measure.h"

#include "edgetx.h"
#include "hrs_tool.h"
#include "pwm_measure.h"
#include "ppm_measure.h"
#include "sbus_measure.h"
#include "crsf_measure.h"
#include "sumd_measure.h"
#include "dji_rs_measure.h"
#include "mavlink_measure.h"

#include <stdio.h>

namespace {

static const char TITLE_AUTO[] = "Auto Measure";

enum {
  STEP_POWER = 0,
  STEP_PROBE,
  STEP_TRY,
  STEP_OPEN,
  STEP_COUNT,
};

static const char* protoName(uint8_t p)
{
  switch (p) {
    case AUTO_MEASURE_PROTO_PWM: return "PWM";
    case AUTO_MEASURE_PROTO_PPM: return "PPM";
    case AUTO_MEASURE_PROTO_SBUS: return "SBUS";
    case AUTO_MEASURE_PROTO_CRSF: return "CRSF";
    case AUTO_MEASURE_PROTO_SUMD: return "SUMD";
    case AUTO_MEASURE_PROTO_DJI_RS: return "DJI RS";
    case AUTO_MEASURE_PROTO_MAVLINK: return "MAVLink";
    default: return "—";
  }
}

static uint8_t stateToStep(uint8_t state)
{
  switch (state) {
    case AUTO_MEASURE_STATE_LINK: return STEP_POWER;
    case AUTO_MEASURE_STATE_PROBE: return STEP_PROBE;
    case AUTO_MEASURE_STATE_TRY: return STEP_TRY;
    case AUTO_MEASURE_STATE_LOCKED: return STEP_OPEN;
    default: return STEP_POWER;
  }
}

class AutoMeasureDialog : public HRSTool
{
 public:
  AutoMeasureDialog() : HRSTool()
  {
    // When handing off to a child measure UI we delete this dialog but must
    setCloseHandler([=]() {
      if (!v15AutoMeasureIsHandoff()) v15AutoMeasureStop();
    });
    if (!v15AutoMeasureIsActive())
      v15AutoMeasureStart();
    else
      v15AutoMeasureReattachUi();
    buildHeader(TITLE_AUTO);
    buildLeftPanel();
    buildRightPanel();
    buildFooter();
    refreshRight(true);
  }

 protected:
  HRSText* lblStep[STEP_COUNT] = {};
  HRSText* lblAction = nullptr;

  uint8_t dispState = 0xFF;
  uint8_t dispTry = 0xFF;
  uint8_t spinPhase = 0;
  tmr10ms_t lastUiRefresh = 0;
  tmr10ms_t lastSpinTick = 0;

  void buildLeftPanel()
  {
    // Fully static left panel — all live progress is on the right.
    auto* left = leftPanel(LEFT_W);
    LAYOUT_VAL_SCALED(H_LABEL, 14)
    LAYOUT_VAL_SCALED(H_TITLE, 40)
    LAYOUT_VAL_SCALED(H_LINE, 18)
    LAYOUT_VAL_SCALED(H_GAP, 14)

    coord_t y = PAD_SMALL;
    new HRSText(left, {0, y, LEFT_W - PAD_MEDIUM, H_LABEL}, "Mode", HRS_COLOR_MUTED, FONT(XS));
    y += H_LABEL;

    new HRSText(left, {0, y, LEFT_W - PAD_MEDIUM, H_TITLE}, "AUTO", HRS_COLOR_ELEC_BLUE,
                FONT(XL));
    y += H_TITLE + PAD_SMALL;

    new HRSText(left, {0, y, LEFT_W - PAD_MEDIUM, H_LINE}, "Signal search", HRS_COLOR_MUTED,
                FONT(XS));
    y += H_LINE + H_GAP;

    new HRSText(left, {0, y, LEFT_W - PAD_MEDIUM, H_LINE}, "PWR ON", HRS_COLOR_NEON_GREEN,
                FONT(XS));
    y += H_LINE + H_GAP;

    new HRSText(left, {0, y, LEFT_W - PAD_MEDIUM, 0},
                "Ext port\n"
                "1 GND\n"
                "2 +5V\n"
                "3 SIG",
                HRS_COLOR_MUTED, FONT(XS));
  }

  void buildRightPanel()
  {
    auto* right =
        createPanel({RIGHT_X, MAIN_TOP, RIGHT_W, LCD_H - MAIN_TOP - FOOTER_H - PAD_MEDIUM});
    right->padLeft(PAD_SMALL);

    LAYOUT_VAL_SCALED(H_LABEL, 14)
    LAYOUT_VAL_SCALED(H_STEP, 20)
    LAYOUT_VAL_SCALED(H_ACTION, 22)

    const coord_t contentW = RIGHT_W - PAD_MEDIUM * 2;
    coord_t y = PAD_SMALL;

    new HRSText(right, {0, y, contentW, H_LABEL}, "Progress", HRS_COLOR_ELEC_BLUE, FONT(XS));
    y += H_LABEL + PAD_SMALL;

    static const char* const kStepIdle[STEP_COUNT] = {
        "  Power",
        "  Probe",
        "  Try",
        "  Measure",
    };
    for (int i = 0; i < STEP_COUNT; ++i) {
      lblStep[i] = new HRSText(right, {0, y, contentW, H_STEP}, kStepIdle[i], HRS_COLOR_MUTED,
                               FONT(XS));
      y += H_STEP;
    }

    y += PAD_SMALL;
    lblAction = new HRSText(right, {0, y, contentW, H_ACTION}, "Starting…", HRS_COLOR_WARN,
                            FONT(XS));
  }

  void buildFooter()
  {
    auto* foot = footerPanel();
    new HRSText(foot, rect_t{}, "AUTO MEASURE", HRS_COLOR_MUTED, FONT(XS));
    new HRSText(foot, {0, 0, LV_PCT(100), 0}, "RTN = Exit", HRS_COLOR_ELEC_BLUE,
                RIGHT | FONT(XS));
  }

  void refreshRight(bool force = false)
  {
    const uint8_t state = v15AutoMeasureGetState();
    const uint8_t tryP = v15AutoMeasureGetTryProtocol();
    const uint8_t proto = v15AutoMeasureGetProtocol();
    const bool fault = v15AutoMeasureOvercurrentFault();
    const uint8_t showTry = (state == AUTO_MEASURE_STATE_LOCKED) ? proto : tryP;
    const tmr10ms_t now = get_tmr10ms();

    // Animate searching dots while waiting / probing / trying.
    // tmr10ms units: 45 ≈ 450 ms per step (was 4 ≈ 40 ms — felt frantic).
    bool spinTick = false;
    if (!fault && state != AUTO_MEASURE_STATE_LOCKED &&
        (tmr10ms_t)(now - lastSpinTick) >= 45) {
      lastSpinTick = now;
      spinPhase = (uint8_t)((spinPhase + 1) % 6);
      spinTick = true;
    }

    const bool stateChanged = (state != dispState) || (showTry != dispTry) || fault;
    if (!force && !stateChanged && !spinTick) return;

    if (stateChanged || force) {
      dispState = state;
      dispTry = showTry;

      const uint8_t cur = stateToStep(state);
      char buf[32];
      for (uint8_t i = 0; i < STEP_COUNT; ++i) {
        if (!lblStep[i]) continue;
        if (i == STEP_TRY &&
            (state == AUTO_MEASURE_STATE_TRY || state == AUTO_MEASURE_STATE_LOCKED)) {
          snprintf(buf, sizeof(buf), "%s Try %s", (i == cur) ? ">" : " ", protoName(showTry));
        } else {
          static const char* const kName[STEP_COUNT] = {"Power", "Probe", "Try", "Measure"};
          snprintf(buf, sizeof(buf), "%s %s", (i == cur) ? ">" : " ", kName[i]);
        }
        lblStep[i]->setText(buf);
        if (fault)
          lblStep[i]->setColor(HRS_COLOR_DANGER);
        else if (i < cur)
          lblStep[i]->setColor(HRS_COLOR_NEON_GREEN);
        else if (i == cur)
          lblStep[i]->setColor(HRS_COLOR_WARN);
        else
          lblStep[i]->setColor(HRS_COLOR_MUTED);
      }
    }

    if (lblAction) {
      char buf[40];
      static const char* const kDots[6] = {
          ".     ", "..    ", "...   ", "....  ", "..... ", "......",
      };
      if (fault) {
        lblAction->setText("Overcurrent — power cut");
        lblAction->setColor(HRS_COLOR_DANGER);
      } else if (state == AUTO_MEASURE_STATE_LINK) {
        snprintf(buf, sizeof(buf), "Searching %s", kDots[spinPhase]);
        lblAction->setText(buf);
        lblAction->setColor(HRS_COLOR_WARN);
      } else if (state == AUTO_MEASURE_STATE_PROBE) {
        snprintf(buf, sizeof(buf), "Sampling %s", kDots[spinPhase]);
        lblAction->setText(buf);
        lblAction->setColor(HRS_COLOR_WARN);
      } else if (state == AUTO_MEASURE_STATE_TRY) {
        snprintf(buf, sizeof(buf), "Trying %s %s", protoName(tryP), kDots[spinPhase]);
        lblAction->setText(buf);
        lblAction->setColor(HRS_COLOR_WARN);
      } else if (state == AUTO_MEASURE_STATE_LOCKED) {
        snprintf(buf, sizeof(buf), "Locked · %s", protoName(proto));
        lblAction->setText(buf);
        lblAction->setColor(HRS_COLOR_NEON_GREEN);
      }
    }
  }

  void onCancel() override
  {
    v15AutoMeasureStop();
    deleteLater();
  }

  void checkEvents() override
  {
    Window::checkEvents();
    v15AutoMeasureTask();

    if (v15AutoMeasureGetState() == AUTO_MEASURE_STATE_LOCKED) {
      openLockedMeasure();
      return;
    }

    const tmr10ms_t now = get_tmr10ms();
    // Keep UI responsive enough for the ~450 ms dot animation.
    if ((tmr10ms_t)(now - lastUiRefresh) >= 10) {
      lastUiRefresh = now;
      refreshRight(false);
    } else if (v15AutoMeasureOvercurrentFault()) {
      refreshRight(false);
    }
  }

  void openLockedMeasure()
  {
    const uint8_t proto = v15AutoMeasureGetProtocol();
    v15AutoMeasureBeginHandoff();
    deleteLater();
    switch (proto) {
      case AUTO_MEASURE_PROTO_PWM: v15PwmMeasureOpen(); break;
      case AUTO_MEASURE_PROTO_PPM: v15PpmMeasureOpen(); break;
      case AUTO_MEASURE_PROTO_SBUS: v15SbusMeasureOpen(); break;
      case AUTO_MEASURE_PROTO_CRSF: v15CrsfMeasureOpen(); break;
      case AUTO_MEASURE_PROTO_SUMD: v15SumdMeasureOpen(); break;
      case AUTO_MEASURE_PROTO_DJI_RS: v15DjiRsMeasureOpen(); break;
      case AUTO_MEASURE_PROTO_MAVLINK: v15MavlinkMeasureOpen(); break;
      default: v15AutoMeasureStop(); break;
    }
  }
};

}  // namespace

void v15AutoMeasureOpen(void)
{
  if (v15AutoMeasureIsActive() && v15AutoMeasureIsUiAttached()) return;
  if (!v15AutoMeasureIsActive()) {
    v15AutoMeasureStart();
    if (!v15AutoMeasureIsActive()) {
      hrsShowExtPortUnsafeWarning();
      return;
    }
  }
  new AutoMeasureDialog();
}
