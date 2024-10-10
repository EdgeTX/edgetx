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
#include "edgetx.h"

void menuModelNotes(event_t event)
{
  if (event == EVT_ENTRY) {
    strcpy(reusableBuffer.viewText.filename, MODELS_PATH "/");
    char *buf = strcat_currentmodelname(
        &reusableBuffer.viewText.filename[sizeof(MODELS_PATH)], ' ');
    strcpy(buf, TEXT_EXT);
    if (!isFileAvailable(reusableBuffer.viewText.filename)) {
      buf = strcat_currentmodelname(
          &reusableBuffer.viewText.filename[sizeof(MODELS_PATH)], 0);
      strcpy(buf, TEXT_EXT);
    }
  }
  reusableBuffer.viewText.pushMenu = true;
  menuTextView(event);
}

void pushModelNotes()
{
  pushMenu(menuModelNotes);
}
