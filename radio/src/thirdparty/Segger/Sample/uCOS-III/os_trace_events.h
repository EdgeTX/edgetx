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

File    : os_trace.h
Purpose : Interface header for Micrium uC/OS-III and SystemView.
Revision: $Rev: 9599 $
*/
#include "SEGGER_SYSVIEW.h"
#include "os.h"
#include "os_cfg_trace.h"


/*
************************************************************************************************************************
*                                               uC/OS-III Trace Macros
************************************************************************************************************************
*/

#if (defined(OS_CFG_TRACE_EN) && (OS_CFG_TRACE_EN > 0u))
#define  OS_TRACE_INIT()                             SEGGER_SYSVIEW_Conf()
#define  OS_TRACE_START()                            SEGGER_SYSVIEW_Start()
#define  OS_TRACE_STOP()                             SEGGER_SYSVIEW_Stop()
#else
#define  OS_TRACE_INIT()
#define  OS_TRACE_START()
#define  OS_TRACE_STOP()
#endif


/*
************************************************************************************************************************
*                                     uC/OS-III Trace fixed defines for SystemView
************************************************************************************************************************
*/

#if (defined(OS_CFG_TRACE_EN) && (OS_CFG_TRACE_EN > 0u))
#define OS_TRACE_ID_OFFSET                           (32u)

#define OS_TRACE_ID_TICK_INCREMENT                   ( 1u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_ISR_REGISTER                     ( 2u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_TASK_MSG_Q_CREATE                ( 3u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_TASK_MSG_Q_POST                  ( 4u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_TASK_MSG_Q_PEND                  ( 5u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_TASK_SEM_CREATE                  ( 6u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_TASK_SEM_POST                    ( 7u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_TASK_SEM_PEND                    ( 8u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_MUTEX_CREATE                     ( 9u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_MUTEX_DEL                        (10u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_MUTEX_POST                       (11u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_MUTEX_PEND                       (12u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_MUTEX_TASK_PRIO_INHERIT          (13u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_MUTEX_TASK_PRIO_DISINHERIT       (14u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_SEM_CREATE                       (15u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_SEM_DEL                          (16u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_SEM_POST                         (17u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_SEM_PEND                         (18u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_Q_CREATE                         (19u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_Q_DEL                            (20u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_Q_POST                           (21u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_Q_PEND                           (22u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_FLAG_CREATE                      (23u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_FLAG_DEL                         (24u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_FLAG_POST                        (25u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_FLAG_PEND                        (26u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_MEM_CREATE                       (27u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_MEM_PUT                          (28u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_MEM_GET                          (29u + OS_TRACE_ID_OFFSET)
#endif


/*
************************************************************************************************************************
*                                          uC/OS-III Trace Kernel-Related Macros
************************************************************************************************************************
*/

#if (defined(OS_CFG_TRACE_EN) && (OS_CFG_TRACE_EN > 0u))
#define  OS_TRACE_TASK_CREATE(p_tcb)                if (p_tcb != 0) {                            \
                                                      SEGGER_SYSVIEW_OnTaskCreate((U32)p_tcb);   \
                                                      SYSVIEW_AddTask((U32)p_tcb,                \
                                                                      &(p_tcb->NamePtr[0]),      \
                                                                      p_tcb->Prio,               \
                                                                      p_tcb->StkBasePtr,         \
                                                                      p_tcb->StkSize             \
                                                                      );                         \
                                                    }

#define  OS_TRACE_TASK_READY(p_tcb)                 SYSVIEW_TaskReady((U32)p_tcb)
#define  OS_TRACE_TASK_SWITCHED_IN(p_tcb)           SYSVIEW_TaskSwitchedIn((U32)p_tcb)
#define  OS_TRACE_TASK_DLY(dly_ticks)
#define  OS_TRACE_TASK_SUSPEND(p_tcb)
#define  OS_TRACE_TASK_SUSPENDED(p_tcb)             SYSVIEW_TaskSuspend((U32)p_tcb)
#define  OS_TRACE_TASK_RESUME(p_tcb)                SYSVIEW_TaskReady((U32)p_tcb)

#define  OS_TRACE_ISR_BEGIN(isr_id)
#define  OS_TRACE_ISR_END()

