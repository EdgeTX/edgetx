/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * Map Ext-port RX measure CH1-8 into trainerInput (MASTER trainer modes).
 * Frame pushes run on the measure RX path so update rate matches the wire.
 */

#include "measure_trainer_feed.h"

#if defined(RADIO_V15) && !defined(SIMU)

#include "edgetx.h"
#include "trainer.h"

namespace {

constexpr uint8_t FEED_CHANNELS = 8;
constexpr uint16_t CH_MIN_US = 800;
constexpr uint16_t CH_MAX_US = 2200;
constexpr int32_t BUS_CH_CENTER = 0x3E0;

volatile bool g_feeding = false;
volatile bool g_warnPending = false;

bool isTrainerMasterMode(uint8_t mode)
{
  switch (mode) {
    case TRAINER_MODE_MASTER_TRAINER_JACK:
    case TRAINER_MODE_MASTER_SBUS_EXTERNAL_MODULE:
    case TRAINER_MODE_MASTER_CPPM_EXTERNAL_MODULE:
    case TRAINER_MODE_MASTER_SERIAL:
    case TRAINER_MODE_MASTER_BLUETOOTH:
    case TRAINER_MODE_MULTI:
    case TRAINER_MODE_CRSF:
      return true;
    default:
      return false;
  }
}

bool masterEnabled()
{
  return isTrainerMasterMode(g_model.trainerData.mode);
}

int16_t usToTrainerInput(uint16_t us)
{
  return (int16_t)(((int32_t)us - 1500) * (g_eeGeneral.PPM_Multiplier + 10) / 10);
}

int16_t busRawToTrainerInput(uint16_t raw11)
{
  return (int16_t)(((int32_t)raw11 - BUS_CH_CENTER) * 5 / 8);
}

uint16_t clampServoUs(uint16_t us)
{
  if (us < CH_MIN_US) return (us < 600) ? 988 : CH_MIN_US;
  if (us > CH_MAX_US) return (us > 1800) ? 2012 : CH_MAX_US;
  return us;
}

void commitTrainer(const int16_t vals[FEED_CHANNELS])
{
  const bool first = !g_feeding;
  for (uint8_t i = 0; i < FEED_CHANNELS; ++i)
    trainerInput[i] = vals[i];
  trainerResetTimer();
  g_feeding = true;
  if (first) g_warnPending = true;
}

}  // namespace

void v15MeasureTrainerFeedPushBusRaw(const uint16_t* raw11, uint8_t count)
{
  if (!raw11 || count < FEED_CHANNELS || !masterEnabled()) return;

  uint8_t inBand = 0;
  int16_t vals[FEED_CHANNELS];
  for (uint8_t i = 0; i < FEED_CHANNELS; ++i) {
    const uint16_t raw = raw11[i];
    // Approx mid-band check on 11-bit slots (stock SBUS/CRSF range).
    if (raw >= 172 && raw <= 1811) ++inBand;
    vals[i] = busRawToTrainerInput(raw);
  }
  if (inBand < 4) return;
  commitTrainer(vals);
}

void v15MeasureTrainerFeedPushUs(const uint16_t* us, uint8_t count)
{
  if (!us || count < FEED_CHANNELS || !masterEnabled()) return;

  uint8_t inBand = 0;
  int16_t vals[FEED_CHANNELS];
  for (uint8_t i = 0; i < FEED_CHANNELS; ++i) {
    const uint16_t v = us[i];
    if (v >= CH_MIN_US && v <= CH_MAX_US) ++inBand;
    vals[i] = usToTrainerInput(clampServoUs(v));
  }
  if (inBand < 4) return;
  commitTrainer(vals);
}

void v15MeasureTrainerFeedPushDjiRs(const uint16_t* ch, uint8_t count)
{
  // ELRS DJI RS Pro packs DJI channel units (not CPPM us): mid 1024, sticks 352..1696.
  constexpr int32_t DJI_CENTER = 1024;
  constexpr int32_t DJI_HALF = 672;  // 1696 - 1024
  constexpr uint16_t DJI_MIN = 176;
  constexpr uint16_t DJI_MAX = 1696;

  if (!ch || count < FEED_CHANNELS || !masterEnabled()) return;

  uint8_t inBand = 0;
  int16_t vals[FEED_CHANNELS];
  for (uint8_t i = 0; i < FEED_CHANNELS; ++i) {
    const uint16_t v = ch[i];
    if (v >= DJI_MIN && v <= DJI_MAX) ++inBand;
    // Map to standard servo us (988..2012, mid 1500) then CPPM trainer scale.
    int32_t us = 1500 + ((int32_t)v - DJI_CENTER) * 512 / DJI_HALF;
    if (us < (int32_t)CH_MIN_US) us = CH_MIN_US;
    if (us > (int32_t)CH_MAX_US) us = CH_MAX_US;
    vals[i] = usToTrainerInput((uint16_t)us);
  }
  if (inBand < 4) return;
  commitTrainer(vals);
}

void v15MeasureTrainerFeedTask(void)
{
  if (!masterEnabled()) {
    g_feeding = false;
    g_warnPending = false;
    return;
  }

  if (g_warnPending) {
    g_warnPending = false;
    checkTrainerSignalWarning();
  }

  // Validity is refreshed on each RX push; mixer path only handles audio.
  if (!isTrainerValid()) g_feeding = false;
}

bool v15MeasureTrainerFeedIsActive(void) { return g_feeding; }

#else

void v15MeasureTrainerFeedTask(void) {}
void v15MeasureTrainerFeedPushBusRaw(const uint16_t*, uint8_t) {}
void v15MeasureTrainerFeedPushUs(const uint16_t*, uint8_t) {}
void v15MeasureTrainerFeedPushDjiRs(const uint16_t*, uint8_t) {}
bool v15MeasureTrainerFeedIsActive(void) { return false; }

#endif
