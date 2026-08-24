/*
* Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "radio_mic_recorder.h"

#if defined(PDM_CLOCK)

#include <stdio.h>

#include "audio.h"
#include "button.h"
#include "dialog.h"
#include "edgetx.h"
#include "ff.h"
#include "sdcard.h"
#include "static.h"
#include "timers_driver.h"


static constexpr coord_t MIC_BTN_W = 260;
static constexpr coord_t MIC_BTN_H = EdgeTxStyles::UI_ELEMENT_HEIGHT * 2 + PAD_LARGE;

// Mirrored peak envelope, drawn as two polylines around a centre line. Fed one
// column at a time while recording, then re-filled from the finished file.
class WaveformView : public Window
{
 public:
  static constexpr coord_t COL_STEP = 2;
  static constexpr uint16_t MAX_COLS = 256;

  WaveformView(Window* parent, const rect_t& rect) : Window(parent, rect)
  {
    padAll(PAD_ZERO);

    uint16_t fit = (uint16_t)((width() - 1) / COL_STEP + 1);
    maxCols = fit < MAX_COLS ? fit : MAX_COLS;

    const lv_coord_t mid = height() / 2;
    basePts[0] = {0, mid};
    basePts[1] = {(lv_coord_t)(width() - 1), mid};
    baseLine = lv_line_create(lvobj);
    etx_obj_add_style(baseLine, styles->graph_dashed, LV_PART_MAIN);
    lv_line_set_points(baseLine, basePts, 2);

    topLine = lv_line_create(lvobj);
    etx_obj_add_style(topLine, styles->graph_line, LV_PART_MAIN);
    etx_obj_add_style(topLine, styles->line_color[COLOR_THEME_ACTIVE_INDEX],
                      LV_PART_MAIN);

    botLine = lv_line_create(lvobj);
    etx_obj_add_style(botLine, styles->graph_line, LV_PART_MAIN);
    etx_obj_add_style(botLine, styles->line_color[COLOR_THEME_ACTIVE_INDEX],
                      LV_PART_MAIN);

    cursorLine = lv_line_create(lvobj);
    etx_obj_add_style(cursorLine, styles->graph_line, LV_PART_MAIN);
    etx_obj_add_style(cursorLine, styles->line_color[COLOR_THEME_WARNING_INDEX],
                      LV_PART_MAIN);
    lv_obj_add_flag(cursorLine, LV_OBJ_FLAG_HIDDEN);

    clear();
  }

  uint16_t columns() const { return maxCols; }

  void clear()
  {
    count = 0;
    merge = 1;
    acc = 0;
    accCount = 0;
    setCursor(-1);
    redraw();
  }

  // Playback position, 0..1000 across the view. Negative hides the cursor.
  void setCursor(int32_t permille)
  {
    if (permille < 0) {
      if (cursorAt >= 0) lv_obj_add_flag(cursorLine, LV_OBJ_FLAG_HIDDEN);
      cursorAt = -1;
      return;
    }
    if (permille > 1000) permille = 1000;

    const lv_coord_t x = (lv_coord_t)((permille * (width() - 1)) / 1000);
    if (cursorAt >= 0 && x == cursorPts[0].x) return;

    cursorPts[0] = {x, 0};
    cursorPts[1] = {x, (lv_coord_t)(height() - 1)};
    lv_line_set_points(cursorLine, cursorPts, 2);
    lv_obj_clear_flag(cursorLine, LV_OBJ_FLAG_HIDDEN);
    cursorAt = permille;
  }

  // Live feed. Once the view is full, columns are merged pairwise so the whole
  // take stays visible at half the time resolution.
  void push(uint8_t level)
  {
    if (level > acc) acc = level;
    if (++accCount < merge) return;
    if (count >= maxCols) compact();
    cols[count++] = acc;
    acc = 0;
    accCount = 0;
    redraw();
  }

  void setEnvelope(const uint8_t* levels, uint16_t n)
  {
    if (n > maxCols) n = maxCols;
    memcpy(cols, levels, n);
    count = n;
    redraw();
  }

 protected:
  uint16_t maxCols = 0;
  uint16_t count = 0;
  uint16_t merge = 1;
  uint16_t accCount = 0;
  uint8_t acc = 0;
  uint8_t cols[MAX_COLS] = {0};

  lv_point_t topPts[MAX_COLS];
  lv_point_t botPts[MAX_COLS];
  lv_point_t basePts[2];
  lv_point_t cursorPts[2];
  int32_t cursorAt = -1;
  lv_obj_t* cursorLine = nullptr;
  lv_obj_t* topLine = nullptr;
  lv_obj_t* botLine = nullptr;
  lv_obj_t* baseLine = nullptr;

  void compact()
  {
    const uint16_t n = count / 2;
    for (uint16_t i = 0; i < n; i++) {
      const uint8_t a = cols[2 * i], b = cols[2 * i + 1];
      cols[i] = a > b ? a : b;
    }
    count = n;
    merge *= 2;
  }

  void redraw()
  {
    if (count < 2) {
      lv_obj_add_flag(topLine, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(botLine, LV_OBJ_FLAG_HIDDEN);
      return;
    }

    const lv_coord_t mid = height() / 2;
    const int32_t half = height() / 2 - 1;

    for (uint16_t i = 0; i < count; i++) {
      const lv_coord_t x = (lv_coord_t)(i * COL_STEP);
      const lv_coord_t a = (lv_coord_t)((cols[i] * half) / 255);
      topPts[i] = {x, (lv_coord_t)(mid - a)};
      botPts[i] = {x, (lv_coord_t)(mid + a)};
    }

    lv_line_set_points(topLine, topPts, count);
    lv_line_set_points(botLine, botPts, count);
    lv_obj_clear_flag(topLine, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(botLine, LV_OBJ_FLAG_HIDDEN);
  }
};

RadioMicRecorder::RadioMicRecorder() :
    Page(ICON_RADIO_TOOLS)
{
  // Bring the PDM hardware up only while this tool is open. Capture is off
  // outside of this scope so the mic clock and DMA stream are idle.
  pdmStart();
  buildHeader(header);
  buildBody(body);
  enterIdle();
}

RadioMicRecorder::~RadioMicRecorder()
{
  // An async call scheduled from the LabelDialog confirm path may still be
  // pending when the page is closed (e.g. user long-presses EXIT while the
  // dialog is being dismissed). Cancelling ensures the callback never fires
  // on a destroyed instance.
  lv_async_call_cancel(&RadioMicRecorder::asyncProcessPendingRename, this);
  if (recorder.isRecording()) recorder.stop();
  if (isPlayingTake()) audioQueue.stopAll();
  pdmStop();
}

void RadioMicRecorder::asyncProcessPendingRename(void* ctx)
{
  static_cast<RadioMicRecorder*>(ctx)->processPendingRename();
}

void RadioMicRecorder::buildHeader(Window* window)
{
  header->setTitle(STR_MENUTOOLS);
  header->setTitle2(STR_MIC_RECORDER);
}

void RadioMicRecorder::buildBody(Window* window)
{
  window->padAll(PAD_ZERO);

  const coord_t w = window->width();
  const coord_t h = window->height();

  const coord_t btnY = h - MIC_BTN_H - PAD_LARGE * 2;
  const coord_t bigH = EdgeTxStyles::UI_ELEMENT_HEIGHT + PAD_MEDIUM;
  const coord_t infoH = EdgeTxStyles::STD_FONT_HEIGHT;
  const coord_t infoY = btnY - infoH - PAD_SMALL;
  const coord_t waveY = PAD_SMALL + bigH + PAD_SMALL;

  bigLabel = new StaticText(
      window,
      {0, PAD_SMALL, w, bigH},
      "", COLOR_THEME_PRIMARY1_INDEX, FONT(XL));
  lv_obj_set_style_text_align(bigLabel->getLvObj(), LV_TEXT_ALIGN_CENTER, 0);

  waveform = new WaveformView(
      window, {PAD_LARGE, waveY, w - PAD_LARGE * 2, infoY - waveY - PAD_SMALL});
  waveform->hide();

  infoLabel = new StaticText(
      window,
      {0, infoY, w, infoH},
      "", COLOR_THEME_SECONDARY1_INDEX, FONT(STD));
  lv_obj_set_style_text_align(infoLabel->getLvObj(), LV_TEXT_ALIGN_CENTER, 0);

  actionButton = new TextButton(
      window,
      {(w - MIC_BTN_W) / 2, btnY, MIC_BTN_W, MIC_BTN_H},
      "", [this]() {
        onActionPressed();
        return 0;
      });
  actionButton->setFont(FONT_XL_INDEX);

  const coord_t revW = (w - PAD_LARGE * 4) / 3;

  playButton = new TextButton(
      window, {PAD_LARGE, btnY, revW, MIC_BTN_H}, STR_PLAY_FILE, [this]() {
        onPlayPressed();
        return 0;
      });
  playButton->setWrap();

  saveButton = new TextButton(
      window, {PAD_LARGE * 2 + revW, btnY, revW, MIC_BTN_H}, STR_SAVE_AS,
      [this]() {
        askSaveAs();
        return 0;
      });
  saveButton->setWrap();

  redoButton = new TextButton(
      window, {PAD_LARGE * 3 + revW * 2, btnY, revW, MIC_BTN_H}, STR_RECORD,
      [this]() {
        discardTake();
        enterCountdown();
        return 0;
      });
  redoButton->setWrap();

  playButton->hide();
  saveButton->hide();
  redoButton->hide();
}

void RadioMicRecorder::showReviewButtons(bool reviewing)
{
  actionButton->show(!reviewing);
  playButton->show(reviewing);
  saveButton->show(reviewing);
  redoButton->show(reviewing);
  lv_group_focus_obj(reviewing ? playButton->getLvObj()
                               : actionButton->getLvObj());
}

bool RadioMicRecorder::isPlayingTake() const
{
  return audioQueue.isPlaying(ID_PLAY_FROM_SD_MANAGER);
}

void RadioMicRecorder::onPlayPressed()
{
  if (isPlayingTake()) {
    audioQueue.stopAll();
  } else {
    audioQueue.stopAll();
    audioQueue.playFile(filename, 0, ID_PLAY_FROM_SD_MANAGER);
    playStart = get_tmr10ms();
  }
  playingShown = isPlayingTake();
  if (!playingShown) waveform->setCursor(-1);
  refreshUI();
}

// Drops the take the user just rejected, unless it was already saved under a
// name of its own.
void RadioMicRecorder::discardTake()
{
  if (!takeSaved && filename[0]) f_unlink(filename);
}

void RadioMicRecorder::onEvent(event_t event)
{
  if (event == EVT_KEY_LONG(KEY_EXIT)) {
    killEvents(event);
    if (recorder.isRecording()) recorder.stop();
    if (isPlayingTake()) audioQueue.stopAll();
    onCancel();
  }
}

void RadioMicRecorder::onActionPressed()
{
  switch (state) {
    case State::IDLE:      enterCountdown(); break;
    case State::COUNTDOWN: enterIdle();      break;   // cancel
    case State::RECORDING: stopRecording();  break;
    case State::REVIEW:    break;  // handled by the review buttons
  }
}

void RadioMicRecorder::enterIdle()
{
  if (recorder.isRecording()) recorder.stop();
  showReviewButtons(false);
  waveform->hide();
  state = State::IDLE;
  stateStart = get_tmr10ms();
  refreshUI();
}

void RadioMicRecorder::enterCountdown()
{
  if (isPlayingTake()) audioQueue.stopAll();
  showReviewButtons(false);
  waveform->hide();
  state = State::COUNTDOWN;
  stateStart = get_tmr10ms();
  refreshUI();
}

void RadioMicRecorder::enterRecording()
{
  takeSaved = false;
  pickNextFilename();
  FRESULT res = recorder.start(filename, 0);
  if (res != FR_OK) {
    char msg[40];
    snprintf(msg, sizeof(msg), "%s %u", STR_OPEN_ERROR, (unsigned)res);
    bigLabel->setText(msg);
    state = State::IDLE;
    stateStart = get_tmr10ms();
    actionButton->setText(STR_RECORD);
    return;
  }
  waveform->clear();
  waveform->show();
  lastWaveSamples = 0;
  takeSamples = 0;
  (void)recorder.takePeakLevel();   // drop the level left over from the last take
  state = State::RECORDING;
  stateStart = get_tmr10ms();
  refreshUI();
}

void RadioMicRecorder::stopRecording()
{
  recorder.stop();

  uint32_t clipped = 0;
  const uint16_t cols = waveform->columns();
  uint8_t env[WaveformView::MAX_COLS];
  if (PdmWavRecorder::finalise(filename, env, cols, &takeSamples, &clipped) == FR_OK)
    waveform->setEnvelope(env, cols);
  TRACE("mic: %u/1000 samples clipped at capture", (unsigned)clipped);

  enterReview();
}

void RadioMicRecorder::enterReview()
{
  playingShown = false;
  showReviewButtons(true);
  waveform->show();
  state = State::REVIEW;
  stateStart = get_tmr10ms();
  refreshUI();
}

void RadioMicRecorder::askSaveAs()
{
  if (isPlayingTake()) audioQueue.stopAll();

  const char* base = strrchr(filename, '/');
  base = base ? base + 1 : filename;

  char baseName[PATH_MAX_LEN] = {};
  strncpy(baseName, base, sizeof(baseName) - 1);
  char* dot = strrchr(baseName, '.');
  if (dot) *dot = '\0';

  new LabelDialog(baseName, LEN_FUNCTION_NAME, STR_SAVE_AS, [this](std::string newName) {
    if (newName.empty()) return;
    char dir[sizeof(SOUNDS_PATH) + 1];
    strcpy(dir, SOUNDS_PATH "/");
    strncpy(dir + SOUNDS_PATH_LNG_OFS, currentLanguagePack->id, 2);
    snprintf(pendingRename, sizeof(pendingRename), "%s%s.wav", dir, newName.c_str());
    if (strcmp(pendingRename, filename) == 0) { takeSaved = true; refreshUI(); return; }

    // Defer the overwrite check until LabelDialog has finished closing —
    // creating a modal now would leave us stacked on top of LabelDialog,
    // which corrupts the lv_group chain when LabelDialog deletes itself.
    // The page dtor cancels this async call if the user bails out before
    // it fires (see ~RadioMicRecorder).
    lv_async_call(&RadioMicRecorder::asyncProcessPendingRename, this);
  });
}

void RadioMicRecorder::processPendingRename()
{
  FILINFO info;
  if (f_stat(pendingRename, &info) == FR_OK) {
    new ConfirmDialog(STR_FILE_EXISTS, STR_ASK_OVERWRITE,
                      [this]() { applyRename(); });
  } else {
    applyRename();
  }
}

void RadioMicRecorder::applyRename()
{
  f_unlink(pendingRename);
  f_rename(filename, pendingRename);
  strncpy(filename, pendingRename, sizeof(filename) - 1);
  filename[sizeof(filename) - 1] = '\0';
  takeSaved = true;
  refreshUI();
}

void RadioMicRecorder::refreshUI()
{
  char buf[64];
  switch (state) {
    case State::IDLE:
      bigLabel->setText(STR_PUSH_TO_RECORD);
      if (recorder.getSamplesWritten() > 0) {
        snprintf(buf, sizeof(buf), "%s %s (%us)", STR_SAVED,
                 filename, (unsigned)recorder.getElapsedSeconds());
        infoLabel->setText(buf);
      } else {
        infoLabel->setText("");
      }
      actionButton->setText(STR_RECORD);
      break;

    case State::COUNTDOWN: {
      const uint32_t elapsed10 = (uint32_t)(get_tmr10ms() - stateStart);
      int remaining = (int)COUNTDOWN_SECONDS - (int)(elapsed10 / 100U);
      if (remaining < 0) remaining = 0;
      snprintf(buf, sizeof(buf), "%s %d", STR_STARTING_IN, remaining);
      bigLabel->setText(buf);
      infoLabel->setText(STR_GET_READY);
      actionButton->setText(STR_CANCEL);
      break;
    }

    case State::RECORDING: {
      const uint32_t s = recorder.getElapsedSeconds();
      snprintf(buf, sizeof(buf), "%s %02u:%02u", STR_REC,
               (unsigned)(s / 60U), (unsigned)(s % 60U));
      bigLabel->setText(buf);
      snprintf(buf, sizeof(buf), "%s  %u KB",
               filename, (unsigned)(recorder.getBytesWritten() / 1024U));
      infoLabel->setText(buf);
      actionButton->setText(STR_STOP);
      break;
    }

    case State::REVIEW: {
      const uint32_t s = takeSamples / PdmWavRecorder::DST_RATE;
      snprintf(buf, sizeof(buf), "%02u:%02u", (unsigned)(s / 60U),
               (unsigned)(s % 60U));
      bigLabel->setText(buf);
      if (takeSaved) {
        snprintf(buf, sizeof(buf), "%s %s", STR_SAVED, filename);
        infoLabel->setText(buf);
      } else {
        infoLabel->setText(filename);
      }
      playButton->setText(isPlayingTake() ? STR_STOP : STR_PLAY_FILE);
      break;
    }
  }
}

void RadioMicRecorder::checkEvents()
{
  Page::checkEvents();

  if (state == State::COUNTDOWN) {
    const uint32_t elapsed10 = (uint32_t)(get_tmr10ms() - stateStart);
    if (elapsed10 >= COUNTDOWN_SECONDS * 100U) {
      enterRecording();
    } else {
      refreshUI();
    }
  } else if (state == State::RECORDING) {
    if (!recorder.isRecording()) {
      stopRecording();   // recorder ended on its own (write error / limit)
    } else {
      const uint32_t written = recorder.getSamplesWritten();
      if (written != lastWaveSamples) {
        lastWaveSamples = written;
        waveform->push(recorder.takePeakLevel());
      }
      refreshUI();
    }
  } else if (state == State::REVIEW) {
    const bool playing = isPlayingTake();

    // The audio queue does not expose a play position, so the cursor runs off
    // the wall clock: playback is sample-accurate once started, so the only
    // error is the queue's own start latency.
    if (playing && takeSamples) {
      const uint32_t elapsed10 = (uint32_t)(get_tmr10ms() - playStart);
      const uint32_t total10 = takeSamples / (PdmWavRecorder::DST_RATE / 100U);
      waveform->setCursor(total10 ? (int32_t)((elapsed10 * 1000U) / total10)
                                  : 1000);
    }

    if (playing != playingShown) {
      playingShown = playing;
      if (!playing) waveform->setCursor(-1);
      refreshUI();
    }
  }
}

void RadioMicRecorder::pickNextFilename()
{
  char dir[sizeof(SOUNDS_PATH) + 1];
  strcpy(dir, SOUNDS_PATH "/");
  strncpy(dir + SOUNDS_PATH_LNG_OFS, currentLanguagePack->id, 2);

  FILINFO info;
  for (int i = 0; i < 100; i++) {
    snprintf(filename, sizeof(filename), "%srec_%02d.wav", dir, i);
    if (f_stat(filename, &info) != FR_OK) return;
  }
  snprintf(filename, sizeof(filename), "%srec.wav", dir);
}

#endif  // PDM_CLOCK