#define  OS_TRACE_ISR_ENTER()                       SEGGER_SYSVIEW_RecordEnterISR()
#define  OS_TRACE_ISR_EXIT()                        SEGGER_SYSVIEW_RecordExitISR()
#define  OS_TRACE_ISR_EXIT_TO_SCHEDULER()           SEGGER_SYSVIEW_RecordExitISRToScheduler()


/*
************************************************************************************************************************
*                                      uC/OS-III Trace Simple Recorder Functions
************************************************************************************************************************
*/

#define  OS_TRACE_TICK_INCREMENT(OSTickCtr)                SEGGER_SYSVIEW_RecordU32  (OS_TRACE_ID_TICK_INCREMENT,             (U32)OSTickCtr)
#define  OS_TRACE_ISR_REGISTER(isr_id, isr_name, isr_prio) SEGGER_SYSVIEW_RecordU32x2(OS_TRACE_ID_ISR_REGISTER,               (U32)isr_id,                         (U32)isr_prio)
#define  OS_TRACE_MUTEX_TASK_PRIO_INHERIT(p_tcb, prio)     SEGGER_SYSVIEW_RecordU32x2(OS_TRACE_ID_MUTEX_TASK_PRIO_INHERIT,    SEGGER_SYSVIEW_ShrinkId((U32)p_tcb), (U32)prio);
#define  OS_TRACE_MUTEX_TASK_PRIO_DISINHERIT(p_tcb, prio)  SEGGER_SYSVIEW_RecordU32x2(OS_TRACE_ID_MUTEX_TASK_PRIO_DISINHERIT, SEGGER_SYSVIEW_ShrinkId((U32)p_tcb), (U32)prio);
#define  OS_TRACE_TASK_DEL(p_tcb)                             


/*
************************************************************************************************************************
*                                      uC/OS-III Trace Complex Recorder Functions
************************************************************************************************************************
*/

#define  OS_TRACE_MUTEX_CREATE(p_mutex, p_name)      SYSVIEW_RecordU32Register(OS_TRACE_ID_MUTEX_CREATE,      ((U32)p_mutex), p_name)
#define  OS_TRACE_TASK_MSG_Q_CREATE(p_msg_q, p_name) SYSVIEW_RecordU32Register(OS_TRACE_ID_TASK_MSG_Q_CREATE, ((U32)p_msg_q), p_name)
#define  OS_TRACE_TASK_SEM_CREATE(p_tcb, p_name)     SYSVIEW_RecordU32Register(OS_TRACE_ID_TASK_SEM_CREATE,   ((U32)p_tcb),   p_name)
#define  OS_TRACE_SEM_CREATE(p_sem, p_name)          SYSVIEW_RecordU32Register(OS_TRACE_ID_SEM_CREATE,        ((U32)p_sem),   p_name)
#define  OS_TRACE_Q_CREATE(p_q, p_name)              SYSVIEW_RecordU32Register(OS_TRACE_ID_Q_CREATE,          ((U32)p_q),     p_name)
#define  OS_TRACE_FLAG_CREATE(p_grp, p_name)         SYSVIEW_RecordU32Register(OS_TRACE_ID_FLAG_CREATE,       ((U32)p_grp),   p_name)
#define  OS_TRACE_MEM_CREATE(p_mem, p_name)          SYSVIEW_RecordU32Register(OS_TRACE_ID_MEM_CREATE,        ((U32)p_mem),   p_name)


/*
************************************************************************************************************************
*                                         uC/OS-III Trace API Enter Functions
************************************************************************************************************************
*/

