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
  case EVT_ROTARY_RIGHT:       \
  case EVT_KEY_BREAK(KEY_PGDN)
//  case EVT_KEY_BREAK(KEY_DOWN)

#define CASE_EVT_KEY_PREVIOUS_LINE \
  case EVT_ROTARY_LEFT:            \
  case EVT_KEY_BREAK(KEY_PGUP):    \
  case EVT_KEY_BREAK(KEY_UP)

constexpr char NON_CHECKABLE_PREFIX = '=';

class CheckBoxStatic : public Window {
  public:
    CheckBoxStatic(Window * parent, const rect_t & rect, std::function<bool()> getChecked, 
                   std::function<bool()> getFocus, std::function<bool()> getVisible, WindowFlags flags = 0) :
      Window(parent, rect, flags, 0),
      _getChecked(getChecked),
      _getFocus(getFocus),
      _getVisible(getVisible)
    {
      coord_t size = min(rect.w, rect.h);
      setWidth(size);
      setHeight(size);
      _checked = _getChecked();
      _focus = _getFocus();
      _visible = _getVisible();
    }

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "CheckBoxStatic";
    }
#endif

    void paint(BitmapBuffer * dc)
    {
      if(_visible)
        theme->drawCheckBox(dc, _checked, 0, FIELD_PADDING_TOP, _focus);
    }

    void checkEvents() override
    {
      Window::checkEvents();
      bool checked = _getChecked();
      bool focus = _getFocus();
      bool visible = _getVisible();
      if (checked != _checked || focus != _focus || visible != _visible) {
        _checked = checked;
        _focus = focus;
        _visible = visible;
        invalidate();
      }
    }

  protected:
    std::function<bool()> _getChecked;
    std::function<bool()> _getFocus;
    std::function<bool()> _getVisible;
    bool _checked;
    bool _focus;
    bool _visible;
};

void ViewTextWindow::extractNameSansExt()
{
  uint8_t nameLength;
  uint8_t extLength;

  const char *ext =
      getFileExtension(name.data(), 0, 0, &nameLength, &extLength);
  extension = std::string(ext);
  if (nameLength > TEXT_FILENAME_MAXLEN) nameLength = TEXT_FILENAME_MAXLEN;

  nameLength -= extLength;
  name.substr(nameLength);
}

void ViewTextWindow::buildBody(Window *window)
{
  FormGridLayout grid(window->width());
  grid.spacer();
  grid.setLabelWidth(PAGE_LINE_HEIGHT + 3UL * PAGE_LINE_SPACING);  // width of "first column" for checkboxes
  int i;

  // assume average characte is 10 pixels wide, round the string length to tens.
  // Font is not fixed width, so this is for the worst case...
  maxLineLength = static_cast<int>(window->width() / 10 / 10) * 10 - 2;
  maxScreenLines = window->height() / (PAGE_LINE_HEIGHT + PAGE_LINE_SPACING);
  window->setFocus();
  readLinesCount = 0;
  lastLoadedLine = 0;

  lines = new char *[maxScreenLines];
  for (i = 0; i < maxScreenLines; i++) {
    lines[i] = new char[maxLineLength + 1];
    memclear(lines[i], maxLineLength + 1);
  }

  longestLine = 0;
  loadFirstScreen();

  if (isInSetup == true) {
    textBottom = false;
    while (!textBottom) {
      sdReadTextFileBlock(fullPath.c_str(), readLinesCount);
      textVerticalOffset += 10;
    }
    maxPos =
        (maxLines - maxScreenLines) * (PAGE_LINE_HEIGHT + PAGE_LINE_SPACING);
    if (maxPos < body.getRect().h) maxPos = body.getRect().h;
  }

  isInSetup = false;
  loadFirstScreen();

  for (i = 0; i < maxScreenLines; i++) {
    if (g_model.checklistInteractive && !fromMenu) {
      new CheckBoxStatic(window, grid.getLabelSlot(), 
                         [=]() { return i < checklistPosition-(int)textVerticalOffset;}, 
                         [=]() { return i == checklistPosition-(int)textVerticalOffset;},
                         [=]() { return lines[i][0] && lines[i][0]!=NON_CHECKABLE_PREFIX;});

      new DynamicText(window, grid.getFieldSlot(), [=]() {
        std::string str = 
            (lines[i][0]) ? std::string(lines[i]).substr(lines[i][0]==NON_CHECKABLE_PREFIX ? 1 : 0, std::string::npos) : std::string(" ");
        return std::string(str);
      });
    }
    else {
      new DynamicText(window, grid.getSlot(), [=]() {
        std::string str = (lines[i][0]) ? std::string(lines[i]) : std::string(" ");
        return std::string(str);
      });
    }
    grid.nextLine();
  }
}

