
#include "batsenser.h"

#include "edgetx.h"
#include "fonts.h"
#include "static.h"
#include "servo_tester.h"
#include "pwm_measure.h"
#include "ppm_measure.h"
#include "sbus_measure.h"
#include "crsf_measure.h"
#include "dshot_tester.h"
#include "hrs_tool.h"

#include <stdio.h>

#include <string>

namespace {

static constexpr uint16_t BATSENSER_MIN_MV = 3300;
static constexpr uint16_t BATSENSER_FULL_MV = 4200;
static constexpr uint16_t BATSENSER_LOW_MV = 3500;
static constexpr uint16_t BATSENSER_DELTA_WARN_MV = 50;
static constexpr uint16_t BATSENSER_ANALYZE_MIN_MV = 3000;
static constexpr uint16_t BATSENSER_QUALITY_LOW_MV = 3600;
static constexpr uint16_t BATSENSER_QUALITY_DELTA_MV = 50;
static constexpr uint16_t BATSENSER_STABLE_JITTER_MV = 20;
static constexpr uint8_t BATSENSER_STABLE_SAMPLE_COUNT = 10;
static constexpr uint8_t BATSENSER_INVALID_HOLD_SAMPLES = 20;
static constexpr tmr10ms_t BATSENSER_ANNOUNCE_DELAY_10MS = 50;  // 0.5s
static constexpr uint8_t BATSENSER_ANNOUNCE_PLAY_ID = ID_PLAY_FROM_SD_MANAGER;

// Layout (480x272 target)
static LAYOUT_VAL_SCALED(CELL_AREA_H, 142)
static LAYOUT_VAL_SCALED(PACK_SECTION_H, 60)
static LAYOUT_VAL_SCALED(BAR_TRACK_W, 16)
static LAYOUT_VAL_SCALED(BAR_TRACK_H, 74)
static constexpr coord_t BAR_BORDER_W = 1;
static constexpr coord_t BAR_FILL_MARGIN =
    1; /* gap between inner edge of frame border and SOC column */
static constexpr coord_t BAR_FILL_W =
    BAR_TRACK_W - 2 * BAR_BORDER_W - 2 * BAR_FILL_MARGIN;
static constexpr coord_t BAR_FILL_ZONE_H =
    BAR_TRACK_H - 2 * BAR_BORDER_W - 2 * BAR_FILL_MARGIN;
/** Width taken from dV band so VBAT label fits one line at FONT(XS). */
static LAYOUT_VAL_SCALED(FOOTER_VBAT_EXTRA_W, 30)

static const char TITLE_VOLT_MON[] = "Battery Meter";
static const char LBL_TOTAL_V[] = "Pack Vtot";
static const char LBL_CELL_RANGE[] = "Cell V span";
static const char LBL_CELL_PANEL[] = "Cells (1-6S)";
static const char LBL_PACK_BAR[] = "Pack SOC bar";

static HRSColor cellBarColor(uint16_t voltageMv)
{
  if (voltageMv < BATSENSER_LOW_MV) {
    return HRS_COLOR_DANGER;
  }
  if (voltageMv < BATSENSER_MIN_MV + 80) {
    return HRS_COLOR_WARN;
  }
  return HRS_COLOR_NEON_GREEN;
}

class Battery6SDialog : public HRSTool
{
 public:
  Battery6SDialog() : HRSTool()
  {
    buildHeader(TITLE_VOLT_MON);
    buildLeftSummary();
    buildCellColumns();
    buildPackBarSection();
    buildFooter();

    closeCondition = []() { return !v15BatteryCell1Present(); };
    batteryInsertedTick = get_tmr10ms();
    refreshValues();
  }

 protected:
  std::function<bool(void)> closeCondition;

  HRSText* lblTotalVolts = nullptr;
  HRSText* lblTotalPct = nullptr;
  HRSText* lblCellRangeDetail = nullptr;

  HRSText* cellIdx[6] = {nullptr};
  HRSText* cellVolt[6] = {nullptr};
  HRSText* cellPct[6] = {nullptr};
  lv_obj_t* cellBarFill[6] = {nullptr};

  lv_obj_t* packBarBg = nullptr;
  lv_obj_t* packMarker = nullptr;
  HRSText* lblPackPctBig = nullptr;

  HRSText* summaryCells = nullptr;
  HRSText* summaryPack = nullptr;
  HRSText* summarySystem = nullptr;
  HRSText* summaryDelta = nullptr;

