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
* ---------------
*           uC/OS-II is provided in source form for FREE short-term evaluation, for educational use or
*           for peaceful research.  If you plan or intend to use uC/OS-II in a commercial application/
*           product then, you need to contact Micrium to properly license uC/OS-II for its use in your
*           application/product.   We provide ALL the source code for your convenience and to help you
*           experience uC/OS-II.  The fact that the source is provided does NOT mean that you can use
*           it commercially without paying a licensing fee.
*
*           Knowledge of the source code may NOT be used to develop a similar product.
*
*           Please help us continue to provide the embedded community with the finest software available.
*           Your honesty is greatly appreciated.
*
*           You can find our product's user manual, API reference, release notes and
*           more information at https://doc.micrium.com.
*           You can contact us at www.micrium.com.
************************************************************************************************************************
*/

#include "SEGGER_SYSVIEW.h"
#include "ucos_ii.h"
#include "os_cfg_trace.h"


/*
************************************************************************************************************************
*                                               uC/OS-II Trace Macros
************************************************************************************************************************
*/

#if (defined(OS_TRACE_EN) && (OS_TRACE_EN > 0u))
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
*                                     uC/OS-II Trace fixed defines for SystemView
************************************************************************************************************************
*/

#if (defined(OS_TRACE_EN) && (OS_TRACE_EN > 0u))
#define OS_TRACE_ID_OFFSET                           (32u)

#define OS_TRACE_ID_TICK_INCREMENT                   ( 1u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_ISR_REGISTER                     ( 2u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_MBOX_CREATE                      ( 3u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_MBOX_DEL                         ( 4u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_MBOX_POST                        ( 5u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_MBOX_POST_OPT                    ( 6u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_MBOX_PEND                        ( 7u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_MUTEX_CREATE                     ( 8u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_MUTEX_DEL                        ( 9u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_MUTEX_POST                       (10u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_MUTEX_PEND                       (11u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_MUTEX_TASK_PRIO_INHERIT          (12u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_MUTEX_TASK_PRIO_DISINHERIT       (13u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_SEM_CREATE                       (14u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_SEM_DEL                          (15u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_SEM_POST                         (16u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_SEM_PEND                         (17u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_Q_CREATE                         (18u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_Q_DEL                            (19u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_Q_POST                           (20u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_Q_POST_FRONT                     (21u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_Q_POST_OPT                       (22u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_Q_PEND                           (23u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_FLAG_CREATE                      (24u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_FLAG_DEL                         (25u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_FLAG_POST                        (26u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_FLAG_PEND                        (27u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_MEM_CREATE                       (28u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_MEM_PUT                          (29u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_MEM_GET                          (30u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_TMR_CREATE                       (31u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_TMR_DEL                          (32u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_TMR_START                        (33u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_TMR_STOP                         (34u + OS_TRACE_ID_OFFSET)
#define OS_TRACE_ID_TMR_EXPIRED                      (35u + OS_TRACE_ID_OFFSET)
#endif


/*
************************************************************************************************************************
*                                          uC/OS-II Trace Kernel-Related Macros
************************************************************************************************************************
*/

#if (defined(OS_TRACE_EN) && (OS_TRACE_EN > 0u))
#define  OS_TRACE_TASK_CREATE(p_tcb)                if (p_tcb != 0) {                              \
                                                      SEGGER_SYSVIEW_OnTaskCreate((U32)p_tcb);     \
                                                      SYSVIEW_AddTask((U32)p_tcb,                  \
                                                                      (const char *)(&(p_tcb->OSTCBTaskName[0])),  \
                                                                      p_tcb->OSTCBPrio,            \
                                                                      p_tcb->OSTCBStkBottom,       \
                                                                      p_tcb->OSTCBStkSize          \
                                                                      );                           \
                                                    }

#define  OS_TRACE_TASK_NAME_SET(p_tcb)              if (p_tcb != 0) {                              \
                                                      SYSVIEW_UpdateTask((U32)p_tcb,               \
                                                                      (const char *)(&(p_tcb->OSTCBTaskName[0])),  \
                                                                      p_tcb->OSTCBPrio,            \
                                                                      p_tcb->OSTCBStkBottom,       \
                                                                      p_tcb->OSTCBStkSize          \
                                                                      );                           \
                                                    }

