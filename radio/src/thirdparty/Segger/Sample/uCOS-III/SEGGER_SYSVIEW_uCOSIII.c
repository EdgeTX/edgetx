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

File    : SEGGER_SYSVIEW_uCOSIII.c
Purpose : Interface between Micrium uC/OS-III and SystemView.
Revision: $Rev: 9599 $
*/

#include <os_trace_events.h>

#ifndef SYSVIEW_MEMSET
  #include <string.h>
  #define SYSVIEW_MEMSET(p, v, n)   memset(p, v, n)
#endif

#ifndef  OS_CFG_TRACE_MAX_RESOURCES
#define  OS_CFG_TRACE_MAX_RESOURCES              0
#endif

typedef struct SYSVIEW_UCOSIII_TASK_STATUS SYSVIEW_UCOSIII_TASK_STATUS;

struct SYSVIEW_UCOSIII_TASK_STATUS {
  U32           TaskID;
  const char*   NamePtr;
  OS_PRIO       Prio;
  CPU_STK*      StkBasePtr;
  CPU_STK_SIZE  StkSize;
};

typedef struct SYSVIEW_UCOSIII_RESOURCE SYSVIEW_UCOSIII_RESOURCE;

struct SYSVIEW_UCOSIII_RESOURCE {
  U32         ResourceId;
  const char* sResource;
  U32         Registered;
};

static SYSVIEW_UCOSIII_TASK_STATUS _aTasks[OS_CFG_TRACE_MAX_TASK];
static unsigned _NumTasks;

#if OS_CFG_TRACE_MAX_RESOURCES > 0
static SYSVIEW_UCOSIII_RESOURCE _aResources[OS_CFG_TRACE_MAX_RESOURCES];
static unsigned                 _NumResources;
#endif

