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

#include "model_audio.h"

#include "edgetx.h"
#include "switches.h"

static const char* const _suffixes[] = {"-off", "-on"};

char* getModelAudioPath(char* path, bool trailingSlash)
{
  strcpy(path, SOUNDS_PATH "/");
  strncpy(path + SOUNDS_PATH_LNG_OFS, currentLanguagePack->id, 2);
  char* buf = strcat_currentmodelname(path + sizeof(SOUNDS_PATH), ' ');

  if (!isFileAvailable(path)) {
    buf = strcat_currentmodelname(path + sizeof(SOUNDS_PATH), 0);
  }

  if (trailingSlash)
    *buf++ = '/';
  *buf = '\0';
  return buf;
}

void getFlightmodeAudioFile(char* path, int index, unsigned int event)
{
  char* str = getModelAudioPath(path);
  char* tmp = strcatFlightmodeName(str, index);
  tmp = strAppend(tmp, _suffixes[event]);
  strAppend(tmp, SOUNDS_EXT);
}

static const char* const _sw_positions[] = {"-up", "-mid", "-down"};

bool getSwitchAudioFile(char* path, swsrc_t index)
{
  char* str = getModelAudioPath(path);

  if (index <= SWSRC_LAST_SWITCH) {
    div_t swinfo = switchInfo(index);
    auto sw_name = switchGetName(swinfo.quot);
    if (!sw_name) return false;
    str = strAppend(str, sw_name);
    str = strAppend(str, _sw_positions[swinfo.rem]);
  } else {
    div_t swinfo =
        div((int)(index - SWSRC_FIRST_MULTIPOS_SWITCH), XPOTS_MULTIPOS_COUNT);
    *str++ = 'S';
    *str++ = '1' + swinfo.quot;
    *str++ = '1' + swinfo.rem;
    *str = '\0';
  }
  strAppend(str, SOUNDS_EXT);
  return true;
}

void getLogicalSwitchAudioFile(char* filename, int index, unsigned int event)
{
  char* str = getModelAudioPath(filename);

  *str++ = 'L';
  if (index >= 9) {
    div_t qr = div(index + 1, 10);
    *str++ = '0' + qr.quot;
    *str++ = '0' + qr.rem;
  } else {
    *str++ = '1' + index;
  }

  strcpy(str, _suffixes[event]);
  strcat(str, SOUNDS_EXT);
}

// Flight modes Audio Files <flightmodename>-[on|off].wav
bool matchModeAudioFile(const char* filename, int& index, int& event)
{
  for (int i = 0; i < MAX_FLIGHT_MODES; i++) {
    auto* c = filename;
    auto* fm_name = g_model.flightModeData[i].name;
    auto fm_name_len = strnlen(fm_name, LEN_FLIGHT_MODE_NAME);
    if (strncasecmp(c, fm_name, fm_name_len) != 0) continue;
    c += fm_name_len;
    for (size_t e = 0; e < DIM(_suffixes); e++) {
      auto suffix_len = strlen(_suffixes[e]);
      if (strncasecmp(c, _suffixes[e], suffix_len) != 0) continue;
      c += suffix_len;
      if (*c != '.') continue;
      index = i;
      event = e;
      return true;
    }
  }
  return false;
}

bool matchSwitchAudioFile(const char* filename, int& sw_pos)
{
  // Switches Audio Files <switchname>-[up|mid|down].wav
  for (int i = 0; i < switchGetMaxSwitches(); i++) {
    auto* c = filename;
    auto sw_name = switchGetName(i);
    auto sw_name_len = strlen(sw_name);
    if (strncasecmp(c, sw_name, sw_name_len) != 0) continue;
    c += sw_name_len;
    for (size_t pos = 0; pos < DIM(_sw_positions); pos++) {
      auto pos_len = strlen(_sw_positions[pos]);
      if (strncasecmp(c, _sw_positions[pos], pos_len) != 0) continue;
      c += pos_len;
      if (*c != '.') continue;
      sw_pos = i * 3 + pos;
      return true;
    }
  }

  // match multipos switches
  {
    auto* c = filename;
    if (*c != 'S' && *c != 's') return false;
    c += 1;
    if (*c < '1' || *c > '9') return false;
    uint8_t xpot = uint8_t(*c++ - '1');
    if (*c < '1' || *c > '9') return false;
    uint8_t pos = uint8_t(*c++ - '1');
    if (pos >= XPOTS_MULTIPOS_COUNT) return false;
    if (*c != '.') return false;

    for (int i = 0; i < MAX_POTS; i++) {
      if (i != xpot) continue;
      if (!IS_POT_MULTIPOS(i)) continue;
      sw_pos = (MAX_SWITCHES * 3) + XPOTS_MULTIPOS_COUNT * xpot + pos;
      return true;
    }
  }

  return false;
}

// Logical Switches Audio Files <switchname>-[on|off].wav
bool matchLogicalSwitchAudioFile(const char* filename, int& index, int& event)
{
  auto* c = filename;
  if (*c != 'L' && *c != 'l') return false;
  c += 1;
  if (*c < '1' && *c > '9') return false;
  int lsw = *c++ - '0';
  if (*c >= '0' && *c <= '9') {
    lsw = (lsw * 10) + (*c++ - '0');
  }
  if (*c != '-' || lsw < 1) return false;

  for (size_t e = 0; e < DIM(_suffixes); e++) {
    auto* s = c;
    auto suffix_len = strlen(_suffixes[e]);
    if (strncasecmp(s, _suffixes[e], suffix_len) != 0) continue;
    s += suffix_len;
    if (*s != '.') continue;
    index = lsw - 1;
    event = e;
    return true;
  }
  return false;
}
