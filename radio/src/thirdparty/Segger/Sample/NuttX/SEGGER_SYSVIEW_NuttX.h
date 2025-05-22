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

File    : SEGGER_SYSVIEW_NuttX.h
Purpose : Interface between NuttX and SystemView.
Revision: $Rev $
*/

#ifndef SYSVIEW_NUTTX_H
#define SYSVIEW_NUTTX_H

#include "SEGGER_SYSVIEW.h"

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/
#ifdef CONFIG_SYSTEMVIEW_MAX_NOF_TASKS
  #define SYSVIEW_NUTTX_MAX_NOF_TASKS  CONFIG_SYSTEMVIEW_MAX_NOF_TASKS
#else
  #define SYSVIEW_NUTTX_MAX_NOF_TASKS  8
#endif

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/
#define _SYSTEMVIEW_ID_OFFSET                              (32u)
#define _TRACE_API_ID_TASK_CREATE                          (1u + _SYSTEMVIEW_ID_OFFSET)
#define _TRACE_API_ID_NXTASK_CREATE                        (2u + _SYSTEMVIEW_ID_OFFSET)
#define _TRACE_API_ID_KTHREAD_CREATE                       (3u + _SYSTEMVIEW_ID_OFFSET)
#define _TRACE_API_ID_PTHREAD_CREATE                       (4u + _SYSTEMVIEW_ID_OFFSET)

#if !defined(CONFIG_SYSTEMVIEW_ENABLE)
  #define TRACE_INIT()
  #define TRACE_START()
  #define TRACE_TASK_CREATED(PID, Name, Priority, StackAddr, StackSize)
  #define TRACE_TASK_READY(PID)
  #define TRACE_TASK_START(pTCB)
  #define TRACE_TASK_BLOCK(PID, State)
  #define TRACE_TASK_TERMINATE(PID)
#else
  #define TRACE_INIT()                              							SEGGER_SYSVIEW_Conf()
  #define TRACE_START()                             							SEGGER_SYSVIEW_Start()
  #define TRACE_TASK_CREATED(PID, Name, Priority, StackAddr, StackSize) 		SEGGER_SYSVIEW_OnTaskCreate(PID); 													\
                                                                                SYSVIEW_AddTask((U32)PID, (const char*)Name, (unsigned)Priority, (U32)StackAddr, (unsigned)StackSize)
  #define TRACE_TASK_READY(PID)													SEGGER_SYSVIEW_OnTaskStartReady(PID)
  #define TRACE_TASK_START(PID)  												{ if (PID == 0) { SEGGER_SYSVIEW_OnIdle(); } else { SEGGER_SYSVIEW_OnTaskStartExec(PID); } }
  #define TRACE_TASK_BLOCK(PID, State) 											SEGGER_SYSVIEW_OnTaskStopReady(PID, (U32)State)
  #define TRACE_TASK_TERMINATE(PID)                                             { SEGGER_SYSVIEW_OnTaskTerminate(PID); SYSVIEW_DeleteTask(PID); }
#endif

#if !defined(CONFIG_SYSTEMVIEW_ENABLE) && !defined(CONFIG_SYSTEMVIEW_RECORD_ISR)
  #define TRACE_ISR_ENTER()
  #define TRACE_ISR_EXIT()
  #define TRACE_ISR_EXIT_TO_SCHEDULER()
#else
  #define TRACE_ISR_ENTER()                         							SEGGER_SYSVIEW_RecordEnterISR()
  #define TRACE_ISR_EXIT()                          							SEGGER_SYSVIEW_RecordExitISR()
  #define TRACE_ISR_EXIT_TO_SCHEDULER()             							SEGGER_SYSVIEW_RecordExitISRToScheduler()
#endif

#if !defined(CONFIG_SYSTEMVIEW_ENABLE) && !defined(CONFIG_SYSTEMVIEW_RECORD_API)
  #define TRACE_API_TASK_CREATE(name, priority, stack_size, entry, argv)
  #define TRACE_API_TASK_CREATE_RETURN(ret)
  #define TRACE_API_NXTASK_CREATE(name, priority, stack_size, entry, argv)
  #define TRACE_API_NXTASK_CREATE_RETURN(ret)
  #define TRACE_API_KTHREAD_CREATE(name, priority, stack_size, entry, argv)
  #define TRACE_API_KTHREAD_CREATE_RETURN(ret)
  #define TRACE_API_PTHREAD_CREATE(thread, attr, start_routine, arg)
  #define TRACE_API_PTHREAD_CREATE_RETURN(ret)
#else
  #define TRACE_API_TASK_CREATE(name, priority, stack_size, entry, argv)        SEGGER_SYSVIEW_RecordU32x5     (_TRACE_API_ID_TASK_CREATE,  (U32)name, (U32)priority, (U32)stack_size, (U32)entry, (U32)argv)
  #define TRACE_API_TASK_CREATE_RETURN(ret)         														SEGGER_SYSVIEW_RecordEndCallU32(_TRACE_API_ID_TASK_CREATE,  ret)
  #define TRACE_API_NXTASK_CREATE(name, priority, stack_size, entry, argv)      SEGGER_SYSVIEW_RecordU32x5     (_TRACE_API_ID_NXTASK_CREATE, (U32)name, (U32)priority, (U32)stack_size, (U32)entry, (U32)argv)
  #define TRACE_API_NXTASK_CREATE_RETURN(ret)                                   SEGGER_SYSVIEW_RecordEndCallU32(_TRACE_API_ID_NXTASK_CREATE, ret)
  #define TRACE_API_KTHREAD_CREATE(name, priority, stack_size, entry, argv)     SEGGER_SYSVIEW_RecordU32x5     (_TRACE_API_ID_KTHREAD_CREATE,  (U32)name, (U32)priority, (U32)stack_size, (U32)entry, (U32)argv)
  #define TRACE_API_KTHREAD_CREATE_RETURN(ret)                                  SEGGER_SYSVIEW_RecordEndCallU32(_TRACE_API_ID_KTHREAD_CREATE,  ret)
  #define TRACE_API_PTHREAD_CREATE(thread, attr, start_routine, arg)            SEGGER_SYSVIEW_RecordU32x4     (_TRACE_API_ID_PTHREAD_CREATE, (U32)thread, (U32)attr, (U32)start_routine, (U32)arg)
  #define TRACE_API_PTHREAD_CREATE_RETURN(ret)                                  SEGGER_SYSVIEW_RecordEndCallU32(_TRACE_API_ID_PTHREAD_CREATE,  ret)
#endif

/*********************************************************************
*
*       API functions
*
**********************************************************************
*/
#ifdef __cplusplus
extern "C" {
#endif
void SYSVIEW_InitTime     (void);
void SYSVIEW_AddTask      (U32 TaskID, const char* sName, unsigned Priority, U32 StackAddr, unsigned StackSize);
void SYSVIEW_UpdateTask   (U32 TaskID, const char* sName, unsigned Priority, U32 StackAddr, unsigned StackSize);
void SYSVIEW_DeleteTask   (U32 TaskID);
void SYSVIEW_SendTaskInfo (U32 TaskID, const char* sName, unsigned Priority, U32 StackAddr, unsigned StackSize);

extern const SEGGER_SYSVIEW_OS_API SYSVIEW_X_OS_TraceAPI;

#ifdef __cplusplus
}
#endif

#endif

/*************************** End of file ****************************/