#define  OS_TRACE_TASK_READY(p_tcb)                 SYSVIEW_TaskReady((U32)p_tcb)
#define  OS_TRACE_TASK_SWITCHED_IN(p_tcb)           SYSVIEW_TaskSwitchedIn((U32)p_tcb)
#define  OS_TRACE_TASK_DLY(dly_ticks)
#define  OS_TRACE_TASK_SUSPEND(p_tcb)
#define  OS_TRACE_TASK_SUSPENDED(p_tcb)             SYSVIEW_TaskSuspend((U32)p_tcb)
#define  OS_TRACE_TASK_RESUME(p_tcb)                SYSVIEW_TaskReady((U32)p_tcb)

#define  OS_TRACE_EVENT_NAME_SET(p_event, p_name)   if (p_event != 0 && p_name != 0) {              \
                                                      SYSVIEW_UpdateResource((U32)p_event,          \
                                                                      (const char *)(p_name));      \
                                                    }

#define  OS_TRACE_ISR_BEGIN(isr_id)
#define  OS_TRACE_ISR_END()

#define  OS_TRACE_ISR_ENTER()                       SEGGER_SYSVIEW_RecordEnterISR()
#define  OS_TRACE_ISR_EXIT()                        SEGGER_SYSVIEW_RecordExitISR()
#define  OS_TRACE_ISR_EXIT_TO_SCHEDULER()           SEGGER_SYSVIEW_RecordExitISRToScheduler()


/*
************************************************************************************************************************
*                                      uC/OS-II Trace Simple Recorder Functions
************************************************************************************************************************
*/

#define  OS_TRACE_TICK_INCREMENT(OSTime)                   SEGGER_SYSVIEW_RecordU32  (OS_TRACE_ID_TICK_INCREMENT,             (U32)OSTime)
#define  OS_TRACE_ISR_REGISTER(isr_id, isr_name, isr_prio) SEGGER_SYSVIEW_RecordU32x2(OS_TRACE_ID_ISR_REGISTER,               (U32)isr_id,                         (U32)isr_prio)
#define  OS_TRACE_MUTEX_TASK_PRIO_INHERIT(p_tcb, prio)     SEGGER_SYSVIEW_RecordU32x2(OS_TRACE_ID_MUTEX_TASK_PRIO_INHERIT,    SEGGER_SYSVIEW_ShrinkId((U32)p_tcb), (U32)prio);
#define  OS_TRACE_MUTEX_TASK_PRIO_DISINHERIT(p_tcb, prio)  SEGGER_SYSVIEW_RecordU32x2(OS_TRACE_ID_MUTEX_TASK_PRIO_DISINHERIT, SEGGER_SYSVIEW_ShrinkId((U32)p_tcb), (U32)prio);
#define  OS_TRACE_TASK_DEL(p_tcb)                             


/*
************************************************************************************************************************
*                                      uC/OS-II Trace Complex Recorder Functions
************************************************************************************************************************
*/

#define  OS_TRACE_MBOX_CREATE(p_mbox, p_name)        SYSVIEW_RecordU32Register(OS_TRACE_ID_MBOX_CREATE,       ((U32)p_mbox),  (const char *)p_name)
#define  OS_TRACE_MUTEX_CREATE(p_mutex, p_name)      SYSVIEW_RecordU32Register(OS_TRACE_ID_MUTEX_CREATE,      ((U32)p_mutex), (const char *)p_name)
#define  OS_TRACE_SEM_CREATE(p_sem, p_name)          SYSVIEW_RecordU32Register(OS_TRACE_ID_SEM_CREATE,        ((U32)p_sem),   (const char *)p_name)
#define  OS_TRACE_Q_CREATE(p_q, p_name)              SYSVIEW_RecordU32Register(OS_TRACE_ID_Q_CREATE,          ((U32)p_q),     (const char *)p_name)
#define  OS_TRACE_FLAG_CREATE(p_grp, p_name)         SYSVIEW_RecordU32Register(OS_TRACE_ID_FLAG_CREATE,       ((U32)p_grp),   (const char *)p_name)
#define  OS_TRACE_MEM_CREATE(p_mem)                  SYSVIEW_RecordU32Register(OS_TRACE_ID_MEM_CREATE,        ((U32)p_mem),   (const char *)"Mem Block")
#define  OS_TRACE_TMR_CREATE(p_tmr, p_name)          SYSVIEW_RecordU32Register(OS_TRACE_ID_TMR_CREATE,        ((U32)p_tmr),   (const char *)p_name)


