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

#if defined(CRSF_CONFIG_MENU)

#include "telemetry/crsf_device_config.h"

extern uint8_t g_moduleIdx;

#define CRSF_CFG_LINES        (LCD_LINES - 1)
#define CRSF_CFG_COL2         (LCD_W / 2)
#define CRSF_CFG_VAL_LEN      24

#define CRSF_BIND_FOLDER      0xFE   // synthetic ExpressLRS binding view
#define CRSF_ENTRY_BIND       0x200  // synthetic root entry opening it

// rows of the binding view, top to bottom
enum {
  BIND_ROW_TX = 0,  // classic bind / stop
  BIND_ROW_RX,      // put the connected receiver in bind mode
  // ELRS >= 4.1 only: bind phrase / UID entry
  BIND_ROW_PHRASE,
  BIND_ROW_UID1,    // 6 UID byte rows
  BIND_ROW_APPLY_TO = BIND_ROW_UID1 + 6,
  BIND_ROW_APPLY,
  BIND_ROW_COUNT
};

// menu.editId markers of the binding view
#define BIND_EDIT_UID         1
#define BIND_EDIT_PHRASE      2

static CrsfMenuState& menu = reusableBuffer.crsfConfig;

// ELRS 4.1 renamed some fields with an appended state ("Band/Enabled"):
// display them without the suffix
static uint8_t displayNameLen(const char* name)
{
  uint8_t len = strlen(name);
  if (len > 8 && strcasecmp(name + len - 8, "/Enabled") == 0) len -= 8;
  return len;
}

// visible entries and the value column, rebuilt only when the fields
// or the folder change: the parse work is per field, not per refresh
static void buildEntries(uint16_t generation, uint8_t loaded)
{
  if (menu.entriesBuilt && generation == menu.builtGeneration &&
      loaded == menu.builtLoaded && menu.folder == menu.builtFolder)
    return;
  menu.entriesBuilt = true;
  menu.builtGeneration = generation;
  menu.builtLoaded = loaded;
  menu.builtFolder = menu.folder;

  uint16_t n = 0;

  // other devices are listed at the top of the root folder
  if (menu.folder == 0 && crsfConfigDeviceCount() > 1) {
    auto current = crsfConfigCurrentDevice();
    for (uint8_t i = 0;
         i < crsfConfigDeviceCount() && n < CRSF_CFG_MAX_ENTRIES; i++) {
      auto dev = crsfConfigGetDevice(i);
      if (dev && (!current || dev->address != current->address))
        menu.entries[n++] = 0x100 | dev->address;
    }
  }

  // the ELRS TX module's own "Bind" command is replaced, in place,
  // by the full native bind view
  menu.bindFieldId = crsfConfigElrsBindFieldId();

  // values align to one column: one space past the longest field name
  // of this folder - names are never clipped
  menu.valCol = 0;

  uint8_t count = crsfConfigFieldCount();
  for (uint8_t id = 1; id <= count && n < CRSF_CFG_MAX_ENTRIES; id++) {
    CrsfConfigField f;
    if (!crsfConfigGetField(id, f) || !f.loaded) continue;
    if (f.hidden || f.parent != menu.folder) continue;
    if (f.type == CRSF_FIELD_OUT_OF_RANGE) continue;
    menu.entries[n++] = (id == menu.bindFieldId) ? CRSF_ENTRY_BIND : id;

    if (f.type == CRSF_FIELD_COMMAND || f.type == CRSF_FIELD_FOLDER) continue;
    if (id == menu.bindFieldId || crsfConfigElrsIsVersionField(f)) continue;
    coord_t w = getTextWidth(f.name, displayNameLen(f.name)) + FW;
    if (w > menu.valCol) menu.valCol = w;
  }
  menu.nEntries = n;
}

static void formatFieldValue(const CrsfConfigField& f, bool editingThis,
                             char* buf, uint8_t size)
{
  buf[0] = '\0';
  int32_t value = editingThis ? menu.editValue : f.value;
  switch (f.type) {
    case CRSF_FIELD_UINT8:
    case CRSF_FIELD_INT8:
    case CRSF_FIELD_UINT16:
    case CRSF_FIELD_INT16:
      crsfConfigFormatNumber(value, 0, f.unit, buf, size);
      break;

    case CRSF_FIELD_FLOAT:
      crsfConfigFormatNumber(value, f.prec, f.unit, buf, size);
      break;

    case CRSF_FIELD_TEXT_SELECTION: {
      crsfConfigGetOption(f.options, value, buf, size);
      // e.g. ELRS "Max Power" options are bare numbers with a mW unit
      uint8_t used = strlen(buf);
      if (f.unit[0] && used + 1 < size)
        strAppend(buf + used, f.unit, size - used - 1);
      break;
    }

    case CRSF_FIELD_STRING:
    case CRSF_FIELD_INFO:
      snprintf(buf, size, "%s", f.svalue);
      break;

    // FOLDER and COMMAND rows are the name alone
  }
}

