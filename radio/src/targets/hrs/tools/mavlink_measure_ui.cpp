/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * ColorLCD UI for V15 MAVLink measure.
 */

#include "mavlink_measure.h"
#include "auto_measure.h"

#include "edgetx.h"
#include "hrs_tool.h"

#include <stdio.h>

namespace {

static const char TITLE_MAVLINK[] = "MAVLink Measure";

class MavlinkMeasureDialog : public HRSTool
{
 public:
  MavlinkMeasureDialog() : HRSTool()
  {
    onClosing([=]() {
      const bool fromAuto = v15AutoMeasureIsHandoff();
      v15MavlinkMeasureStop();
      if (fromAuto) v15AutoMeasureStop();
    });

    if (!v15MavlinkMeasureIsActive())
      v15MavlinkMeasureStart();

    buildHeader(TITLE_MAVLINK);
    buildLeftPanel();
    buildRightPanel();
    buildFooter();
    refreshAll(true);
  }

 protected:
  HRSText* lblBaud = nullptr;
  HRSText* lblPolarity = nullptr;
  HRSText* lblMsg = nullptr;
  HRSText* lblPower = nullptr;
  HRSText* lblCurrent = nullptr;
  HRSText* lblSignal = nullptr;
  HRSText* lblLines[MAVLINK_MEASURE_MAX_LINES] = {};

  uint8_t dispPolarity = 0xFF;
  uint32_t dispBaud = 0xFFFFFFFF;
  uint32_t dispMsgId = 0xFFFFFFFF;
  uint16_t dispHz = 0xFFFF;
  int16_t dispCurrent = -1;
  bool dispSignal = false;
  bool dispFault = false;
  bool dispPower = false;
  tmr10ms_t lastUiRefresh = 0;

  void buildLeftPanel()
  {
    auto* leftCard = leftPanel(LEFT_W);

    LAYOUT_VAL_SCALED(H1, 18)
    LAYOUT_VAL_SCALED(H2, 22)
    LAYOUT_VAL_SCALED(H3, 20)

    coord_t y = PAD_MEDIUM;

    lblBaud = new HRSText(leftCard, {0, y, 0, 0}, "MAVLink 460k/115k/57k", HRS_COLOR_MUTED, FONT(XS));
    y += H1;

    lblPolarity = new HRSText(leftCard, {0, y, 0, 0}, "Pol  AUTO...", HRS_COLOR_WARN, FONT(STD));
    y += H2;

    lblMsg = new HRSText(leftCard, {0, y, 0, 0}, "Msg  ----", HRS_COLOR_ELEC_BLUE, FONT(XS));
    y += H3;

    lblPower = new HRSText(leftCard, {0, y, 0, 0}, "PWR ---", HRS_COLOR_NEON_GREEN, FONT(STD));
    y += H2;

    lblCurrent = new HRSText(leftCard, {0, y, 0, 0}, "I --- mA", HRS_COLOR_NEON_GREEN, FONT(STD));
    y += H2;

    lblSignal = new HRSText(leftCard, {0, y, 0, 0}, "SIGNAL LOST", HRS_COLOR_WARN, FONT(XS));
    y += H2;

    new HRSText(leftCard, {0, y, 0, 0}, "Ext:\n1=GND 2=+5V 3=MAVLink", HRS_COLOR_MUTED, FONT(XS));
  }

  void buildRightPanel()
  {
    auto* rightCard =
        createPanel({RIGHT_X, MAIN_TOP, RIGHT_W, LCD_H - MAIN_TOP - FOOTER_H - PAD_MEDIUM});
    rightCard->padLeft(PAD_SMALL);

    LAYOUT_VAL_SCALED(LINE_H, 15)
    coord_t y = PAD_SMALL;

    new HRSText(rightCard, {0, y, 0, 0}, "Decoded telemetry", HRS_COLOR_ELEC_BLUE, FONT(XS));
    y += PAD_LARGE + 4;  // keep title clear of first telemetry line

    for (int i = 0; i < MAVLINK_MEASURE_MAX_LINES; ++i) {
      lblLines[i] = new HRSText(rightCard, {0, y, RIGHT_W - PAD_SMALL, 0}, "—",
                                HRS_COLOR_MUTED, FONT(XS));
      y += LINE_H;
    }
  }

  void buildFooter()
  {
    auto* foot = footerPanel();
    new HRSText(foot, rect_t{}, "Ext  MAVLink IN", HRS_COLOR_MUTED, FONT(XS));
    new HRSText(foot, {0, 0, LV_PCT(100), 0}, "RTN = Exit", HRS_COLOR_ELEC_BLUE,
                RIGHT | FONT(XS));
  }