/*
************************************************************************************************************************
*                                         uC/OS-II Trace API Enter Functions
************************************************************************************************************************
*/

#if (defined(OS_TRACE_API_ENTER_EN) && (OS_TRACE_API_ENTER_EN > 0u))
#define  OS_TRACE_MBOX_DEL_ENTER(p_mbox, opt)                                      SEGGER_SYSVIEW_RecordU32x2(OS_TRACE_ID_MBOX_DEL,        SEGGER_SYSVIEW_ShrinkId((U32)p_mbox),  (U32)opt)
#define  OS_TRACE_MBOX_POST_ENTER(p_mbox)                                          SEGGER_SYSVIEW_RecordU32  (OS_TRACE_ID_MBOX_POST,       SEGGER_SYSVIEW_ShrinkId((U32)p_mbox))
#define  OS_TRACE_MBOX_POST_OPT_ENTER(p_mbox, opt)                                 SEGGER_SYSVIEW_RecordU32x2(OS_TRACE_ID_MBOX_POST_OPT,   SEGGER_SYSVIEW_ShrinkId((U32)p_mbox),  (U32)opt)
#define  OS_TRACE_MBOX_PEND_ENTER(p_mbox, timeout)                                 SEGGER_SYSVIEW_RecordU32x2(OS_TRACE_ID_MBOX_PEND,       SEGGER_SYSVIEW_ShrinkId((U32)p_mbox),  (U32)timeout)
#define  OS_TRACE_MUTEX_DEL_ENTER(p_mutex, opt)                                    SEGGER_SYSVIEW_RecordU32x2(OS_TRACE_ID_MUTEX_DEL,       SEGGER_SYSVIEW_ShrinkId((U32)p_mutex), (U32)opt)
#define  OS_TRACE_MUTEX_POST_ENTER(p_mutex)                                        SEGGER_SYSVIEW_RecordU32  (OS_TRACE_ID_MUTEX_POST,      SEGGER_SYSVIEW_ShrinkId((U32)p_mutex))
#define  OS_TRACE_MUTEX_PEND_ENTER(p_mutex, timeout)                               SEGGER_SYSVIEW_RecordU32x2(OS_TRACE_ID_MUTEX_PEND,      SEGGER_SYSVIEW_ShrinkId((U32)p_mutex), (U32)timeout)
#define  OS_TRACE_SEM_DEL_ENTER(p_sem, opt)                                        SEGGER_SYSVIEW_RecordU32x2(OS_TRACE_ID_SEM_DEL,         SEGGER_SYSVIEW_ShrinkId((U32)p_sem),   (U32)opt)
#define  OS_TRACE_SEM_POST_ENTER(p_sem)                                            SEGGER_SYSVIEW_RecordU32  (OS_TRACE_ID_SEM_POST,        SEGGER_SYSVIEW_ShrinkId((U32)p_sem))
#define  OS_TRACE_SEM_PEND_ENTER(p_sem, timeout)                                   SEGGER_SYSVIEW_RecordU32x2(OS_TRACE_ID_SEM_PEND,        SEGGER_SYSVIEW_ShrinkId((U32)p_sem),   (U32)timeout)
#define  OS_TRACE_Q_DEL_ENTER(p_q, opt)                                            SEGGER_SYSVIEW_RecordU32x2(OS_TRACE_ID_Q_DEL,           SEGGER_SYSVIEW_ShrinkId((U32)p_q),     (U32)opt)
#define  OS_TRACE_Q_POST_ENTER(p_q)                                                SEGGER_SYSVIEW_RecordU32  (OS_TRACE_ID_Q_POST,          SEGGER_SYSVIEW_ShrinkId((U32)p_q))
#define  OS_TRACE_Q_POST_FRONT_ENTER(p_q)                                          SEGGER_SYSVIEW_RecordU32  (OS_TRACE_ID_Q_POST_FRONT,    SEGGER_SYSVIEW_ShrinkId((U32)p_q))
#define  OS_TRACE_Q_POST_OPT_ENTER(p_q, opt)                                       SEGGER_SYSVIEW_RecordU32x2(OS_TRACE_ID_Q_POST_OPT,      SEGGER_SYSVIEW_ShrinkId((U32)p_q),     (U32)opt)
#define  OS_TRACE_Q_PEND_ENTER(p_q, timeout)                                       SEGGER_SYSVIEW_RecordU32x2(OS_TRACE_ID_Q_PEND,          SEGGER_SYSVIEW_ShrinkId((U32)p_q),     (U32)timeout)
#define  OS_TRACE_FLAG_DEL_ENTER(p_grp, opt)                                       SEGGER_SYSVIEW_RecordU32x2(OS_TRACE_ID_FLAG_DEL,        SEGGER_SYSVIEW_ShrinkId((U32)p_grp),   (U32)opt)
#define  OS_TRACE_FLAG_POST_ENTER(p_grp, flags, opt)                               SEGGER_SYSVIEW_RecordU32x3(OS_TRACE_ID_FLAG_POST,       SEGGER_SYSVIEW_ShrinkId((U32)p_grp),   (U32)flags,    (U32)opt)
#define  OS_TRACE_FLAG_PEND_ENTER(p_grp, flags, timeout, opt)                      SYSVIEW_RecordU32x4       (OS_TRACE_ID_FLAG_PEND,       SEGGER_SYSVIEW_ShrinkId((U32)p_grp),   (U32)flags,    (U32)timeout, (U32)opt)
#define  OS_TRACE_MEM_PUT_ENTER(p_mem, p_blk)                                      SEGGER_SYSVIEW_RecordU32x2(OS_TRACE_ID_MEM_PUT,         SEGGER_SYSVIEW_ShrinkId((U32)p_mem),   (U32)p_blk)
#define  OS_TRACE_MEM_GET_ENTER(p_mem)                                             SEGGER_SYSVIEW_RecordU32  (OS_TRACE_ID_MEM_GET,         SEGGER_SYSVIEW_ShrinkId((U32)p_mem))
#define  OS_TRACE_TMR_DEL_ENTER(p_tmr)                                             SEGGER_SYSVIEW_RecordU32  (OS_TRACE_ID_TMR_DEL,         SEGGER_SYSVIEW_ShrinkId((U32)p_tmr))
#define  OS_TRACE_TMR_START_ENTER(p_tmr)                                           SEGGER_SYSVIEW_RecordU32  (OS_TRACE_ID_TMR_START,       SEGGER_SYSVIEW_ShrinkId((U32)p_tmr))
#define  OS_TRACE_TMR_STOP_ENTER(p_tmr)                                            SEGGER_SYSVIEW_RecordU32  (OS_TRACE_ID_TMR_STOP,        SEGGER_SYSVIEW_ShrinkId((U32)p_tmr))
#define  OS_TRACE_TMR_EXPIRED(p_tmr)                                               SEGGER_SYSVIEW_RecordU32  (OS_TRACE_ID_TMR_EXPIRED,     SEGGER_SYSVIEW_ShrinkId((U32)p_tmr))
#else
#define  OS_TRACE_MBOX_DEL_ENTER(p_mbox, opt)     
#define  OS_TRACE_MBOX_POST_ENTER(p_mbox)
#define  OS_TRACE_MBOX_POST_OPT_ENTER(p_mbox, opt)
#define  OS_TRACE_MBOX_PEND_ENTER(p_mbox, timeout)
#define  OS_TRACE_MUTEX_DEL_ENTER(p_mutex, opt)
#define  OS_TRACE_MUTEX_POST_ENTER(p_mutex)
#define  OS_TRACE_MUTEX_PEND_ENTER(p_mutex, timeout)
#define  OS_TRACE_SEM_DEL_ENTER(p_sem, opt)
#define  OS_TRACE_SEM_POST_ENTER(p_sem)
#define  OS_TRACE_SEM_PEND_ENTER(p_sem, timeout)
#define  OS_TRACE_Q_DEL_ENTER(p_q, opt)
#define  OS_TRACE_Q_POST_ENTER(p_q)
#define  OS_TRACE_Q_POST_FRONT_ENTER(p_q)
#define  OS_TRACE_Q_POST_OPT_ENTER(p_q, opt)
#define  OS_TRACE_Q_PEND_ENTER(p_q, timeout)
#define  OS_TRACE_FLAG_DEL_ENTER(p_grp, opt)
#define  OS_TRACE_FLAG_POST_ENTER(p_grp, flags, opt)
#define  OS_TRACE_FLAG_PEND_ENTER(p_grp, flags, timeout, opt)
#define  OS_TRACE_MEM_PUT_ENTER(p_mem, p_blk)
#define  OS_TRACE_MEM_GET_ENTER(p_mem)
#define  OS_TRACE_TMR_DEL_ENTER(p_tmr)
#define  OS_TRACE_TMR_START_ENTER(p_tmr) 
#define  OS_TRACE_TMR_STOP_ENTER(p_tmr)
#define  OS_TRACE_TMR_EXPIRED(p_tmr)
#endif


