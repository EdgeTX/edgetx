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

#include "opentx.h"
#include "VirtualFS.h"

#if defined(ROTARY_ENCODER_NAVIGATION)
#define EVT_KEY_NEXT_LINE              EVT_ROTARY_RIGHT
#define EVT_KEY_PREVIOUS_LINE          EVT_ROTARY_LEFT
#else
#define EVT_KEY_NEXT_LINE              EVT_KEY_FIRST(KEY_DOWN)
#define EVT_KEY_PREVIOUS_LINE          EVT_KEY_FIRST(KEY_UP)
#endif

constexpr uint32_t TEXT_FILE_MAXSIZE = 2048;

static void storageReadTextFile(const char * filename, char lines[TEXT_VIEWER_LINES][LCD_COLS + 1], int & lines_count)
{
  VfsFile file;
  VfsError result;
  char c;
  unsigned int sz;
  int line_length = 0;
  uint8_t escape = 0;
  char escape_chars[4] = {0};
  int current_line = 0;

  memclear(lines, TEXT_VIEWER_LINES * (LCD_COLS + 1));

  result = VirtualFS::instance().openFile(file, filename, VfsOpenFlags::OPEN_EXISTING | VfsOpenFlags::READ);
  if (result == VfsError::OK) {
    for (uint32_t i = 0; i < TEXT_FILE_MAXSIZE && file.read(&c, 1, sz) == VfsError::OK && sz == 1 && (lines_count == 0 || current_line - menuVerticalOffset < int(TEXT_VIEWER_LINES)); i++) {
      if (c == '\n') {
        ++current_line;
        line_length = 0;
        escape = 0;
      }
      else if (c != '\r' && current_line >= menuVerticalOffset && current_line - menuVerticalOffset < int(TEXT_VIEWER_LINES) && line_length < LCD_COLS) {
        if (c == '\\' && escape == 0) {
          escape = 1;
          continue;
        }
        else if (c != '\\' && escape > 0 && escape < sizeof(escape_chars)) {
          escape_chars[escape - 1] = c;
          if (escape == 2 && !strncmp(escape_chars, "up", 2)) {
            lines[current_line-menuVerticalOffset][line_length++] = STR_CHAR_UP[0];
            c = STR_CHAR_UP[1];
          }
          else if (escape == 2 && !strncmp(escape_chars, "dn", 2)) {
            lines[current_line-menuVerticalOffset][line_length++] = STR_CHAR_DOWN[0];
            c = STR_CHAR_DOWN[1];
          }
          else if (escape == 3) {
            int val = atoi(escape_chars);
            if (val >= 200 && val < 225) {
              lines[current_line-menuVerticalOffset][line_length++] = '\302';
              c = '\200' + val - 200;
            }
          }
          else {
            escape++;
            continue;
          }
        }
        else if (c=='~') {
          c = 'z'+1;
        }
        else if (c=='\t') {
          c = 0x1D; //tab
        }
        escape = 0;
        lines[current_line-menuVerticalOffset][line_length++] = c;
      }
    }
    if (c != '\n') {
      current_line += 1;
    }
    file.close();
  }

  if (lines_count == 0) {
    lines_count = current_line;
  }
}

void readModelNotes()
{
  LED_ERROR_BEGIN();

  strcpy(reusableBuffer.viewText.filename, MODELS_PATH "/");
  char *buf = strcat_currentmodelname(&reusableBuffer.viewText.filename[sizeof(MODELS_PATH)], 0);
  strcpy(buf, TEXT_EXT);

  waitKeysReleased();
  event_t event = EVT_ENTRY;
  while (event != EVT_KEY_BREAK(KEY_EXIT)) {
    uint32_t power = pwrCheck();
    if (power != e_power_press) {
      lcdRefreshWait();
      lcdClear();
      menuTextView(event);
      lcdRefresh();
    }
    if (power == e_power_off){
      drawSleepBitmap();
      boardOff();
      break;
    }
    event = getEvent();
    WDG_RESET();
  }

  LED_ERROR_END();
}

void menuTextView(event_t event)
{
  switch (event) {
    case EVT_ENTRY:
      menuVerticalOffset = 0;
      reusableBuffer.viewText.linesCount = 0;
      storageReadTextFile(reusableBuffer.viewText.filename, reusableBuffer.viewText.lines, reusableBuffer.viewText.linesCount);
      break;

    case EVT_KEY_PREVIOUS_LINE:
      if (menuVerticalOffset == 0)
        break;
      else
        menuVerticalOffset--;
      storageReadTextFile(reusableBuffer.viewText.filename, reusableBuffer.viewText.lines, reusableBuffer.viewText.linesCount);
      break;

    case EVT_KEY_NEXT_LINE:
      if (menuVerticalOffset+LCD_LINES-1 >= reusableBuffer.viewText.linesCount)
        break;
      else
        ++menuVerticalOffset;
      storageReadTextFile(reusableBuffer.viewText.filename, reusableBuffer.viewText.lines, reusableBuffer.viewText.linesCount);
      break;

    case EVT_KEY_BREAK(KEY_EXIT):
      popMenu();
      break;
  }

  for (int i=0; i<LCD_LINES-1; i++) {
    lcdDrawText(0, i*FH+FH+1, reusableBuffer.viewText.lines[i], FIXEDWIDTH);
  }

  char * title = reusableBuffer.viewText.filename;
#if defined(SIMU)
  if (!strncmp(title, "./", 2)) title += 2;
#else
  // TODO?
#endif
  lcdDrawText(LCD_W/2, 0, getBasename(title), CENTERED);
  lcdInvertLine(0);

  if (reusableBuffer.viewText.linesCount > LCD_LINES-1) {
    drawVerticalScrollbar(LCD_W-1, FH, LCD_H-FH, menuVerticalOffset, reusableBuffer.viewText.linesCount, LCD_LINES-1);
  }
}

void pushMenuTextView(const char *filename)
{
  if (strlen(filename) < TEXT_FILENAME_MAXLEN) {
    strcpy(reusableBuffer.viewText.filename, filename);
    pushMenu(menuTextView);
  }
}

#undef EVT_KEY_NEXT_LINE
#undef EVT_KEY_PREVIOUS_LINE
