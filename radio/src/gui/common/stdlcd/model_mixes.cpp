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
#include "tasks/mixer_task.h"
#include "hal/adc_driver.h"
#include "input_mapping.h"

#define _STR_MAX(x)                     "/" #x
#define STR_MAX(x)                     _STR_MAX(x)

uint8_t getMixesCount()
{
  uint8_t count = 0;
  uint8_t ch;

  for (int i=MAX_MIXERS-1; i>=0; i--) {
    ch = mixAddress(i)->srcRaw;
    if (ch != 0) {
      count++;
    }
  }
  return count;
}

bool reachMixesLimit()
{
  if (getMixesCount() >= MAX_MIXERS) {
    POPUP_WARNING(STR_NOFREEMIXER);
    return true;
  }
  return false;
}

void deleteMix(uint8_t idx)
{
  mixerTaskStop();
  MixData * mix = mixAddress(idx);
  memmove(mix, mix+1, (MAX_MIXERS-(idx+1))*sizeof(MixData));
  memclear(&g_model.mixData[MAX_MIXERS-1], sizeof(MixData));
  mixerTaskStart();
  storageDirty(EE_MODEL);
}

void insertMix(uint8_t idx)
{
  mixerTaskStop();
  MixData * mix = mixAddress(idx);
  memmove(mix+1, mix, (MAX_MIXERS-(idx+1))*sizeof(MixData));
  memclear(mix, sizeof(MixData));
  mix->destCh = s_currCh-1;
  mix->srcRaw = s_currCh;
  if (!isSourceAvailable(mix->srcRaw)) {
    if (s_currCh > adcGetMaxInputs(ADC_INPUT_MAIN)) {
      mix->srcRaw = MIXSRC_FIRST_STICK - 1 + s_currCh;
    } else {
      mix->srcRaw = MIXSRC_FIRST_STICK + inputMappingChannelOrder(s_currCh - 1);
    }
    while (!isSourceAvailable(mix->srcRaw)) {
      mix->srcRaw += 1;
    }
  }
  mix->weight = 100;
  mixerTaskStart();
  storageDirty(EE_MODEL);
}

void copyMix(uint8_t idx)
{
  mixerTaskStop();
  MixData * mix = mixAddress(idx);
  memmove(mix+1, mix, (MAX_MIXERS-(idx+1))*sizeof(MixData));
  mixerTaskStart();
  storageDirty(EE_MODEL);
}

bool swapMixes(uint8_t & idx, uint8_t up)
{
  MixData * x, * y;
  int8_t tgt_idx = (up ? idx-1 : idx+1);

  x = mixAddress(idx);

  if (tgt_idx < 0) {
    if (x->destCh == 0)
      return false;
    x->destCh--;
    return true;
  }

  if (tgt_idx == MAX_MIXERS) {
    if (x->destCh == MAX_OUTPUT_CHANNELS-1)
      return false;
    x->destCh++;
    return true;
  }

  y = mixAddress(tgt_idx);
  uint8_t destCh = x->destCh;
  if(!y->srcRaw || destCh != y->destCh) {
    if (up) {
      if (destCh>0) x->destCh--;
      else return false;
    }
    else {
      if (destCh<MAX_OUTPUT_CHANNELS-1) x->destCh++;
      else return false;
    }
    return true;
  }

  mixerTaskStop();
  memswap(x, y, sizeof(MixData));
  mixerTaskStart();

  idx = tgt_idx;
  return true;
}

