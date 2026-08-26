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
#include "form.h"
#include "hal/rotary_encoder.h"
#include "sdcard.h"
#include "static.h"
#include "timers_driver.h"


static constexpr coord_t MIC_BTN_W = 260;
static constexpr coord_t MIC_BTN_H = EdgeTxStyles::UI_ELEMENT_HEIGHT * 2 + PAD_LARGE;

// Review needs two rows in the space the single record button occupies.
static constexpr coord_t REV_BTN_H = EdgeTxStyles::UI_ELEMENT_HEIGHT + PAD_MEDIUM;

// Mirrored peak envelope, drawn as two polylines around a centre line. Fed one
// column at a time while recording, then re-filled from the finished file.
// It doubles as the trim editor: a marker line dragged straight over the trace,
// with the part that would be cut greyed out.
class WaveformView : public FormField
{
 public:
  static constexpr uint16_t MAX_COLS = 400;
  static constexpr coord_t KNOB_W = PAD_LARGE * 2;
  static constexpr coord_t KNOB_H = PAD_MEDIUM;

  WaveformView(Window* parent, const rect_t& rect,
               std::function<void(int)> onMove, std::function<void()> onConfirm,
               std::function<void()> onDismiss) :
      FormField(parent, rect),
      moveHandler(std::move(onMove)),
      confirmHandler(std::move(onConfirm)),
      dismissHandler(std::move(onDismiss))
  {
    padAll(PAD_ZERO);

    // No scrolling: a horizontal drag has to reach the marker, not pan the view.
    lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_SCROLLABLE);
    // No click focus either: LVGL clears the group's edit mode on every focus
    // call, so a touch would otherwise leave the encoder stuck in navigation.
    lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    lv_obj_add_flag(lvobj, LV_OBJ_FLAG_ENCODER_ACCEL);
    lv_obj_add_event_cb(lvobj, WaveformView::on_pressing, LV_EVENT_PRESSING, this);
    lv_obj_add_event_cb(lvobj, WaveformView::on_released, LV_EVENT_RELEASED, this);
    lv_obj_add_event_cb(lvobj, WaveformView::on_key, LV_EVENT_KEY, this);
    etx_obj_add_style(lvobj, styles->outline, LV_PART_MAIN | LV_STATE_EDITED);
    etx_obj_add_style(lvobj, styles->outline_color_edit,
                      LV_PART_MAIN | LV_STATE_EDITED);
    setFocusHandler([this](bool focus) {
      if (!focus && isEditMode()) setEditMode(false);
    });

    // Widen the columns rather than leaving the trace short of the edge:
    // 800 px screens need more than MAX_COLS columns at 2 px.
    colStep = (coord_t)((width() + MAX_COLS - 1) / MAX_COLS);
    if (colStep < 2) colStep = 2;
    maxCols = (uint16_t)((width() - 1) / colStep + 1);
    if (maxCols > MAX_COLS) maxCols = MAX_COLS;
    traceW = (coord_t)((maxCols - 1) * colStep);

    const lv_coord_t mid = height() / 2;
    basePts[0] = {0, mid};
    basePts[1] = {traceW, mid};
    baseLine = lv_line_create(lvobj);
    etx_obj_add_style(baseLine, styles->graph_dashed, LV_PART_MAIN);
    lv_line_set_points(baseLine, basePts, 2);

    topLine = lv_line_create(lvobj);
    etx_obj_add_style(topLine, styles->graph_line, LV_PART_MAIN);
    etx_obj_add_style(topLine, styles->line_color[COLOR_THEME_SECONDARY1_INDEX],
                      LV_PART_MAIN);

    botLine = lv_line_create(lvobj);
    etx_obj_add_style(botLine, styles->graph_line, LV_PART_MAIN);
    etx_obj_add_style(botLine, styles->line_color[COLOR_THEME_SECONDARY1_INDEX],
                      LV_PART_MAIN);