#if defined(HARDWARE_TOUCH)
bool ViewTextWindow::onTouchSlide(coord_t x, coord_t y, coord_t startX,
                                  coord_t startY, coord_t slideX,
                                  coord_t slideY)
{
  if (&body == Window::focusWindow) {
    const int step = PAGE_LINE_HEIGHT + PAGE_LINE_SPACING;
    int deltaY = -slideY;
    int lineStep = deltaY / step;

    textVerticalOffset += lineStep;
    if (textVerticalOffset < 0) textVerticalOffset = 0;

    //  if (textBottom && lineStep > 0) textVerticalOffset -= lineStep;
    if (textVerticalOffset > maxLines) textVerticalOffset = maxLines;
    sdReadTextFileBlock(fullPath.c_str(), readLinesCount);
  }
  return Page::onTouchSlide(x, y, startX, startY, slideX, slideY);
}
#endif

void ViewTextWindow::checkEvents()
{
  if (&body == Window::focusWindow) {
    const int step = PAGE_LINE_HEIGHT + PAGE_LINE_SPACING;
    coord_t deltaY;
    event_t event = getWindowEvent();

#if defined(ROTARY_ENCODER_NAVIGATION)
    if (event == EVT_ROTARY_LEFT || event == EVT_ROTARY_RIGHT) {
      deltaY = ROTARY_ENCODER_SPEED() * step;
    } else {
      deltaY = step;
    }
#else
    deltaY = step;
#endif

    int lineStep = deltaY / step;
    if (lineStep > (maxScreenLines >> 1)) lineStep = maxScreenLines >> 1;

    switch (event) {
    case EVT_KEY_BREAK(KEY_ENTER):
      if (g_model.checklistInteractive && !fromMenu) {
        if (checklistPosition < readLinesCount) {
          ++checklistPosition;
          if (checklistPosition-(int)textVerticalOffset >= maxScreenLines-1 && textVerticalOffset + maxScreenLines < readLinesCount) {
            ++textVerticalOffset;
            sdReadTextFileBlock(fullPath.c_str(), readLinesCount);
          }
        }
        else {
          Page::onEvent(EVT_KEY_BREAK(KEY_EXIT));
        }
      }
      break;

    CASE_EVT_KEY_NEXT_LINE:
      if(textVerticalOffset + maxScreenLines >= readLinesCount)
        break;
      else {
        textVerticalOffset += lineStep;
        if (textVerticalOffset > maxLines) textVerticalOffset = maxLines;
      }
      sdReadTextFileBlock(fullPath.c_str(), readLinesCount);
      break;

    CASE_EVT_KEY_PREVIOUS_LINE:
      if (textVerticalOffset == 0)
        break;
      else {
        textVerticalOffset -= lineStep;
        if (textVerticalOffset < 0) textVerticalOffset = 0;
      }

      sdReadTextFileBlock(fullPath.c_str(), readLinesCount);
      break;
    
    case EVT_KEY_FIRST(KEY_EXIT):
    case EVT_KEY_LONG(KEY_EXIT):
    case EVT_KEY_REPT(KEY_EXIT):
    case EVT_KEY_BREAK(KEY_EXIT):
      if (!(g_model.checklistInteractive && !fromMenu)) {
        Page::onEvent(event);
      }
      break;

    default:
      Page::onEvent(event);
      break;
    }
  }
  Page::checkEvents();
}

void ViewTextWindow::loadFirstScreen()
{
  textVerticalOffset = 0;
  readLinesCount = 0;
  sdReadTextFileBlock(fullPath.c_str(), readLinesCount);
}