#if (defined(OS_CFG_TRACE_API_ENTER_EN) && (OS_CFG_TRACE_API_ENTER_EN > 0u))
#define  OS_TRACE_MUTEX_DEL_ENTER(p_mutex, opt)                                    SEGGER_SYSVIEW_RecordU32x2(OS_TRACE_ID_MUTEX_DEL,       SEGGER_SYSVIEW_ShrinkId((U32)p_mutex), (U32)opt)
#define  OS_TRACE_MUTEX_POST_ENTER(p_mutex, opt)                                   SEGGER_SYSVIEW_RecordU32x2(OS_TRACE_ID_MUTEX_POST,      SEGGER_SYSVIEW_ShrinkId((U32)p_mutex), (U32)opt)
#define  OS_TRACE_MUTEX_PEND_ENTER(p_mutex, timeout, opt, p_ts)                    SEGGER_SYSVIEW_RecordU32x3(OS_TRACE_ID_MUTEX_PEND,      SEGGER_SYSVIEW_ShrinkId((U32)p_mutex), (U32)timeout,  (U32)opt)
#define  OS_TRACE_TASK_MSG_Q_POST_ENTER(p_msg_q, p_void, msg_size, opt)            SEGGER_SYSVIEW_RecordU32x3(OS_TRACE_ID_TASK_MSG_Q_POST, SEGGER_SYSVIEW_ShrinkId((U32)p_msg_q), (U32)msg_size, (U32)opt)
#define  OS_TRACE_TASK_MSG_Q_PEND_ENTER(p_msg_q, timeout, opt, p_msg_size, p_ts)   SEGGER_SYSVIEW_RecordU32x3(OS_TRACE_ID_TASK_MSG_Q_PEND, SEGGER_SYSVIEW_ShrinkId((U32)p_msg_q), (U32)timeout,  (U32)opt)
#define  OS_TRACE_TASK_SEM_POST_ENTER(p_tcb, opt)                                  SEGGER_SYSVIEW_RecordU32x2(OS_TRACE_ID_TASK_SEM_POST,   SEGGER_SYSVIEW_ShrinkId((U32)p_tcb),   (U32)opt)
#define  OS_TRACE_TASK_SEM_PEND_ENTER(p_tcb, timeout, opt, p_ts)                   SEGGER_SYSVIEW_RecordU32x3(OS_TRACE_ID_TASK_SEM_PEND,   SEGGER_SYSVIEW_ShrinkId((U32)p_tcb),   (U32)timeout,  (U32)opt)
#define  OS_TRACE_SEM_DEL_ENTER(p_sem, opt)                                        SEGGER_SYSVIEW_RecordU32x2(OS_TRACE_ID_SEM_DEL,         SEGGER_SYSVIEW_ShrinkId((U32)p_sem),   (U32)opt)
#define  OS_TRACE_SEM_POST_ENTER(p_sem, opt)                                       SEGGER_SYSVIEW_RecordU32x2(OS_TRACE_ID_SEM_POST,        SEGGER_SYSVIEW_ShrinkId((U32)p_sem),   (U32)opt)
#define  OS_TRACE_SEM_PEND_ENTER(p_sem, timeout, opt, p_ts)                        SEGGER_SYSVIEW_RecordU32x3(OS_TRACE_ID_SEM_PEND,        SEGGER_SYSVIEW_ShrinkId((U32)p_sem),   (U32)timeout,  (U32)opt)
#define  OS_TRACE_Q_DEL_ENTER(p_q, opt)                                            SEGGER_SYSVIEW_RecordU32x2(OS_TRACE_ID_Q_DEL,           SEGGER_SYSVIEW_ShrinkId((U32)p_q),     (U32)opt)
#define  OS_TRACE_Q_POST_ENTER(p_q, p_void, msg_size, opt)                         SEGGER_SYSVIEW_RecordU32x3(OS_TRACE_ID_Q_POST,          SEGGER_SYSVIEW_ShrinkId((U32)p_q),     (U32)msg_size, (U32)opt)
#define  OS_TRACE_Q_PEND_ENTER(p_q, timeout, opt, p_msg_size, p_ts)                SEGGER_SYSVIEW_RecordU32x3(OS_TRACE_ID_Q_PEND,          SEGGER_SYSVIEW_ShrinkId((U32)p_q),     (U32)timeout,  (U32)opt)
#define  OS_TRACE_FLAG_DEL_ENTER(p_grp, opt)                                       SEGGER_SYSVIEW_RecordU32x2(OS_TRACE_ID_FLAG_DEL,        SEGGER_SYSVIEW_ShrinkId((U32)p_grp),   (U32)opt)
#define  OS_TRACE_FLAG_POST_ENTER(p_grp, flags, opt)                               SEGGER_SYSVIEW_RecordU32x3(OS_TRACE_ID_FLAG_POST,       SEGGER_SYSVIEW_ShrinkId((U32)p_grp),   (U32)flags,    (U32)opt)
#define  OS_TRACE_FLAG_PEND_ENTER(p_grp, flags, timeout, opt, p_ts)                SYSVIEW_RecordU32x4       (OS_TRACE_ID_FLAG_PEND,       SEGGER_SYSVIEW_ShrinkId((U32)p_grp),   (U32)flags,    (U32)timeout, (U32)opt)
#define  OS_TRACE_MEM_PUT_ENTER(p_mem, p_blk)                                      SEGGER_SYSVIEW_RecordU32x2(OS_TRACE_ID_MEM_PUT,         SEGGER_SYSVIEW_ShrinkId((U32)p_mem),   (U32)p_blk)
#define  OS_TRACE_MEM_GET_ENTER(p_mem)                                             SEGGER_SYSVIEW_RecordU32  (OS_TRACE_ID_MEM_GET,         SEGGER_SYSVIEW_ShrinkId((U32)p_mem))
#else
#define  OS_TRACE_MUTEX_DEL_ENTER(p_mutex, opt)
#define  OS_TRACE_MUTEX_POST_ENTER(p_mutex, opt)
#define  OS_TRACE_MUTEX_PEND_ENTER(p_mutex, timeout, opt, p_ts)
#define  OS_TRACE_TASK_MSG_Q_POST_ENTER(p_msg_q, p_void, msg_size, opt)
#define  OS_TRACE_TASK_MSG_Q_PEND_ENTER(p_msg_q, timeout, opt, p_msg_size, p_ts)
#define  OS_TRACE_TASK_SEM_POST_ENTER(p_tcb, opt)
#define  OS_TRACE_TASK_SEM_PEND_ENTER(p_tcb, timeout, opt, p_ts)
#define  OS_TRACE_SEM_DEL_ENTER(p_sem, opt)
#define  OS_TRACE_SEM_POST_ENTER(p_sem, opt)
#define  OS_TRACE_SEM_PEND_ENTER(p_sem, timeout, opt, p_ts)
#define  OS_TRACE_Q_DEL_ENTER(p_q, opt)
#define  OS_TRACE_Q_POST_ENTER(p_q, p_void, msg_size, opt)
#define  OS_TRACE_Q_PEND_ENTER(p_q, timeout, opt, p_msg_size, p_ts)
#define  OS_TRACE_FLAG_DEL_ENTER(p_grp, opt)
#define  OS_TRACE_FLAG_POST_ENTER(p_grp, flags, opt)
#define  OS_TRACE_FLAG_PEND_ENTER(p_grp, flags, timeout, opt, p_ts)
#define  OS_TRACE_MEM_PUT_ENTER(p_mem, p_blk)
#define  OS_TRACE_MEM_GET_ENTER(p_mem)
#endif