    cursorLine = lv_line_create(lvobj);
    etx_obj_add_style(cursorLine, styles->graph_line, LV_PART_MAIN);
    etx_obj_add_style(cursorLine, styles->line_color[COLOR_THEME_WARNING_INDEX],
                      LV_PART_MAIN);
    lv_obj_add_flag(cursorLine, LV_OBJ_FLAG_HIDDEN);

    // Created last so they sit on top of the trace.
    cutShade = lv_obj_create(lvobj);
    lv_obj_clear_flag(cutShade, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(cutShade, LV_OBJ_FLAG_SCROLLABLE);
    etx_bg_color(cutShade, COLOR_THEME_DISABLED_INDEX, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(cutShade, LV_OPA_60, LV_PART_MAIN);
    lv_obj_set_style_border_width(cutShade, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(cutShade, 0, LV_PART_MAIN);
    lv_obj_add_flag(cutShade, LV_OBJ_FLAG_HIDDEN);

    trimLine = lv_line_create(lvobj);
    etx_obj_add_style(trimLine, styles->graph_line, LV_PART_MAIN);
    etx_obj_add_style(trimLine, styles->line_color[COLOR_THEME_EDIT_INDEX],
                      LV_PART_MAIN);
    lv_obj_set_style_line_width(trimLine, PAD_THREE, LV_PART_MAIN);
    lv_obj_add_flag(trimLine, LV_OBJ_FLAG_HIDDEN);

    trimKnob = lv_obj_create(lvobj);
    lv_obj_clear_flag(trimKnob, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(trimKnob, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(trimKnob, KNOB_W, KNOB_H);
    etx_solid_bg(trimKnob, COLOR_THEME_EDIT_INDEX);
    lv_obj_set_style_border_width(trimKnob, 0, LV_PART_MAIN);
    etx_obj_add_style(trimKnob, styles->rounded, LV_PART_MAIN);
    lv_obj_add_flag(trimKnob, LV_OBJ_FLAG_HIDDEN);

    resetTrace();
  }

  uint16_t columns() const { return maxCols; }

  void resetTrace()
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

    const lv_coord_t x = (lv_coord_t)((permille * traceW) / 1000);
    if (cursorAt >= 0 && x == cursorPts[0].x) return;

    cursorPts[0] = {x, 0};
    cursorPts[1] = {x, (lv_coord_t)(height() - 1)};
    lv_line_set_points(cursorLine, cursorPts, 2);
    lv_obj_clear_flag(cursorLine, LV_OBJ_FLAG_HIDDEN);
    cursorAt = permille;
  }

  // Trim editing: the marker takes the focus so the encoder drags it directly,
  // and a touch anywhere on the trace drops it at that point.
  void showTrim(int32_t permille, bool cutBefore)
  {
    trimShown = true;
    cutLeft = cutBefore;
    lv_obj_clear_flag(trimLine, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(trimKnob, LV_OBJ_FLAG_HIDDEN);
    setTrim(permille);
    lv_group_add_obj((lv_group_t*)lv_group_get_default(), lvobj);
    lv_group_focus_obj(lvobj);
    setEditMode(true);
  }

  void hideTrim()
  {
    if (!trimShown) return;
    trimShown = false;
    if (isEditMode()) setEditMode(false);
    lv_group_remove_obj(lvobj);
    lv_obj_add_flag(trimLine, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(trimKnob, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(cutShade, LV_OBJ_FLAG_HIDDEN);
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
  coord_t colStep = 2;
  coord_t traceW = 0;
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
  lv_point_t trimPts[2];
  int32_t cursorAt = -1;
  lv_obj_t* cursorLine = nullptr;
  lv_obj_t* topLine = nullptr;
  lv_obj_t* botLine = nullptr;
  lv_obj_t* baseLine = nullptr;
  lv_obj_t* trimLine = nullptr;
  lv_obj_t* trimKnob = nullptr;
  lv_obj_t* cutShade = nullptr;

  std::function<void(int)> moveHandler;
  std::function<void()> confirmHandler;
  std::function<void()> dismissHandler;
  bool trimShown = false;
  bool cutLeft = false;
  int32_t trimAt = -1;

  void setTrim(int32_t permille)
  {
    trimAt = permille;

    const lv_coord_t x = (lv_coord_t)((permille * traceW) / 1000);
    trimPts[0] = {x, 0};
    trimPts[1] = {x, (lv_coord_t)(height() - 1)};
    lv_line_set_points(trimLine, trimPts, 2);
    lv_coord_t knobX = x - KNOB_W / 2;
    if (knobX < 0) knobX = 0;
    if (knobX > traceW - KNOB_W) knobX = traceW - KNOB_W;
    lv_obj_set_pos(trimKnob, knobX, 0);

    const lv_coord_t w = cutLeft ? x : (lv_coord_t)(traceW - x);
    if (w > 0) {
      lv_obj_set_pos(cutShade, cutLeft ? 0 : x, 0);
      lv_obj_set_size(cutShade, w, height());
      lv_obj_clear_flag(cutShade, LV_OBJ_FLAG_HIDDEN);
    } else {
      lv_obj_add_flag(cutShade, LV_OBJ_FLAG_HIDDEN);
    }
  }

  void moveTo(int32_t permille)
  {
    if (permille < 0) permille = 0;
    if (permille > 1000) permille = 1000;
    if (permille == trimAt) return;
    setTrim(permille);
    if (moveHandler) moveHandler(permille);
  }

  static void on_pressing(lv_event_t* e)
  {
    auto wv = (WaveformView*)lv_event_get_user_data(e);
    if (!wv || wv->deleted() || !wv->trimShown || wv->traceW <= 0) return;

    auto indev = (lv_indev_t*)lv_event_get_param(e);
    if (!indev || lv_indev_get_type(indev) != LV_INDEV_TYPE_POINTER) return;

    lv_area_t coords;
    lv_obj_get_coords(lv_event_get_target(e), &coords);
    lv_point_t point;
    lv_indev_get_point(indev, &point);

    const int32_t x = point.x - coords.x1;
    wv->moveTo((x * 1000) / wv->traceW);
    wv->rearmEdit();
  }

  static void on_released(lv_event_t* e)
  {
    auto wv = (WaveformView*)lv_event_get_user_data(e);
    if (wv && !wv->deleted()) wv->rearmEdit();
  }

  // Anything that focuses an object drops edit mode group wide, so take it back
  // after a touch: the encoder has to keep driving the marker.
  void rearmEdit()
  {
    if (!trimShown || isEditMode()) return;
    auto g = (lv_group_t*)lv_obj_get_group(lvobj);
    if (g && lv_group_get_focused(g) == lvobj) setEditMode(true);
  }

  static void on_key(lv_event_t* e)
  {
    auto wv = (WaveformView*)lv_event_get_user_data(e);
    if (!wv || wv->deleted() || !wv->trimShown) return;

    const uint32_t key = lv_event_get_key(e);
    if (key != LV_KEY_LEFT && key != LV_KEY_RIGHT) return;

    // One detent moves the marker by one column, more when spun fast.
    int32_t step = wv->maxCols ? 1000 / wv->maxCols : 1;
    if (step < 1) step = 1;
    step += rotaryEncoderGetAccel() / 4;
    wv->moveTo(wv->trimAt + (key == LV_KEY_RIGHT ? step : -step));
  }

  // A touch only drops the marker; ENTER is what confirms the trim.
  void onClicked() override
  {
    if (!trimShown) {
      FormField::onClicked();
      return;
    }
    if (lv_indev_get_type(lv_indev_get_act()) == LV_INDEV_TYPE_POINTER) return;
    if (confirmHandler) confirmHandler();
  }

  void onCancel() override
  {
    if (trimShown && dismissHandler) {
      dismissHandler();
      return;
    }
    FormField::onCancel();
  }

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
      const lv_coord_t x = (lv_coord_t)(i * colStep);
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

void RadioMicRecorder::deleteLater(bool detach, bool trash)
{
  if (!deleted()) {
    if (recorder.isRecording()) recorder.stop();
    if (isPlayingTake()) audioQueue.stopAll();
    pdmStop();

    Page::deleteLater(detach, trash);
  }
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
  const coord_t rowY2 = h - REV_BTN_H - PAD_LARGE;      // play / save / record
  const coord_t rowY1 = rowY2 - REV_BTN_H - PAD_SMALL;  // the trim actions
  const coord_t bigH = EdgeTxStyles::UI_ELEMENT_HEIGHT + PAD_MEDIUM;
  const coord_t infoH = EdgeTxStyles::STD_FONT_HEIGHT;
  const coord_t infoY = rowY1 - infoH - PAD_SMALL;
  const coord_t waveY = PAD_SMALL + bigH + PAD_SMALL;

  bigLabel = new StaticText(
      window,
      {0, PAD_SMALL, w, bigH},
      "", COLOR_THEME_PRIMARY1_INDEX, FONT(XL));
  lv_obj_set_style_text_align(bigLabel->getLvObj(), LV_TEXT_ALIGN_CENTER, 0);

  waveform = new WaveformView(
      window, {PAD_LARGE, waveY, w - PAD_LARGE * 2, infoY - waveY - PAD_SMALL},
      [this](int v) { onTrimMoved(v); }, [this]() { onApplyTrim(); },
      [this]() { exitTrim(); });
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
  const coord_t revX[3] = {PAD_LARGE, (coord_t)(PAD_LARGE * 2 + revW),
                           (coord_t)(PAD_LARGE * 3 + revW * 2)};

  autoTrimButton = new TextButton(
      window, {revX[0], rowY1, revW, REV_BTN_H}, STR_AUTO_TRIM, [this]() {
        onAutoTrim();
        return 0;
      });

  trimStartButton = new TextButton(
      window, {revX[1], rowY1, revW, REV_BTN_H}, STR_TRIM_START, [this]() {
        enterTrim(TrimMode::START);
        return 0;
      });

  trimEndButton = new TextButton(
      window, {revX[2], rowY1, revW, REV_BTN_H}, STR_TRIM_END, [this]() {
        enterTrim(TrimMode::END);
        return 0;
      });

  playButton = new TextButton(
      window, {revX[0], rowY2, revW, REV_BTN_H}, STR_PLAY_FILE, [this]() {
        onPlayPressed();
        return 0;
      });

  saveButton = new TextButton(
      window, {revX[1], rowY2, revW, REV_BTN_H}, STR_SAVE_AS, [this]() {
        askSaveAs();
        return 0;
      });

  redoButton = new TextButton(
      window, {revX[2], rowY2, revW, REV_BTN_H}, STR_RECORD, [this]() {
        discardTake();
        enterCountdown();
        return 0;
      });

  applyButton = new TextButton(
      window, {revX[0], rowY2, revW, REV_BTN_H}, STR_OK, [this]() {
        onApplyTrim();
        return 0;
      });

  cancelButton = new TextButton(
      window, {revX[1], rowY2, revW, REV_BTN_H}, STR_CANCEL, [this]() {
        exitTrim();
        return 0;
      });

  updateButtons();
  built = true;
}

// The marker runs 0..1000 over the take so its range never has to change as
// the take gets shorter.
uint32_t RadioMicRecorder::trimSample() const
{
  if (takeSamples == 0) return 0;
  return (uint32_t)(((uint64_t)trimPermille * (takeSamples - 1)) / 1000U);
}

void RadioMicRecorder::enterTrim(TrimMode mode)
{
  if (isPlayingTake()) audioQueue.stopAll();
  if (takeSamples < 2) return;

  trimMode = mode;
  trimPermille = (mode == TrimMode::START) ? 0 : 1000;
  waveform->setCursor(-1);
  waveform->showTrim(trimPermille, mode == TrimMode::START);
  updateButtons();
  refreshUI();
}

void RadioMicRecorder::exitTrim()
{
  trimMode = TrimMode::NONE;
  waveform->hideTrim();
  waveform->setCursor(-1);
  updateButtons();
  refreshUI();
}

void RadioMicRecorder::onTrimMoved(int value)
{
  trimPermille = value;
  refreshUI();
}

void RadioMicRecorder::onApplyTrim()
{
  const uint32_t cut = trimSample();
  const TrimMode mode = trimMode;

  exitTrim();

  if (mode == TrimMode::START) {
    applyTrim(cut, takeSamples - 1);
  } else {
    applyTrim(0, cut);
  }
}

// Trims are sample surgery on the finished file: cut, refade the new edge,
// refresh the trace. Playback stops because the length just changed.
void RadioMicRecorder::applyTrim(uint32_t from, uint32_t to)
{
  if (isPlayingTake()) audioQueue.stopAll();
  waveform->hideTrim();
  waveform->setCursor(-1);

  const uint16_t cols = waveform->columns();
  uint8_t env[WaveformView::MAX_COLS];
  if (PdmWavRecorder::cut(filename, from, to, env, cols, &takeSamples) == FR_OK)
    waveform->setEnvelope(env, cols);

  playingShown = false;
  refreshUI();
}

void RadioMicRecorder::onAutoTrim()
{
  uint32_t from = 0, to = 0;
  if (PdmWavRecorder::silenceBounds(filename, &from, &to) == FR_OK)
    applyTrim(from, to);
}

void RadioMicRecorder::updateButtons()
{
  const bool review = (state == State::REVIEW);
  const bool trimming = review && trimMode != TrimMode::NONE;

  actionButton->show(!review);
  autoTrimButton->show(review && !trimming);
  trimStartButton->show(review && !trimming);
  trimEndButton->show(review && !trimming);
  playButton->show(review && !trimming);
  saveButton->show(review && !trimming);
  redoButton->show(review && !trimming);
  applyButton->show(trimming);
  cancelButton->show(trimming);

  if (!built) return;
  // While trimming the marker itself holds the focus.
  if (trimming)
    return;
  else if (review)
    lv_group_focus_obj(playButton->getLvObj());
  else
    lv_group_focus_obj(actionButton->getLvObj());
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
  state = State::IDLE;
  trimMode = TrimMode::NONE;
  waveform->hideTrim();
  updateButtons();
  waveform->hide();
  stateStart = get_tmr10ms();
  refreshUI();
}

void RadioMicRecorder::enterCountdown()
{
  if (isPlayingTake()) audioQueue.stopAll();
  state = State::COUNTDOWN;
  trimMode = TrimMode::NONE;
  waveform->hideTrim();
  updateButtons();
  waveform->hide();
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
  waveform->resetTrace();
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
  // Silence is left in place; Auto-trim is the deliberate action.
  if (PdmWavRecorder::finalise(filename, env, cols, false, &takeSamples,
                               &clipped) == FR_OK)
    waveform->setEnvelope(env, cols);
  TRACE("mic: %u/1000 samples clipped at capture", (unsigned)clipped);

  enterReview();
}

void RadioMicRecorder::enterReview()
{
  playingShown = false;
  state = State::REVIEW;
  trimMode = TrimMode::NONE;
  waveform->hideTrim();
  updateButtons();
  waveform->show();
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
    processPendingRename();
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
      if (trimMode != TrimMode::NONE) {
        // Show what would be left, so the marker reads as a result not a cut.
        const uint32_t cut = trimSample();
        const uint32_t kept = (trimMode == TrimMode::START)
                                  ? takeSamples - cut
                                  : cut + 1;
        const uint32_t k = kept / PdmWavRecorder::DST_RATE;
        snprintf(buf, sizeof(buf), "%02u:%02u", (unsigned)(k / 60U),
                 (unsigned)(k % 60U));
        bigLabel->setText(buf);
        infoLabel->setText(trimMode == TrimMode::START ? STR_TRIM_START
                                                       : STR_TRIM_END);
        break;
      }

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
