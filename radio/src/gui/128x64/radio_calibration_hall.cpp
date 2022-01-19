/*
 * Copyright (C) OpenTX
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

#include "opentx.h"
#include "io/crsf/crossfire.h"

#define XPOT_DELTA 10
#define XPOT_DELAY 10 /* cycles */

void menuCommonCalib(event_t event);
void menuCommonCalibPots(event_t event);

enum{
  GIMBAL_LEFT = 0,
  GIMBAL_RIGHT,
  GIMBAL_COUNT
};

enum{
  GIMBAL_LEFT_SEL = 0,
  GIMBAL_BOTH_SEL,
  GIMBAL_RIGHT_SEL,
  GIMBAL_SEL_COUNT
};
#define CALIB_POINT_COUNT   (CALIB_SET_P8 - CALIB_SET_P0 + 1)
const int16_t point_pos[CALIB_POINT_COUNT][2] = {{0,0}, {1024,0}, {1024,1024}, {0,1024}, {-1024,1024}, {-1024,0}, {-1024,-1024}, {0,-1024}, {1024,-1024}};
#define LLABEL_CENTERX            (15)
#define BOTHLABEL_CENTERX         (53)
#define RLABEL_CENTERX            (90)
#define POINT_CAL_COUNTDOWN       (3)
#if LCD_H > 64
#define LABEL_YPOS                MENU_HEADER_HEIGHT+4*FH
#else
#define LABEL_YPOS                MENU_HEADER_HEIGHT+4
#endif

