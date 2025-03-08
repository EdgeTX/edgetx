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

#if defined(RADIO_V16)
#include "menu_model.h"
#include "menu_radio.h"

//#include "radio_CurrentSensor.h"
#include "view_channels.h"
#include "model_telemetry.h"

#include "layer.h"
#endif

#include "CI1302.h"
#include "imu_42627.h"

#include "delays_driver.h"

#include "timers_driver.h"

#include "debug.h"

#include "audio.h"

//#include <stdlib.h>
//#include <string.h>


#define VOICE_FRAME_DELAY_US 500

//cn：0x55 0xAA	0x01 - 0xFF	0xFF-0x01	0xA5 0x5A
//en：0x55 0xAB	‘’	        ‘’	      0xA5 0x5A


#define VOICE_START_BYTE        0x55
#define VOICE_STARTCN_BYTE      0xAA
#define VOICE_STARTEN_BYTE      0xAB

#define VOICE_FRAME_SIZE        6

#define VOICE_END_BYTE1         0xA5
#define VOICE_END_BYTE2         0x5A

//COMMAND
#define VOICE_WKUP                      0x01

#define VOICE_password	                0x02
#define VOICE_Aileron_adjustment	      0x03
#define VOICE_Pitch_adjustment	        0x04
#define VOICE_Rotating_adjustment	      0x05

#define VOICE_Up	                      0x0A
#define VOICE_Down	                    0x0B

#define VOICE_Left_one	                0x14
#define VOICE_Right_one	                0x15

#define VOICE_Confirm                   0x1A
#define VOICE_Cancel                    0x1B

#define VOICE_Model_Menu                0x21
#define VOICE_System_Menu               0x22
#define VOICE_Channel_Setup             0x23

#define VOICE_Channel_Monitor           0X32
#define VOICE_Internal_Module           0X33
#define VOICE_External_Module           0X34

#define VOICE_Aileron_Reverse	          0x4A
#define VOICE_Elevator_Reverse	        0x4B
#define VOICE_Rudder_Reverse	          0x4C

#define VOICE_Motion_Control	          0x55
#define VOICE_Telemetry                 0x5B

#define VOICE_Open_Gear                 0x7E
#define VOICE_Close_Gear                0x80
#define VOICE_Open_Flap                 0x82
#define VOICE_Close_Flap                0x85

#define VOICE_POWEROFF                  0x9E

#define VOICE_Start_recording	          0x88
#define VOICE_End_recording	            0x89
#define VOICE_Photo	                    0x8A
#define VOICE_Take_a_Photo	            0x8B
#define VOICE_go_home	                  0x8C
#define VOICE_GPS_Return	              0x91
	
#define VOICE_hi_take_off	              0x9F
#define VOICE_hi_land	                  0xA1
#define VOICE_Sensorstatus	            0xA2

struct voice_cmd_inputs_t {
  uint8_t VOICESTARTBYTE1;
  uint8_t VOICELanguage;
  uint8_t VOICECMD;
  uint8_t VOICECMDR;
  uint8_t VOICEENDBYTE1;
  uint8_t VOICEENDBYTE2;
};

struct voice_cmmand {
  uint8_t indexid;
  char    *name;
};
static const voice_cmmand _voice_inputs_list[] = {
  {VOICE_WKUP,                    (char*)"wkup"},
  {VOICE_password,                (char*)"password"},
  {VOICE_Aileron_adjustment,      (char*)"Aileron adjustment"},
  {VOICE_Pitch_adjustment,        (char*)"Pitch adjustment"},
  {VOICE_Rotating_adjustment,     (char*)"Rotating adjustment"},
  {VOICE_Up,                      (char*)"Up"},
  {VOICE_Down,                    (char*)"Down"},
  {VOICE_Left_one,                (char*)"Left"},
  {VOICE_Right_one,               (char*)"Right"},
  {VOICE_Confirm,                 (char*)"Confirm"},
  {VOICE_Cancel,                  (char*)"Cancel"},
  {VOICE_Model_Menu,              (char*)"Model Menu"},
  {VOICE_System_Menu,             (char*)"System Menu"},
  {VOICE_Channel_Setup,           (char*)"Channel Setup"},
  {VOICE_Channel_Monitor,         (char*)"Channel Monitor"},
  {VOICE_Aileron_Reverse,         (char*)"Ail Reverse"},
  {VOICE_Elevator_Reverse,        (char*)"Ele Reverse"},
  {VOICE_Rudder_Reverse,          (char*)"Rud Reverse"},
  {VOICE_Motion_Control,          (char*)"Motion Control"},
  {VOICE_Telemetry,               (char*)"Telemetry"},
  {VOICE_Open_Gear,               (char*)"Open Gear"},
  {VOICE_Close_Gear,              (char*)"Close Gaer"},
  {VOICE_Open_Flap,               (char*)"Open Flap"},
  {VOICE_Close_Flap,              (char*)"Close Flap"},
  {VOICE_POWEROFF,                (char*)"Power off"},
  {VOICE_Start_recording,	        (char*)"Start recording"},
  {VOICE_End_recording,	          (char*)"End recording"},
  {VOICE_Photo,	                  (char*)"Photo"},
  {VOICE_Take_a_Photo,	          (char*)"Take a Photo"},
  {VOICE_go_home,	                (char*)"go home"},
  {VOICE_GPS_Return,	            (char*)"GPS Return"},
  {VOICE_hi_take_off,	            (char*)"hi take off"},
  {VOICE_hi_land,	                (char*)"hi land"},
  {VOICE_Sensorstatus,	          (char*)"Sensor status"}
};

