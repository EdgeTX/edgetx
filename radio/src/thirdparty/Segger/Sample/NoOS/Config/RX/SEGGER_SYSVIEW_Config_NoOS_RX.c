/*********************************************************************
*                    SEGGER Microcontroller GmbH                     *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*            (c) 1995 - 2024 SEGGER Microcontroller GmbH             *
*                                                                    *
*       www.segger.com     Support: support@segger.com               *
*                                                                    *
**********************************************************************
*                                                                    *
*       SEGGER SystemView * Real-time application analysis           *
*                                                                    *
**********************************************************************
*                                                                    *
* All rights reserved.                                               *
*                                                                    *
* SEGGER strongly recommends to not make any changes                 *
* to or modify the source code of this software in order to stay     *
* compatible with the SystemView and RTT protocol, and J-Link.       *
*                                                                    *
* Redistribution and use in source and binary forms, with or         *
* without modification, are permitted provided that the following    *
* condition is met:                                                  *
*                                                                    *
* o Redistributions of source code must retain the above copyright   *
*   notice, this condition and the following disclaimer.             *
*                                                                    *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND             *
* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,        *
* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF           *
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE           *
* DISCLAIMED. IN NO EVENT SHALL SEGGER Microcontroller BE LIABLE FOR *
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR           *
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT  *
* OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;    *
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF      *
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT          *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE  *
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH   *
* DAMAGE.                                                            *
*                                                                    *
**********************************************************************
*                                                                    *
*       SystemView version: 3.60e                                    *
*                                                                    *
**********************************************************************
-------------------------- END-OF-HEADER -----------------------------

File    : SEGGER_SYSVIEW_Config_NoOS_RX.c
Purpose : Sample setup configuration of SystemView on Renesas RX 
          systems without an operating system.
Revision: $Rev: 18540 $
*/
#include "RTOS.h"
#include "SEGGER_SYSVIEW.h"
#include "SEGGER_SYSVIEW_embOS.h"

//
// SystemcoreClock can be used in most CMSIS compatible projects.
// In non-CMSIS projects define SYSVIEW_CPU_FREQ directly.
//
extern unsigned int SystemCoreClock;

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/
// The application name to be displayed in SystemViewer
#ifndef   SYSVIEW_APP_NAME
  #define SYSVIEW_APP_NAME          "Demo Application"
#endif

// The target device name
#ifndef   SYSVIEW_DEVICE_NAME
  #define SYSVIEW_DEVICE_NAME       "RX64M"
#endif

// System Frequency. SystemcoreClock is used in most CMSIS compatible projects.
#ifndef   SYSVIEW_CPU_FREQ
  #define SYSVIEW_CPU_FREQ        (SystemCoreClock)
#endif

// Frequency of the timestamp. Must match SEGGER_SYSVIEW_Conf.h and RTOSInit.c
#ifndef   SYSVIEW_TIMESTAMP_FREQ
  #define SYSVIEW_TIMESTAMP_FREQ  (SYSVIEW_CPU_FREQ/2u/8u) // Assume system timer runs at 1/16th of the CPU frequency
#endif

// The lowest RAM address used for IDs (pointers)
#ifndef   SYSVIEW_RAM_BASE
  #define SYSVIEW_RAM_BASE        (0)
#endif

#ifndef   SYSVIEW_SYSDESC0
  #define SYSVIEW_SYSDESC0        "I#0=IntPrio0,I#1=IntPrio1,I#2=IntPrio2,I#3=IntPrio3,I#4=IntPrio4"
#endif

//#ifndef   SYSVIEW_SYSDESC1
//  #define SYSVIEW_SYSDESC1      "I#5=IntPrio5,I#6=IntPrio6,I#7=IntPrio7,I#8=IntPrio8,I#9=IntPrio9,I#10=IntPrio10"
//#endif

//#ifndef   SYSVIEW_SYSDESC2
//  #define SYSVIEW_SYSDESC2      "I#11=IntPrio11,I#12=IntPrio12,I#13=IntPrio13,I#14=IntPrio14,I#15=IntPrio15"
//#endif

// System Timer configuration
#define IRR_BASE_ADDR        (0x00087000u)
#define CMT0_VECT            28u
#define OS_TIMER_VECT        CMT0_VECT
#define TIMER_PRESCALE       (8u)
#define CMT0_BASE_ADDR       (0x00088000u)
#define CMT0_CMCNT           (*(volatile U16*) (CMT0_BASE_ADDR + 0x04u))

extern unsigned SEGGER_SYSVIEW_TickCnt;  // Tick Counter value incremented in the tick handler.

/*********************************************************************
*
*       _cbSendSystemDesc()
*
*  Function description
*    Sends SystemView description strings.
*/
static void _cbSendSystemDesc(void) {
  SEGGER_SYSVIEW_SendSysDesc("N="SYSVIEW_APP_NAME",D="SYSVIEW_DEVICE_NAME);
#ifdef SYSVIEW_SYSDESC0
  SEGGER_SYSVIEW_SendSysDesc(SYSVIEW_SYSDESC0);
#endif
#ifdef SYSVIEW_SYSDESC1
  SEGGER_SYSVIEW_SendSysDesc(SYSVIEW_SYSDESC1);
#endif
#ifdef SYSVIEW_SYSDESC2
  SEGGER_SYSVIEW_SendSysDesc(SYSVIEW_SYSDESC2);
#endif
}

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/
void SEGGER_SYSVIEW_Conf(void) {
  SEGGER_SYSVIEW_Init(SYSVIEW_TIMESTAMP_FREQ, SYSVIEW_CPU_FREQ,
                      0, _cbSendSystemDesc);
  SEGGER_SYSVIEW_SetRAMBase(SYSVIEW_RAM_BASE);
}

/*********************************************************************
*
*       SEGGER_SYSVIEW_X_GetTimestamp()
*
* Function description
*   Returns the current timestamp in ticks using the system tick
*   count and the SysTick counter.
*   All parameters of the SysTick have to be known and are set via
*   configuration defines on top of the file.
*
* Return value
*   The current timestamp.
*
* Additional information
*   SEGGER_SYSVIEW_X_GetTimestamp is always called when interrupts are
*   disabled. 
*   Therefore locking here is not required and OS_GetTime_Cycles() may
*   be called.
*/
U32 SEGGER_SYSVIEW_X_GetTimestamp(void) {
  U32 Time;
  U32 Cnt;

  Time = SEGGER_SYSVIEW_TickCnt;
  Cnt  = CMT0_CMCNT;
  //
  // Check if timer interrupt pending ...
  //
  if ((*(volatile U8*)(IRR_BASE_ADDR + OS_TIMER_VECT) & (1u << 0u)) != 0u) {
    Cnt = CMT0_CMCNT;      // Interrupt pending, re-read timer and adjust result
    Time++;
  }
  return ((SYSVIEW_TIMESTAMP_FREQ/1000) * Time) + Cnt;
}

/*********************************************************************
*
*       SEGGER_SYSVIEW_X_GetInterruptId()
*
*  Function description
*    Return the priority of the currently active interrupt.
*/
U32 SEGGER_SYSVIEW_X_GetInterruptId(void) {
  U32 IntId;
 __asm volatile ("mvfc    PSW, %0           \t\n" // Load current PSW
                 "and     #0x0F000000, %0   \t\n" // Clear all except IPL ([27:24])
                 "shlr    #24, %0           \t\n" // Shift IPL to [3:0]
                 : "=r" (IntId)                   // Output result
                 :                                // Input
                 :                                // Clobbered list
                );
  return IntId;
}

/*************************** End of file ****************************/