void menuCommonCalib(event_t event)
{
#if defined(SIMU)
  reusableBuffer.calib.state = CALIB_FINISHED;
#else
  static uint8_t gim_select = GIMBAL_BOTH_SEL;
  int16_t force_point_pos[4];
  int16_t *countdown_timer = &reusableBuffer.calib.midVals[0];
  int16_t *count = &reusableBuffer.calib.midVals[1];
  int16_t curr_time;

  if( reusableBuffer.calib.state > CALIB_START && reusableBuffer.calib.state < CALIB_FINISHED && crossfireSharedData.stick_state > reusableBuffer.calib.state ) {
    // to sync the state from crossfire
    reusableBuffer.calib.state = crossfireSharedData.stick_state;
  }

  resetBacklightTimeout();

  menuCalibrationState = reusableBuffer.calib.state; // make sure we don't scroll while calibrating

  switch (event) {
    case EVT_KEY_BREAK(KEY_PAGE):
#if NUM_POTS > 0
      chainMenu(menuPotsCalibration);
#endif
      break;

    case EVT_ENTRY:
    case EVT_KEY_BREAK(KEY_EXIT):
      reusableBuffer.calib.state = CALIB_START;
      crossfireSharedData.stick_state = CALIB_FINISHED;
      break;

    case EVT_KEY_BREAK(KEY_ENTER):
      if (reusableBuffer.calib.state == CALIB_SET_STICKS_RANGE)
        reusableBuffer.calib.state = CALIB_STORE;
      else
        reusableBuffer.calib.state++;
      break;

    case EVT_KEY_BREAK(KEY_MENU):
      *count = POINT_CAL_COUNTDOWN;
      *countdown_timer = get_tmr10ms() / 100;
      STOP_PLAY(0);
      if (reusableBuffer.calib.state >= CALIB_SET_P0 && reusableBuffer.calib.state <= CALIB_CAL_POINTS)
        reusableBuffer.calib.state = CALIB_SET_P0;
      else if (reusableBuffer.calib.state == CALIB_SET_STICKS_RANGE)
        reusableBuffer.calib.state = CALIB_CAL_POINTS;
      break;

    case EVT_ROTARY_LEFT:
      if( reusableBuffer.calib.state == CALIB_START ){
#if defined(RADIO_TANGO)
        if( gim_select != GIMBAL_RIGHT_SEL )
          gim_select = (( gim_select + 1 ) % 3 );
#elif defined(RADIO_MAMBO)
        if( gim_select != GIMBAL_LEFT_SEL )
          gim_select = (( gim_select - 1 ) % 3 );
#endif
      }
      break;

    case EVT_ROTARY_RIGHT:
      if( reusableBuffer.calib.state == CALIB_START ){
#if defined(RADIO_TANGO)
        if( gim_select != GIMBAL_LEFT_SEL )
          gim_select = (( gim_select - 1 ) % 3 );
#elif defined(RADIO_MAMBO)
        if( gim_select != GIMBAL_RIGHT_SEL )
          gim_select = (( gim_select + 1 ) % 3 );
#endif
      }
      break;
  }

  switch (reusableBuffer.calib.state) {
    case CALIB_START:
      // START CALIBRATION
      if (!READ_ONLY()) {
        lcdDrawTextAlignedLeft(MENU_HEADER_HEIGHT+2*FH, STR_MENUTOSTART);
        for (uint8_t j = 0; j < GIMBAL_SEL_COUNT; j++) {
          switch (j) {
            case 0:
              lcdDrawTextAtIndex(LLABEL_CENTERX, LABEL_YPOS, STR_LEFT, 0, (gim_select == GIMBAL_LEFT_SEL ? INVERS : 0));
              break;
            case 1:
              lcdDrawTextAtIndex(BOTHLABEL_CENTERX, LABEL_YPOS, STR_BOTH, 0, (gim_select == GIMBAL_BOTH_SEL ? INVERS : 0));
              break;
            case 2:
              lcdDrawTextAtIndex(RLABEL_CENTERX, LABEL_YPOS, STR_RIGHT, 0, (gim_select == GIMBAL_RIGHT_SEL ? INVERS : 0));
              break;
          }
        }
      }
      crossfireSharedData.gim_select = gim_select;
      *countdown_timer = get_tmr10ms() / 100;
      *count = POINT_CAL_COUNTDOWN;
      break;

    case CALIB_SET_P0 ... CALIB_SET_P8:
      crossfireSharedData.stick_state = reusableBuffer.calib.state;
      if( gim_select == GIMBAL_BOTH_SEL ){
        force_point_pos[CONVERT_MODE(1)] = point_pos[reusableBuffer.calib.state - CALIB_SET_P0][1];
        force_point_pos[CONVERT_MODE(0)] = point_pos[reusableBuffer.calib.state - CALIB_SET_P0][0];
        force_point_pos[CONVERT_MODE(2)] = point_pos[reusableBuffer.calib.state - CALIB_SET_P0][1];
        force_point_pos[CONVERT_MODE(3)] = point_pos[reusableBuffer.calib.state - CALIB_SET_P0][0];
      }
      else if( gim_select == GIMBAL_LEFT_SEL ){
        force_point_pos[CONVERT_MODE(1)] = point_pos[reusableBuffer.calib.state - CALIB_SET_P0][1];
        force_point_pos[CONVERT_MODE(0)] = point_pos[reusableBuffer.calib.state - CALIB_SET_P0][0];
        force_point_pos[CONVERT_MODE(2)] = 0;
        force_point_pos[CONVERT_MODE(3)] = 0;
      }
      else if( gim_select == GIMBAL_RIGHT_SEL ){
        force_point_pos[CONVERT_MODE(1)] = 0;
        force_point_pos[CONVERT_MODE(0)] = 0;
        force_point_pos[CONVERT_MODE(2)] = point_pos[reusableBuffer.calib.state - CALIB_SET_P0][1];
        force_point_pos[CONVERT_MODE(3)] = point_pos[reusableBuffer.calib.state - CALIB_SET_P0][0];
      }
      if( reusableBuffer.calib.state == CALIB_SET_P0 )
        lcdDrawText(0*FW, LABEL_YPOS, STR_MOVESTICK_P0, INVERS);
      else if( reusableBuffer.calib.state == CALIB_SET_P1 )
        lcdDrawText(0*FW, LABEL_YPOS, STR_MOVESTICK_P1, INVERS);
      else if( reusableBuffer.calib.state == CALIB_SET_P2 )
        lcdDrawText(0*FW, LABEL_YPOS, STR_MOVESTICK_P2, INVERS);
      else if( reusableBuffer.calib.state == CALIB_SET_P3 )
        lcdDrawText(0*FW, LABEL_YPOS, STR_MOVESTICK_P3, INVERS);
      else if( reusableBuffer.calib.state == CALIB_SET_P4 )
        lcdDrawText(0*FW, LABEL_YPOS, STR_MOVESTICK_P4, INVERS);
      else if( reusableBuffer.calib.state == CALIB_SET_P5 )
        lcdDrawText(0*FW, LABEL_YPOS, STR_MOVESTICK_P5, INVERS);
      else if( reusableBuffer.calib.state == CALIB_SET_P6 )
        lcdDrawText(0*FW, LABEL_YPOS, STR_MOVESTICK_P6, INVERS);
      else if( reusableBuffer.calib.state == CALIB_SET_P7 )
        lcdDrawText(0*FW, LABEL_YPOS, STR_MOVESTICK_P7, INVERS);
      else if( reusableBuffer.calib.state == CALIB_SET_P8 )
        lcdDrawText(0*FW, LABEL_YPOS, STR_MOVESTICK_P8, INVERS);
      lcdDrawTextAlignedLeft(MENU_HEADER_HEIGHT+2*FH, STR_MENUWHENDONE);
      // 3s countdown
      curr_time = get_tmr10ms() / 100;
      if( *count == -1 ){
        AUDIO_KEY_PRESS();
        *countdown_timer = curr_time;
        *count = POINT_CAL_COUNTDOWN;
        reusableBuffer.calib.state++;
      }
      else if( curr_time  - *countdown_timer >= 1 ){
        if( *count != 0 )
          playNumber( *count, 0, 0, 0 );
        (*count)--;
        *countdown_timer = curr_time;
      }
      break;

    case CALIB_CAL_POINTS:
      lcdDrawText(0*FW, LABEL_YPOS, STR_CAL_POINTS, INVERS);
      lcdDrawTextAlignedLeft(MENU_HEADER_HEIGHT+2*FH, STR_RELEASE_STICK);
      for (uint8_t i=0; i<NUM_STICKS; i++) {
        reusableBuffer.calib.loVals[i] = 15000;
        reusableBuffer.calib.hiVals[i] = -15000;
        reusableBuffer.calib.midVals[i] = 0;
      }
      crossfireSharedData.stick_state = reusableBuffer.calib.state;
      break;

    case CALIB_SET_STICKS_RANGE:
      // SET STICKS RANGE
      crossfireSharedData.stick_state = reusableBuffer.calib.state;
      lcdDrawText(0*FW, LABEL_YPOS, STR_MOVE_PLUS, INVERS);
      lcdDrawTextAlignedLeft(MENU_HEADER_HEIGHT+2*FH, STR_MENUWHENDONE);

      for (uint8_t i=0; i<NUM_STICKS; i++) {
        // skip the unselected gimbal
        if( gim_select == GIMBAL_RIGHT_SEL && ( i == 0 || i == 1 ))
            continue;
        else if( gim_select == GIMBAL_LEFT_SEL && ( i == 2 || i == 3))
            continue;
        int16_t vt = anaIn(i);
        reusableBuffer.calib.loVals[i] = min(vt, reusableBuffer.calib.loVals[i]);
        reusableBuffer.calib.hiVals[i] = max(vt, reusableBuffer.calib.hiVals[i]);
        if (abs(reusableBuffer.calib.loVals[i]-reusableBuffer.calib.hiVals[i]) > 50) {
          g_eeGeneral.calib[i].mid = reusableBuffer.calib.midVals[i];
          int16_t v = reusableBuffer.calib.midVals[i] - reusableBuffer.calib.loVals[i];
          g_eeGeneral.calib[i].spanNeg = v - v/STICK_TOLERANCE;
          v = reusableBuffer.calib.hiVals[i] - reusableBuffer.calib.midVals[i];
          g_eeGeneral.calib[i].spanPos = v - v/STICK_TOLERANCE;
        }
      }
      break;

    case CALIB_STORE:
      crossfireSharedData.stick_state = reusableBuffer.calib.state;
      g_eeGeneral.chkSum = evalChkSum();
      storageDirty(EE_GENERAL);
      storageCheck(true);
      reusableBuffer.calib.state = CALIB_FINISHED;
      break;

    default:
      reusableBuffer.calib.state = CALIB_START;
      break;
  }

  if( crossfireSharedData.stick_state >= CALIB_SET_P0 && crossfireSharedData.stick_state <= CALIB_SET_P8 )
    doMainScreenGraphics( MAINSCREEN_GRAPHICS_STICKS, force_point_pos );
  else
    doMainScreenGraphics( MAINSCREEN_GRAPHICS_STICKS, 0 );
#endif
}