/*
************************************************************************************************************************
*                                         uC/OS-III Trace API Exit Functions
************************************************************************************************************************
*/

#if (defined(OS_CFG_TRACE_API_EXIT_EN) && (OS_CFG_TRACE_API_EXIT_EN > 0u))
#define  OS_TRACE_MUTEX_DEL_EXIT(RetVal)                                                  SEGGER_SYSVIEW_RecordEndCallU32(OS_TRACE_ID_MUTEX_DEL,       RetVal)
#define  OS_TRACE_MUTEX_POST_EXIT(RetVal)                                                 SEGGER_SYSVIEW_RecordEndCallU32(OS_TRACE_ID_MUTEX_POST,      RetVal)
#define  OS_TRACE_MUTEX_PEND_EXIT(RetVal)                                                 SEGGER_SYSVIEW_RecordEndCallU32(OS_TRACE_ID_MUTEX_PEND,      RetVal)
#define  OS_TRACE_TASK_MSG_Q_POST_EXIT(RetVal)                                            SEGGER_SYSVIEW_RecordEndCallU32(OS_TRACE_ID_TASK_MSG_Q_POST, RetVal)
#define  OS_TRACE_TASK_MSG_Q_PEND_EXIT(RetVal)                                            SEGGER_SYSVIEW_RecordEndCallU32(OS_TRACE_ID_TASK_MSG_Q_PEND, RetVal)
#define  OS_TRACE_TASK_SEM_POST_EXIT(RetVal)                                              SEGGER_SYSVIEW_RecordEndCallU32(OS_TRACE_ID_TASK_SEM_POST,   RetVal)
#define  OS_TRACE_TASK_SEM_PEND_EXIT(RetVal)                                              SEGGER_SYSVIEW_RecordEndCallU32(OS_TRACE_ID_TASK_SEM_PEND,   RetVal)
#define  OS_TRACE_SEM_DEL_EXIT(RetVal)                                                    SEGGER_SYSVIEW_RecordEndCallU32(OS_TRACE_ID_SEM_DEL,         RetVal)
#define  OS_TRACE_SEM_POST_EXIT(RetVal)                                                   SEGGER_SYSVIEW_RecordEndCallU32(OS_TRACE_ID_SEM_POST,        RetVal)
#define  OS_TRACE_SEM_PEND_EXIT(RetVal)                                                   SEGGER_SYSVIEW_RecordEndCallU32(OS_TRACE_ID_SEM_PEND,        RetVal)
#define  OS_TRACE_Q_DEL_EXIT(RetVal)                                                      SEGGER_SYSVIEW_RecordEndCallU32(OS_TRACE_ID_Q_DEL,           RetVal)
#define  OS_TRACE_Q_POST_EXIT(RetVal)                                                     SEGGER_SYSVIEW_RecordEndCallU32(OS_TRACE_ID_Q_POST,          RetVal)
#define  OS_TRACE_Q_PEND_EXIT(RetVal)                                                     SEGGER_SYSVIEW_RecordEndCallU32(OS_TRACE_ID_Q_PEND,          RetVal)
#define  OS_TRACE_FLAG_DEL_EXIT(RetVal)                                                   SEGGER_SYSVIEW_RecordEndCallU32(OS_TRACE_ID_FLAG_DEL,        RetVal)
#define  OS_TRACE_FLAG_POST_EXIT(RetVal)                                                  SEGGER_SYSVIEW_RecordEndCallU32(OS_TRACE_ID_FLAG_POST,       RetVal)
#define  OS_TRACE_FLAG_PEND_EXIT(RetVal)                                                  SEGGER_SYSVIEW_RecordEndCallU32(OS_TRACE_ID_FLAG_PEND,       RetVal)
#define  OS_TRACE_MEM_PUT_EXIT(RetVal)                                                    SEGGER_SYSVIEW_RecordEndCallU32(OS_TRACE_ID_MEM_PUT,         RetVal)
#define  OS_TRACE_MEM_GET_EXIT(RetVal)                                                    SEGGER_SYSVIEW_RecordEndCallU32(OS_TRACE_ID_MEM_GET,         RetVal)
#else
#define  OS_TRACE_MUTEX_DEL_EXIT(RetVal)
#define  OS_TRACE_MUTEX_POST_EXIT(RetVal)
#define  OS_TRACE_MUTEX_PEND_EXIT(RetVal)
#define  OS_TRACE_TASK_MSG_Q_POST_EXIT(RetVal)
#define  OS_TRACE_TASK_MSG_Q_PEND_EXIT(RetVal)
#define  OS_TRACE_TASK_SEM_POST_EXIT(RetVal)
#define  OS_TRACE_TASK_SEM_PEND_EXIT(RetVal)
#define  OS_TRACE_SEM_DEL_EXIT(RetVal)
#define  OS_TRACE_SEM_POST_EXIT(RetVal)
#define  OS_TRACE_SEM_PEND_EXIT(RetVal)
#define  OS_TRACE_Q_DEL_EXIT(RetVal)
#define  OS_TRACE_Q_POST_EXIT(RetVal)
#define  OS_TRACE_Q_PEND_EXIT(RetVal)
#define  OS_TRACE_FLAG_DEL_EXIT(RetVal)
#define  OS_TRACE_FLAG_POST_EXIT(RetVal)
#define  OS_TRACE_FLAG_PEND_EXIT(RetVal)
#define  OS_TRACE_MEM_PUT_EXIT(RetVal)
#define  OS_TRACE_MEM_GET_EXIT(RetVal)
#endif


