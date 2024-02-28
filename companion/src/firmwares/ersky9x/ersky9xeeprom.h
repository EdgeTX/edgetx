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

#ifndef _ERSKY9XEEPROM_H_
#define _ERSKY9XEEPROM_H_

#include <inttypes.h>
#include "eeprominterface.h"

extern RawSwitch getEr9xTimerMode(int mode);

//eeprom data
#define ERSKY9X_MAX_MIXERS_V10  32
#define ERSKY9X_MAX_MIXERS_V11  48
#define ERSKY9X_MAX_CURVE5  8
#define ERSKY9X_MAX_CURVE9  8
#define ERSKY9X_MAX_CSFUNC 13

#define ERSKY9X_MDVERS10      10
#define ERSKY9X_MDVERS11      11

#define ERSKY9X_GENERAL_OWNER_NAME_LEN  10
#define ERSKY9X_NUM_CHNOUT_V10    16 //number of real outputchannels CH1-CH8
#define ERSKY9X_NUM_CHNOUT_V11    24 //number of real outputchannels CH1-CH8
#define ERSKY9X_NUM_CSW_V10         12 //number of custom switches
#define ERSKY9X_NUM_CSW_V11         24
#define ERSKY9X_NUM_FSW                 16
#define ERSKY9X_NUM_VOICE	         8
#define ERSKY9X_MAX_GVARS	         7

#define CPN_MAX_STICKSnPOTS 7  //number of sticks and pots

PACK(typedef struct t_Ersky9xTrainerMix {
  uint8_t srcChn:3; //0-7 = ch1-8
  int8_t  swtch:5;
  int8_t  studWeight:6;
  uint8_t mode:2;   // off, add-mode, subst-mode

  operator TrainerMix();
  t_Ersky9xTrainerMix();
}) Ersky9xTrainerMix; //

PACK(typedef struct t_Ersky9xTrainerData {
  int16_t        calib[4];
  Ersky9xTrainerMix mix[4];

  operator TrainerData();
  t_Ersky9xTrainerData();
}) Ersky9xTrainerData;

PACK(typedef struct t_Ersky9xGeneral {
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
  Ersky9xTrainerData trainer;
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
  uint8_t   alarmsFlash:1;
  uint8_t   disableSplashScreen:1;
  uint8_t   disablePotScroll:1;
  uint8_t   disableBG:1;
  uint8_t   res1:1;
  uint8_t   filterInput;
  uint8_t   lightAutoOff;
  uint8_t   templateSetup;  //RETA order according to chout_ar array
  int8_t    PPM_Multiplier;
  uint8_t   FRSkyYellow:4;
  uint8_t   FRSkyOrange:4;
  uint8_t   FRSkyRed:4;  //mike please check these are correct
  uint8_t   hideNameOnSplash:1;
  uint8_t   optrexDisplay:1;
  uint8_t   spare:2;
  uint8_t   speakerPitch;
  uint8_t   hapticStrength;
  uint8_t   speakerMode;
  uint8_t   lightOnStickMove;

  char      ownerName[10];
  uint8_t   switchWarningStates;
  uint8_t	 volume ;
  uint8_t   bright ;			// backlight
  uint8_t   stickGain;
  uint8_t	 mAh_alarm ;
  uint16_t mAh_used ;
  uint16_t run_time ;
  int8_t	 current_calib ;
  uint8_t	 bt_baudrate ;
  uint8_t	 rotaryDivisor ;
  operator GeneralSettings();
  t_Ersky9xGeneral();
}) Ersky9xGeneral;

PACK(typedef struct t_Ersky9xExpoData {
  int8_t  expo[3][2][2];
  int8_t  drSw1;
  int8_t  drSw2;

  t_Ersky9xExpoData() { memset(reinterpret_cast<void *>(this), 0, sizeof(t_Ersky9xExpoData)); }
}) Ersky9xExpoData;


PACK(typedef struct t_Ersky9xLimitData {
  int8_t  min;
  int8_t  max;
  bool    revert;
  int16_t  offset;

  operator LimitData();
  t_Ersky9xLimitData() ;
}) Ersky9xLimitData;

#define MLTPX_ADD  0
#define MLTPX_MUL  1
#define MLTPX_REP  2

PACK(typedef struct t_Ersky9xMixData_v10 {
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
  uint8_t mltpx:3;           // multiplex method 0=+ 1=* 2=replace
  uint8_t mixWarn:2;         // mixer warning
  uint8_t enableFmTrim:1;
  uint8_t mixres:1;
  int8_t  sOffset;
  int8_t  res;

  operator MixData();
  t_Ersky9xMixData_v10();
}) Ersky9xMixData_v10;