#if NUM_POTS > 0 || NUM_SLIDERS > 0
void menuCommonCalibPots(event_t event)
{
#if defined(SIMU)
  reusableBuffer.calib.state = CALIB_FINISHED;
#else
  for (uint8_t i=NUM_STICKS; i<NUM_STICKS+NUM_POTS+NUM_SLIDERS; i++) { // get low and high vals for sticks and trims
    int16_t vt = anaIn(i);
    reusableBuffer.calib.loVals[i] = min(vt, reusableBuffer.calib.loVals[i]);
    reusableBuffer.calib.hiVals[i] = max(vt, reusableBuffer.calib.hiVals[i]);
    if (i >= POT1 && i <= POT_LAST) {
      if (IS_POT_WITHOUT_DETENT(i)) {
        reusableBuffer.calib.midVals[i] = (reusableBuffer.calib.hiVals[i] + reusableBuffer.calib.loVals[i]) / 2;
      }
      uint8_t idx = i - POT1;
      int count = reusableBuffer.calib.xpotsCalib[idx].stepsCount;
      if (IS_POT_MULTIPOS(i) && count <= XPOTS_MULTIPOS_COUNT) {
        // use raw analog value for multipos calibraton, anaIn() already has multipos decoded value
        vt = getAnalogValue(i) >> 1;
        if (reusableBuffer.calib.xpotsCalib[idx].lastCount == 0 || vt < reusableBuffer.calib.xpotsCalib[idx].lastPosition - XPOT_DELTA || vt > reusableBuffer.calib.xpotsCalib[idx].lastPosition + XPOT_DELTA) {
          reusableBuffer.calib.xpotsCalib[idx].lastPosition = vt;
          reusableBuffer.calib.xpotsCalib[idx].lastCount = 1;
        }
        else {
          if (reusableBuffer.calib.xpotsCalib[idx].lastCount < 255) reusableBuffer.calib.xpotsCalib[idx].lastCount++;
        }
        if (reusableBuffer.calib.xpotsCalib[idx].lastCount == XPOT_DELAY) {
          int16_t position = reusableBuffer.calib.xpotsCalib[idx].lastPosition;
          bool found = false;
          for (int j=0; j<count; j++) {
            int16_t step = reusableBuffer.calib.xpotsCalib[idx].steps[j];
            if (position >= step-XPOT_DELTA && position <= step+XPOT_DELTA) {
              found = true;
              break;
            }
          }
          if (!found) {
            if (count < XPOTS_MULTIPOS_COUNT) {
              reusableBuffer.calib.xpotsCalib[idx].steps[count] = position;
            }
            reusableBuffer.calib.xpotsCalib[idx].stepsCount += 1;
          }
        }
      }
    }
  }

  menuCalibrationState = reusableBuffer.calib.state; // make sure we don't scroll while calibrating

  switch (event) {
    case EVT_KEY_BREAK(KEY_PAGE):
      chainMenu(menuRadioCalibration);
      break;

    case EVT_ENTRY:
    case EVT_KEY_BREAK(KEY_EXIT):
      reusableBuffer.calib.state = CALIB_START;
      break;

    case EVT_KEY_BREAK(KEY_ENTER):
      if (reusableBuffer.calib.state == CALIB_START)
        reusableBuffer.calib.state = CALIB_SET_MIDPOINT;
      else
        reusableBuffer.calib.state++;
      break;
  }

  switch (reusableBuffer.calib.state) {
    case CALIB_START:
      // START CALIBRATION
      if (!READ_ONLY()) {
        lcdDrawTextAlignedLeft(MENU_HEADER_HEIGHT+2*FH, STR_MENUTOSTART);
      }
      break;

    case CALIB_SET_MIDPOINT:
      // SET MIDPOINT
      lcdDrawText(0*FW, MENU_HEADER_HEIGHT+FH, STR_SETMIDPOINT, INVERS);
      lcdDrawTextAlignedLeft(MENU_HEADER_HEIGHT+2*FH, STR_MENUWHENDONE);
      for (uint8_t i=NUM_STICKS; i<NUM_STICKS+NUM_POTS+NUM_SLIDERS; i++) {
        reusableBuffer.calib.loVals[i] = 15000;
        reusableBuffer.calib.hiVals[i] = -15000;
        reusableBuffer.calib.midVals[i] = getAnalogValue(i) >> 1;
        if (i<NUM_XPOTS) {
          reusableBuffer.calib.xpotsCalib[i].stepsCount = 0;
          reusableBuffer.calib.xpotsCalib[i].lastCount = 0;
        }
      }
      break;

    case CALIB_MOVE_STICKS:
      // MOVE STICKS/POTS
      lcdDrawText(0*FW, MENU_HEADER_HEIGHT+FH, STR_MOVESTICKSPOTS, INVERS);
      lcdDrawTextAlignedLeft(MENU_HEADER_HEIGHT+2*FH, STR_MENUWHENDONE);
      for (uint8_t i=NUM_STICKS; i<NUM_STICKS+NUM_POTS+NUM_SLIDERS; i++) {
        if (abs(reusableBuffer.calib.loVals[i]-reusableBuffer.calib.hiVals[i]) > 50) {
          g_eeGeneral.calib[i].mid = reusableBuffer.calib.midVals[i];
          int16_t v = reusableBuffer.calib.midVals[i] - reusableBuffer.calib.loVals[i];
          g_eeGeneral.calib[i].spanNeg = v - v/STICK_TOLERANCE;
          v = reusableBuffer.calib.hiVals[i] - reusableBuffer.calib.midVals[i];
          g_eeGeneral.calib[i].spanPos = v - v/STICK_TOLERANCE;
        }
      }
      break;

    case CALIB_STORE:
      for (uint8_t i=POT1; i<=POT_LAST; i++) {
        int idx = i - POT1;
        int count = reusableBuffer.calib.xpotsCalib[idx].stepsCount;
        if (IS_POT_MULTIPOS(i)) {
          if (count > 1 && count <= XPOTS_MULTIPOS_COUNT) {
            for (int j=0; j<count; j++) {
              for (int k=j+1; k<count; k++) {
                if (reusableBuffer.calib.xpotsCalib[idx].steps[k] < reusableBuffer.calib.xpotsCalib[idx].steps[j]) {
                  SWAP(reusableBuffer.calib.xpotsCalib[idx].steps[j], reusableBuffer.calib.xpotsCalib[idx].steps[k]);
                }
              }
            }
            StepsCalibData * calib = (StepsCalibData *) &g_eeGeneral.calib[i];
            calib->count = count - 1;
            for (int j=0; j<calib->count; j++) {
              calib->steps[j] = (reusableBuffer.calib.xpotsCalib[idx].steps[j+1] + reusableBuffer.calib.xpotsCalib[idx].steps[j]) >> 5;
            }
          }
          else {
            g_eeGeneral.potsConfig &= ~(0x03<<(2*idx));
          }
        }
      }
      g_eeGeneral.chkSum = evalChkSum();
      storageDirty(EE_GENERAL);
      storageCheck(true);
      reusableBuffer.calib.state = CALIB_FINISHED;
      break;

    default:
      reusableBuffer.calib.state = CALIB_START;
      break;
  }

  doMainScreenGraphics(MAINSCREEN_GRAPHICS_POTS, 0);
#endif
}
#endif