/*
************************************************************************************************************************
*                                  uC/OS-III Trace Unused Macros (other recorders)
************************************************************************************************************************
*/

#define  OS_TRACE_MUTEX_DEL(p_mutex)
#define  OS_TRACE_MUTEX_POST(p_mutex)
#define  OS_TRACE_MUTEX_PEND(p_mutex)
#define  OS_TRACE_TASK_MSG_Q_POST(p_msg_q)
#define  OS_TRACE_TASK_MSG_Q_PEND(p_msg_q)
#define  OS_TRACE_TASK_SEM_POST(p_tcb)
#define  OS_TRACE_TASK_SEM_PEND(p_tcb)
#define  OS_TRACE_SEM_DEL(p_sem)
#define  OS_TRACE_SEM_POST(p_sem)
#define  OS_TRACE_SEM_PEND(p_sem)
#define  OS_TRACE_Q_DEL(p_q)
#define  OS_TRACE_Q_POST(p_q)
#define  OS_TRACE_Q_PEND(p_q)
#define  OS_TRACE_FLAG_DEL(p_grp)
#define  OS_TRACE_FLAG_POST(p_grp)
#define  OS_TRACE_FLAG_PEND(p_grp)
#define  OS_TRACE_MEM_PUT(p_mem)
#define  OS_TRACE_MEM_GET(p_mem)
#define  OS_TRACE_MUTEX_POST_FAILED(p_mutex)
#define  OS_TRACE_MUTEX_PEND_FAILED(p_mutex)
#define  OS_TRACE_MUTEX_PEND_BLOCK(p_mutex)
#define  OS_TRACE_TASK_CREATE_FAILED(p_tcb)
#define  OS_TRACE_TASK_MSG_Q_POST_FAILED(p_msg_q)
#define  OS_TRACE_TASK_MSG_Q_PEND_FAILED(p_msg_q)
#define  OS_TRACE_TASK_MSG_Q_PEND_BLOCK(p_msg_q)
#define  OS_TRACE_TASK_SEM_POST_FAILED(p_tcb)
#define  OS_TRACE_TASK_SEM_PEND_FAILED(p_tcb)
#define  OS_TRACE_TASK_SEM_PEND_BLOCK(p_tcb)
#define  OS_TRACE_SEM_POST_FAILED(p_sem)
#define  OS_TRACE_SEM_PEND_FAILED(p_sem)
#define  OS_TRACE_SEM_PEND_BLOCK(p_sem)
#define  OS_TRACE_Q_POST_FAILED(p_q)
#define  OS_TRACE_Q_PEND_FAILED(p_q)
#define  OS_TRACE_Q_PEND_BLOCK(p_q)
#define  OS_TRACE_FLAG_POST_FAILED(p_grp)
#define  OS_TRACE_FLAG_PEND_FAILED(p_grp)
#define  OS_TRACE_FLAG_PEND_BLOCK(p_grp)
#define  OS_TRACE_MEM_PUT_FAILED(p_mem)
#define  OS_TRACE_MEM_GET_FAILED(p_mem)
#define  OS_TRACE_TASK_PRIO_CHANGE(p_tcb, prio)