static bool fieldEditable(const CrsfConfigField& f)
{
  switch (f.type) {
    case CRSF_FIELD_UINT8:
    case CRSF_FIELD_INT8:
    case CRSF_FIELD_UINT16:
    case CRSF_FIELD_INT16:
    case CRSF_FIELD_FLOAT:
      return f.min < f.max;
    case CRSF_FIELD_TEXT_SELECTION: {
      // needs at least 2 selectable (non empty) options
      uint8_t selectable = 0;
      uint8_t count = crsfConfigOptionCount(f.options);
      char tmp[2];
      for (uint8_t i = 0; i < count && selectable < 2; i++) {
        if (crsfConfigGetOption(f.options, i, tmp, sizeof(tmp)) && tmp[0])
          selectable++;
      }
      return selectable >= 2;
    }
    default:
      return false;
  }
}

static void adjustEditValue(const CrsfConfigField& f, int8_t dir)
{
  if (f.type == CRSF_FIELD_TEXT_SELECTION) {
    menu.editValue = crsfConfigNextAllowedOption(f.id, menu.editValue, dir);
  } else {
    int32_t step = (f.type == CRSF_FIELD_FLOAT && f.step > 0) ? f.step : 1;
    menu.editValue = limit<int32_t>(f.min, menu.editValue + dir * step, f.max);
  }
}

static void enterFolder(uint8_t folderId)
{
  if (menu.folderDepth < CRSF_CFG_FOLDER_DEPTH) {
    menu.folderStack[menu.folderDepth] = menu.folder;
    menu.selStack[menu.folderDepth++] = menu.selected;
    menu.folder = folderId;
    menu.selected = 0;
    menu.scrollOfs = 0;
    menu.editId = 0;
    menu.selTime = get_tmr10ms();
    if (folderId != CRSF_BIND_FOLDER) crsfConfigReloadFolder(folderId);
  }
}

static bool leaveFolder()
{
  if (menu.folderDepth == 0) return false;
  menu.folder = menu.folderStack[--menu.folderDepth];
  // return to the line the folder was entered from
  menu.selected = menu.selStack[menu.folderDepth];
  menu.scrollOfs = 0;
  menu.editId = 0;
  menu.selTime = get_tmr10ms();
  crsfConfigReloadFolder(menu.folder);
  // back at the root: rescan the bus for devices
  if (menu.folder == 0) crsfConfigRefreshDevices();
  return true;
}

static void setInfo(const char* msg)
{
  strAppend(menu.info, msg, sizeof(menu.info) - 1);
  menu.infoUntil = get_tmr10ms() + 300;  // 3s
}

// getTextWidth() returns uint8_t and would wrap on a long bind phrase;
// only "is it wider than the column" is needed anyway
static bool textWiderThan(const char* s, coord_t availPx)
{
  coord_t width = 0;
  for (const char* p = s; *p; p++) {
    width += getTextWidth(p, 1);
    if (width > availPx) return true;
  }
  return false;
}

// slow marquee for text wider than the available pixels: holds ~1.2s,
// then scrolls one character every 0.6s, briefly holding the end; the
// cycle restarts whenever the selection moves
static const char* scrollText(const char* s, coord_t availPx)
{
  if (!textWiderThan(s, availPx)) return s;
  // shift until the remainder fits
  uint8_t len = strlen(s);
  uint8_t span = 1;
  while (span < len && textWiderThan(s + span, availPx)) span++;
  uint8_t raw = ((tmr10ms_t)(get_tmr10ms() - menu.selTime) / 60) % (span + 3);
  uint8_t ofs = raw < 2 ? 0 : min<uint8_t>(raw - 2, span);
  return s + ofs;
}

static const CrsfConfigDevice* findModuleDevice()
{
  for (uint8_t i = 0; i < crsfConfigDeviceCount(); i++) {
    auto d = crsfConfigGetDevice(i);
    if (d && d->address == MODULE_ADDRESS) return d;
  }
  return nullptr;
}

