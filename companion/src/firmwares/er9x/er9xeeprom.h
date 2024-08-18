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

#ifndef _ER9XEEPROM_H_
#define _ER9XEEPROM_H_

#include <inttypes.h>
#include "eeprominterface.h"

//eeprom data
#define ER9X_MAX_MIXERS  32
#define ER9X_MAX_CURVE5  8
#define ER9X_MAX_CURVE9  8
#define ER9X_MAX_CSFUNC  13

#define MDVERS      10

#define ER9X_NUM_CHNOUT      16 //number of real outputchannels CH1-CH8
#define ER9X_NUM_CSW         12 //number of custom switches
#define CPN_MAX_STICKSnPOTS  7  //number of sticks and pots
#define ER9X_MAX_GVARS 7

#define ER9X_MAX_MODES		4

PACK(typedef struct t_Er9xTrainerMix {
  uint8_t srcChn:3; //0-7 = ch1-8
  int8_t  swtch:5;
  int8_t  studWeight:6;
  uint8_t mode:2;   // off, add-mode, subst-mode

  operator TrainerMix();
  t_Er9xTrainerMix();
}) Er9xTrainerMix; //

PACK(typedef struct t_Er9xTrainerData {
  int16_t        calib[4];
  Er9xTrainerMix mix[4];

  operator TrainerData();
  t_Er9xTrainerData();
}) Er9xTrainerData;

PACK(typedef struct t_Er9xGeneral {
  uint8_t   myVers;
  int16_t   calibMid[CPN_MAX_STICKSnPOTS];
  int16_t   calibSpanNeg[CPN_MAX_STICKSnPOTS];
  int16_t   calibSpanPos[CPN_MAX_STICKSnPOTS];
  uint16_t  chkSum;
  uint8_t   currModel; //0..15
  uint8_t   contrast;
  uint8_t   vBatWarn;
  int8_t    txVoltageCalibration;
  int8_t    lightSw;
  Er9xTrainerData trainer;
  uint8_t   view;
  uint8_t   disableThrottleWarning:1;
  uint8_t   disableSwitchWarning:1;
  uint8_t   disableMemoryWarning:1;
  uint8_t   beeperVal:3;
  uint8_t   reserveWarning:1;
  uint8_t   disableAlarmWarning:1;
  uint8_t   stickMode;
  int8_t    inactivityTimer;
  uint8_t   throttleReversed:1;
  uint8_t   minuteBeep:1;
  uint8_t   preBeep:1;
  uint8_t   flashBeep:1;
  uint8_t   disableSplashScreen:1;
  uint8_t   disablePotScroll:1;
  uint8_t   disableBG:1;
  uint8_t   frskyinternalalarm:1;
  uint8_t   spare_filter ;		// No longer needed, left for eepe compatibility for now
  uint8_t   lightAutoOff;
  uint8_t   templateSetup;  //RETA order according to chout_ar array
  int8_t    PPM_Multiplier;
  uint8_t unused1;
  uint8_t unused2:4;
  uint8_t hideNameOnSplash:1;
  uint8_t enablePpmsim:1;
  uint8_t blightinv:1;
  uint8_t stickScroll:1;
  uint8_t speakerPitch;
  uint8_t hapticStrength;
  uint8_t speakerMode;
  uint8_t lightOnStickMove;

  char     ownerName[10];
  uint8_t switchWarningStates;
  uint8_t volume ;
  uint8_t res[3];
  uint8_t crosstrim:1;
  uint8_t spare1:7;  

  operator GeneralSettings();
  t_Er9xGeneral();
}) Er9xGeneral;

PACK(typedef struct t_Er9xExpoData {
  int8_t  expo[3][2][2];
  int8_t  drSw1;
  int8_t  drSw2;

  t_Er9xExpoData() { memset(this, 0, sizeof(t_Er9xExpoData)); }
}) Er9xExpoData;


PACK(typedef struct t_Er9xLimitData {
  int8_t  min;
  int8_t  max;
  bool    revert;
  int16_t  offset;

  operator LimitData();
  t_Er9xLimitData();
}) Er9xLimitData;

#define MLTPX_ADD  0
#define MLTPX_MUL  1
#define MLTPX_REP  2

PACK(typedef struct t_Er9xMixData {
  uint8_t destCh;            //        1..CPN_MAX_CHNOUT
  uint8_t srcRaw;            //
  int8_t  weight;
  int8_t  swtch;
  uint8_t curve;             //0=symmetrisch 1=no neg 2=no pos
  uint8_t delayUp:4;
  uint8_t delayDown:4;
  uint8_t speedUp:4;         // Servogeschwindigkeit aus Tabelle (10ms Cycle)
  uint8_t speedDown:4;       // 0 nichts
  uint8_t carryTrim:1;
  uint8_t mltpx:2;           // multiplex method 0=+ 1=* 2=replace
  uint8_t lateOffset:1;      // Add offset later
  uint8_t mixWarn:2;         // mixer warning
  uint8_t enableFmTrim:1;
  uint8_t differential:1;
  int8_t  sOffset;
  int8_t  res;

  operator MixData();
  t_Er9xMixData();
}) Er9xMixData;