PACK(typedef struct t_Ersky9xMixData_v11 {
  uint8_t destCh;            //        1..CPN_MAX_CHNOUT
  uint8_t srcRaw;            //
  int8_t  weight;
  int8_t  swtch;
  uint8_t curve;             //0=symmetrisch 1=no neg 2=no pos
  uint8_t delayUp;
  uint8_t delayDown;
  uint8_t speedUp;         // Servogeschwindigkeit aus Tabelle (10ms Cycle)
  uint8_t speedDown;       // 0 nichts
  uint8_t carryTrim:1;
  uint8_t mltpx:3;           // multiplex method 0=+ 1=* 2=replace
  uint8_t mixWarn:2;         // mixer warning
  uint8_t enableFmTrim:1;
  uint8_t mixres:1;
  int8_t  sOffset;
  uint8_t  res[4];
  operator MixData();
  t_Ersky9xMixData_v11();
}) Ersky9xMixData_v11;

PACK(typedef struct t_Ersky9xLogicalSwitchData_v10 { // Custom Switches data
  int8_t  v1; //input
  int8_t  v2; //offset
  uint8_t func;

  operator LogicalSwitchData();
  t_Ersky9xLogicalSwitchData_v10() { memset(reinterpret_cast<void *>(this), 0, sizeof(t_Ersky9xLogicalSwitchData_v10)); }
}) Ersky9xLogicalSwitchData_v10;

PACK(typedef struct t_Ersky9xLogicalSwitchData_v11 { // Custom Switches data
  int8_t  v1; //input
  int8_t  v2; 		//offset
  uint8_t func;
  uint8_t andsw;
  uint8_t res ;

  operator LogicalSwitchData();
  t_Ersky9xLogicalSwitchData_v11() { memset(reinterpret_cast<void *>(this), 0, sizeof(t_Ersky9xLogicalSwitchData_v11)); }
}) Ersky9xLogicalSwitchData_v11;

PACK(typedef struct t_Ersky9xSafetySwData_v10 { // Custom Switches data
  int8_t  swtch;
  int8_t  val;

  t_Ersky9xSafetySwData_v10();
}) Ersky9xSafetySwData_v10;

PACK(typedef struct t_Ersky9xSafetySwData_v11 { // Custom Switches data
  union opt {
    struct ss {
      int8_t  swtch ;
      uint8_t mode ;
      int8_t  val ;
      uint8_t res ;
    } ss ;
    struct vs {
      uint8_t vswtch ;
      uint8_t vmode ; // ON, OFF, BOTH, 15Secs, 30Secs, 60Secs, Varibl
      uint8_t vval;
      uint8_t vres ;
    } vs ;
  } opt ;

  t_Ersky9xSafetySwData_v11();
}) Ersky9xSafetySwData_v11;

PACK(typedef struct t_Ersky9xgvar {
  int8_t gvar ;
  uint8_t gvsource ;
//	int8_t gvswitch ;
}) Ersky9xGvarData ;

PACK(typedef struct t_Ersky9xFlightModeData {
  int16_t trim[4];     // -500..500 => trim value, 501 => use trim of phase 0, 502, 503, 504 => use trim of phases 1|2|3|4 instead
  int8_t swtch;       // swtch of phase[0] is not used
  char name[6];
  uint8_t fadeIn:4;
  uint8_t fadeOut:4;
  uint16_t spare ;		// Future expansion
}) Ersky9xFlightModeData;

PACK(typedef struct t_Ersky9xCustomFunctionData { // Function Switches data
  int8_t  swtch; //input
  uint8_t func;
  char param[6];
  uint8_t delay;
  uint8_t spare;
}) Ersky9xCustomFunctionData;

PACK(typedef struct t_Ersky9xFrSkyChannelData_v10 {
  uint8_t   ratio;                // 0.0 means not used, 0.1V steps EG. 6.6 Volts = 66. 25.1V = 251, etc.
  uint8_t   alarms_value[2];      // 0.1V steps EG. 6.6 Volts = 66. 25.1V = 251, etc.
  uint8_t   alarms_level:4;
  uint8_t   alarms_greater:2;     // 0=LT(<), 1=GT(>)
  uint8_t   type:2;               // future use: 0=volts, ...

  operator FrSkyChannelData();
  t_Ersky9xFrSkyChannelData_v10();
}) Ersky9xFrSkyChannelData_v10;

PACK(typedef struct t_Ersky9xFrSkyData_v10 {
  Ersky9xFrSkyChannelData_v10 channels[2];

  operator FrSkyData();
  t_Ersky9xFrSkyData_v10();
}) Ersky9xFrSkyData_v10;

