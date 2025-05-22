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

File    : SEGGER_SYSVIEW_NuttX.c
Purpose : Interface between NuttX and SystemView.
Revision: $Rev $
*/
#include <nuttx/config.h>

#include "SEGGER_SYSVIEW.h"
#include "SEGGER_SYSVIEW_NuttX.h"
#include "string.h" // Required for memset

#include <time.h>


#ifdef CONFIG_CLOCK_MONOTONIC
#  define _TIME_CLOCK CLOCK_MONOTONIC
#else
#  define _TIME_CLOCK CLOCK_REALTIME
#endif


typedef struct _SYSVIEW_NUTTX_TASK_STATUS _SYSVIEW_NUTTX_TASK_STATUS;

struct _SYSVIEW_NUTTX_TASK_STATUS {
  U32         TaskID;
  const char* sName;
  unsigned    Priority;
  U32         StackAddr;
  unsigned    StackSize;
};

static _SYSVIEW_NUTTX_TASK_STATUS _aTasks[SYSVIEW_NUTTX_MAX_NOF_TASKS];
static unsigned _NumTasks;


static U64      _Time;
static U64      _TimeStart;

/*********************************************************************
*
*       _cbSendTaskList()
*
*  Function description
*    This function is part of the link between NuttX and SYSVIEW.
*    Called from SystemView when asked by the host, it uses SYSVIEW
*    functions to send the entire task list to the host.
*/
static void _cbSendTaskList(void) {
  unsigned n;

  for (n = 0; n < _NumTasks; n++) {
    SYSVIEW_SendTaskInfo(_aTasks[n].TaskID, _aTasks[n].sName, _aTasks[n].Priority, _aTasks[n].StackAddr, _aTasks[n].StackSize);
  }
}

/*********************************************************************
*
*       _cbGetTime()
*
*  Function description
*    This function is part of the link between FreeRTOS and SYSVIEW.
*    Called from SystemView when asked by the host, returns the
*    current system time in micro seconds.
*/
static U64 _cbGetTime(void) {
  struct timespec TimeSpec;
  int             r;

  r = clock_gettime(_TIME_CLOCK, &TimeSpec);

  if (r >= 0) {
    _Time = (U64)TimeSpec.tv_sec;
    _Time *= 1000000uLL;
    _Time += TimeSpec.tv_nsec / 1000u;
  }
  return _Time - _TimeStart;
}

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/
/*********************************************************************
*
*       SYSVIEW_InitTime()
*
*  Function description
*    Initialize system time of SystemView, to start at 0 with the initialization.
*/
void SYSVIEW_InitTime(void) {
  _TimeStart = 0;
  _TimeStart = _cbGetTime();
}

/*********************************************************************
*
*       SYSVIEW_AddTask()
*
*  Function description
*    Add a task to the internal list and record its information.
*/
void SYSVIEW_AddTask(U32 TaskID, const char* sName, unsigned Priority, U32 StackAddr, unsigned StackSize) {
  
  if (_NumTasks >= SYSVIEW_NUTTX_MAX_NOF_TASKS) {
    SEGGER_SYSVIEW_Warn("SYSTEMVIEW: Could not record task information. Maximum number of tasks reached.");
    return;
  }

  _aTasks[_NumTasks].TaskID = TaskID;
  _aTasks[_NumTasks].sName = sName;
  _aTasks[_NumTasks].Priority = Priority;
  _aTasks[_NumTasks].StackAddr = StackAddr;
  _aTasks[_NumTasks].StackSize = StackSize;

  _NumTasks++;

  SYSVIEW_SendTaskInfo(TaskID, sName, Priority, StackAddr, StackSize);
}

/*********************************************************************
*
*       SYSVIEW_UpdateTask()
*
*  Function description
*    Update a task in the internal list and record its information.
*/
void SYSVIEW_UpdateTask(U32 TaskID, const char* sName, unsigned Priority, U32 StackAddr, unsigned StackSize) {
  unsigned n;

  for (n = 0; n < _NumTasks; n++) {
    if (_aTasks[n].TaskID == TaskID) {
      break;
    }
  }
  if (n < _NumTasks) {
    _aTasks[n].sName = sName;
    _aTasks[n].Priority = Priority;
    _aTasks[n].StackAddr = StackAddr;
    _aTasks[n].StackSize = StackSize;

    SYSVIEW_SendTaskInfo(TaskID, sName, Priority, StackAddr, StackSize);
  } else {
    SYSVIEW_AddTask(TaskID, sName, Priority, StackAddr, StackSize);
  }
}

/*********************************************************************
*
*       SYSVIEW_DeleteTask()
*
*  Function description
*    Delete a task from the internal list.
*/
void SYSVIEW_DeleteTask(U32 TaskID) {
  unsigned n;
  
  if (_NumTasks == 0) {
    return; // Early out
  }  
  for (n = 0; n < _NumTasks; n++) {
    if (_aTasks[n].TaskID == TaskID) {
      break;
    }
  }
  if (n == (_NumTasks - 1)) {  
    //
    // Task is last item in list.
    // Simply zero the item and decrement number of tasks.
    //
    memset(&_aTasks[n], 0, sizeof(_aTasks[n]));
    _NumTasks--;
  } else if (n < _NumTasks) {
    //
    // Task is in the middle of the list.
    // Move last item to current position and decrement number of tasks.
    // Order of tasks does not really matter, so no need to move all following items.
    //
    _aTasks[n].TaskID         = _aTasks[_NumTasks - 1].TaskID;
    _aTasks[n].sName          = _aTasks[_NumTasks - 1].sName;
    _aTasks[n].Priority       = _aTasks[_NumTasks - 1].Priority;
    _aTasks[n].StackAddr      = _aTasks[_NumTasks - 1].StackAddr;
    _aTasks[n].StackSize      = _aTasks[_NumTasks - 1].StackSize;
    memset(&_aTasks[_NumTasks - 1], 0, sizeof(_aTasks[_NumTasks - 1]));
    _NumTasks--;
  }
}

/*********************************************************************
*
*       SYSVIEW_SendTaskInfo()
*
*  Function description
*    Record task information.
*/
void SYSVIEW_SendTaskInfo(U32 TaskID, const char* sName, unsigned Prio, U32 StackAddr, unsigned StackSize) {
  SEGGER_SYSVIEW_TASKINFO TaskInfo;

  memset(&TaskInfo, 0, sizeof(TaskInfo)); // Fill all elements with 0 to allow extending the structure in future version without breaking the code
  TaskInfo.TaskID     = TaskID;
  TaskInfo.sName      = sName;
  TaskInfo.Prio       = Prio;
  TaskInfo.StackBase  = StackAddr;
  TaskInfo.StackSize  = StackSize;
  SEGGER_SYSVIEW_SendTaskInfo(&TaskInfo);
}

/*********************************************************************
*
*       Public API structures
*
**********************************************************************
*/
// Callbacks provided to SYSTEMVIEW by FreeRTOS
const SEGGER_SYSVIEW_OS_API SYSVIEW_X_OS_TraceAPI = {
  _cbGetTime,
  _cbSendTaskList,
};

/*************************** End of file ****************************/