/*
************************************************************************************************************************
*                                         uC/OS-II Trace API Exit Functions
************************************************************************************************************************
*/

#if (defined(OS_TRACE_API_EXIT_EN) && (OS_TRACE_API_EXIT_EN > 0u))
#define  OS_TRACE_MBOX_DEL_EXIT(RetVal)                                                   SEGGER_SYSVIEW_RecordEndCallU32(OS_TRACE_ID_MBOX_DEL,        RetVal)
#define  OS_TRACE_MBOX_POST_EXIT(RetVal)                                                  SEGGER_SYSVIEW_RecordEndCallU32(OS_TRACE_ID_MBOX_POST,       RetVal)
#define  OS_TRACE_MBOX_POST_OPT_EXIT(RetVal)                                              SEGGER_SYSVIEW_RecordEndCallU32(OS_TRACE_ID_MBOX_POST_OPT,   RetVal)
#define  OS_TRACE_MBOX_PEND_EXIT(RetVal)                                                  SEGGER_SYSVIEW_RecordEndCallU32(OS_TRACE_ID_MBOX_PEND,       RetVal)
#define  OS_TRACE_MUTEX_DEL_EXIT(RetVal)                                                  SEGGER_SYSVIEW_RecordEndCallU32(OS_TRACE_ID_MUTEX_DEL,       RetVal)
#define  OS_TRACE_MUTEX_POST_EXIT(RetVal)                                                 SEGGER_SYSVIEW_RecordEndCallU32(OS_TRACE_ID_MUTEX_POST,      RetVal)
#define  OS_TRACE_MUTEX_PEND_EXIT(RetVal)                                                 SEGGER_SYSVIEW_RecordEndCallU32(OS_TRACE_ID_MUTEX_PEND,      RetVal)
#define  OS_TRACE_SEM_DEL_EXIT(RetVal)                                                    SEGGER_SYSVIEW_RecordEndCallU32(OS_TRACE_ID_SEM_DEL,         RetVal)
#define  OS_TRACE_SEM_POST_EXIT(RetVal)                                                   SEGGER_SYSVIEW_RecordEndCallU32(OS_TRACE_ID_SEM_POST,        RetVal)
#define  OS_TRACE_SEM_PEND_EXIT(RetVal)                                                   SEGGER_SYSVIEW_RecordEndCallU32(OS_TRACE_ID_SEM_PEND,        RetVal)
#define  OS_TRACE_Q_DEL_EXIT(RetVal)                                                      SEGGER_SYSVIEW_RecordEndCallU32(OS_TRACE_ID_Q_DEL,           RetVal)
#define  OS_TRACE_Q_POST_EXIT(RetVal)                                                     SEGGER_SYSVIEW_RecordEndCallU32(OS_TRACE_ID_Q_POST,          RetVal)
#define  OS_TRACE_Q_POST_FRONT_EXIT(RetVal)                                               SEGGER_SYSVIEW_RecordEndCallU32(OS_TRACE_ID_Q_POST_FRONT,    RetVal)
#define  OS_TRACE_Q_POST_OPT_EXIT(RetVal)                                                 SEGGER_SYSVIEW_RecordEndCallU32(OS_TRACE_ID_Q_POST_OPT,      RetVal)
#define  OS_TRACE_Q_PEND_EXIT(RetVal)                                                     SEGGER_SYSVIEW_RecordEndCallU32(OS_TRACE_ID_Q_PEND,          RetVal)
#define  OS_TRACE_FLAG_DEL_EXIT(RetVal)                                                   SEGGER_SYSVIEW_RecordEndCallU32(OS_TRACE_ID_FLAG_DEL,        RetVal)
#define  OS_TRACE_FLAG_POST_EXIT(RetVal)                                                  SEGGER_SYSVIEW_RecordEndCallU32(OS_TRACE_ID_FLAG_POST,       RetVal)
#define  OS_TRACE_FLAG_PEND_EXIT(RetVal)                                                  SEGGER_SYSVIEW_RecordEndCallU32(OS_TRACE_ID_FLAG_PEND,       RetVal)
#define  OS_TRACE_MEM_PUT_EXIT(RetVal)                                                    SEGGER_SYSVIEW_RecordEndCallU32(OS_TRACE_ID_MEM_PUT,         RetVal)
#define  OS_TRACE_MEM_GET_EXIT(RetVal)                                                    SEGGER_SYSVIEW_RecordEndCallU32(OS_TRACE_ID_MEM_GET,         RetVal)
#define  OS_TRACE_TMR_DEL_EXIT(RetVal)                                                    SEGGER_SYSVIEW_RecordEndCallU32(OS_TRACE_ID_TMR_DEL,         RetVal)
#define  OS_TRACE_TMR_START_EXIT(RetVal)                                                  SEGGER_SYSVIEW_RecordEndCallU32(OS_TRACE_ID_TMR_START,       RetVal)
#define  OS_TRACE_TMR_STOP_EXIT(RetVal)                                                   SEGGER_SYSVIEW_RecordEndCallU32(OS_TRACE_ID_TMR_STOP,        RetVal)
#else
#define  OS_TRACE_MBOX_DEL_EXIT(RetVal)
#define  OS_TRACE_MBOX_POST_EXIT(RetVal)
#define  OS_TRACE_MBOX_POST_OPT_EXIT(RetVal)
#define  OS_TRACE_MBOX_PEND_EXIT(RetVal)
#define  OS_TRACE_MUTEX_DEL_EXIT(RetVal)
#define  OS_TRACE_MUTEX_POST_EXIT(RetVal)
#define  OS_TRACE_MUTEX_PEND_EXIT(RetVal)
#define  OS_TRACE_SEM_DEL_EXIT(RetVal)
#define  OS_TRACE_SEM_POST_EXIT(RetVal)
#define  OS_TRACE_SEM_PEND_EXIT(RetVal)
#define  OS_TRACE_Q_DEL_EXIT(RetVal)
#define  OS_TRACE_Q_POST_EXIT(RetVal)
#define  OS_TRACE_Q_POST_FRONT_EXIT(RetVal)
#define  OS_TRACE_Q_POST_OPT_EXIT(RetVal)
#define  OS_TRACE_Q_PEND_EXIT(RetVal)
#define  OS_TRACE_FLAG_DEL_EXIT(RetVal)
#define  OS_TRACE_FLAG_POST_EXIT(RetVal)
#define  OS_TRACE_FLAG_PEND_EXIT(RetVal)
#define  OS_TRACE_MEM_PUT_EXIT(RetVal)
#define  OS_TRACE_MEM_GET_EXIT(RetVal)
#define  OS_TRACE_TMR_DEL_EXIT(RetVal)
#define  OS_TRACE_TMR_START_EXIT(RetVal)
#define  OS_TRACE_TMR_STOP_EXIT(RetVal)
#endif