#else                                                           /* End of OS_CFG_TRACE_EN > 0                           */

#define  OS_TRACE_TICK_INCREMENT(OSTickCtr)

#define  OS_TRACE_TASK_CREATE(p_tcb)
#define  OS_TRACE_TASK_CREATE_FAILED(p_tcb)
#define  OS_TRACE_TASK_DEL(p_tcb)
#define  OS_TRACE_TASK_READY(p_tcb)
#define  OS_TRACE_TASK_SWITCHED_IN(p_tcb)
#define  OS_TRACE_TASK_DLY(dly_ticks)
#define  OS_TRACE_TASK_SUSPEND(p_tcb)
#define  OS_TRACE_TASK_SUSPENDED(p_tcb)
#define  OS_TRACE_TASK_RESUME(p_tcb)

#define  OS_TRACE_ISR_BEGIN(isr_id)
#define  OS_TRACE_ISR_END()

#define  OS_TRACE_ISR_ENTER()
#define  OS_TRACE_ISR_EXIT()
#define  OS_TRACE_ISR_EXIT_TO_SCHEDULER()

#define  OS_TRACE_TASK_MSG_Q_CREATE(p_msg_q, p_name)
#define  OS_TRACE_TASK_MSG_Q_POST(p_msg_q)
#define  OS_TRACE_TASK_MSG_Q_POST_FAILED(p_msg_q)
#define  OS_TRACE_TASK_MSG_Q_PEND(p_msg_q)
#define  OS_TRACE_TASK_MSG_Q_PEND_FAILED(p_msg_q)
#define  OS_TRACE_TASK_MSG_Q_PEND_BLOCK(p_msg_q)