bool VoiceRunStatus=true;
bool VoiceGearStatus=false;
uint8_t VoiceFlapStatus=false;

bool MotionControlStatus=false;

static uint32_t wkup_delay=0;

static uint32_t Ail_adjustment_delay=0;
static uint32_t Ele_adjustment_delay=0;
static uint32_t Rud_adjustment_delay=0;

static uint32_t Ail_adjustment_redelay=0;
static uint32_t Ele_adjustment_redelay=0;
static uint32_t Rud_adjustment_redelay=0;

static uint16_t repushTrimEvent=0;

#define wkup_delayms  10000   //10S

extern uint8_t GetstickMode(void);
extern void pushTrimEvent(event_t evt);

extern volatile rotenc_t rotencValue;

uint8_t getvoivech56switch(void);


void AUDIOExtWarn(void)
{
  //AUDIO_ExtWarn();
}
void AUDIOExtWarncut(void)
{
  //AUDIO_ExtWarncut();
}

// Range for pulses (ppm input) is [-512:+512]
void processVoiceFrame(uint8_t * voicedata, uint32_t size)
{
//cn：0x55 0xAA	0x01 - 0xFF	0xFF-0x01	0xA5 0x5A
//en：0x55 0xAB	‘’	        ‘’	      0xA5 0x5A
//55 AB 01 FE A5 5A   helloradio

  voice_cmd_inputs_t *voiceframe=(voice_cmd_inputs_t*)voicedata;

  if (size != VOICE_FRAME_SIZE || voiceframe->VOICESTARTBYTE1 != VOICE_START_BYTE ||
      voiceframe->VOICEENDBYTE1 != VOICE_END_BYTE1||
      voiceframe->VOICEENDBYTE2 != VOICE_END_BYTE2) {

    return;  // not a valid voice frame
  }
  if((voiceframe->VOICECMD+voiceframe->VOICECMDR)!=0xff)return;
  if(voiceframe->VOICELanguage==VOICE_STARTEN_BYTE||voiceframe->VOICELanguage==VOICE_STARTCN_BYTE)
  {
      if(voiceframe->VOICECMD==VOICE_WKUP){
        Ail_adjustment_delay=0;
        Ele_adjustment_delay=0;
        Rud_adjustment_delay=0;
        wkup_delay=timersGetMsTick();

        //AUDIO_HELLORADIO();
      }
      else if(voiceframe->VOICECMD==VOICE_Open_Gear){
        wkup_delay=timersGetMsTick(); //updata wkup timer

        if (!getvoivech56switch()){
        if(VoiceGearStatus==false)
        {
          //AUDIO_OKAY();
          //AUDIO_OPENGEAR();
        }
        else {
          //AUDIO_OPENED();
          }
        VoiceGearStatus=true;
        }
      }
      else if(voiceframe->VOICECMD==VOICE_Close_Gear){
        wkup_delay=timersGetMsTick(); //updata wkup timer
        if (!getvoivech56switch()){
        if(VoiceGearStatus){
          //AUDIO_CLOSEGEAR();
        }
        else {
          //AUDIO_CLOSED();
        }
        VoiceGearStatus=false;
        }
      }
      else if(voiceframe->VOICECMD==VOICE_Open_Flap){
        wkup_delay=timersGetMsTick(); //updata wkup timer
        if (!getvoivech56switch()){
        if(VoiceFlapStatus==0)
        {
          VoiceFlapStatus=1;
          {
            //AUDIO_OKAY();
            //AUDIO_OPENFLAP();
          }
        }
        else if(VoiceFlapStatus==1){
          VoiceFlapStatus=2;
          //AUDIO_OKAY();
          //AUDIO_OPENFLAP();
        }
        else {
          //AUDIO_OPENED();
        }
        //VoiceFlapStatus=true;
        }
      }
      else if(voiceframe->VOICECMD==VOICE_Close_Flap){
        wkup_delay=timersGetMsTick(); //updata wkup timer
        if (!getvoivech56switch()){
        if(VoiceFlapStatus)
        {
         // AUDIO_CLOSEFLAP();
        }
        else {
          //AUDIO_CLOSED();
        }
        VoiceFlapStatus=0;
        }
      }
      else if(voiceframe->VOICECMD==VOICE_Motion_Control)
      {
        wkup_delay=timersGetMsTick(); //updata wkup timer

        if(gyroinitflag==true)
        {
          MotionControlStatus=true;
        
          //AUDIO_OKAY();
        }
      }
      else if(voiceframe->VOICECMD==VOICE_Start_recording){
        wkup_delay=timersGetMsTick(); //updata wkup timer
        //AUDIO_OKAY();
      }
      else if(voiceframe->VOICECMD==VOICE_End_recording){
        wkup_delay=timersGetMsTick(); //updata wkup timer
        //AUDIO_OKAY();
      }
      else if(voiceframe->VOICECMD==VOICE_Photo){
        wkup_delay=timersGetMsTick(); //updata wkup timer
       // AUDIO_OKAY();
      }
      else if(voiceframe->VOICECMD==VOICE_go_home){
        wkup_delay=timersGetMsTick(); //updata wkup timer
        //AUDIO_OKAY();
      }
      else if(voiceframe->VOICECMD==VOICE_GPS_Return){
        wkup_delay=timersGetMsTick(); //updata wkup timer
        //AUDIO_OKAY();
      }
      else if(voiceframe->VOICECMD==VOICE_hi_take_off){
        wkup_delay=timersGetMsTick(); //updata wkup timer
       // AUDIO_OKAY();
      }
      else if(voiceframe->VOICECMD==VOICE_hi_land){
        wkup_delay=timersGetMsTick(); //updata wkup timer
       // AUDIO_OKAY();
      }
      else if(voiceframe->VOICECMD==VOICE_Telemetry){
        wkup_delay=timersGetMsTick(); //updata wkup timer
        //AUDIO_OKAY();
      #if defined(RADIO_V16)
        //pushEvent(EVT_KEY_BREAK(KEY_TELEVIEW)); //
      #endif
      }
      else if(voiceframe->VOICECMD==VOICE_Sensorstatus){
        wkup_delay=timersGetMsTick(); //updata wkup timer
        
        //AUDIO_OKAY();
      #if defined(RADIO_V16)
        //pushEvent(EVT_KEY_BREAK(KEY_SENSORVIEW)); //
      #endif
      }
      else if(voiceframe->VOICECMD==VOICE_Channel_Monitor){
        wkup_delay=timersGetMsTick(); //updata wkup timer 
        //AUDIO_OKAY();
      #if defined(RADIO_V16)
        //pushEvent(EVT_KEY_BREAK(KEY_CHVIEW)); //
      #endif
      }
      else if(voiceframe->VOICECMD==VOICE_Internal_Module){
        wkup_delay=timersGetMsTick(); //updata wkup timer 
        //AUDIO_OKAY();
      #if defined(RADIO_V16)
        //pushEvent(EVT_KEY_BREAK(KEY_INTModule)); //
      #endif
      }
      else if(voiceframe->VOICECMD==VOICE_External_Module){
        wkup_delay=timersGetMsTick(); //updata wkup timer 
        //AUDIO_OKAY();
      #if defined(RADIO_V16)
        //pushEvent(EVT_KEY_BREAK(KEY_EXTModule)); //
      #endif
      }

      else if(voiceframe->VOICECMD==VOICE_Model_Menu){
        wkup_delay=timersGetMsTick(); //updata wkup timer EVT_KEY_BREAK
        //AUDIO_OKAY();
      #if defined(RADIO_V16)
        //pushEvent(EVT_KEY_BREAK(KEY_MODEL)); //601  201
      #endif
      }
      else if(voiceframe->VOICECMD==VOICE_System_Menu){
        wkup_delay=timersGetMsTick(); //updata wkup timer EVT_KEY_BREAK
        //AUDIO_OKAY();
      #if defined(RADIO_V16)
        //pushEvent(EVT_KEY_BREAK(KEY_SYS)); //601  201
      #endif
      }
      /*
      else if(Ele_adjustment_delay==0&&voiceframe->VOICECMD==VOICE_Up){
        wkup_delay=timersGetMsTick(); //updata wkup timer EVT_KEY_BREAK 
        AUDIO_OKAY();

        rotencValue-=2;
      }
      else if(Ele_adjustment_delay==0&&voiceframe->VOICECMD==VOICE_Down){
        wkup_delay=timersGetMsTick(); //updata wkup timer EVT_KEY_BREAK
        AUDIO_OKAY();

        rotencValue+=2;
      }
      else if((Ail_adjustment_delay==0&&Rud_adjustment_delay==0)&&voiceframe->VOICECMD==VOICE_Left_one){
        wkup_delay=timersGetMsTick(); //updata wkup timer EVT_KEY_BREAK
        //pushEvent(EVT_KEY_FIRST(KEY_PAGEDN)); //601  201
        AUDIO_OKAY();
        //delay_ms(60);

        //pushEvent(0x200|1); //601  201
        pushEvent(EVT_KEY_BREAK(KEY_PAGEDN)); //601  201
      }
      else if((Ail_adjustment_delay==0&&Rud_adjustment_delay==0)&&voiceframe->VOICECMD==VOICE_Right_one){
        wkup_delay=timersGetMsTick(); //updata wkup timer EVT_KEY_BREAK
        //pushEvent(EVT_KEY_FIRST(KEY_PAGEUP)); //601  201
        AUDIO_OKAY();
        //delay_ms(60);
        //pushEvent(0x200|1); //601  201
        pushEvent(EVT_KEY_BREAK(KEY_PAGEUP)); //601  201
      }*/
     
      else if(voiceframe->VOICECMD==VOICE_Confirm){
        wkup_delay=timersGetMsTick(); //updata wkup timer EVT_KEY_BREAK

       // pushEvent(EVT_KEY_FIRST(KEY_ENTER)); //601  201
       // AUDIO_OKAY();
       // delay_ms(100);
        //pushEvent(0x200|1); //601  201
      //  pushEvent(EVT_KEY_BREAK(KEY_ENTER)); //601  201 
      }
      else if(voiceframe->VOICECMD==VOICE_Cancel){
        wkup_delay=timersGetMsTick(); //updata wkup timer EVT_KEY_BREAK
        //AUDIO_OKAY();

        //pushEvent(0x200|1); //601  201
        //pushEvent(EVT_KEY_BREAK(KEY_EXIT)); //601  201 VOICE_Confirm
      }

      else if(wkup_delay&&voiceframe->VOICECMD==VOICE_Aileron_adjustment){
        wkup_delay=timersGetMsTick(); //updata wkup timer

        Ail_adjustment_redelay=0;
        Rud_adjustment_redelay=0;
        Ele_adjustment_delay=0;
        Rud_adjustment_delay=0;
        Ail_adjustment_delay=timersGetMsTick();
        
       // AUDIO_OKAY();
      }
      else if(wkup_delay&&voiceframe->VOICECMD==VOICE_Pitch_adjustment){
        wkup_delay=timersGetMsTick();

        Ail_adjustment_delay=0;
        Rud_adjustment_delay=0;
        Ele_adjustment_delay=timersGetMsTick();
        Ele_adjustment_redelay=0;
        //AUDIO_OKAY();
      }
      else if(wkup_delay&&voiceframe->VOICECMD==VOICE_Rotating_adjustment){
        wkup_delay=timersGetMsTick();

        Ail_adjustment_delay=0;
        Ele_adjustment_delay=0;
        Ail_adjustment_redelay=0;
        Rud_adjustment_redelay=0;
        Rud_adjustment_delay=timersGetMsTick();
       // AUDIO_OKAY();
      }

      //L 0 1 R 67  L^v 32 R^v 54 
      else if(voiceframe->VOICECMD==VOICE_Up){
        if(Ele_adjustment_delay){
          wkup_delay=timersGetMsTick();

          Ele_adjustment_delay=timersGetMsTick();
          Ele_adjustment_redelay=timersGetMsTick();

          if(GetstickMode()==0||GetstickMode()==2)
            pushTrimEvent(0x600|2);
          else 
            pushTrimEvent(0x600|4);

          //repushTrimEvent=0x600|2;
        }
      }
      else if(voiceframe->VOICECMD==VOICE_Down){
        if(Ele_adjustment_delay){
          wkup_delay=timersGetMsTick();

          Ele_adjustment_delay=timersGetMsTick();
          Ele_adjustment_redelay=timersGetMsTick();

          if(GetstickMode()==0||GetstickMode()==2)
            pushTrimEvent(0x600|3);
          else 
            pushTrimEvent(0x600|5);

          //repushTrimEvent=0x600|3;
        }
      }
      else if(voiceframe->VOICECMD==VOICE_Left_one){
        if(Ail_adjustment_delay) {
          wkup_delay=timersGetMsTick();

          Ail_adjustment_delay=timersGetMsTick();
          Ail_adjustment_redelay=timersGetMsTick();
          pushTrimEvent(0x600|6);
          //repushTrimEvent=0x600|6;
        }
        if(Rud_adjustment_delay){
          wkup_delay=timersGetMsTick();

          Rud_adjustment_delay=timersGetMsTick();
          Rud_adjustment_redelay=timersGetMsTick();
          pushTrimEvent(0x600|0);
          //repushTrimEvent=0x600|0;
        }
      }
      else if(voiceframe->VOICECMD==VOICE_Right_one){
        if(Ail_adjustment_delay) {
          wkup_delay=timersGetMsTick();

          Ail_adjustment_delay=timersGetMsTick();
          Ail_adjustment_redelay=timersGetMsTick();
          pushTrimEvent(0x600|7);
          //repushTrimEvent=0x600|7;
        }
        if(Rud_adjustment_delay) {
          wkup_delay=timersGetMsTick();
          Rud_adjustment_delay=timersGetMsTick();
          Rud_adjustment_redelay=timersGetMsTick();
          pushTrimEvent(0x600|1);
          //repushTrimEvent=0x600|1;
        }
      }
  }

  //trainerResetTimer();
}
extern "C" int GetVoiceInput(uint8_t *rxchar)
{//cli 
  // TODO: place this outside of the function
  auto _getByte = voiceGetByte;
  //auto _getByte = dbgGetByte;
  while (_getByte && (_getByte(rxchar) > 0)) {
    
    //dbgSerialPutc(rxchar);
    return 0;
  }
  return -1;
}
void processUpdataInput(void)
{
  uint8_t rxchar;
  //auto _getByte = voiceGetByte;
  auto _getByte = dbgGetByte;
  while (_getByte && (_getByte(&rxchar) > 0)) {
    
    voiceSerialPutc(rxchar);
  }
}
void processVoiceInput(void)
{
  // TODO: place this outside of the function
  static uint8_t VoiceIndex = 0;
  static uint32_t VoiceTimer;
  static uint8_t VoiceFrame[VOICE_FRAME_SIZE];

  uint32_t active = 0;

  if(VoiceRunStatus==false)return;  //cli updata 

  // Drain input first (if existing)
  uint8_t rxchar;
  auto _getByte = voiceGetByte;
  //auto _getByte = dbgGetByte;
  while (_getByte && (_getByte(&rxchar) > 0)) {
    active = 1;
    if (VoiceIndex > VOICE_FRAME_SIZE - 1) {
      VoiceIndex = VOICE_FRAME_SIZE - 1;
    }
    VoiceFrame[VoiceIndex++] = rxchar;

    dbgSerialPutc(rxchar);
  }

  if (wkup_delay)
  {
    if ((uint32_t)(timersGetMsTick() - wkup_delay) > wkup_delayms) {
      wkup_delay = 0; //timer over
      Ail_adjustment_delay = 0;
      Ele_adjustment_delay = 0;
      Rud_adjustment_delay = 0;
    }
  }
  
  if(Ail_adjustment_delay)
  {
    if ((uint32_t)(timersGetMsTick() - Ail_adjustment_delay) > wkup_delayms) {
      Ail_adjustment_delay = 0;
    }
  }
  if(Ele_adjustment_delay)
  {
    if ((uint32_t)(timersGetMsTick() - Ele_adjustment_delay) > wkup_delayms) {
      Ele_adjustment_delay = 0;
    }
  }
  if(Rud_adjustment_delay)
  {
    if ((uint32_t)(timersGetMsTick() - Rud_adjustment_delay) > wkup_delayms) {
      Rud_adjustment_delay = 0;
    }
  }
  if(repushTrimEvent)
  {
    //pushTrimEvent(repushTrimEvent);
    repushTrimEvent=0;
  }
  // Data has been received
  if (active) {
    VoiceTimer = timersGetUsTick();
    return;
  }
  // Check if end-of-frame is detected
  if (VoiceIndex) {
    if ((uint32_t)(timersGetUsTick() - VoiceTimer) > VOICE_FRAME_DELAY_US) {
      processVoiceFrame(VoiceFrame, VoiceIndex);
      VoiceIndex = 0;
    }
  }
}