/*********************************************************************
*
*       _cbSendTaskList()
*
*  Function description
*    This function is part of the link between FreeRTOS and SYSVIEW.
*    Called from SystemView when asked by the host, it uses SYSVIEW
*    functions to send the entire task list to the host.
*/
static void _cbSendTaskList(void) {
  unsigned n;

  for (n = 0; n < _NumTasks; n++) {
    SYSVIEW_SendTaskInfo((U32)_aTasks[n].TaskID, _aTasks[n].NamePtr, (unsigned)_aTasks[n].Prio, (U32)_aTasks[n].StkBasePtr, (unsigned)_aTasks[n].StkSize);
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
  OS_ERR Err;
  OS_TICK Tick;

  Tick = OSTimeGet(&Err);
  if (Err != OS_ERR_NONE) {
    Tick = 0;
  }
  return Tick * 1000;
}

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/

/*********************************************************************
*
*       SYSVIEW_TaskReady()
*
*  Function description
*    Record when a task is ready for execution.
*/
void SYSVIEW_TaskReady(U32 TaskID) {
  if (TaskID != (U32)&OSIdleTaskTCB) {
    SEGGER_SYSVIEW_OnTaskStartReady(TaskID);
  }
}

/*********************************************************************
*
*       SYSVIEW_TaskSwitchedIn()
*
*  Function description
*    Record when a task starts/continues execution.
*    If the idle task continues, record an on idle event.
*/
void SYSVIEW_TaskSwitchedIn(U32 TaskID) {
  if (TaskID != (U32)&OSIdleTaskTCB) {
    SEGGER_SYSVIEW_OnTaskStartExec(TaskID);
  } else {
    SEGGER_SYSVIEW_OnIdle();
  }
}

/*********************************************************************
*
*       SYSVIEW_TaskSuspend()
*
*  Function description
*    Record when a task is suspended.
*/
void SYSVIEW_TaskSuspend(U32 TaskID) {
  if (TaskID != (U32)&OSIdleTaskTCB) {
    SEGGER_SYSVIEW_OnTaskStopReady(TaskID,  (1u << 2));
  }
}

/*********************************************************************
*
*       SYSVIEW_AddTask()
*
*  Function description
*    Add a task to the internal list and record its information.
*/
void SYSVIEW_AddTask(U32 TaskID, const char* NamePtr, OS_PRIO Prio, CPU_STK* StkBasePtr, CPU_STK_SIZE StkSize) {
  if (TaskID != (U32)&OSIdleTaskTCB) {
    if (_NumTasks >= OS_CFG_TRACE_MAX_TASK) {
      SEGGER_SYSVIEW_Warn("SYSTEMVIEW: Could not record task information. Maximum number of tasks reached.");
      return;
    }

    _aTasks[_NumTasks].TaskID = TaskID;
    _aTasks[_NumTasks].NamePtr = NamePtr;
    _aTasks[_NumTasks].Prio = Prio;
    _aTasks[_NumTasks].StkBasePtr = StkBasePtr;
    _aTasks[_NumTasks].StkSize = StkSize;

    _NumTasks++;

    SYSVIEW_SendTaskInfo(TaskID, NamePtr, (unsigned)Prio, (U32)StkBasePtr, (unsigned)StkSize);
  }
}

/*********************************************************************
*
*       SYSVIEW_UpdateTask()
*
*  Function description
*    Update a task in the internal list and record its information.
*/
void SYSVIEW_UpdateTask(U32 TaskID, const char* NamePtr, OS_PRIO Prio, CPU_STK* StkBasePtr, CPU_STK_SIZE StkSize) {
  unsigned n;

  if (TaskID != (U32)&OSIdleTaskTCB) {
    for (n = 0; n < _NumTasks; n++) {
      if (_aTasks[n].TaskID == TaskID) {
        break;
      }
    }
    if (n < _NumTasks) {
      _aTasks[n].NamePtr = NamePtr;
      _aTasks[n].Prio = Prio;
      _aTasks[n].StkBasePtr = StkBasePtr;
      _aTasks[n].StkSize = StkSize;

      SYSVIEW_SendTaskInfo(TaskID, NamePtr, (unsigned)Prio, (U32)StkBasePtr, (unsigned)StkSize);
    } else {
      SYSVIEW_AddTask(TaskID, NamePtr, Prio, StkBasePtr, StkSize);
    }
  }
}

/*********************************************************************
*
*       SYSVIEW_SendTaskInfo()
*
*  Function description
*    Record task information.
*/
void SYSVIEW_SendTaskInfo(U32 TaskID, const char* sName, unsigned Prio, U32 StackBase, unsigned StackSize) {
  SEGGER_SYSVIEW_TASKINFO TaskInfo;

  //
  // Fill all elements with 0 to allow extending the structure in future version without breaking the code
  //
  SYSVIEW_MEMSET(&TaskInfo, 0, sizeof(TaskInfo));
  TaskInfo.TaskID     = TaskID;
  TaskInfo.sName      = sName;
  TaskInfo.Prio       = Prio;
  TaskInfo.StackBase  = StackBase;
  TaskInfo.StackSize  = StackSize;
  SEGGER_SYSVIEW_SendTaskInfo(&TaskInfo);
}

/*********************************************************************
*
*       SYSVIEW_RecordU32x4()
*
*  Function description
*    Record an event with 4 parameters
*/
void SYSVIEW_RecordU32x4(unsigned Id, U32 Para0, U32 Para1, U32 Para2, U32 Para3) {
      U8  aPacket[SEGGER_SYSVIEW_INFO_SIZE + 4 * SEGGER_SYSVIEW_QUANTA_U32];
      U8* pPayload;
      //
      pPayload = SEGGER_SYSVIEW_PREPARE_PACKET(aPacket);                // Prepare the packet for SystemView
      pPayload = SEGGER_SYSVIEW_EncodeU32(pPayload, Para0);             // Add the first parameter to the packet
      pPayload = SEGGER_SYSVIEW_EncodeU32(pPayload, Para1);             // Add the second parameter to the packet
      pPayload = SEGGER_SYSVIEW_EncodeU32(pPayload, Para2);             // Add the third parameter to the packet
      pPayload = SEGGER_SYSVIEW_EncodeU32(pPayload, Para3);             // Add the fourth parameter to the packet
      //
      SEGGER_SYSVIEW_SendPacket(&aPacket[0], pPayload, Id);             // Send the packet
}

/*********************************************************************
*
*       SYSVIEW_RecordU32x5()
*
*  Function description
*    Record an event with 5 parameters
*/
void SYSVIEW_RecordU32x5(unsigned Id, U32 Para0, U32 Para1, U32 Para2, U32 Para3, U32 Para4) {
      U8  aPacket[SEGGER_SYSVIEW_INFO_SIZE + 5 * SEGGER_SYSVIEW_QUANTA_U32];
      U8* pPayload;
      //
      pPayload = SEGGER_SYSVIEW_PREPARE_PACKET(aPacket);                // Prepare the packet for SystemView
      pPayload = SEGGER_SYSVIEW_EncodeU32(pPayload, Para0);             // Add the first parameter to the packet
      pPayload = SEGGER_SYSVIEW_EncodeU32(pPayload, Para1);             // Add the second parameter to the packet
      pPayload = SEGGER_SYSVIEW_EncodeU32(pPayload, Para2);             // Add the third parameter to the packet
      pPayload = SEGGER_SYSVIEW_EncodeU32(pPayload, Para3);             // Add the fourth parameter to the packet
      pPayload = SEGGER_SYSVIEW_EncodeU32(pPayload, Para4);             // Add the fifth parameter to the packet
      //
      SEGGER_SYSVIEW_SendPacket(&aPacket[0], pPayload, Id);             // Send the packet
}
/*********************************************************************
*
*       SYSVIEW_RecordU32Register()
*
*  Function description
*    Record an event with 1 parameter and register the resource to be 
*    sent in the system description.
*/
void SYSVIEW_RecordU32Register(unsigned EventId, U32 ResourceId, const char* sResource) {  
  SEGGER_SYSVIEW_NameResource(ResourceId, sResource);
  SEGGER_SYSVIEW_RecordU32(EventId, SEGGER_SYSVIEW_ShrinkId(ResourceId));
#if OS_CFG_TRACE_MAX_RESOURCES > 0
  if (_NumResources >= OS_CFG_TRACE_MAX_RESOURCES) {
    SEGGER_SYSVIEW_Warn("SYSTEMVIEW: Could not register resource name. Maximum number of resources reached.");
    return;
  }

  _aResources[_NumResources].ResourceId = ResourceId;
  _aResources[_NumResources].sResource  = sResource;
  _aResources[_NumResources].Registered = 0;

  _NumResources++;
#endif
}

void SYSVIEW_SendResourceList(void) {
#if OS_CFG_TRACE_MAX_RESOURCES > 0
  unsigned int n;

  for (n = 0; n < _NumResources; n++) {
    if (_aResources[n].Registered == 0) {
      SEGGER_SYSVIEW_NameResource(_aResources[n].ResourceId, _aResources[n].sResource);
      _aResources[n].Registered = 1;
    }
  }
#endif
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
