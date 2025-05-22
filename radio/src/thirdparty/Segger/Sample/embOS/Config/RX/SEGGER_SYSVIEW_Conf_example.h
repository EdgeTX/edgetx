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

File    : SEGGER_SYSVIEW_Conf.h
Purpose : SEGGER SystemView configuration file.
          Set defines which deviate from the defaults (see SEGGER_SYSVIEW_ConfDefaults.h) here.          
Revision: $Rev: 25331 $

Additional information:
  Required defines which must be set are:
    SEGGER_SYSVIEW_GET_TIMESTAMP
    SEGGER_SYSVIEW_GET_INTERRUPT_ID
  For known compilers and cores, these might be set to good defaults
  in SEGGER_SYSVIEW_ConfDefaults.h.
  
  SystemView needs a (nestable) locking mechanism.
  If not defined, the RTT locking mechanism is used,
  which then needs to be properly configured.
*/

#ifndef SEGGER_SYSVIEW_CONF_H
#define SEGGER_SYSVIEW_CONF_H

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

// The application name to be displayed in SystemViewer
#define SEGGER_SYSVIEW_APP_NAME        "embOS start project"

// The target device name
#define SEGGER_SYSVIEW_DEVICE_NAME     "RX"

// System Frequency
#define SEGGER_SYSVIEW_CPU_FREQ        (96000000u)

// Frequency of the timestamp
#define SEGGER_SYSVIEW_TIMESTAMP_FREQ  (SEGGER_SYSVIEW_CPU_FREQ/2u/8u) // Assume system timer runs at 1/16th of the CPU frequency

// Lowest Id reported by the Application.
#define SEGGER_SYSVIEW_ID_BASE         (0x00000000)

#define SEGGER_SYSVIEW_SYSDESC0        "I#0=IntPrio0,I#1=IntPrio1,I#2=IntPrio2,I#3=IntPrio3,I#4=IntPrio4"
//#define SEGGER_SYSVIEW_SYSDESC1        "I#5=IntPrio5,I#6=IntPrio6,I#7=IntPrio7,I#8=IntPrio8,I#9=IntPrio9,I#10=IntPrio10"
//#define SEGGER_SYSVIEW_SYSDESC2        "I#11=IntPrio11,I#12=IntPrio12,I#13=IntPrio13,I#14=IntPrio14,I#15=IntPrio15"

// Retrieve a system timestamp via user-defined function
#define SEGGER_SYSVIEW_GET_TIMESTAMP() SEGGER_SYSVIEW_X_GetTimestamp()

// Get the currently active interrupt Id from the user-provided function.
#define SEGGER_SYSVIEW_GET_INTERRUPT_ID()   SEGGER_SYSVIEW_X_GetInterruptId()

#endif  // SEGGER_SYSVIEW_CONF_H

/*************************** End of file ****************************/
