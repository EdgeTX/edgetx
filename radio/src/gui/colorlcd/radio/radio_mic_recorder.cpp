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

#include "button.h"
#include "dialog.h"
#include "edgetx.h"
#include "ff.h"
#include "sdcard.h"
#include "static.h"
#include "timers_driver.h"

static constexpr coord_t MIC_BTN_W = 260;
static constexpr coord_t MIC_BTN_H = EdgeTxStyles::UI_ELEMENT_HEIGHT * 2 + PAD_LARGE;

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

  bigLabel = new StaticText(
      window,
      {0, h / 3 - 20, w, 40},
      "", COLOR_THEME_PRIMARY1_INDEX, FONT(XL));
  lv_obj_set_style_text_align(bigLabel->getLvObj(), LV_TEXT_ALIGN_CENTER, 0);

  infoLabel = new StaticText(
      window,
      {0, h / 3 + 30, w, 24},
      "", COLOR_THEME_SECONDARY1_INDEX, FONT(STD));
  lv_obj_set_style_text_align(infoLabel->getLvObj(), LV_TEXT_ALIGN_CENTER, 0);

  actionButton = new TextButton(
      window,
      {(w - MIC_BTN_W) / 2,
       h - MIC_BTN_H - PAD_LARGE * 2,
       MIC_BTN_W, MIC_BTN_H},
      "", [this]() {
        onActionPressed();
        return 0;
      });
  actionButton->setFont(FONT_XL_INDEX);
}

void RadioMicRecorder::onEvent(event_t event)
{
  if (event == EVT_KEY_LONG(KEY_EXIT)) {
    killEvents(event);
    if (recorder.isRecording()) recorder.stop();
    onCancel();
  }
}

void RadioMicRecorder::onActionPressed()
{
  switch (state) {
    case State::IDLE:      enterCountdown(); break;
    case State::COUNTDOWN: enterIdle();      break;   // cancel
    case State::RECORDING: stopRecording();  break;
  }
}

void RadioMicRecorder::enterIdle()
{
  if (recorder.isRecording()) recorder.stop();
  state = State::IDLE;
  stateStart = get_tmr10ms();
  refreshUI();
}

void RadioMicRecorder::enterCountdown()
{
  state = State::COUNTDOWN;
  stateStart = get_tmr10ms();
  refreshUI();
}

void RadioMicRecorder::enterRecording()
{
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
  state = State::RECORDING;
  stateStart = get_tmr10ms();
  refreshUI();
}

void RadioMicRecorder::stopRecording()
{
  recorder.stop();
  PdmWavRecorder::trimSilence(filename);
  state = State::IDLE;
  stateStart = get_tmr10ms();
  refreshUI();

  const char* base = strrchr(filename, '/');
  base = base ? base + 1 : filename;

  char baseName[PATH_MAX_LEN] = {};
  strncpy(baseName, base, sizeof(baseName) - 1);
  char* dot = strrchr(baseName, '.');
  if (dot) *dot = '\0';

  new LabelDialog(baseName, PATH_MAX_LEN - 1, STR_SAVE_AS, [this](std::string newName) {
    if (newName.empty()) return;
    char dir[sizeof(SOUNDS_PATH) + 1];
    strcpy(dir, SOUNDS_PATH "/");
    strncpy(dir + SOUNDS_PATH_LNG_OFS, currentLanguagePack->id, 2);
    snprintf(pendingRename, sizeof(pendingRename), "%s%s.wav", dir, newName.c_str());
    if (strcmp(pendingRename, filename) == 0) { refreshUI(); return; }

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
      state = State::IDLE;
      stateStart = get_tmr10ms();
    }
    refreshUI();
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