void ViewTextWindow::sdReadTextFileBlock(const char *filename, int &lines_count)
{
  FIL file;
  int result;
  char c;
  unsigned int sz = 0;
  int line_length = 0;
  uint8_t escape = 0;
  char escape_chars[4] = {0};
  int current_line = 0;
  textBottom = false;

  for (int i = 0; i < maxScreenLines; i++) {
    memclear(lines[i], maxLineLength + 1);
  }

  result = f_open(&file, (TCHAR *)filename, FA_OPEN_EXISTING | FA_READ);
  if (result == FR_OK) {
    for (int i = 0; i < TEXT_FILE_MAXSIZE &&
                    f_read(&file, &c, 1, &sz) == FR_OK && sz == 1 &&
                    (lines_count == 0 ||
                     current_line - textVerticalOffset < maxScreenLines);
         i++) {
      if (c == '\n' || line_length >= maxLineLength) {
        ++current_line;
        line_length = 0;
        escape = 0;
      }
      if (c != '\r' && c != '\n' && current_line >= textVerticalOffset &&
          current_line - textVerticalOffset < maxScreenLines &&
          line_length < maxLineLength) {
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

        lines[current_line - textVerticalOffset][line_length++] = c;
        if (longestLine < line_length) longestLine = line_length;
      } else if (current_line < textVerticalOffset) {
        ++line_length;
      }
    }
    if (c != '\n') {
      ++current_line;
    }

    if (f_eof(&file)) {
      textBottom = true;
      if (isInSetup) maxLines = current_line;
    }

    f_close(&file);
  }

  if (lastLoadedLine < textVerticalOffset) lastLoadedLine = textVerticalOffset;

  if (lines_count == 0) {
    lines_count = current_line;
  }
}

void ViewTextWindow::drawVerticalScrollbar(BitmapBuffer *dc)
{
  int readPos = textVerticalOffset * (PAGE_LINE_HEIGHT + PAGE_LINE_SPACING);

  if (readPos < header.getRect().h << 1) readPos = header.getRect().h << 1;

  coord_t yofs = divRoundClosest(body.getRect().h * readPos, maxPos);
  coord_t yhgt = divRoundClosest(body.getRect().h * body.getRect().h, maxPos);
  if (yhgt < 15) yhgt = 15;
  if (yhgt + yofs > maxPos) yhgt = maxPos - yofs;
  dc->drawSolidFilledRect(body.getRect().w - SCROLLBAR_WIDTH, yofs,
                          SCROLLBAR_WIDTH, yhgt, COLOR_THEME_PRIMARY3);
}

#include "datastructs.h"

static void replaceSpaceWithUnderscore(std::string &name)
{
  size_t index;
  do {
    index = name.find(' ');
    if (index != std::string::npos) name[index] = '_';
  } while (index != std::string::npos);
}

#if defined(SDCARD_YAML)
#define MODEL_FILE_EXT YAML_EXT
#else
#define MODEL_FILE_EXT MODELS_EXT
#endif

bool openNotes(const char buf[], std::string modelNotesName, bool fromMenu = false)
{
  if (isFileAvailable(modelNotesName.c_str())) { 
    new ViewTextWindow(std::string(buf), modelNotesName, ICON_MODEL, fromMenu);  
    return true;
  } else {
    return false;
  }
}

void readModelNotes(bool fromMenu)
{
  bool notesFound = false;
  LED_ERROR_BEGIN();

  std::string modelNotesName(g_model.header.name);
  modelNotesName.append(TEXT_EXT);
  const char buf[] = {MODELS_PATH};
  f_chdir((TCHAR *)buf);

  notesFound = openNotes(buf, modelNotesName, fromMenu);
  if (!notesFound) {
    replaceSpaceWithUnderscore(modelNotesName);
    notesFound = openNotes(buf, modelNotesName, fromMenu);
  }

#if !defined(EEPROM)
  if (!notesFound) {
    modelNotesName.assign(g_eeGeneral.currModelFilename);
    size_t index = modelNotesName.find(MODEL_FILE_EXT);
    if (index != std::string::npos) {
      modelNotesName.erase(index);
      modelNotesName.append(TEXT_EXT);
      notesFound = openNotes(buf, modelNotesName, fromMenu);
    }
    if (!notesFound) {
      replaceSpaceWithUnderscore(modelNotesName);
      notesFound = openNotes(buf, modelNotesName, fromMenu);
    }
  }
#endif

  LED_ERROR_END();
}