  bool qualityAnnounced = false;
  bool hasStableBaseline = false;
  uint8_t stableSamples = 0;
  uint8_t invalidSamples = 0;
  uint8_t baselineCells = 0;
  uint16_t baselineCellMv[6] = {0};
  tmr10ms_t batteryInsertedTick = 0;

  void buildLeftSummary()
  {
    auto leftCard = createPanel({PAD_MEDIUM, MAIN_TOP, LEFT_W, LCD_H - MAIN_TOP - FOOTER_H - PAD_MEDIUM});

    coord_t y = PAD_SMALL;

    static LAYOUT_VAL_SCALED(H1, 14)
    static LAYOUT_VAL_SCALED(H2, 38)
    static LAYOUT_VAL_SCALED(H3, 26)
    static LAYOUT_VAL_SCALED(H4, 16)

    new HRSText(leftCard, {PAD_SMALL, y, 0, 0}, LBL_TOTAL_V, HRS_COLOR_MUTED, FONT(XS));
    y += H1;

    lblTotalVolts = new HRSText(leftCard, {PAD_SMALL, y, 0, 0}, "--.-- V", HRS_COLOR_NEON_GREEN, FONT(XL));
    y += H2;

    lblTotalPct = new HRSText(leftCard, {PAD_SMALL, y, 0, 0}, "--%", HRS_COLOR_NEON_GREEN, FONT(L));
    y += H3;

    new HRSText(leftCard, {PAD_SMALL, y,0, 0}, LBL_CELL_RANGE, HRS_COLOR_ELEC_BLUE, FONT(XS));
    y += H4;

    lblCellRangeDetail = new HRSText(leftCard, {PAD_SMALL, y, 0, 0}, "--.--V ~ --.--V", HRS_COLOR_NEON_GREEN, FONT(STD));
    y += EdgeTxStyles::UI_ELEMENT_HEIGHT;

    new HRSText(leftCard, {PAD_SMALL, y, 0, 0}, "SOC ref (6S eq.): \n19.8V-25.2V", HRS_COLOR_MUTED, FONT(XS));
    y += EdgeTxStyles::UI_ELEMENT_HEIGHT;

    new HRSText(leftCard, {PAD_SMALL, y, 0, 0}, "SOC ref (cell): \n3.30V-4.20V", HRS_COLOR_MUTED, FONT(XS));
  }

