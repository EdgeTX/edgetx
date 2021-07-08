/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
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

#include "view_text.h"

#include "gridlayout.h"
#include "menu.h"
#include "opentx.h"
#include "sdcard.h"

#define CASE_EVT_KEY_NEXT_LINE \
  case EVT_ROTARY_RIGHT: \
  case EVT_KEY_BREAK(KEY_PGDN)
//  case EVT_KEY_BREAK(KEY_DOWN)

#define CASE_EVT_KEY_PREVIOUS_LINE \
  case EVT_ROTARY_LEFT: \
  case EVT_KEY_BREAK(KEY_PGUP): \
  case EVT_KEY_BREAK(KEY_UP)

#define CASE_EVT_START \
  case EVT_ENTRY: \
  case EVT_KEY_BREAK(KEY_ENTER): \
  case EVT_KEY_BREAK(KEY_TELEM)

void ViewTextWindow::extractNameSansExt()
{
  uint8_t nameLength;
  uint8_t extLength;

  const char *ext =
      getFileExtension(name.data(), 0, 0, &nameLength, &extLength);
  extension = std::string(ext);
  if (nameLength > TEXT_FILENAME_MAXLEN) nameLength = TEXT_FILENAME_MAXLEN;

  memset(reusableBuffer.viewText.filename, 0, TEXT_FILENAME_MAXLEN);

  strncpy(reusableBuffer.viewText.filename, name.c_str(), nameLength);
  reusableBuffer.viewText.filename[nameLength] = '\0';

  nameLength -= extLength;
  name = (std::string(reusableBuffer.viewText.filename)).substr(0, nameLength);
}


void ViewTextWindow::buildBody(Window *window)
{
  GridLayout grid(window);
  grid.spacer();
  int i;
  const int numLines = (LCD_H - PAGE_TITLE_TOP) / PAGE_LINE_HEIGHT - 1;
  const int dispLines = min(numLines, (int)NUM_BODY_LINES);
  // assume average characte is 10 pixels wide, round the string length to tens. 
  // Font is not fixed width, so this is for the worst case...
  const int maxLineLength = int(floor(window->width() / 10 / 10)) * 10 -2;
  //const int maxLineLength = min(120, int(LCD_COLS * TEXT_VIEWER_LINES - 10));
  window->setFocus();
  
  for (i = 0; i < dispLines; i++) {
    memclear(&reusableBuffer.viewText.lines[i],
             sizeof(reusableBuffer.viewText.lines[i]));
  }

  readCount = 0;
  longestLine = 0;
  lastLine = false;
  for (i = 0; i < TEXT_FILE_MAXSIZE && !lastLine; i++) {
    lastLine =
        sdReadTextLine(reusableBuffer.viewText.filename,
                       reusableBuffer.viewText.lines[0], maxLineLength);

    new StaticText(window, grid.getSlot(), reusableBuffer.viewText.lines[0]);
    grid.nextLine();
  }

  window->setInnerWidth( (longestLine + 4) * 10);

  window->setInnerHeight(grid.getWindowHeight());
}

void ViewTextWindow::checkEvents()
{
  if (&body == Window::focusWindow) 
  {  
    
    const int step = PAGE_LINE_HEIGHT + PAGE_LINE_SPACING;
    coord_t currentPos = body.getScrollPositionY();
    coord_t deltaY = step;
    event_t event = getWindowEvent();    
    
    if(event == EVT_ROTARY_LEFT || event == EVT_ROTARY_RIGHT) {
        deltaY = ROTARY_ENCODER_SPEED() * step;
    }

    switch (event) {
    CASE_EVT_KEY_NEXT_LINE:
      currentPos += deltaY;
      break;

    CASE_EVT_KEY_PREVIOUS_LINE:
      currentPos -= deltaY;
      break;

    default:
      Page::onEvent(event);
      return;        
    }
    body.setScrollPositionY(currentPos);
  }
  Page::checkEvents();
}

bool ViewTextWindow::sdReadTextLine(const char *filename, char line[],
                                    const uint8_t maxLineLength)
{
  FIL file;
  int result;
  char c;
  unsigned int sz;
  uint8_t line_length = 0;
  uint8_t escape = 0;
  char escape_chars[4] = {0};
  int current_line = 0;

  memclear(line, maxLineLength);
  line[line_length++] = 0x20;

  result = f_open(&file, (TCHAR *)filename, FA_OPEN_EXISTING | FA_READ);
  if (result != FR_OK) {
    return true;
  } else {
    result = f_lseek(&file, readCount);
    if (result != FR_OK) {
      return true;
    }

    for (uint8_t i = 0; i < maxLineLength && readCount < (int)TEXT_FILE_MAXSIZE;
         ++i) {
      if ( (f_read(&file, &c, 1, &sz) != FR_OK || !sz) && line_length < maxLineLength) {
        f_close(&file);
        return true;
      }
      readCount++;

      if (c == '\n') {
        ++current_line;
        // line_length = 0;
        escape = 0;
        f_close(&file);
        return false;
      } else if (c != '\r' ) {
        if (c == '\\' && escape == 0) {
          escape = 1;
          continue;
        } else if (c != '\\' && escape > 0 && escape < sizeof(escape_chars)) {
          escape_chars[escape - 1] = c;
          if (escape == 2 && !strncmp(escape_chars, "up", 2)) {
            c = CHAR_UP;
          } else if (escape == 2 && !strncmp(escape_chars, "dn", 2)) {
            c = CHAR_DOWN;
          } else if (escape == 3) {
            int val = atoi(escape_chars);
            if (val >= 200 && val < 225) {
              c = '\200' + val - 200;
            }
          } else {
            escape++;
            continue;
          }
        } else if (c == '~') {
          c = 'z' + 1;
        } else if (c == '\t') {
          c = 0x1D;  // tab
        }
        escape = 0;
        line[line_length++] = c;
        if(longestLine < line_length) longestLine = line_length;
      }
    }
    if (c != '\n') {
      current_line += 1;
      f_close(&file);
      return false;
    }
    f_close(&file);
  }

  return false;
}

#include "../../storage/eeprom_common.h"
#include "datastructs.h"

void readModelNotes()
{
  LED_ERROR_BEGIN();

  std::string modelNotesName(g_model.header.name);
  modelNotesName.append(TEXT_EXT);
  const char buf[] = {MODELS_PATH};
  f_chdir((TCHAR*)buf);
  new ViewTextWindow(std::string(buf), modelNotesName, ICON_MODEL);

  LED_ERROR_END();
}