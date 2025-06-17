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

File    : SEGGER_SYSVIEW_ThreadX.c
Purpose : Interface between ThreadX and SystemView.
Revision: $Rev: 7947 $
*/
#include "tx_api.h"
#include "tx_block_pool.h"
#include "tx_byte_pool.h"
#include "tx_event_flags.h"
#include "tx_semaphore.h"
#include "tx_mutex.h"
#include "tx_queue.h"
#include "tx_thread.h"
#include "tx_timer.h"
#include "SEGGER_SYSVIEW.h"
#include "SEGGER_SYSVIEW_ThreadX.h"

/*********************************************************************
*
*       _cbGetTime()
*
*  Function description
*    This function is part of the link between ThreadX and SYSVIEW.
*    Called from SystemView when asked by the host, returns the
*    current system time in micro seconds.
*/
static U64 _cbGetTime(void) {
  U64 Time;

  Time = tx_time_get();
  Time /= TX_TIMER_TICKS_PER_SECOND;
  Time *= 1000000;
  return Time;
}

/*********************************************************************
*
*       _SendTaskList()
*
*  Function description
*    This function is part of the link between ThreadX and SYSVIEW.
*    Uses SYSVIEW functions to send the entire task list to the host.
*/
static void _SendTaskList(void) {
  TX_THREAD* pThread;
  ULONG i;

  if (_tx_thread_created_count == 0) {
    return;
  }

  pThread = _tx_thread_created_ptr->tx_thread_created_next;
  for (i = 0; i < _tx_thread_created_count; i++) {
    SEGGER_SYSVIEW_TASKINFO TaskInfo;

    memset(&TaskInfo, 0, sizeof(TaskInfo)); // Fill all elements with 0 to allow extending the structure in future version without breaking the code
    TaskInfo.TaskID = TX_POINTER_TO_ULONG_CONVERT(pThread);
    TaskInfo.sName = pThread->tx_thread_name;
    TaskInfo.Prio = pThread->tx_thread_priority;
    TaskInfo.StackBase = TX_POINTER_TO_ULONG_CONVERT(pThread->tx_thread_stack_start);
    TaskInfo.StackSize = pThread->tx_thread_stack_size;
    SEGGER_SYSVIEW_SendTaskInfo(&TaskInfo);
    pThread = pThread->tx_thread_created_next;
  }
}

/*********************************************************************
*
*       _SendBlockPoolList()
*
*  Function description
*    This function is part of the link between ThreadX and SYSVIEW.
*    Registers all block pools to SystemView.
*/
static void _SendBlockPoolList(void) {
  TX_BLOCK_POOL* pBlockPool;
  ULONG i;

  if (_tx_block_pool_created_count == 0) {
    return;
  }

  pBlockPool = _tx_block_pool_created_ptr->tx_block_pool_created_next;
  for (i = 0; i < _tx_block_pool_created_count; i++) {
    SEGGER_SYSVIEW_NameResource(TX_POINTER_TO_ULONG_CONVERT(pBlockPool), pBlockPool->tx_block_pool_name);
    pBlockPool = pBlockPool->tx_block_pool_created_next;
  }
}

/*********************************************************************
*
*       _SendBytePoolList()
*
*  Function description
*    This function is part of the link between ThreadX and SYSVIEW.
*    Registers all byte pools to SystemView.
*/
static void _SendBytePoolList(void) {
  TX_BYTE_POOL* pBytePool;
  ULONG i;

  if (_tx_byte_pool_created_count == 0) {
    return;
  }

  pBytePool = _tx_byte_pool_created_ptr->tx_byte_pool_created_next;
  for (i = 0; i < _tx_byte_pool_created_count; i++) {
    SEGGER_SYSVIEW_NameResource(TX_POINTER_TO_ULONG_CONVERT(pBytePool), pBytePool->tx_byte_pool_name);
    pBytePool = pBytePool->tx_byte_pool_created_next;
  }
}

/*********************************************************************
*
*       _SendEventsFlagsList()
*
*  Function description
*    This function is part of the link between ThreadX and SYSVIEW.
*    Registers all event flags to SystemView.
*/
static void _SendEventsFlagsList(void) {
  TX_EVENT_FLAGS_GROUP* pEvent;
  ULONG i;

  if (_tx_event_flags_created_count == 0) {
    return;
  }

  pEvent = _tx_event_flags_created_ptr->tx_event_flags_group_created_next;
  for (i = 0; i < _tx_event_flags_created_count; i++) {
    SEGGER_SYSVIEW_NameResource(TX_POINTER_TO_ULONG_CONVERT(pEvent), pEvent->tx_event_flags_group_name);
    pEvent = pEvent->tx_event_flags_group_created_next;
  }
}

/*********************************************************************
*
*       _SendSemaphoreList()
*
*  Function description
*    This function is part of the link between ThreadX and SYSVIEW.
*    Registers all semaphores to SystemView.
*/
static void _SendSemaphoreList(void) {
  TX_SEMAPHORE* pSemaphore;
  ULONG i;

  if (_tx_semaphore_created_count == 0) {
    return;
  }

  pSemaphore = _tx_semaphore_created_ptr->tx_semaphore_created_next;
  for (i = 0; i < _tx_semaphore_created_count; i++) {
    SEGGER_SYSVIEW_NameResource(TX_POINTER_TO_ULONG_CONVERT(pSemaphore), pSemaphore->tx_semaphore_name);
    pSemaphore = pSemaphore->tx_semaphore_created_next;
  }
}