static void drawCommandPopup()
{
  CrsfConfigField f;
  crsfConfigGetField(crsfConfigCommandField(), f);

  constexpr coord_t x = 4;
  constexpr coord_t w = LCD_W - 8;
  constexpr coord_t y = 12;
  constexpr coord_t h = LCD_H - 24;
  lcdDrawFilledRect(x, y, w, h, SOLID, ERASE);
  lcdDrawRect(x, y, w, h);

  lcdDrawText(x + 3, y + 3, f.loaded ? f.name : "...", BOLD);
  const char* info =
      (f.loaded && f.svalue[0]) ? f.svalue : STR_WAITING_FOR_MODULE;
  lcdDrawText(x + 3, y + 3 + FH, info);

  lcdDrawText(x + 3, y + h - FH - 2, STR_POPUPS_ENTER_EXIT, SMLSIZE);
}

// bind phrase entry: the character set and key semantics of editName()
// (rotate = change, ENTER = next, long ENTER = case toggle, or done on a
// space), but the value is transient and longer than the screen, so the
// visible part slides along with the cursor instead
static const char bindPhraseChars[] = " abcdefghijklmnopqrstuvwxyz0123456789_-,.";

static void setPhraseChar(uint8_t pos, char c)
{
  if (menu.phrase[pos] == '\0') menu.phrase[pos + 1] = '\0';
  menu.phrase[pos] = c;
}

static void cyclePhraseChar(int8_t dir)
{
  char c = menu.phrase[menu.phraseCursor];
  if (!c) c = ' ';
  bool caps = (c >= 'A' && c <= 'Z');
  if (caps) c += 'a' - 'A';
  int8_t idx = 0;
  for (uint8_t i = 0; bindPhraseChars[i]; i++) {
    if (bindPhraseChars[i] == c) {
      idx = i;
      break;
    }
  }
  idx = limit<int8_t>(0, idx + dir, DIM(bindPhraseChars) - 2);
  c = bindPhraseChars[idx];
  if (caps && c >= 'a' && c <= 'z') c += 'A' - 'a';
  setPhraseChar(menu.phraseCursor, c);
}

static void finishPhraseEdit()
{
  uint8_t len = strlen(menu.phrase);
  while (len > 0 && menu.phrase[len - 1] == ' ') menu.phrase[--len] = '\0';
  menu.editId = 0;
}