PACK(typedef struct t_Ersky9xFrSkyChannelData_v11 {
  uint8_t   ratio ;               // 0.0 means not used, 0.1V steps EG. 6.6 Volts = 66. 25.1V = 251, etc.
  uint8_t   offset ;              //
  uint8_t   gain ;                //
  uint8_t   alarms_value[2] ;     // 0.1V steps EG. 6.6 Volts = 66. 25.1V = 251, etc.
  uint8_t   alarms_level ;
  uint8_t   alarms_greater ;      // 0=LT(<), 1=GT(>)
  uint8_t   type ;                // 0=volts, 1=raw, 2=volts*2, 3=Amps

  operator FrSkyChannelData();
  t_Ersky9xFrSkyChannelData_v11();
}) Ersky9xFrSkyChannelData_v11;

PACK(typedef struct t_Ersky9xFrSkyData_v11 {
  Ersky9xFrSkyChannelData_v11 channels[2];

  operator FrSkyData();
  t_Ersky9xFrSkyData_v11();
}) Ersky9xFrSkyData_v11;

PACK(typedef struct t_Ersky9xFrSkyalarms_v11 {
  uint8_t frskyAlarmType ;
  uint8_t frskyAlarmLimit ;
  uint8_t frskyAlarmSound ;
}) Ersky9xFrSkyAlData_v11;

PACK(typedef struct t_Ersky9xFrSkyAlarmData_v11 {
  Ersky9xFrSkyAlData_v11 alarmData[8] ;
}) Ersky9xFrSkyAlarmData_v11;

PACK(typedef struct t_Ersky9xTimerMode_v10 {
    uint8_t   tmrModeA:7 ;          // timer trigger source -> off, abs, stk, stk%, cx%
    uint8_t   tmrDir:1 ;                                                // Timer direction
    int8_t    tmrModeB ;            // timer trigger source -> !sw, none, sw, m_sw
    uint16_t  tmrVal ;
}) Ersky9xTimerMode_v10;

PACK(typedef struct t_Ersky9xTimerMode_v11 {
    uint8_t tmrModeA:7 ;          // timer trigger source -> off, abs, stk, stk%, cx%
    uint8_t tmrDir:1 ;                                                // Timer direction
    int8_t tmrModeB ;            // timer trigger source -> !sw, none, sw, m_sw
    uint16_t tmrVal ;
}) Ersky9xTimerMode_v11;

PACK(typedef struct te_Ersky9xswVoice {
  uint8_t swtch ;
  uint8_t mode ; // ON, OFF, BOTH, 15Secs, 30Secs, 60Secs
  uint8_t val ;
  uint8_t vres ;
}) Ersky9xvoiceSwData ;

PACK(typedef struct t_Ersky9xModelData_v10 {
  char      name[10];             // 10 must be first for eeLoadModelName
  uint8_t   reserved_spare;               //used to be MDVERS - now depreciated
  int8_t    spare21;          // was timer trigger source -> off, abs, stk, stk%, sw/!sw, !m_sw/!m_sw
  //    uint8_t   sparex:1;     // was tmrDir, now use tmrVal>0 => count down
  uint8_t   sparex:1;                         // was tmrDir, now use tmrVal>0 => count down
  //    uint8_t   tmrDir:1;     // was tmrDir, now use tmrVal>0 => count down
  uint8_t   traineron:1;                // 0 disable trainer, 1 allow trainer
  uint8_t   spare22:1 ;                         // Start timer2 using throttle
  uint8_t   FrSkyUsrProto:1 ; // Protocol in FrSky User Data, 0=FrSky Hub, 1=WS HowHigh
  uint8_t   FrSkyGpsAlt:1 ;             // Use Gps Altitude as main altitude reading
  uint8_t   FrSkyImperial:1 ; // Convert FrSky values to imperial units
  uint8_t   FrSkyAltAlarm:2;
  uint16_t  spare_u16 ;                         // Was timerval
  uint8_t   protocol;
  int8_t    ppmNCH;
  int8_t    thrTrim:4;        // Enable Throttle Trim
  int8_t    thrExpo:4;        // Enable Throttle Expo
  int8_t    trimInc;          // Trim Increments
  int8_t    ppmDelay;
  int8_t    trimSw;
  uint8_t   beepANACenter;    // 1<<0->A1.. 1<<6->A7
  uint8_t   pulsePol:1;
  uint8_t   extendedLimits:1;
  uint8_t   swashInvertELE:1;
  uint8_t   swashInvertAIL:1;
  uint8_t   swashInvertCOL:1;
  uint8_t   swashType:3;
  uint8_t   swashCollectiveSource;
  uint8_t   swashRingValue;
  int8_t    ppmFrameLength;   //0=22.5  (10msec-30msec) 0.5msec increments
  Ersky9xMixData_v10   mixData[ERSKY9X_MAX_MIXERS_V10];
  Ersky9xLimitData limitData[ERSKY9X_NUM_CHNOUT_V10];
  Ersky9xExpoData expoData[4];
  int8_t    trim[4];
  int8_t    curves5[ERSKY9X_MAX_CURVE5][5];
  int8_t    curves9[ERSKY9X_MAX_CURVE9][9];
  Ersky9xLogicalSwitchData_v10   logicalSw[ERSKY9X_NUM_CSW_V10];
  uint8_t   frSkyVoltThreshold ;
  uint8_t   res3[2];
  Ersky9xSafetySwData_v10  safetySw[ERSKY9X_NUM_CHNOUT_V10];
  Ersky9xFrSkyData_v10 frsky;
  Ersky9xTimerMode_v10 timer[2] ;
  operator ModelData();
  t_Ersky9xModelData_v10() { memset(reinterpret_cast<void *>(this), 0, sizeof(t_Ersky9xModelData_v10)); }
}) Ersky9xModelData_v10;