#define  OS_TRACE_TASK_SEM_CREATE(p_tcb, p_name)
#define  OS_TRACE_TASK_SEM_POST(p_tcb)
#define  OS_TRACE_TASK_SEM_POST_FAILED(p_tcb)
#define  OS_TRACE_TASK_SEM_PEND(p_tcb)
#define  OS_TRACE_TASK_SEM_PEND_FAILED(p_tcb)
#define  OS_TRACE_TASK_SEM_PEND_BLOCK(p_tcb)

#define  OS_TRACE_MUTEX_CREATE(p_mutex, p_name)
#define  OS_TRACE_MUTEX_DEL(p_mutex)
#define  OS_TRACE_MUTEX_POST(p_mutex)
#define  OS_TRACE_MUTEX_POST_FAILED(p_mutex)
#define  OS_TRACE_MUTEX_PEND(p_mutex)
#define  OS_TRACE_MUTEX_PEND_FAILED(p_mutex)
#define  OS_TRACE_MUTEX_PEND_BLOCK(p_mutex)

#define  OS_TRACE_MUTEX_TASK_PRIO_INHERIT(p_tcb, prio)
#define  OS_TRACE_MUTEX_TASK_PRIO_DISINHERIT(p_tcb, prio)

#define  OS_TRACE_SEM_CREATE(p_sem, p_name)
#define  OS_TRACE_SEM_DEL(p_sem)
#define  OS_TRACE_SEM_POST(p_sem)
#define  OS_TRACE_SEM_POST_FAILED(p_sem)
#define  OS_TRACE_SEM_PEND(p_sem)
#define  OS_TRACE_SEM_PEND_FAILED(p_sem)
#define  OS_TRACE_SEM_PEND_BLOCK(p_sem)

#define  OS_TRACE_Q_CREATE(p_q, p_name)
#define  OS_TRACE_Q_DEL(p_q)
#define  OS_TRACE_Q_POST(p_q)
#define  OS_TRACE_Q_POST_FAILED(p_q)
#define  OS_TRACE_Q_PEND(p_q)
#define  OS_TRACE_Q_PEND_FAILED(p_q)
#define  OS_TRACE_Q_PEND_BLOCK(p_q)

#define  OS_TRACE_FLAG_CREATE(p_grp, p_name)
#define  OS_TRACE_FLAG_DEL(p_grp)
#define  OS_TRACE_FLAG_POST(p_grp)
#define  OS_TRACE_FLAG_POST_FAILED(p_grp)
#define  OS_TRACE_FLAG_PEND(p_grp)
#define  OS_TRACE_FLAG_PEND_FAILED(p_grp)
#define  OS_TRACE_FLAG_PEND_BLOCK(p_grp)

#define  OS_TRACE_MEM_CREATE(p_mem, p_name)
#define  OS_TRACE_MEM_PUT(p_mem)
#define  OS_TRACE_MEM_PUT_FAILED(p_mem)
#define  OS_TRACE_MEM_GET(p_mem)
#define  OS_TRACE_MEM_GET_FAILED(p_mem)

#define  OS_TRACE_TASK_PRIO_CHANGE(p_tcb, prio)