/*
************************************************************************************************************************
*                                  uC/OS-II Trace Unused Macros (other recorders)
************************************************************************************************************************
*/

#define  OS_TRACE_MUTEX_DEL(p_mutex)
#define  OS_TRACE_MUTEX_POST(p_mutex)
#define  OS_TRACE_MUTEX_PEND(p_mutex)
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

#else                                                           /* End of OS_TRACE_EN > 0                               */

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

#define  OS_TRACE_MEM_CREATE(p_mem)
#define  OS_TRACE_MEM_PUT(p_mem)
#define  OS_TRACE_MEM_PUT_FAILED(p_mem)
#define  OS_TRACE_MEM_GET(p_mem)
#define  OS_TRACE_MEM_GET_FAILED(p_mem)

#define  OS_TRACE_TMR_CREATE(p_tmr, p_name)

#define  OS_TRACE_TASK_PRIO_CHANGE(p_tcb, prio)

#define  OS_TRACE_MBOX_DEL_ENTER(p_mbox, opt)     
#define  OS_TRACE_MBOX_POST_ENTER(p_mbox)
#define  OS_TRACE_MBOX_POST_OPT_ENTER(p_mbox, opt)
#define  OS_TRACE_MBOX_PEND_ENTER(p_mbox, timeout)
#define  OS_TRACE_MUTEX_DEL_ENTER(p_mutex, opt)
#define  OS_TRACE_MUTEX_POST_ENTER(p_mutex)
#define  OS_TRACE_MUTEX_PEND_ENTER(p_mutex, timeout)
#define  OS_TRACE_SEM_DEL_ENTER(p_sem, opt)
#define  OS_TRACE_SEM_POST_ENTER(p_sem)
#define  OS_TRACE_SEM_PEND_ENTER(p_sem, timeout)
#define  OS_TRACE_Q_DEL_ENTER(p_q, opt)
#define  OS_TRACE_Q_POST_ENTER(p_q)
#define  OS_TRACE_Q_POST_FRONT_ENTER(p_q)
#define  OS_TRACE_Q_POST_OPT_ENTER(p_q, opt)
#define  OS_TRACE_Q_PEND_ENTER(p_q, timeout)
#define  OS_TRACE_FLAG_DEL_ENTER(p_grp, opt)
#define  OS_TRACE_FLAG_POST_ENTER(p_grp, flags, opt)
#define  OS_TRACE_FLAG_PEND_ENTER(p_grp, flags, timeout, opt)
#define  OS_TRACE_MEM_PUT_ENTER(p_mem, p_blk)
#define  OS_TRACE_MEM_GET_ENTER(p_mem)
#define  OS_TRACE_TMR_DEL_ENTER(p_tmr)
#define  OS_TRACE_TMR_START_ENTER(p_tmr) 
#define  OS_TRACE_TMR_STOP_ENTER(p_tmr)  
#define  OS_TRACE_TMR_EXPIRED(p_tmr)

