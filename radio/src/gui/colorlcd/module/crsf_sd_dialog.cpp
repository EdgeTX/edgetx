/*
 * Copyright (C) EdgeTX
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

// Session screen for a CRSF module's SD bulk session. Modal and blocking, the
// same way FlashDialog drives PXX2 OTA: the session loop runs in the UI task,
// so Lua is paused underneath and nothing else touches the card.

#include "edgetx.h"
#include "dialog.h"
#include "fullscreen_dialog.h"
#include "os/time.h"
#include "progress.h"
#include "pulses/crossfire_sd.h"

class CrsfSdDialog : public FullScreenDialog
{
 public:
  explicit CrsfSdDialog(const char* title) :
    FullScreenDialog(WARNING_TYPE_INFO, title, "Starting...")
  {
    progress = new Progress(
        this, {LCD_W / 2 - PROGRESS_W / 2, LCD_H / 2 + PROGRESS_YO, PROGRESS_W,
               EdgeTxStyles::UI_ELEMENT_HEIGHT});
    lv_refr_now(nullptr);
  }

  void run(uint8_t module)
  {
    CrsfSdUi ui;
    ui.progress = [=](const char* text, uint32_t done, uint32_t total) {
      // LVGL redraws are slow next to a 512 B round trip; cap at ~10 Hz
      uint32_t now = time_get_ms();
      if (now - lastDraw < 100 && done < total) return;
      lastDraw = now;
      setMessage(text);
      progress->setValue(total ? (uint64_t)done * 100 / total : 0);
      lv_refr_now(nullptr);
    };
    ui.cancelled = []() { return keysGetState(KEY_EXIT); };

    bool ok;
    const char* msg = crsfSdRunSession(module, ui, ok);
    closeWindow();

    AUDIO_PLAY(AU_SPECIAL_SOUND_BEEP1);
    BACKLIGHT_ENABLE();
    // not POPUP_WARNING: that one is modal and would hold the UI task here
    new MessageDialog(ok ? "Module SD session" : "Module SD session failed", msg);
  }

 protected:
  Progress* progress = nullptr;
  uint32_t lastDraw = 0;

  static LAYOUT_VAL_SCALED(PROGRESS_YO, 27)
  static LAYOUT_VAL_SCALED(PROGRESS_W, 200)
};

void crsfSdNotify(const char* title, const char* text)
{
  new MessageDialog(title, text);
}

void crsfSdRunWithUi(uint8_t module, uint8_t kind)
{
  const char* title = kind == CRSF_SD_KIND_RX_UPDATE    ? "Update receiver"
                      : kind == CRSF_SD_KIND_IMAGE_SYNC ? "Sync receiver firmware"
                                                        : "Module SD session";
  auto dialog = new CrsfSdDialog(title);
  dialog->run(module);
}