#define  OS_TRACE_MUTEX_DEL_ENTER(p_mutex, opt)
#define  OS_TRACE_MUTEX_POST_ENTER(p_mutex, opt)
#define  OS_TRACE_MUTEX_PEND_ENTER(p_mutex, timeout, opt, p_ts)
#define  OS_TRACE_TASK_MSG_Q_POST_ENTER(p_msg_q, p_void, msg_size, opt)
#define  OS_TRACE_TASK_MSG_Q_PEND_ENTER(p_msg_q, timeout, opt, p_msg_size, p_ts)
#define  OS_TRACE_TASK_SEM_POST_ENTER(p_tcb, opt)
#define  OS_TRACE_TASK_SEM_PEND_ENTER(p_tcb, timeout, opt, p_ts)
#define  OS_TRACE_SEM_DEL_ENTER(p_sem, opt)
#define  OS_TRACE_SEM_POST_ENTER(p_sem, opt)
#define  OS_TRACE_SEM_PEND_ENTER(p_sem, timeout, opt, p_ts)
#define  OS_TRACE_Q_DEL_ENTER(p_q, opt)
#define  OS_TRACE_Q_POST_ENTER(p_q, p_void, msg_size, opt)
#define  OS_TRACE_Q_PEND_ENTER(p_q, timeout, opt, p_msg_size, p_ts)
#define  OS_TRACE_FLAG_DEL_ENTER(p_grp, opt)
#define  OS_TRACE_FLAG_POST_ENTER(p_grp, flags, opt)
#define  OS_TRACE_FLAG_PEND_ENTER(p_grp, flags, timeout, opt, p_ts)
#define  OS_TRACE_MEM_PUT_ENTER(p_mem, p_blk)
#define  OS_TRACE_MEM_GET_ENTER(p_mem)

#define  OS_TRACE_MUTEX_DEL_EXIT(RetVal)
#define  OS_TRACE_MUTEX_POST_EXIT(RetVal)
#define  OS_TRACE_MUTEX_PEND_EXIT(RetVal)
#define  OS_TRACE_TASK_MSG_Q_POST_EXIT(RetVal)
#define  OS_TRACE_TASK_MSG_Q_PEND_EXIT(RetVal)
#define  OS_TRACE_TASK_SEM_POST_EXIT(RetVal)
#define  OS_TRACE_TASK_SEM_PEND_EXIT(RetVal)
#define  OS_TRACE_SEM_DEL_EXIT(RetVal)
#define  OS_TRACE_SEM_POST_EXIT(RetVal)
#define  OS_TRACE_SEM_PEND_EXIT(RetVal)
#define  OS_TRACE_Q_DEL_EXIT(RetVal)
#define  OS_TRACE_Q_POST_EXIT(RetVal)
#define  OS_TRACE_Q_PEND_EXIT(RetVal)
#define  OS_TRACE_FLAG_DEL_EXIT(RetVal)
#define  OS_TRACE_FLAG_POST_EXIT(RetVal)
#define  OS_TRACE_FLAG_PEND_EXIT(RetVal)
#define  OS_TRACE_MEM_PUT_EXIT(RetVal)
#define  OS_TRACE_MEM_GET_EXIT(RetVal)

#endif


/*
************************************************************************************************************************
*                                                   API Functions
************************************************************************************************************************
*/

#ifdef __cplusplus
extern "C" {
#endif
void  SYSVIEW_TaskReady      (U32 TaskID);
void  SYSVIEW_TaskSwitchedIn (U32 TaskID);
void  SYSVIEW_TaskSuspend    (U32 TaskID);
void  SYSVIEW_AddTask        (U32 TaskID, const char* NamePtr, OS_PRIO Prio, CPU_STK* StkBasePtr, CPU_STK_SIZE StkSize);
void  SYSVIEW_UpdateTask     (U32 TaskID, const char* NamePtr, OS_PRIO Prio, CPU_STK* StkBasePtr, CPU_STK_SIZE StkSize);
void  SYSVIEW_SendTaskInfo   (U32 TaskID, const char* sName, unsigned Prio, U32 StackBase, unsigned StackSize);
void  SYSVIEW_RecordU32x4    (unsigned Id, U32 Para0, U32 Para1, U32 Para2, U32 Para3);
void  SYSVIEW_RecordU32x5    (unsigned Id, U32 Para0, U32 Para1, U32 Para2, U32 Para3, U32 Para4);

void  SYSVIEW_RecordU32Register(unsigned EventId, U32 ResourceId, const char* sResource);
void  SYSVIEW_SendResourceList (void);

#ifdef __cplusplus
}
#endif