/*********************************************************************
*
*       _SendTimerList()
*
*  Function description
*    This function is part of the link between ThreadX and SYSVIEW.
*    Registers all timers to SystemView.
*/
static void _SendTimerList(void) {
  TX_TIMER* pTimer;
  ULONG i;

  if (_tx_timer_created_count == 0) {
    return;
  }

  pTimer = _tx_timer_created_ptr->tx_timer_created_next;
  for (i = 0; i < _tx_timer_created_count; i++) {
    SEGGER_SYSVIEW_NameResource(TX_POINTER_TO_ULONG_CONVERT(&pTimer->tx_timer_internal), pTimer->tx_timer_name);
    pTimer = pTimer->tx_timer_created_next;
  }
}

/*********************************************************************
*
*       _SendMutexList()
*
*  Function description
*    This function is part of the link between ThreadX and SYSVIEW.
*    Registers all mutexes to SystemView.
*/
static void _SendMutexList(void) {
  TX_MUTEX* pMutex;
  ULONG i;

  if (_tx_mutex_created_count == 0) {
    return;
  }

  pMutex = _tx_mutex_created_ptr->tx_mutex_created_next;
  for (i = 0; i < _tx_mutex_created_count; i++) {
    SEGGER_SYSVIEW_NameResource(TX_POINTER_TO_ULONG_CONVERT(pMutex), pMutex->tx_mutex_name);
    pMutex = pMutex->tx_mutex_created_next;
  }
}

/*********************************************************************
*
*       _SendQueueList()
*
*  Function description
*    This function is part of the link between ThreadX and SYSVIEW.
*    Registers all queues to SystemView.
*/
static void _SendQueueList(void) {
  TX_QUEUE* pQueue;
  ULONG i;

  if (_tx_queue_created_count == 0) {
    return;
  }

  pQueue = _tx_queue_created_ptr->tx_queue_created_next;
  for (i = 0; i < _tx_queue_created_count; i++) {
    SEGGER_SYSVIEW_NameResource(TX_POINTER_TO_ULONG_CONVERT(pQueue), pQueue->tx_queue_name);
    pQueue = pQueue->tx_queue_created_next;
  }
}

/*********************************************************************
*
*       _cbSendTaskList()
*
*  Function description
*    This function is part of the link between ThreadX and SYSVIEW.
*    Called from SystemView when asked by the host, it uses SYSVIEW
*    functions to send the entire resource list to the host.
*/
static void _cbSendResourceList(void) {
  _SendTaskList();
  _SendSemaphoreList();
  _SendMutexList();
  _SendQueueList();
  _SendEventsFlagsList();
  _SendBytePoolList();
  _SendBlockPoolList();
  _SendTimerList();
}

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/

/*********************************************************************
*
*       SYSVIEW_EnterTimer()
*
*  Function description
*    Record a SystemView EnterTimer event.
*/
void SYSVIEW_EnterTimer(void) {
  SEGGER_SYSVIEW_RecordEnterTimer((U32)_tx_timer_current_ptr);
}

/*********************************************************************
*
*       SYSVIEW_ExitTimer()
*
*  Function description
*    Record a SystemView ExitTimer event.
*/
void SYSVIEW_ExitTimer(void) {
  SEGGER_SYSVIEW_RecordExitTimer();
}

/*********************************************************************
*
*       trace_task_stop()
*
*  Function description
*    Record a SystemView TaskStopReady event.
*/
void trace_task_stop(void) {
  TX_THREAD *pThread;

  if (SEGGER_SYSVIEW_IsStarted() > 0) {
    TX_THREAD_GET_CURRENT(pThread);
    if (pThread != TX_NULL) {
      SEGGER_SYSVIEW_OnTaskStopReady(TX_POINTER_TO_ULONG_CONVERT(pThread), pThread->tx_thread_state);
    }
  }

  if (_tx_thread_execute_ptr == TX_NULL) {
    SEGGER_SYSVIEW_OnIdle();
  }
}

/*********************************************************************
*
*       trace_task_exec()
*
*  Function description
*    Record a SystemView TaskStartExec event.
*/
void trace_task_exec(void) {
  TX_THREAD *pThread;

  if (SEGGER_SYSVIEW_IsStarted() > 0) {
    TX_THREAD_GET_CURRENT(pThread);
    if (pThread != TX_NULL) {
      SEGGER_SYSVIEW_OnTaskStartExec(TX_POINTER_TO_ULONG_CONVERT(pThread));
    }
  }
}

/*********************************************************************
*
*       trace_isr_enter()
*
*  Function description
*    Record a SystemView ISREntry event.
*/
void trace_isr_enter(void) {
  SEGGER_SYSVIEW_RecordEnterISR();
}

/*********************************************************************
*
*       trace_isr_exit()
*
*  Function description
*    Record a SystemView ISRExit event.
*/
void trace_isr_exit(void) {
  if (_tx_thread_execute_ptr != TX_NULL) {
    SEGGER_SYSVIEW_RecordExitISRToScheduler();
  } else {
    SEGGER_SYSVIEW_RecordExitISR();
  }
}

/*********************************************************************
*
*       Public API structures
*
**********************************************************************
*/
// Callbacks provided to SYSTEMVIEW by ThreadX
const SEGGER_SYSVIEW_OS_API SYSVIEW_X_OS_TraceAPI = {
  _cbGetTime,
  _cbSendResourceList,
};

/*************************** End of file ****************************/
