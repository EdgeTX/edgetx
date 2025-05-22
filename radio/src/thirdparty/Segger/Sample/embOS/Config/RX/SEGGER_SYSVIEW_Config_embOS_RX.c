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

File    : SEGGER_SYSVIEW_Config_embOS_RX.c
Purpose : Sample setup configuration of SystemView with embOS
          on Renesas RX systems.
Revision: $Rev: 25330 $

Additional information:
  SEGGER_SYSVIEW_TickCnt must be incremented in the SysTick
  handler before any SYSVIEW event is generated.

  Example in embOS RTOSInit.c:

  void SysTick_Handler(void) {
  #if (OS_PROFILE != 0)
    SEGGER_SYSVIEW_TickCnt++;  // Increment SEGGER_SYSVIEW_TickCnt before calling OS_EnterNestableInterrupt().
  #endif
    OS_EnterNestableInterrupt();
    OS_TICK_Handle();
    OS_LeaveNestableInterrupt();
  }
*/
#include "RTOS.h"
#include "SEGGER_SYSVIEW.h"
#include "SEGGER_SYSVIEW_embOS.h"

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/
#define IRR_BASE_ADDR        (0x00087000u)
#define CMT0_VECT            28u
#define OS_TIMER_VECT        CMT0_VECT
#define TIMER_PRESCALE       (8u)
#define CMT0_BASE_ADDR       (0x00088000u)
#define CMT0_CMCNT           (*(volatile U16*) (CMT0_BASE_ADDR + 0x04u))

/*********************************************************************
*
*       Local functions
*
**********************************************************************
*/
/*********************************************************************
*
*       _cbSendSystemDesc()
*
*  Function description
*    Sends SystemView description strings.
*/
static void _cbSendSystemDesc(void) {
  SEGGER_SYSVIEW_SendSysDesc("N=" SEGGER_SYSVIEW_APP_NAME ",O=embOS,D=" SEGGER_SYSVIEW_DEVICE_NAME );
#ifdef SEGGER_SYSVIEW_SYSDESC0
  SEGGER_SYSVIEW_SendSysDesc(SEGGER_SYSVIEW_SYSDESC0);
#endif
#ifdef SEGGER_SYSVIEW_SYSDESC1
  SEGGER_SYSVIEW_SendSysDesc(SEGGER_SYSVIEW_SYSDESC1);
#endif
#ifdef SEGGER_SYSVIEW_SYSDESC2
  SEGGER_SYSVIEW_SendSysDesc(SEGGER_SYSVIEW_SYSDESC2);
#endif
}

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/
/*********************************************************************
*
*       SEGGER_SYSVIEW_Conf()
*
* Function description
*   Configure and initialize SystemView and register it with embOS.
*
* Additional information
*   If enabled, SEGGER_SYSVIEW_Conf() will also immediately start
*   recording events with SystemView.
*/
void SEGGER_SYSVIEW_Conf(void) {
  SEGGER_SYSVIEW_Init(SEGGER_SYSVIEW_TIMESTAMP_FREQ, SEGGER_SYSVIEW_CPU_FREQ,
                      &SYSVIEW_X_OS_TraceAPI, _cbSendSystemDesc);
  OS_SetTraceAPI(&embOS_TraceAPI_SYSVIEW);   // Configure embOS to use SYSVIEW.
#if SEGGER_SYSVIEW_START_ON_INIT
  SEGGER_SYSVIEW_Start();                    // Start recording to catch system initialization.
#endif
}

/*********************************************************************
*
*       SEGGER_SYSVIEW_X_GetTimestamp()
*
* Function description
*   Returns the current timestamp in cycles using the system tick
*   count and the SysTick counter.
*   All parameters of the SysTick have to be known and are set via
*   configuration defines on top of the file.
*
* Return value
*   The current timestamp.
*
* Additional information
*   SEGGER_SYSVIEW_X_GetTimestamp is always called when interrupts are
*   disabled. Therefore locking here is not required.
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
  return ((SEGGER_SYSVIEW_TIMESTAMP_FREQ / 1000) * Time) + Cnt;
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
#ifdef __RX  // Renesas CCRX
  IntId = (get_psw() & 0x0F000000) >> 24u;
#else
 __asm volatile ("mvfc    PSW, %0           \t\n" // Load current PSW
                 "and     #0x0F000000, %0   \t\n" // Clear all except IPL ([27:24])
                 "shlr    #24, %0           \t\n" // Shift IPL to [3:0]
                 : "=r" (IntId)                   // Output result
                 :                                // Input
                 :                                // Clobbered list
                );
#endif
  return IntId;
}

/*************************** End of file ****************************/
