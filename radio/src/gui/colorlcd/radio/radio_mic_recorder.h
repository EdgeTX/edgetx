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

#pragma once

#include "page.h"
#include "pdm_wav_recorder.h"
#include "sdcard.h"

#if defined(PDM_CLOCK)

class TextButton;
class StaticText;
class WaveformView;

class RadioMicRecorder : public Page
{
 public:
  RadioMicRecorder();
  ~RadioMicRecorder() override;

 protected:
  enum class State : uint8_t { IDLE, COUNTDOWN, RECORDING, REVIEW };

  static constexpr uint32_t COUNTDOWN_SECONDS = 5;
  static constexpr int PATH_MAX_LEN = sizeof(SOUNDS_PATH) + 14; // "/lang/rec_00.wav\0"

  State state = State::IDLE;
  tmr10ms_t stateStart = 0;
  char filename[PATH_MAX_LEN] = {0};
  char pendingRename[PATH_MAX_LEN] = {0};
  PdmWavRecorder recorder;

  bool takeSaved = false;
  bool playingShown = false;
  uint32_t lastWaveSamples = 0;
  uint32_t takeSamples = 0;
  tmr10ms_t playStart = 0;

  StaticText* bigLabel = nullptr;
  StaticText* infoLabel = nullptr;
  TextButton* actionButton = nullptr;
  TextButton* playButton = nullptr;
  TextButton* saveButton = nullptr;
  TextButton* redoButton = nullptr;
  WaveformView* waveform = nullptr;

  void buildHeader(Window* window);
  void buildBody(Window* window);
  void checkEvents() override;
  void onEvent(event_t event) override;

  void onActionPressed();
  void onPlayPressed();
  void enterIdle();
  void enterCountdown();
  void enterRecording();
  void enterReview();
  void stopRecording();
  void discardTake();
  void askSaveAs();
  void showReviewButtons(bool reviewing);
  bool isPlayingTake() const;
  void processPendingRename();
  void applyRename();
  void refreshUI();
  void pickNextFilename();

  // Stable callback address so the dtor can cancel a pending lv_async call
  // scheduled from the LabelDialog confirm path.
  static void asyncProcessPendingRename(void* ctx);
};

#endif  // PDM_CLOCK