// synthetic ExpressLRS binding view: bind mode + phrase / UID entry
static void handleBindView(event_t event)
{
  bool canSet = crsfConfigElrsCanSetBindInfo(findModuleDevice());
  bool loopActive = crsfConfigBindLoopActive();

  if (loopActive != menu.loopWasActive) {
    menu.loopWasActive = loopActive;
    if (!loopActive && crsfConfigBindLoopBound()) setInfo(STR_CONNECTED);
  }

  uint8_t rows = canSet ? BIND_ROW_COUNT : 2;
  if (menu.selected >= rows) {
    menu.selected = rows - 1;
    menu.editId = 0;
  }

  if (canSet && !menu.uidPrefilled && crsfConfigGetUid(menu.uidEdit))
    menu.uidPrefilled = true;

  bool editing = (menu.editId != 0);

  if (editing && menu.editId == BIND_EDIT_PHRASE) {
    switch (event) {
      case EVT_ROTARY_RIGHT:
      case EVT_KEY_BREAK(KEY_UP):
      case EVT_KEY_REPT(KEY_UP):
      case EVT_KEY_BREAK(KEY_PLUS):
      case EVT_KEY_REPT(KEY_PLUS):
        cyclePhraseChar(1);
        break;

      case EVT_ROTARY_LEFT:
      case EVT_KEY_BREAK(KEY_DOWN):
      case EVT_KEY_REPT(KEY_DOWN):
      case EVT_KEY_BREAK(KEY_MINUS):
      case EVT_KEY_REPT(KEY_MINUS):
        cyclePhraseChar(-1);
        break;

      case EVT_KEY_BREAK(KEY_ENTER):
        if (menu.phraseCursor < CRSF_BIND_PHRASE_MAXLEN - 1) {
          if (!menu.phrase[menu.phraseCursor])
            setPhraseChar(menu.phraseCursor, ' ');
          menu.phraseCursor++;
        } else {
          finishPhraseEdit();
        }
        killEvents(event);
        break;

      case EVT_KEY_BREAK(KEY_SHIFT):
      case EVT_KEY_LONG(KEY_ENTER): {
        killEvents(event);
        char c = menu.phrase[menu.phraseCursor];
        if (c >= 'A' && c <= 'Z')
          setPhraseChar(menu.phraseCursor, c + 'a' - 'A');
        else if (c >= 'a' && c <= 'z')
          setPhraseChar(menu.phraseCursor, c + 'A' - 'a');
        else if (event == EVT_KEY_LONG(KEY_ENTER) && (c == ' ' || c == '\0'))
          finishPhraseEdit();
        break;
      }

      case EVT_KEY_BREAK(KEY_EXIT):
        finishPhraseEdit();
        killEvents(event);
        break;
    }
  } else if (editing) {
    // value editing follows the usual editor orientation:
    // rotating right / plus increments
    switch (event) {
      case EVT_ROTARY_RIGHT:
      case EVT_KEY_BREAK(KEY_UP):
      case EVT_KEY_REPT(KEY_UP):
      case EVT_KEY_BREAK(KEY_PLUS):
      case EVT_KEY_REPT(KEY_PLUS):
        menu.editValue = limit<int32_t>(0, menu.editValue + 1, 255);
        break;

      case EVT_ROTARY_LEFT:
      case EVT_KEY_BREAK(KEY_DOWN):
      case EVT_KEY_REPT(KEY_DOWN):
      case EVT_KEY_BREAK(KEY_MINUS):
      case EVT_KEY_REPT(KEY_MINUS):
        menu.editValue = limit<int32_t>(0, menu.editValue - 1, 255);
        break;

      case EVT_KEY_BREAK(KEY_ENTER):
        menu.uidEdit[menu.selected - BIND_ROW_UID1] = menu.editValue;
        menu.editId = 0;
        killEvents(event);
        break;

      case EVT_KEY_BREAK(KEY_EXIT):
        menu.editId = 0;
        killEvents(event);
        break;
    }
  } else {
    switch (event) {
      case EVT_ROTARY_LEFT:
      case EVT_KEY_BREAK(KEY_UP):
      case EVT_KEY_REPT(KEY_UP):
      case EVT_KEY_BREAK(KEY_PLUS):
      case EVT_KEY_REPT(KEY_PLUS):
        if (menu.selected > 0)
          menu.selected--;
        else
          menu.selected = rows - 1;
        menu.selTime = get_tmr10ms();
        break;

      case EVT_ROTARY_RIGHT:
      case EVT_KEY_BREAK(KEY_DOWN):
      case EVT_KEY_REPT(KEY_DOWN):
      case EVT_KEY_BREAK(KEY_MINUS):
      case EVT_KEY_REPT(KEY_MINUS):
        menu.selected = (menu.selected + 1) % rows;
        menu.selTime = get_tmr10ms();
        break;

      case EVT_KEY_BREAK(KEY_ENTER):
        if (menu.selected == BIND_ROW_TX) {
          if (loopActive) {
            crsfConfigBindLoopStop();
            menu.info[0] = '\0';
          } else {
            crsfConfigBindLoopStart();
            setInfo(STR_MODULE_BINDING);
          }
        } else if (menu.selected == BIND_ROW_RX) {
          if (crsfConfigSendBindCommand(RECEIVER_ADDRESS))
            setInfo(STR_WAITING_FOR_RX);
        } else if (menu.selected == BIND_ROW_PHRASE) {
          menu.editId = BIND_EDIT_PHRASE;
          menu.phraseCursor = 0;
        } else if (menu.selected == BIND_ROW_APPLY_TO) {
          menu.applyMode = !menu.applyMode;
        } else if (menu.selected == BIND_ROW_APPLY) {
          // an entered phrase wins; otherwise the UID digits are sent
          char uidText[24];
          const char* text = menu.phrase;
          if (!menu.phrase[0]) {
            snprintf(uidText, sizeof(uidText), "%u,%u,%u,%u,%u,%u",
                     menu.uidEdit[0], menu.uidEdit[1], menu.uidEdit[2],
                     menu.uidEdit[3], menu.uidEdit[4], menu.uidEdit[5]);
            text = uidText;
          }
          if (crsfConfigApplyBindInfo(text, menu.applyMode == 1)) {
            setInfo(menu.applyMode == 1 ? STR_WAITING_FOR_RX
                                        : STR_CRSF_REBOOTING);
            menu.uidPrefilled = false;  // refill from the re-read UID
          }
        } else {
          menu.editId = BIND_EDIT_UID;
          menu.editValue = menu.uidEdit[menu.selected - BIND_ROW_UID1];
        }
        killEvents(event);
        break;

      case EVT_KEY_BREAK(KEY_EXIT):
        if (loopActive) {
          // first press only stops the binding loop
          crsfConfigBindLoopStop();
          menu.info[0] = '\0';
        } else {
          leaveFolder();
        }
        killEvents(event);
        break;
    }
  }

  editing = (menu.editId != 0);

  // scrolling
  if (menu.selected < menu.scrollOfs)
    menu.scrollOfs = menu.selected;
  else if (menu.selected >= menu.scrollOfs + CRSF_CFG_LINES)
    menu.scrollOfs = menu.selected - CRSF_CFG_LINES + 1;

  // header
  lcdDrawTextAlignedLeft(0, STR_MODULE_BINDING);
  if (menu.info[0] &&
      (tmr10ms_t)(menu.infoUntil - get_tmr10ms()) < 0x8000) {
    lcdDrawText(LCD_W - 1, 0, menu.info, RIGHT);
  } else {
    menu.info[0] = '\0';
    uint8_t uid[6];
    if (canSet && crsfConfigGetUid(uid)) {
      char buf[26];
      snprintf(buf, sizeof(buf), "%u.%u.%u.%u.%u.%u", uid[0], uid[1], uid[2],
               uid[3], uid[4], uid[5]);
      lcdDrawText(LCD_W - 1, 0, buf, RIGHT);
    }
  }
  lcdInvertLine(0);

  // body
  for (uint8_t line = 0; line < CRSF_CFG_LINES; line++) {
    uint8_t idx = menu.scrollOfs + line;
    if (idx >= rows) break;
    coord_t y = MENU_HEADER_HEIGHT + 1 + line * FH;
    bool sel = (idx == menu.selected);

    if (idx <= BIND_ROW_RX) {
      char buf[24];
      if (idx == BIND_ROW_TX && loopActive)
        snprintf(buf, sizeof(buf), "[%s]", STR_STOP);
      else if (idx == BIND_ROW_TX)
        snprintf(buf, sizeof(buf), "%s", STR_MODULE_BIND);
      else
        snprintf(buf, sizeof(buf), "[%s]", STR_CRSF_RX_BIND);
      lcdDrawText(0, y, buf,
                  (sel ? INVERS : 0) |
                      (idx == BIND_ROW_TX && loopActive ? BLINK : 0));
    } else if (idx == BIND_ROW_PHRASE) {
      lcdDrawText(0, y, STR_CRSF_BIND_PHRASE, sel && !editing ? INVERS : 0);
      if (sel && menu.editId == BIND_EDIT_PHRASE) {
        // the visible part slides to keep the cursor on screen
        constexpr uint8_t visN = (LCD_W - CRSF_CFG_COL2) / FW;
        uint8_t ws =
            menu.phraseCursor < visN ? 0 : menu.phraseCursor - visN + 1;
        char win[visN + 1];
        uint8_t wn = min<uint8_t>(visN, CRSF_BIND_PHRASE_MAXLEN - ws);
        for (uint8_t i = 0; i < wn; i++) {
          char c = menu.phrase[ws + i];
          win[i] = c ? c : ' ';
        }
        win[wn] = '\0';
        lcdDrawText(CRSF_CFG_COL2, y, win, FIXEDWIDTH);
        lcdDrawChar(CRSF_CFG_COL2 + (menu.phraseCursor - ws) * FW, y,
                    win[menu.phraseCursor - ws],
                    ERASEBG | INVERS | FIXEDWIDTH);
      } else {
        const char* v = menu.phrase[0] ? menu.phrase : "---";
        lcdDrawText(CRSF_CFG_COL2, y,
                    sel ? scrollText(v, LCD_W - CRSF_CFG_COL2) : v, 0);
      }
    } else if (idx < BIND_ROW_APPLY_TO) {
      char buf[12];
      snprintf(buf, sizeof(buf), "UID %u", idx - BIND_ROW_UID1 + 1);
      lcdDrawText(0, y, buf, sel && !editing ? INVERS : 0);
      int32_t v = (sel && menu.editId == BIND_EDIT_UID)
                      ? menu.editValue
                      : menu.uidEdit[idx - BIND_ROW_UID1];
      lcdDrawNumber(CRSF_CFG_COL2, y, v,
                    sel && menu.editId == BIND_EDIT_UID ? INVERS | BLINK : 0);
    } else if (idx == BIND_ROW_APPLY_TO) {
      lcdDrawText(0, y, STR_CRSF_APPLY_TO, sel ? INVERS : 0);
      lcdDrawText(CRSF_CFG_COL2, y, STR_CRSF_BIND_APPLY_MODES[menu.applyMode],
                  0);
    } else {
      char buf[24];
      snprintf(buf, sizeof(buf), "[%s]", STR_CRSF_APPLY);
      lcdDrawText(0, y, buf, sel ? INVERS : 0);
    }
  }
  if (!canSet)
    lcdDrawCenteredText(LCD_H - FH, STR_CRSF_NEEDS_41, SMLSIZE);
}