  void buildCellColumns()
  {
    auto cellStrip = new Window(this, {RIGHT_X, MAIN_TOP, RIGHT_W, CELL_AREA_H});

    new HRSText(cellStrip, rect_t{}, LBL_CELL_PANEL, HRS_COLOR_ELEC_BLUE, FONT(XS));

    const coord_t cellW = (RIGHT_W - PAD_LARGE) / 6;
    static LAYOUT_VAL_SCALED(ROW_TOP, 18)
    constexpr coord_t CELL_VOLT_Y = 0;
    constexpr coord_t CELL_BAR_Y = ROW_TOP;
    static LAYOUT_VAL_SCALED(CELL_IDX_H, 13)
    constexpr coord_t CELL_ROW_GAP = PAD_TINY;
    constexpr coord_t CELL_IDX_TO_PCT_GAP = 1;
    constexpr coord_t CELL_PCT_UP = PAD_TINY;
    constexpr coord_t CELL_IDX_PCT_UP = PAD_TINY;
    constexpr coord_t CELL_CONTENT_UP =
        PAD_TINY; /* shift volt / bar / idx / pct inside card; ROW_TOP unchanged */

    const coord_t voltTop = (coord_t)(CELL_VOLT_Y - CELL_CONTENT_UP);
    const coord_t barTop = (coord_t)(CELL_BAR_Y - CELL_CONTENT_UP);

    for (uint8_t i = 0; i < 6; ++i) {
      const coord_t x = PAD_SMALL + static_cast<coord_t>(i) * cellW;
      auto cell =
        createPanel(cellStrip, {x, ROW_TOP, cellW - PAD_TINY, CELL_AREA_H - ROW_TOP});

      lv_obj_t* cardObj = cell->getLvObj();

      cellVolt[i] = new HRSText(cell,
          {0, voltTop, cellW - PAD_SMALL, 0}, "--",
          HRS_COLOR_MUTED, CENTERED | FONT(STD));

      lv_coord_t barLeft =
          (lv_coord_t)((cellW - PAD_TINY - BAR_TRACK_W) / 2);
      auto track = lv_obj_create(cardObj);
      lv_obj_set_size(track, BAR_TRACK_W, BAR_TRACK_H);
      lv_obj_set_pos(track, barLeft, barTop);
      lv_obj_set_style_bg_color(track, colDashBg(), LV_PART_MAIN);
      lv_obj_set_style_border_color(track, colCardBorder(),
                                    LV_PART_MAIN);
      lv_obj_set_style_border_width(track, BAR_BORDER_W, LV_PART_MAIN);
      lv_obj_set_style_radius(track, PAD_TINY, LV_PART_MAIN);

      cellBarFill[i] = lv_obj_create(track);
      lv_obj_set_width(cellBarFill[i], BAR_FILL_W);
      lv_obj_set_style_radius(cellBarFill[i], 1, LV_PART_MAIN);
      lv_obj_set_style_border_width(cellBarFill[i], 0, LV_PART_MAIN);
      lv_obj_set_style_bg_color(cellBarFill[i], colDanger(), LV_PART_MAIN | HRS_COLOR_DANGER);
      lv_obj_set_style_bg_color(cellBarFill[i], colWarn(), LV_PART_MAIN | HRS_COLOR_WARN);
      lv_obj_set_style_bg_color(cellBarFill[i], colNeonGreen(), LV_PART_MAIN | HRS_COLOR_NEON_GREEN);
      lv_obj_set_style_bg_opa(cellBarFill[i], LV_OPA_COVER, LV_PART_MAIN);
      lv_obj_align(cellBarFill[i], LV_ALIGN_BOTTOM_MID, 0,
                   -(BAR_BORDER_W + BAR_FILL_MARGIN));

      const coord_t idxY = (coord_t)(barTop + BAR_TRACK_H + CELL_ROW_GAP -
                                      CELL_IDX_PCT_UP);
      cellIdx[i] = new HRSText(cell,
                                  {0, idxY, cellW - PAD_SMALL, 0}, "",
                                  HRS_COLOR_MUTED,
                                  CENTERED | FONT(XS));

      cellPct[i] = new HRSText(
          cell,
          {0,
           (coord_t)(idxY + CELL_IDX_H + CELL_IDX_TO_PCT_GAP - CELL_PCT_UP),
           cellW - PAD_SMALL, 0},
          "--%", HRS_COLOR_MUTED, CENTERED | FONT(XS));
    }
  }