#define  OS_TRACE_MBOX_DEL_EXIT(RetVal)
#define  OS_TRACE_MBOX_POST_EXIT(RetVal)
#define  OS_TRACE_MBOX_POST_OPT_EXIT(RetVal)
#define  OS_TRACE_MBOX_PEND_EXIT(RetVal)
#define  OS_TRACE_MUTEX_DEL_EXIT(RetVal)
#define  OS_TRACE_MUTEX_POST_EXIT(RetVal)
#define  OS_TRACE_MUTEX_PEND_EXIT(RetVal)
#define  OS_TRACE_SEM_DEL_EXIT(RetVal)
#define  OS_TRACE_SEM_POST_EXIT(RetVal)
#define  OS_TRACE_SEM_PEND_EXIT(RetVal)
#define  OS_TRACE_Q_DEL_EXIT(RetVal)
#define  OS_TRACE_Q_POST_EXIT(RetVal)
#define  OS_TRACE_Q_POST_FRONT_EXIT(RetVal)
#define  OS_TRACE_Q_POST_OPT_EXIT(RetVal)
#define  OS_TRACE_Q_PEND_EXIT(RetVal)
#define  OS_TRACE_FLAG_DEL_EXIT(RetVal)
#define  OS_TRACE_FLAG_POST_EXIT(RetVal)
#define  OS_TRACE_FLAG_PEND_EXIT(RetVal)
#define  OS_TRACE_MEM_PUT_EXIT(RetVal)
#define  OS_TRACE_MEM_GET_EXIT(RetVal)
#define  OS_TRACE_TMR_DEL_EXIT(RetVal)
#define  OS_TRACE_TMR_START_EXIT(RetVal)
#define  OS_TRACE_TMR_STOP_EXIT(RetVal)

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
void  SYSVIEW_AddTask        (U32 TaskID, const char* OSTCBTaskName, INT8U OSTCBPrio, OS_STK* OSTCBStkBottom, INT32U OSTCBStkSize);
void  SYSVIEW_UpdateTask     (U32 TaskID, const char* OSTCBTaskName, INT8U OSTCBPrio, OS_STK* OSTCBStkBottom, INT32U OSTCBStkSize);
void  SYSVIEW_UpdateResource (U32 EventID, const char* OSEventName);
void  SYSVIEW_SendTaskInfo   (U32 TaskID, const char* sName, unsigned Prio, U32 StackBase, unsigned StackSize);
void  SYSVIEW_RecordU32x4    (unsigned Id, U32 Para0, U32 Para1, U32 Para2, U32 Para3);
void  SYSVIEW_RecordU32x5    (unsigned Id, U32 Para0, U32 Para1, U32 Para2, U32 Para3, U32 Para4);

void  SYSVIEW_RecordU32Register(unsigned EventId, U32 ResourceId, const char* sResource);
void  SYSVIEW_SendResourceList (void);

#ifdef __cplusplus
}
#endif