PACK(typedef struct t_Ersky9xModelData_v11 {
  char      name[10];                    // 10 must be first for eeLoadModelName
  uint8_t   modelVoice ;               // Index to model name voice (261+value)
  uint8_t   RxNum ;                     // was timer trigger source, now RxNum for model match
  uint8_t   sparex:1;                    // was tmrDir, now use tmrVal>0 => count down
  uint8_t   traineron:1;                 // 0 disable trainer, 1 allow trainer
  uint8_t   spare22:1 ;                 // Start timer2 using throttle
  uint8_t   FrSkyUsrProto:1 ;       // Protocol in FrSky User Data, 0=FrSky Hub, 1=WS HowHigh
  uint8_t   FrSkyGpsAlt:1 ;          // Use Gps Altitude as main altitude reading
  uint8_t   FrSkyImperial:1 ;         // Convert FrSky values to imperial units
  uint8_t   FrSkyAltAlarm:2;
  uint8_t   version ;
  uint8_t   protocol;
  int8_t    ppmNCH;
  uint8_t   thrTrim:1;            // Enable Throttle Trim
  uint8_t   numBlades:2;					// RPM scaling
  uint8_t   spare10:1;
  uint8_t   thrExpo:1;            // Enable Throttle Expo
  uint8_t   spare11:3;
  int8_t    trimInc;          // Trim Increments
  int8_t    ppmDelay;
  int8_t    trimSw;
  uint8_t   beepANACenter;    // 1<<0->A1.. 1<<6->A7
  uint8_t   pulsePol:1;
  uint8_t   extendedLimits:1;
  uint8_t   swashInvertELE:1;
  uint8_t   swashInvertAIL:1;
  uint8_t   swashInvertCOL:1;
  uint8_t   swashType:3;
  uint8_t   swashCollectiveSource;
  uint8_t   swashRingValue;
  int8_t    ppmFrameLength;   //0=22.5  (10msec-30msec) 0.5msec increments
  Ersky9xMixData_v11 mixData[ERSKY9X_MAX_MIXERS_V11];
  Ersky9xLimitData limitData[ERSKY9X_NUM_CHNOUT_V11];
  Ersky9xExpoData expoData[4];
  int8_t    trim[4];
  int8_t    curves5[ERSKY9X_MAX_CURVE5][5];
  int8_t    curves9[ERSKY9X_MAX_CURVE9][9];
  int8_t    curvexy[18];
  Ersky9xLogicalSwitchData_v11   logicalSw[ERSKY9X_NUM_CSW_V11];
  uint8_t   frSkyVoltThreshold ;
  uint8_t   bt_telemetry;
  uint8_t   numVoice;		// 0-16, rest are Safety switches
  Ersky9xSafetySwData_v11  safetySw[ERSKY9X_NUM_CHNOUT_V11];
  Ersky9xvoiceSwData voiceSwitches[ERSKY9X_NUM_VOICE] ;
  Ersky9xFrSkyData_v11 frsky;
  Ersky9xTimerMode_v11 timer[2] ;
  Ersky9xFrSkyAlarmData_v11 frskyAlarms ;
// Add 6 bytes for custom telemetry screen
  uint8_t customDisplayIndex[6] ;
  Ersky9xCustomFunctionData   customFn[ERSKY9X_NUM_FSW];
  Ersky9xFlightModeData flightModeData[6] ;
  Ersky9xGvarData gvars[ERSKY9X_MAX_GVARS] ;

  operator ModelData();
  t_Ersky9xModelData_v11() { memset(reinterpret_cast<void *>(this), 0, sizeof(t_Ersky9xModelData_v11)); }
}) Ersky9xModelData_v11;

#endif // _ERSKY9XEEPROM_H_