  void buildPackBarSection()
  {
    auto packStrip = createPanel({RIGHT_X, MAIN_TOP + CELL_AREA_H + PAD_SMALL, RIGHT_W, PACK_SECTION_H});

    new HRSText(packStrip, {PAD_SMALL, PAD_TINY, 0, 0}, LBL_PACK_BAR, HRS_COLOR_ELEC_BLUE, FONT(XS));

    static LAYOUT_VAL_SCALED(BAR_H, 14)
    static LAYOUT_VAL_SCALED(BAR_Y, 22)
    static LAYOUT_VAL_SCALED(VR_Y, 38)

    packBarBg = lv_obj_create(packStrip->getLvObj());
    lv_obj_set_size(packBarBg, LV_PCT(75), BAR_H);
    lv_obj_set_pos(packBarBg, PAD_SMALL, BAR_Y);
    lv_obj_set_style_radius(packBarBg, PAD_MEDIUM, LV_PART_MAIN);
    lv_obj_set_style_bg_color(packBarBg, colRed(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(packBarBg, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(packBarBg, colNeonGreen(), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_dir(packBarBg, LV_GRAD_DIR_HOR, LV_PART_MAIN);

    packMarker = lv_obj_create(packBarBg);
    lv_obj_set_size(packMarker, PAD_LARGE, BAR_H);
    lv_obj_set_style_radius(packMarker, PAD_TINY, LV_PART_MAIN);
    lv_obj_set_style_bg_color(packMarker, colWhite(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(packMarker, LV_OPA_COVER, LV_PART_MAIN);

    lblPackPctBig = new HRSText(packStrip,
                  {0, PAD_MEDIUM * 2, LV_PCT(100), 0}, "--%",
                  HRS_COLOR_NEON_GREEN, RIGHT | FONT(L));

    new HRSText(
        packStrip, {PAD_SMALL, VR_Y, 0, 0},
        "MaxV 2S=8.4 3S=12.6 4S=16.8 5S=21.0 6S=25.2", HRS_COLOR_MUTED, FONT(XS));
  }

  void buildFooter()
  {
    auto footer = footerPanel();

    summaryCells = new HRSText(footer, rect_t{}, "", HRS_COLOR_MUTED, FONT(XS));
    summaryPack = new HRSText(footer, {LV_PCT(20), 0, 0, 0}, "", HRS_COLOR_MUTED, FONT(XS));
    summaryDelta = new HRSText(footer, {LV_PCT(45), 0, 0, 0}, "", HRS_COLOR_MUTED, FONT(XS));
    summarySystem = new HRSText(footer, {0, 0, LV_PCT(100), 0}, "", HRS_COLOR_MUTED, RIGHT | FONT(XS));
  }

  static uint8_t voltageToPercent(uint16_t voltage)
  {
    if (voltage <= BATSENSER_MIN_MV) {
      return 0;
    }
    if (voltage >= BATSENSER_FULL_MV) {
      return 100;
    }
    return (uint8_t)(((voltage - BATSENSER_MIN_MV) * 100) /
                     (BATSENSER_FULL_MV - BATSENSER_MIN_MV));
  }

  void resetQualityStability()
  {
    hasStableBaseline = false;
    stableSamples = 0;
    baselineCells = 0;
    invalidSamples = 0;
  }

  bool isReasonableMeasurement(uint8_t cells, uint16_t minCell,
                                  uint16_t maxCell) const
  {
    return (cells > 0 && minCell >= BATSENSER_ANALYZE_MIN_MV &&
            maxCell <= BATSENSER_FULL_MV + 200);
  }

  bool isMeasurementStable(const uint16_t* cellVoltages, uint8_t cells)
  {
    if (!hasStableBaseline || baselineCells != cells) {
      for (uint8_t i = 0; i < cells; ++i) {
        baselineCellMv[i] = cellVoltages[i];
      }
      baselineCells = cells;
      hasStableBaseline = true;
      stableSamples = 1;
      return false;
    }

    bool stable = true;
    for (uint8_t i = 0; i < cells; ++i) {
      int16_t diff = static_cast<int16_t>(cellVoltages[i]) -
                     static_cast<int16_t>(baselineCellMv[i]);
      if (diff < 0) {
        diff = -diff;
      }
      if (diff > static_cast<int16_t>(BATSENSER_STABLE_JITTER_MV)) {
        stable = false;
      }
      baselineCellMv[i] = cellVoltages[i];
    }

    if (stable) {
      if (stableSamples < 255) {
        ++stableSamples;
      }
    } else {
      stableSamples = 1;
    }

    return (stableSamples >= BATSENSER_STABLE_SAMPLE_COUNT);
  }

  bool playBatteryResultFile(const char* wavName)
  {
    auto tryPlayPath = [](const char* fullPath) {
      STOP_PLAY(BATSENSER_ANNOUNCE_PLAY_ID);
      PLAY_FILE(fullPath, 0, BATSENSER_ANNOUNCE_PLAY_ID);
      return IS_PLAYING(BATSENSER_ANNOUNCE_PLAY_ID);
    };

    char path[AUDIO_FILENAME_MAXLEN + 1];
    char* str = getAudioPath(path);
    strcpy(str, SYSTEM_SUBDIR "/");
    strcat(path, wavName);
    if (tryPlayPath(path)) {
      return true;
    }
    return false;
  }

  void analyzeAndAnnounceQuality(uint16_t minCell, uint16_t delta)
  {
    if (qualityAnnounced) {
      return;
    }

    bool played = false;
    if (minCell < BATSENSER_QUALITY_LOW_MV) {
      played = playBatteryResultFile("batterlow.wav");
    } else if (delta >= BATSENSER_QUALITY_DELTA_MV) {
      played = playBatteryResultFile("batternc.wav");
    } else {
      played = playBatteryResultFile("battergood.wav");
    }

    if (played) {
      qualityAnnounced = true;
    }
  }

  void refreshValues()
  {
    char line[40] = {0};
    uint16_t cellVoltages[6] = {0};

    uint8_t cells = v15BatteryDetectedCells();
    uint16_t minCell = 0xFFFF;
    uint16_t maxCell = 0;
    uint32_t totalVoltageMv = 0;
    uint32_t sumCellPct = 0;

    for (uint8_t i = 1; i <= 6; ++i) {
      const uint8_t idx = i - 1;

      snprintf(line, sizeof(line), "%u", static_cast<unsigned>(i));
      cellIdx[idx]->setText(line);

      if (i <= cells) {
        uint16_t voltage = v15BatteryCellVoltage(i);
        cellVoltages[idx] = voltage;
        uint8_t percent = voltageToPercent(voltage);
        sumCellPct += static_cast<uint32_t>(percent);
        HRSColor barCol = cellBarColor(voltage);

        if (voltage < minCell) {
          minCell = voltage;
        }
        if (voltage > maxCell) {
          maxCell = voltage;
        }
        totalVoltageMv += voltage;

        uint16_t voltageCv = static_cast<uint16_t>((voltage + 5) / 10);
        snprintf(line, sizeof(line), "%u.%02uV", voltageCv / 100,
                 voltageCv % 100);
        cellVolt[idx]->setText(line);
        cellVolt[idx]->setColor(barCol);

        snprintf(line, sizeof(line), "%u%%", static_cast<unsigned>(percent));
        cellPct[idx]->setText(line);
        cellPct[idx]->setColor(barCol);

        lv_coord_t fillH =
            (lv_coord_t)((BAR_FILL_ZONE_H * percent) / 100);
        if (fillH < PAD_TINY) {
          fillH = (percent > 0) ? PAD_TINY : 1;
        }
        setColor(cellBarFill[idx], barCol);
        lv_obj_set_height(cellBarFill[idx], fillH);
      } else {
        cellVolt[idx]->setText("--");
        cellVolt[idx]->setColor(HRS_COLOR_MUTED);
        cellPct[idx]->setText("--");
        cellPct[idx]->setColor(HRS_COLOR_MUTED);
        lv_obj_set_height(cellBarFill[idx], 0);
      }
    }

    if (cells > 0) {
      uint32_t totalVoltageCv = (totalVoltageMv + 5) / 10;
      snprintf(line, sizeof(line), "%lu.%02lu V",
               static_cast<unsigned long>(totalVoltageCv / 100),
               static_cast<unsigned long>(totalVoltageCv % 100));
      lblTotalVolts->setText(line);
      lblTotalVolts->setColor(HRS_COLOR_NEON_GREEN);

      uint8_t packPct = static_cast<uint8_t>(
          (sumCellPct + static_cast<uint32_t>(cells) / 2U) /
          static_cast<uint32_t>(cells));
      snprintf(line, sizeof(line), "%u%%", static_cast<unsigned>(packPct));
      lblTotalPct->setText(line);
      lblTotalPct->setColor(HRS_COLOR_NEON_GREEN);

      lblPackPctBig->setText(line);
      lblPackPctBig->setColor(HRS_COLOR_NEON_GREEN);

      if (minCell != 0xFFFF && maxCell >= minCell) {
        uint16_t minCv = static_cast<uint16_t>((minCell + 5) / 10);
        uint16_t maxCv = static_cast<uint16_t>((maxCell + 5) / 10);
        snprintf(line, sizeof(line), "%u.%02uV ~ %u.%02uV", minCv / 100,
                 minCv % 100, maxCv / 100, maxCv % 100);
      } else {
        snprintf(line, sizeof(line), "--.--V ~ --.--V");
      }
      lblCellRangeDetail->setText(line);
      lblCellRangeDetail->setColor(HRS_COLOR_NEON_GREEN);

      const lv_coord_t barW = lv_obj_get_width(packBarBg);
      const lv_coord_t mx =
          PAD_SMALL + (lv_coord_t)((barW - PAD_MEDIUM * 2) * packPct / 100);
      lv_obj_set_x(packMarker, mx);
    } else {
      lblTotalVolts->setText("--.-- V");
      lblTotalVolts->setColor(HRS_COLOR_MUTED);
      lblTotalPct->setText("--%");
      lblTotalPct->setColor(HRS_COLOR_MUTED);
      lblPackPctBig->setText("--%");
      lblPackPctBig->setColor(HRS_COLOR_MUTED);
      lblCellRangeDetail->setText("--.--V ~ --.--V");
      lblCellRangeDetail->setColor(HRS_COLOR_MUTED);
      lv_obj_set_x(packMarker, PAD_SMALL);
    }

    uint16_t sys = v15BatterySystemVoltage();
    int16_t sysCurrent = v15BatterySystemCurrent();
    uint16_t sysCurrentAbs =
      (sysCurrent < 0) ? static_cast<uint16_t>(-sysCurrent)
                       : static_cast<uint16_t>(sysCurrent);
    uint16_t delta = 0;
    bool deltaValid = (cells >= 2 && minCell != 0xFFFF);
    if (deltaValid) {
      delta = maxCell - minCell;
    }

    snprintf(line, sizeof(line), "%u/6S", static_cast<unsigned>(cells));
    summaryCells->setText(line);

    if (cells > 0) {
      uint32_t totalVoltageCv = (totalVoltageMv + 5) / 10;
      snprintf(line, sizeof(line), "%lu.%02luV",
               static_cast<unsigned long>(totalVoltageCv / 100),
               static_cast<unsigned long>(totalVoltageCv % 100));
    } else {
      snprintf(line, sizeof(line), "--.--V");
    }
    summaryPack->setText(line);

    uint16_t sysCv = static_cast<uint16_t>((sys + 5) / 10);
    snprintf(line, sizeof(line), "VBAT %u.%02uV %s%u.%02uA", sysCv / 100,
             sysCv % 100, (sysCurrent < 0) ? "-" : " ",
             sysCurrentAbs / 1000, (sysCurrentAbs % 1000) / 10);
    summarySystem->setText(line);

    if (deltaValid) {
      uint16_t deltaCv = static_cast<uint16_t>((delta + 5) / 10);
      snprintf(line, sizeof(line), "dV %u.%02uV", deltaCv / 100,
               deltaCv % 100);
      summaryDelta->setColor(delta >= BATSENSER_DELTA_WARN_MV ? HRS_COLOR_DANGER : HRS_COLOR_WARN);
    } else {
      snprintf(line, sizeof(line), "dV --");
      summaryDelta->setColor(HRS_COLOR_MUTED);
    }
    summaryDelta->setText(line);

    bool announceDelayElapsed =
      ((tmr10ms_t)(get_tmr10ms() - batteryInsertedTick) >=
       BATSENSER_ANNOUNCE_DELAY_10MS);

    bool hasAnyCellMeasurement = (cells > 0 && minCell != 0xFFFF);
    bool deltaAnomaly = (deltaValid && delta >= BATSENSER_QUALITY_DELTA_MV);
    bool readyForQuality =
      announceDelayElapsed && hasAnyCellMeasurement;

    if (!qualityAnnounced && readyForQuality) {
      if (deltaAnomaly) {
        analyzeAndAnnounceQuality(minCell, delta);
        return;
      }

      if (isReasonableMeasurement(cells, minCell, maxCell)) {
        invalidSamples = 0;
        if (isMeasurementStable(cellVoltages, cells)) {
          analyzeAndAnnounceQuality(minCell, deltaValid ? delta : 0);
        }
      } else {
        if (invalidSamples < 255) {
          ++invalidSamples;
        }
        if (invalidSamples >= BATSENSER_INVALID_HOLD_SAMPLES) {
          resetQualityStability();
        }
      }
    } else if (!qualityAnnounced) {
      if (!announceDelayElapsed) {
        resetQualityStability();
      }
    }
  }

  void checkEvents() override
  {
    Window::checkEvents();

    if (closeCondition && closeCondition()) {
      closeWindow();
    } else {
      refreshValues();
    }
  }
};

Battery6SDialog* s_dialog = nullptr;

}  // namespace

void v15BatteryUiTask(void)
{
  if (v15ServoTesterIsActive() || v15PwmMeasureIsActive() ||
      v15PpmMeasureIsActive() || v15SbusMeasureIsActive() ||
      v15CrsfMeasureIsActive() || v15DshotTesterIsActive()) {
    if (s_dialog) {
      s_dialog->closeWindow();
      s_dialog = nullptr;
    }
    return;
  }

  if (s_dialog && s_dialog->deleted()) {
    s_dialog = nullptr;
  }

  if (!v15BatteryCell1Present()) {
    if (s_dialog) {
      s_dialog->closeWindow();
      s_dialog = nullptr;
    }
    return;
  }

  if (!s_dialog && v15BatteryUiTriggerPending()) {
    s_dialog = new Battery6SDialog();
    s_dialog->onClosing([]() { s_dialog = nullptr; });
    v15BatteryUiAcknowledgeTrigger();
  }
}