void menuRadioCalibration(event_t event)
{
  char menu_title[32];
  strcpy(&menu_title[0], STR_MENUCALIBRATION);
  menu_title[strlen(STR_MENUCALIBRATION)] = '-';
  strcpy(&menu_title[strlen(STR_MENUCALIBRATION)+1], STR_STICKS);

  check_submenu_simple(event, 0);
  title(menu_title);
  menuCommonCalib(READ_ONLY() ? 0 : event);
  if (menuEvent) {
    menuCalibrationState = CALIB_START;
  }
}

#if NUM_POTS > 0
void menuPotsCalibration(event_t event)
{
  char menu_title[32];
  strcpy(&menu_title[0], STR_MENUCALIBRATION);
  menu_title[strlen(STR_MENUCALIBRATION)] = '-';
  strcpy(&menu_title[strlen(STR_MENUCALIBRATION)+1], STR_POTS);

  check_submenu_simple(event, 0);
  title(menu_title);
  menuCommonCalibPots(READ_ONLY() ? 0 : event);
  if (menuEvent) {
    menuCalibrationState = CALIB_START;
  }
}
#endif

void menuFirstCalib(event_t event)
{
  if (event == EVT_KEY_BREAK(KEY_EXIT) || reusableBuffer.calib.state == CALIB_FINISHED) {
    menuCalibrationState = CALIB_START;
    chainMenu(menuMainView);
  }
  else {
    lcdDrawText(LCD_W / 2, 0, STR_MENUCALIBRATION, CENTERED);
    lcdInvertLine(0);
    menuCommonCalib(event);
  }
}