void onMixesMenu(const char * result)
{
  uint8_t chn = mixAddress(s_currIdx)->destCh + 1;

  if (result == STR_EDIT) {
    pushMenu(menuModelMixOne);
  }
  else if (result == STR_INSERT_BEFORE || result == STR_INSERT_AFTER) {
    if (!reachMixesLimit()) {
      s_currCh = chn;
      if (result == STR_INSERT_AFTER) { s_currIdx++; menuVerticalPosition++; }
      insertMix(s_currIdx);
      pushMenu(menuModelMixOne);
    }
  }
  else if (result == STR_COPY || result == STR_MOVE) {
    s_copyMode = (result == STR_COPY ? COPY_MODE : MOVE_MODE);
    s_copySrcIdx = s_currIdx;
    s_copySrcCh = chn;
    s_copySrcRow = menuVerticalPosition;
  }
  else if (result == STR_DELETE) {
    deleteMix(s_currIdx);
  }
}

#if LCD_W >= 212
#define MIX_LINE_WEIGHT_POS            2*FW+34
#define MIX_LINE_SRC_POS               7*FW+5
#define MIX_LINE_CURVE_POS             13*FW+3
#define MIX_LINE_SWITCH_POS            19*FW+1
#define MIX_LINE_FM_POS                13*FW+3
#define MIX_LINE_DELAY_POS             24*FW+3
#define MIX_LINE_NAME_POS              LCD_W-LEN_EXPOMIX_NAME*FW-MENUS_SCROLLBAR_WIDTH
#define MIX_HDR_GAUGE_POS_X            127

void displayHeaderChannelName(uint8_t ch)
{
  if (g_model.limitData[ch].name[0] != '\0') {
    lcdDrawSizedText(MIX_HDR_GAUGE_POS_X - FWNUM * 5 - 1, 1, g_model.limitData[ch].name, ZLEN(g_model.limitData[ch].name), SMLSIZE|RIGHT);
  }
}

void displayMixInfos(coord_t y, MixData * md)
{
  drawCurveRef(MIX_LINE_CURVE_POS, y, md->curve, 0);

  if (md->swtch) {
    drawSwitch(MIX_LINE_SWITCH_POS, y, md->swtch);
  }
}

void displayMixLine(coord_t y, MixData * md)
{
  if (md->name[0])
    lcdDrawSizedText(MIX_LINE_NAME_POS, y, md->name, sizeof(md->name), 0);
  if (!md->flightModes || ((md->curve.value || md->swtch) && ((get_tmr10ms() / 200) & 1)))
    displayMixInfos(y, md);
  else
    displayFlightModes(MIX_LINE_FM_POS, y, md->flightModes);

  char cs = ' ';
  if (md->speedDown || md->speedUp)
    cs = 'S';
  if (md->delayUp || md->delayDown)
    cs = (cs == 'S' ? '*' : 'D');
  lcdDrawChar(MIX_LINE_DELAY_POS, y, cs);
}
#else // LCD_W >= 212
#define MIX_LINE_WEIGHT_POS            6*FW+8
#define MIX_LINE_SRC_POS               7*FW+3
#define MIX_LINE_CURVE_POS             12*FW+3
#define MIX_LINE_SWITCH_POS            16*FW+5
#define MIX_LINE_FM_POS                19*FW
#define MIX_LINE_DELAY_POS             20*FW+2
#define MIX_LINE_NAME_POS              LCD_W-LEN_EXPOMIX_NAME*FW

void displayHeaderChannelName(uint8_t ch)
{
  uint8_t len = zlen(g_model.limitData[ch].name, sizeof(g_model.limitData[ch].name));
  if (len) {
    lcdDrawSizedText(80, 1, g_model.limitData[ch].name, len, SMLSIZE);
  }
}

void displayMixInfos(coord_t y, MixData * md)
{
  drawCurveRef(MIX_LINE_CURVE_POS, y, md->curve, 0);

  if (md->swtch) {
    drawSwitch(MIX_LINE_SWITCH_POS, y, md->swtch);
  }

  char cs = ' ';
  if (md->speedDown || md->speedUp)
    cs = 'S';
  if (md->delayUp || md->delayDown)
    cs = (cs == 'S' ? '*' : 'D');
  lcdDrawChar(MIX_LINE_DELAY_POS, y, cs);
}