PACK(typedef struct t_Er9xLogicalSwitchData { // Custom Switches data
  int8_t  v1; //input
  int8_t  v2; //offset
  uint8_t func:4;
  uint8_t andsw:4;

  operator LogicalSwitchData();
  t_Er9xLogicalSwitchData() { memset(this, 0, sizeof(t_Er9xLogicalSwitchData)); }
}) Er9xLogicalSwitchData;

PACK(typedef struct t_Er9xSafetySwData { // Custom Switches data
  int8_t  swtch;
  int8_t  val;

  t_Er9xSafetySwData();
}) Er9xSafetySwData;

PACK(typedef struct t_Er9xFrSkyChannelData {
  uint8_t   ratio;                // 0.0 means not used, 0.1V steps EG. 6.6 Volts = 66. 25.1V = 251, etc.
  uint8_t   alarms_value[2];      // 0.1V steps EG. 6.6 Volts = 66. 25.1V = 251, etc.
  uint8_t   alarms_level:4;
  uint8_t   alarms_greater:2;     // 0=LT(<), 1=GT(>)
  uint8_t   type:2;               // future use: 0=volts, ...

  operator FrSkyChannelData();
  t_Er9xFrSkyChannelData();
}) Er9xFrSkyChannelData;

PACK(typedef struct t_Er9xFrSkyData {
  Er9xFrSkyChannelData channels[2];

  operator FrSkyData();
  t_Er9xFrSkyData();
}) Er9xFrSkyData;

PACK(typedef struct t_gvar {
  int8_t gvar ;
  uint8_t gvsource ;
//	int8_t gvswitch ;
}) Er9xGvarData ;

PACK(typedef struct t_FlightModeData {
	// Trim store as -1001 to -1, trim value-501, 0-5 use trim of phase 0-5
  int16_t trim[4];     // -500..500 => trim value, 501 => use trim of phase 0, 502, 503, 504 => use trim of modes 1|2|3|4 instead
  int8_t swtch;        // Try 0-5 use trim of phase 0-5, 1000-2000, trim + 1500 ???
  uint8_t fadeIn:4;
  uint8_t fadeOut:4;
}) Er9xFlightModeData;

PACK(typedef struct t_Er9xModelData {
  char      name[10];             // 10 must be first for eeLoadModelName
  uint8_t   modelVoice ;		// Index to model name voice (260+value)
  int8_t    tmrMode;              // timer trigger source -> off, abs, stk, stk%, sw/!sw, !m_sw/!m_sw
  uint8_t   tmrDir:1;    //0=>Count Down, 1=>Count Up
  uint8_t   traineron:1;  // 0 disable trainer, 1 allow trainer
  uint8_t   t2throttle:1 ;  // Start timer2 using throttle
  uint8_t   FrSkyUsrProto:1 ;  // Protocol in FrSky User Data, 0=FrSky Hub, 1=WS HowHigh
  uint8_t   FrSkyGpsAlt:1 ;  	// Use Gps Altitude as main altitude reading
  uint8_t   FrSkyImperial:1 ;  // Convert FrSky values to imperial units
  uint8_t   FrSkyAltAlarm:2;
  uint16_t  tmrVal;
  uint8_t   protocol;
  int8_t    ppmNCH;
  uint8_t   thrTrim:1;            // Enable Throttle Trim
  uint8_t   xnumBlades:2;					// RPM scaling
  uint8_t   spare10:1;
  uint8_t   thrExpo:1;            // Enable Throttle Expo
  uint8_t   ppmStart:3 ;					// Start channel for PPM
  int8_t    trimInc;              // Trim Increments
  int8_t    ppmDelay;
  int8_t    trimSw;
  uint8_t   beepANACenter;        // 1<<0->A1.. 1<<6->A7
  uint8_t   pulsePol:1;
  uint8_t   extendedLimits:1;
  uint8_t   swashInvertELE:1;
  uint8_t   swashInvertAIL:1;
  uint8_t   swashInvertCOL:1;
  uint8_t   swashType:3;
  uint8_t   swashCollectiveSource;
  uint8_t   swashRingValue;
  int8_t    ppmFrameLength;
  Er9xMixData   mixData[ER9X_MAX_MIXERS];
  Er9xLimitData limitData[ER9X_NUM_CHNOUT];
  Er9xExpoData  expoData[4];
  int8_t    trim[4];
  int8_t    curves5[ER9X_MAX_CURVE5][5];
  int8_t    curves9[ER9X_MAX_CURVE9][9];
  Er9xLogicalSwitchData   logicalSw[ER9X_NUM_CSW];
  uint8_t   frSkyVoltThreshold ;
  int8_t   tmrModeB;
  uint8_t   numVoice;
  Er9xSafetySwData  safetySw[ER9X_NUM_CHNOUT];
  Er9xFrSkyData frsky;
  uint8_t numBlades ;
  uint8_t unused1[8] ;
  uint8_t CustomDisplayIndex[6] ;
  Er9xGvarData gvars[ER9X_MAX_GVARS] ;
  Er9xFlightModeData flightModeData[ER9X_MAX_MODES] ;
  
  operator ModelData();
  t_Er9xModelData() { memset(reinterpret_cast<void *>(this), 0, sizeof(t_Er9xModelData)); }
}) Er9xModelData;

#endif // _ER9XEEPROM_H_
