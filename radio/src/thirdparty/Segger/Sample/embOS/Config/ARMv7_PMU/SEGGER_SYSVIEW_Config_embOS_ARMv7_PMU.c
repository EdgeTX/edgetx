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

File    : SEGGER_SYSVIEW_Config_embOS_ARMv7_PMU.c
Purpose : Sample setup configuration of SystemView with embOS
          on ARMv7-A and ARMv7-R devices that support the Performance
          Monitors Extension and thus have a Performance Monitors
          Unit (PMU) with cycle counter implemented.
Revision: $Rev: 25331 $

Additional information:
  The PMU cycle counter provides the most accurate timestamps, because
  the cycle counter increments on each CPU clock cycle. Devices that
  run at high frequencies need to send at least one SystemView event
  each iteration of the 32-Bit counter to prevent unnoticed counter
  overflows. A device running at 1GHz would thus need to send at least
  one event every ~4.29 seconds. If this is not possible, the cycle
  counter can be initialized to increment only each 64th CPU clock cycle.

  SEGGER_SYSVIEW_InterruptId has to be set in the IRQ handler
  to identify the active interrupt.
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
#if (OS_DEBUG != 0)
  unsigned long DebugFeatures;

  //
  // Check if performance monitors extension is supported.
  //
  __asm volatile("mrc P15, 0, %0, C0, C1, 2\n"    // Read ID_DFR0
                 : "=r" (DebugFeatures)
                 :
                 : );
  DebugFeatures = (DebugFeatures >> 24) & 0x0Fu;  // Extract information about the performance monitors extension
  if (DebugFeatures == 0x0Fu) {
    //
    // ARM performance monitors extension not supported on this device.
    // Another counter needs to be used for this device.
    //
    OS_Error(OS_ERR_HW_NOT_AVAILABLE);
  }
#endif  // (OS_DEBUG != 0)
  //
  // Enable and reset PMU cycle counter.
  // Requires performance monitors extension.
  //
  __asm volatile("mov R12, #0x80000000       \n"
                 "mcr P15, 0, R12, C9, C12, 1\n"  // Set PMCNTSET.C (PMCCNTR enable bit)
                 "mrc P15, 0, R12, C9, C12, 0\n"  // Read PMCR (performance counter control register)
                 "orr R12, R12, #5           \n"  // Set PMCR.E (enables PMU) and PMCR.C (reset PMCCNTR to 0)
                 "mcr P15, 0, R12, C9, C12, 0\n"  // Write modified value back to PMCR
                 :
                 :
                 : "r12", "cc" );
  SEGGER_SYSVIEW_Init(SEGGER_SYSVIEW_TIMESTAMP_FREQ, SEGGER_SYSVIEW_CPU_FREQ, &SYSVIEW_X_OS_TraceAPI, _cbSendSystemDesc);
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
  U32 Cycles;
  __asm volatile ("mrc P15, 0, %0, C9, C13, 0\n"  // Read PMCCNTR (performance monitors cycle count register)
                  : "=r" (Cycles)
                  :
                  : );
  return Cycles;
}

/*********************************************************************
*
*       SEGGER_SYSVIEW_X_GetInterruptId()
*/
U32 SEGGER_SYSVIEW_X_GetInterruptId(void) {
  return SEGGER_SYSVIEW_InterruptId;
}

/*************************** End of file ****************************/