void displayMixLine(coord_t y, MixData * md, bool active)
{
  if(active && md->name[0]) {
    lcdDrawFilledRect(FW*sizeof(TR_MIXES)+FW/2, 0, FW*4+1, MENU_HEADER_HEIGHT, 0xFF, ERASE);
    lcdDrawSizedText(FW*sizeof(TR_MIXES)+FW/2, 0, md->name, sizeof(md->name), 0);
    if (!md->flightModes || ((md->curve.value || md->swtch) && ((get_tmr10ms() / 200) & 1)))
      displayMixInfos(y, md);
    else
      displayFlightModes(MIX_LINE_FM_POS, y, md->flightModes);
  }
  else {
    if (md->name[0])
      lcdDrawSizedText(MIX_LINE_NAME_POS, y, md->name, sizeof(md->name), 0);
    else if (!md->flightModes || ((md->curve.value || md->swtch) && ((get_tmr10ms() / 200) & 1)))
      displayMixInfos(y, md);
    else
      displayFlightModes(MIX_LINE_FM_POS, y, md->flightModes);
  }
}
#endif // LCD_W >= 212

void menuModelMixAll(event_t event)
{
  int8_t sub = menuVerticalPosition - HEADER_LINE;

  if (s_editMode > 0) {
    s_editMode = 0;
  }

  uint8_t chn = mixAddress(s_currIdx)->destCh + 1;

  switch (event) {
    case EVT_ENTRY:
    case EVT_ENTRY_UP:
      s_copyMode = 0;
      s_copyTgtOfs = 0;
      break;
    case EVT_KEY_LONG(KEY_EXIT):
      if (s_copyMode && s_copyTgtOfs == 0) {
        deleteMix(s_currIdx);
        killEvents(event);
        event = 0;
      }
      // no break
    case EVT_KEY_BREAK(KEY_EXIT):
      if (s_copyMode) {
        if (s_copyTgtOfs) {
          // cancel the current copy / move operation
          if (s_copyMode == COPY_MODE) {
            deleteMix(s_currIdx);
          }
          else {
            do {
              swapMixes(s_currIdx, s_copyTgtOfs > 0);
              s_copyTgtOfs += (s_copyTgtOfs < 0 ? +1 : -1);
            } while (s_copyTgtOfs != 0);
            storageDirty(EE_MODEL);
          }
          menuVerticalPosition = s_copySrcRow + HEADER_LINE;
          s_copyTgtOfs = 0;
        }
        s_copyMode = 0;
        event = 0;
      }
      break;
    case EVT_KEY_BREAK(KEY_ENTER):
      if ((!s_currCh || (s_copyMode && !s_copyTgtOfs)) && !READ_ONLY()) {
        s_copyMode = (s_copyMode == COPY_MODE ? MOVE_MODE : COPY_MODE);
        s_copySrcIdx = s_currIdx;
        s_copySrcCh = chn;
        s_copySrcRow = sub;
        break;
      }
      // no break

    case EVT_KEY_LONG(KEY_ENTER):
      killEvents(event);
      if (s_copyTgtOfs) {
        s_copyMode = 0;
        s_copyTgtOfs = 0;
      }
      else {
        if (READ_ONLY()) {
          if (!s_currCh) {
            pushMenu(menuModelMixOne);
          }
        }
        else {
          if (s_copyMode) s_currCh = 0;
          if (s_currCh) {
            if (reachMixesLimit()) break;
            insertMix(s_currIdx);
            pushMenu(menuModelMixOne);
            s_copyMode = 0;
          }
          else {
            event = 0;
            s_copyMode = 0;
            POPUP_MENU_ADD_ITEM(STR_EDIT);
            POPUP_MENU_ADD_ITEM(STR_INSERT_BEFORE);
            POPUP_MENU_ADD_ITEM(STR_INSERT_AFTER);
            POPUP_MENU_ADD_ITEM(STR_COPY);
            POPUP_MENU_ADD_ITEM(STR_MOVE);
            POPUP_MENU_ADD_ITEM(STR_DELETE);
            POPUP_MENU_START(onMixesMenu);
          }
        }
      }
      break;

      // TODO: add PLUS / MINUS?
    // case EVT_KEY_LONG(KEY_LEFT):
    // case EVT_KEY_LONG(KEY_RIGHT):
    //   if (s_copyMode && !s_copyTgtOfs) {
    //     if (reachMixesLimit()) break;
    //     s_currCh = chn;
    //     if (event == EVT_KEY_LONG(KEY_RIGHT)) { s_currIdx++; menuVerticalPosition++; }
    //     insertMix(s_currIdx);
    //     pushMenu(menuModelMixOne);
    //     s_copyMode = 0;
    //     killEvents(event);
    //   }
    //   break;
  }

  if (s_copyMode &&
      (IS_NEXT_EVENT(event) || IS_PREVIOUS_EVENT(event))) {

    uint8_t next_ofs = (IS_PREVIOUS_EVENT(event) ? s_copyTgtOfs - 1 : s_copyTgtOfs + 1);

    if (s_copyTgtOfs==0 && s_copyMode==COPY_MODE) {
      // insert a mix on the same channel (just above / just below)
      if (!reachMixesLimit()) {
        copyMix(s_currIdx);
        if (IS_NEXT_EVENT(event))
          s_currIdx++;
        else if (sub - menuVerticalOffset >= 6)
          menuVerticalOffset++;
      }
    }
    else if (next_ofs==0 && s_copyMode==COPY_MODE) {
      // delete the mix
      deleteMix(s_currIdx);
      if (IS_PREVIOUS_EVENT(event))
        s_currIdx--;
    }
    else {
      // only swap the mix with its neighbor
      if (swapMixes(s_currIdx, IS_PREVIOUS_EVENT(event))) {
        storageDirty(EE_MODEL);
      }
    }

    s_copyTgtOfs = next_ofs;
  }

  lcdDrawNumber(FW*sizeof(TR_MIXES)+FW/2, 0, getMixesCount(), 0);
  lcdDrawText(lcdNextPos, 0, STR_MAX(MAX_MIXERS));

  // Value
  uint8_t index = mixAddress(s_currIdx)->destCh;
  if (!s_currCh) {
    displayHeaderChannelName(index);
#if LCD_W >= 212
    lcdDrawNumber(MIX_HDR_GAUGE_POS_X, 2, calcRESXto1000(ex_chans[index]), PREC1|TINSIZE|RIGHT);
#endif
  }

  SIMPLE_MENU(STR_MIXES, menuTabModel, MENU_MODEL_MIXES, HEADER_LINE + s_maxLines);

#if LCD_W >= 212
  // Gauge
  if (!s_currCh) {
    drawGauge(MIX_HDR_GAUGE_POS_X, 1, 58, 6, ex_chans[index], 2048);
  }
#endif

  sub = menuVerticalPosition - HEADER_LINE;
  s_currCh = 0;
  int cur = 0;
  int i = 0;

  for (uint8_t ch=1; ch<=MAX_OUTPUT_CHANNELS; ch++) {
    MixData * md;
    coord_t y = MENU_HEADER_HEIGHT+1+(cur-menuVerticalOffset)*FH;
    if (i<MAX_MIXERS && (md=mixAddress(i))->srcRaw && md->destCh+1 == ch) {
      if (cur-menuVerticalOffset >= 0 && cur-menuVerticalOffset < NUM_BODY_LINES) {
        putsChn(0, y, ch, 0); // show CHx
      }
      uint8_t mixCnt = 0;
      do {
        if (s_copyMode) {
          if (s_copyMode == MOVE_MODE && cur-menuVerticalOffset >= 0 && cur-menuVerticalOffset < NUM_BODY_LINES && s_copySrcCh == ch && s_copyTgtOfs != 0 && i == (s_copySrcIdx + (s_copyTgtOfs<0))) {
            lcdDrawRect(22, y-1, LCD_W-22, 9, DOTTED);
            cur++; y+=FH;
          }
          if (s_currIdx == i) {
            sub = cur;
            menuVerticalPosition = cur + HEADER_LINE;
            s_currCh = ch;
          }
        }
        else if (sub == cur) {
          s_currIdx = i;
        }
        if (cur-menuVerticalOffset >= 0 && cur-menuVerticalOffset < NUM_BODY_LINES) {
          LcdFlags attr = ((s_copyMode || sub != cur) ? 0 : INVERS);

          if (mixCnt > 0) lcdDrawTextAtIndex(FW, y, STR_VMLTPX2, md->mltpx, 0);

          drawSource(MIX_LINE_SRC_POS, y, md->srcRaw, 0);

          if (mixCnt == 0 && md->mltpx == 1) {
            lcdDrawText(MIX_LINE_WEIGHT_POS, y, "MULT!", RIGHT | attr | (isMixActive(i) ? BOLD : 0));
          }
          else {
            gvarWeightItem(MIX_LINE_WEIGHT_POS, y, md, RIGHT | attr | (isMixActive(i) ? BOLD : 0), 0);
          }

#if LCD_W >= 212
          displayMixLine(y, md);
#else
          displayMixLine(y, md, (sub == cur));
#endif

          if (s_copyMode) {
            if ((s_copyMode==COPY_MODE || s_copyTgtOfs == 0) && s_copySrcCh == ch && i == (s_copySrcIdx + (s_copyTgtOfs<0))) {
              /* draw a border around the raw on selection mode (copy/move) */
              lcdDrawRect(22, y-1, LCD_W-22, 9, s_copyMode == COPY_MODE ? SOLID : DOTTED);
            }
            if (cur == sub) {
              /* invert the raw when it's the current one */
              lcdDrawSolidFilledRect(23, y, LCD_W-24, 7);
            }
          }
        }
        cur++; y+=FH; mixCnt++; i++; md++;
      } while (i<MAX_MIXERS && md->srcRaw && md->destCh+1 == ch);
      if (s_copyMode == MOVE_MODE && cur-menuVerticalOffset >= 0 && cur-menuVerticalOffset < NUM_BODY_LINES && s_copySrcCh == ch && i == (s_copySrcIdx + (s_copyTgtOfs<0))) {
        lcdDrawRect(22, y-1, LCD_W-22, 9, DOTTED);
        cur++;
      }
    }
    else {
      uint8_t attr = 0;
      if (sub == cur) {
        s_currIdx = i;
        s_currCh = ch;
        if (!s_copyMode) {
          attr = INVERS;
          displayHeaderChannelName(ch - 1);
#if LCD_W >= 212
          if (g_model.limitData[ch - 1].name[0] != '\0') {
            coord_t xPos = MIX_HDR_GAUGE_POS_X - FWNUM * 5 - 50;
            lcdDrawFilledRect(lcdNextPos, 0, lcdNextPos - xPos, MENU_HEADER_HEIGHT, SOLID, FILL_WHITE | GREY_DEFAULT);
          }
#endif
        }
      }
      if (cur-menuVerticalOffset >= 0 && cur-menuVerticalOffset < NUM_BODY_LINES) {
        putsChn(0, y, ch, attr); // show CHx
        if (s_copyMode == MOVE_MODE && s_copySrcCh == ch) {
          lcdDrawRect(22, y-1, LCD_W-22, 9, DOTTED);
        }
      }
      cur++;
    }
  }
  s_maxLines = cur;
  if (sub >= s_maxLines-1) {
    menuVerticalPosition = s_maxLines - 1 + HEADER_LINE;
  }
}