void menuRadioCrsfDeviceConfig(event_t event)
{
  if (event == EVT_ENTRY) {
    memclear(&menu, sizeof(menu));
    crsfConfigStart(g_moduleIdx);
  }

  crsfConfigTick();

  // leaving for another menu: shut the client down
  if (menuEvent) {
    crsfConfigStop();
    return;
  }

  const CrsfElrsStatus& status = crsfConfigElrsStatus();

  // incompatible ELRS 1.x module
  if (crsfConfigState() == CRSF_STATE_ELRS_V1) {
    if (event == EVT_KEY_BREAK(KEY_EXIT)) {
      crsfConfigStop();
      popMenu();
      return;
    }
    lcdDrawCenteredText(LCD_H / 2, STR_CRSF_ELRS_V1);
    return;
  }

  // critical link flags need an explicit user ack (ELRS)
  if (status.flags & CRSF_ELRS_FLAG_CRITICAL_MASK) {
    if (event == EVT_KEY_BREAK(KEY_ENTER) ||
        event == EVT_KEY_BREAK(KEY_EXIT)) {
      crsfConfigAckCriticalFlags();
      killEvents(event);
    }
    lcdDrawCenteredText(LCD_H / 2 - FH, status.flagInfo, BLINK);
    lcdDrawCenteredText(LCD_H / 2 + FH, STR_POPUPS_ENTER_EXIT, SMLSIZE);
    return;
  }

  // synthetic ExpressLRS binding view
  if (menu.folder == CRSF_BIND_FOLDER) {
    handleBindView(event);
    return;
  }

  // a completed command keeps its final message on screen until
  // the user dismisses it
  if (crsfConfigCommandActive()) {
    menu.cmdWasActive = true;
  } else if (menu.cmdWasActive) {
    menu.cmdWasActive = false;
    CrsfConfigField f;
    crsfConfigGetField(crsfConfigCommandField(), f);
    if (f.loaded && f.svalue[0])
      strAppend(menu.cmdResult, f.svalue, sizeof(menu.cmdResult) - 1);
  }

  // the popup handlers consume the event so it cannot fall through and
  // be processed a second time by the list below
  if (menu.cmdResult[0]) {
    if (event == EVT_KEY_BREAK(KEY_ENTER) ||
        event == EVT_KEY_BREAK(KEY_EXIT)) {
      menu.cmdResult[0] = '\0';
      killEvents(event);
      event = 0;
    }
  }

  if (crsfConfigCommandActive()) {
    switch (event) {
      case EVT_KEY_BREAK(KEY_ENTER):
        if (crsfConfigCommandStatus() == CRSF_CMD_CONFIRMATION_NEEDED)
          crsfConfigCommandConfirm();
        killEvents(event);
        event = 0;
        break;
      case EVT_KEY_BREAK(KEY_EXIT):
        // back stops the command; holding it instead leaves the
        // command (WiFi, BLE joystick, ...) running in the background
        crsfConfigCommandCancel();
        menu.cmdWasActive = false;  // closed by the user: no result popup
        killEvents(event);
        event = 0;
        break;
      case EVT_KEY_LONG(KEY_EXIT):
        crsfConfigCommandDetach();
        menu.cmdWasActive = false;
        killEvents(event);
        event = 0;
        break;
    }
  }

  uint8_t count = crsfConfigFieldCount();
  uint8_t loaded = crsfConfigLoadedFieldCount();
  buildEntries(crsfConfigGeneration(), loaded);
  const uint16_t* entries = menu.entries;
  uint16_t nEntries = menu.nEntries;
  if (menu.selected >= nEntries && nEntries > 0) menu.selected = nEntries - 1;

  CrsfConfigField selField;
  bool selIsField = false;
  if (nEntries > 0 && !(entries[menu.selected] & 0x300))
    selIsField = crsfConfigGetField(entries[menu.selected], selField);

  // a background reload can reshuffle the list: abort a pending edit
  // if the selection no longer points at the field it started on
  bool editing = menu.editId != 0 &&
                 selIsField && selField.id == menu.editId;
  if (menu.editId != 0 && !editing) menu.editId = 0;

  if (!crsfConfigCommandActive() && !menu.cmdResult[0]) {
    if (editing) {
      // value editing follows the usual editor orientation:
      // rotating right / plus increments
      switch (event) {
        case EVT_ROTARY_RIGHT:
        case EVT_KEY_BREAK(KEY_UP):
        case EVT_KEY_REPT(KEY_UP):
        case EVT_KEY_BREAK(KEY_PLUS):
        case EVT_KEY_REPT(KEY_PLUS):
          adjustEditValue(selField, 1);
          break;

        case EVT_ROTARY_LEFT:
        case EVT_KEY_BREAK(KEY_DOWN):
        case EVT_KEY_REPT(KEY_DOWN):
        case EVT_KEY_BREAK(KEY_MINUS):
        case EVT_KEY_REPT(KEY_MINUS):
          adjustEditValue(selField, -1);
          break;

        case EVT_KEY_BREAK(KEY_ENTER):
          crsfConfigWriteValue(selField.id, menu.editValue);
          menu.editId = 0;
          killEvents(event);
          break;

        case EVT_KEY_BREAK(KEY_EXIT):
          menu.editId = 0;
          killEvents(event);
          break;
      }
    } else {
      switch (event) {
        case EVT_ROTARY_LEFT:
        case EVT_KEY_BREAK(KEY_UP):
        case EVT_KEY_REPT(KEY_UP):
        case EVT_KEY_BREAK(KEY_PLUS):
        case EVT_KEY_REPT(KEY_PLUS):
          if (menu.selected > 0)
            menu.selected--;
          else if (nEntries > 0)
            menu.selected = nEntries - 1;
          menu.selTime = get_tmr10ms();
          break;

        case EVT_ROTARY_RIGHT:
        case EVT_KEY_BREAK(KEY_DOWN):
        case EVT_KEY_REPT(KEY_DOWN):
        case EVT_KEY_BREAK(KEY_MINUS):
        case EVT_KEY_REPT(KEY_MINUS):
          if (nEntries > 0)
            menu.selected = (menu.selected + 1) % nEntries;
          menu.selTime = get_tmr10ms();
          break;

        case EVT_KEY_BREAK(KEY_ENTER):
          if (nEntries > 0) {
            uint16_t entry = entries[menu.selected];
            if (entry == CRSF_ENTRY_BIND) {
              enterFolder(CRSF_BIND_FOLDER);
              menu.uidPrefilled = false;
              menu.loopWasActive = false;
              if (crsfConfigElrsCanSetBindInfo(findModuleDevice()))
                crsfConfigRequestUid(MODULE_ADDRESS);
            } else if (entry & 0x100) {
              crsfConfigSelectDevice(entry & 0xFF);
              menu.selected = 0;
              menu.scrollOfs = 0;
            } else if (selIsField && selField.loaded) {
              if (selField.type == CRSF_FIELD_FOLDER) {
                enterFolder(selField.id);
              } else if (selField.type == CRSF_FIELD_COMMAND) {
                crsfConfigCommandStart(selField.id);
              } else if (fieldEditable(selField)) {
                menu.editId = selField.id;
                menu.editValue = selField.value;
                menu.selTime = get_tmr10ms();
              }
            }
          }
          killEvents(event);
          break;

        case EVT_KEY_BREAK(KEY_EXIT):
          if (!leaveFolder()) {
            if (menu.selected != 0) {
              // first back returns to the top of the list...
              menu.selected = 0;
              menu.scrollOfs = 0;
              menu.selTime = get_tmr10ms();
            } else {
              // ...back from the top leaves the menu
              crsfConfigStop();
              popMenu();
              return;
            }
          }
          killEvents(event);
          break;

        case EVT_KEY_LONG(KEY_EXIT):
          crsfConfigStop();
          popMenu();
          return;
      }
    }
  }

  editing = (menu.editId != 0);

  // scrolling
  if (menu.selected < menu.scrollOfs)
    menu.scrollOfs = menu.selected;
  else if (menu.selected >= menu.scrollOfs + CRSF_CFG_LINES)
    menu.scrollOfs = menu.selected - CRSF_CFG_LINES + 1;

  // header
  auto dev = crsfConfigCurrentDevice();
  lcdDrawTextAlignedLeft(0, dev ? dev->name : STR_CRSF_CONFIG);

  if (count > 0 && loaded < count) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%u/%u", loaded, count);
    lcdDrawText(LCD_W - 1, 0, buf, RIGHT);
  } else if ((status.flags & CRSF_ELRS_FLAG_WARNING_MASK) &&
             status.flagInfo[0]) {
    lcdDrawText(LCD_W - 1, 0, status.flagInfo, RIGHT | BLINK);
  } else if (status.goodPkts > 0) {
    char buf[20];
    snprintf(buf, sizeof(buf), "%u:%u%s%s", status.badPkts, status.goodPkts,
             (status.flags & CRSF_ELRS_FLAG_CONNECTED) ? " C" : "",
             (status.flags & CRSF_ELRS_FLAG_ARMED) ? " A" : "");
    lcdDrawText(LCD_W - 1, 0, buf, RIGHT);
  }
  lcdInvertLine(0);

  if (nEntries == 0) {
    lcdDrawCenteredText(LCD_H / 2, dev ? "..." : STR_WAITING_FOR_MODULE);
  }

  coord_t valCol = menu.valCol;

  // body
  for (uint8_t line = 0; line < CRSF_CFG_LINES; line++) {
    uint16_t idx = menu.scrollOfs + line;
    if (idx >= nEntries) break;

    coord_t y = MENU_HEADER_HEIGHT + 1 + line * FH;
    bool sel = (idx == menu.selected);
    uint16_t entry = entries[idx];

    if (entry == CRSF_ENTRY_BIND) {
      lcdDrawText(0, y, STR_MODULE_BIND, sel ? INVERS : 0);
      continue;
    }

    if (entry & 0x100) {
      const CrsfConfigDevice* d = nullptr;
      for (uint8_t i = 0; i < crsfConfigDeviceCount(); i++) {
        auto dev2 = crsfConfigGetDevice(i);
        if (dev2 && dev2->address == (entry & 0xFF)) {
          d = dev2;
          break;
        }
      }
      char buf[24];
      snprintf(buf, sizeof(buf), "> %s", d ? d->name : "?");
      lcdDrawText(0, y, buf, sel ? INVERS : 0);
      continue;
    }

    CrsfConfigField f;
    if (!crsfConfigGetField(entry, f)) continue;

    bool editingThis = editing && f.id == menu.editId;
    LcdFlags nameAttr = 0;
    LcdFlags valAttr = 0;
    if (sel) {
      if (editingThis)
        valAttr = INVERS | BLINK;
      else
        nameAttr = INVERS;
    }

    if (f.type == CRSF_FIELD_COMMAND) {
      char buf[32];
      snprintf(buf, sizeof(buf), "[%s]", f.name);
      lcdDrawText(0, y, buf, nameAttr);
      continue;
    }

    // the ELRS version field carries the version as its NAME and the
    // commit hash as its value: shown as one line, like the color UI,
    // slowly scrolling when it does not fit
    if (crsfConfigElrsIsVersionField(f)) {
      char buf[36];
      if (f.svalue[0])
        snprintf(buf, sizeof(buf), "%s (%s)", f.name, f.svalue);
      else
        strAppend(buf, f.name, sizeof(buf) - 1);
      lcdDrawText(0, y, sel ? scrollText(buf, LCD_W) : buf, nameAttr);
      continue;
    }

    lcdDrawSizedText(0, y, f.name, displayNameLen(f.name), nameAttr);

    char value[CRSF_CFG_VAL_LEN];
    formatFieldValue(f, editingThis, value, sizeof(value));
    if (value[0]) {
      // the value of the row being hovered or edited scrolls through
      // when it is too wide for its column
      lcdDrawText(valCol, y, sel ? scrollText(value, LCD_W - valCol) : value,
                  valAttr);
    }
  }

  if (crsfConfigCommandActive()) {
    drawCommandPopup();
  } else if (menu.cmdResult[0]) {
    constexpr coord_t x = 4;
    constexpr coord_t w = LCD_W - 8;
    constexpr coord_t y = 12;
    constexpr coord_t h = LCD_H - 24;
    lcdDrawFilledRect(x, y, w, h, SOLID, ERASE);
    lcdDrawRect(x, y, w, h);
    lcdDrawText(x + 3, y + 3, menu.cmdResult);
    lcdDrawText(x + 3, y + h - FH - 2, STR_POPUPS_ENTER_EXIT, SMLSIZE);
  }
}

#endif  // defined(CRSF_CONFIG_MENU)