  void refreshLines(bool force)
  {
    char buf[MAVLINK_MEASURE_LINE_TEXT_LEN];
    const uint8_t n = v15MavlinkMeasureGetLineCount();
    for (int i = 0; i < MAVLINK_MEASURE_MAX_LINES; ++i) {
      if (!lblLines[i]) continue;
      if ((uint8_t)i < n) {
        v15MavlinkMeasureGetLineText((uint8_t)i, buf, sizeof(buf));
        lblLines[i]->setText(buf);
        lblLines[i]->setColor(HRS_COLOR_NEON_GREEN);
      } else if (force) {
        lblLines[i]->setText("—");
        lblLines[i]->setColor(HRS_COLOR_MUTED);
      }
    }
  }

  void refreshAll(bool force = false)
  {
    const uint8_t polarity = v15MavlinkMeasureGetPolarity();
    const uint32_t baud = v15MavlinkMeasureGetBaudrate();
    const uint32_t msgId = v15MavlinkMeasureGetMsgId();
    const uint16_t hz = v15MavlinkMeasureGetMsgHz();
    const int16_t current = v15MavlinkMeasureGetCurrentMa();
    const bool hasSignal = v15MavlinkMeasureHasSignal();
    const bool fault = v15MavlinkMeasureOvercurrentFault();
    const bool power = v15MavlinkMeasureIsPowerOn();

    const bool metaChanged =
        force || polarity != dispPolarity || baud != dispBaud || msgId != dispMsgId ||
        hz != dispHz || hasSignal != dispSignal || fault != dispFault ||
        power != dispPower || current != dispCurrent;
    if (!metaChanged && !force) return;

    dispPolarity = polarity;
    dispBaud = baud;
    dispMsgId = msgId;
    dispHz = hz;
    dispSignal = hasSignal;
    dispFault = fault;
    dispPower = power;
    dispCurrent = current;

    char buf[48];

    if (lblBaud) {
      if (!fault && hasSignal && baud > 0) {
        snprintf(buf, sizeof(buf), "Baud  %lu", (unsigned long)baud);
        lblBaud->setColor(HRS_COLOR_ELEC_BLUE);
      } else {
        snprintf(buf, sizeof(buf), "Baud  auto");
        lblBaud->setColor(HRS_COLOR_MUTED);
      }
      lblBaud->setText(buf);
    }

    if (lblPolarity) {
      if (fault) {
        lblPolarity->setText("Pol  --");
        lblPolarity->setColor(HRS_COLOR_DANGER);
      } else if (polarity == MAVLINK_MEASURE_POL_INVERTED) {
        lblPolarity->setText("Pol  INVERTED");
        lblPolarity->setColor(HRS_COLOR_NEON_GREEN);
      } else if (polarity == MAVLINK_MEASURE_POL_NORMAL) {
        lblPolarity->setText("Pol  NORMAL");
        lblPolarity->setColor(HRS_COLOR_NEON_GREEN);
      } else {
        lblPolarity->setText("Pol  AUTO...");
        lblPolarity->setColor(HRS_COLOR_WARN);
      }
    }

    if (lblMsg) {
      if (!fault && hasSignal) {
        snprintf(buf, sizeof(buf), "Msg  %lu  %u Hz", (unsigned long)msgId, (unsigned)hz);
        lblMsg->setColor(HRS_COLOR_ELEC_BLUE);
      } else {
        snprintf(buf, sizeof(buf), "Msg  ----");
        lblMsg->setColor(HRS_COLOR_MUTED);
      }
      lblMsg->setText(buf);
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

    refreshLines(force || hasSignal);
  }

  void onCancel() override
  {
    const bool fromAuto = v15AutoMeasureIsHandoff();
    v15MavlinkMeasureStop();
    if (fromAuto) v15AutoMeasureStop();
    closeWindow();
  }

  void checkEvents() override
  {
    Window::checkEvents();
    v15MavlinkMeasureTask();

    if (v15AutoMeasureWatchHandoffLoss(v15MavlinkMeasureHasSignal())) {
      v15AutoMeasureResumeSearch();
      v15MavlinkMeasureStop();
      closeWindow();
      v15AutoMeasureOpen();
      return;
    }

    const tmr10ms_t now = get_tmr10ms();
    if ((tmr10ms_t)(now - lastUiRefresh) >= 5) {
      lastUiRefresh = now;
      refreshAll(false);
    } else if (v15MavlinkMeasureOvercurrentFault() != dispFault) {
      refreshAll(false);
    }
  }
};

}  // namespace

void v15MavlinkMeasureOpen(void)
{
  if (v15MavlinkMeasureIsActive() && !v15AutoMeasureIsHandoff()) return;
  if (!v15MavlinkMeasureIsActive()) {
    v15MavlinkMeasureStart();
    if (!v15MavlinkMeasureIsActive()) {
      hrsShowExtPortUnsafeWarning();
      return;
    }
  }
  new MavlinkMeasureDialog();
}
