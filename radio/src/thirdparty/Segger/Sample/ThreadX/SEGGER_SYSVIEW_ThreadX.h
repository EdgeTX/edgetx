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

File    : SEGGER_SYSVIEW_ThreadX.h
Purpose : Interface between ThreadX and SystemView.

Revision: $Rev: 7745 $

Notes:

*/

#include "SEGGER_SYSVIEW.h"

#ifndef SEGGER_SEGGER_SYSVIEW_THREADX_H_
#define SEGGER_SEGGER_SYSVIEW_THREADX_H_

#define TRACE_API_TX_KERNEL_ENTER                             (32u)
#define TRACE_API_TX_BLOCK_ALLOCATE                           (33u)
#define TRACE_API_TX_BLOCK_POOL_CREATE                        (34u)
#define TRACE_API_TX_BLOCK_POOL_DELETE                        (35u)
#define TRACE_API_TX_BLOCK_POOL_INFO_GET                      (36u)
#define TRACE_API_TX_BLOCK_POOL_PERFORMANCE_INFO_GET          (37u)
#define TRACE_API_TX_BLOCK_POOL_PERFORMANCE_SYSTEM_INFO_GET   (38u)
#define TRACE_API_TX_BLOCK_PRIORITIZE                         (39u)
#define TRACE_API_TX_BLOCK_RELEASE                            (40u)
#define TRACE_API_TX_BYTE_ALLOCATE                            (41u)
#define TRACE_API_TX_BYTE_POOL_CREATE                         (42u)
#define TRACE_API_TX_BYTE_POOL_DELETE                         (43u)
#define TRACE_API_TX_BYTE_POOL_INFO_GET                       (44u)
#define TRACE_API_TX_BYTE_POOL_PERFORMANCE_INFO_GET           (45u)
#define TRACE_API_TX_BYTE_POOL_PERFORMANCE_SYSTEM_INFO_GET    (46u)
#define TRACE_API_TX_BYTE_POOL_PRIORITIZE                     (47u)
#define TRACE_API_TX_BYTE_POOL_RELEASE                        (48u)
#define TRACE_API_TX_EVENT_FLAGS_CREATE                       (49u)
#define TRACE_API_TX_EVENT_FLAGS_DELETE                       (50u)
#define TRACE_API_TX_EVENT_FLAGS_GET                          (51u)
#define TRACE_API_TX_EVENT_FLAGS_INFO_GET                     (52u)
#define TRACE_API_TX_EVENT_FLAGS_PERFORMANCE_INFO_GET         (53u)
#define TRACE_API_TX_EVENT_FLAGS_PERFORMANCE_SYSTEM_INFO_GET  (54u)
#define TRACE_API_TX_EVENT_FLAGS_SET                          (55u)
#define TRACE_API_TX_EVENT_FLAGS_NOTIFY                       (56u)
#define TRACE_API_TX_INTERRUPT_CONTROL                        (57u)
#define TRACE_API_TX_MUTEX_CREATE                             (58u)
#define TRACE_API_TX_MUTEX_DELETE                             (59u)
#define TRACE_API_TX_MUTEX_GET                                (60u)
#define TRACE_API_TX_MUTEX_INFO_GET                           (61u)
#define TRACE_API_TX_MUTEX_PERFORMANCE_INFO_GET               (62u)
#define TRACE_API_TX_MUTEX_PERFORMANCE_SYSTEM_INFO_GET        (63u)
#define TRACE_API_TX_MUTEX_PRIORITIZE                         (64u)
#define TRACE_API_TX_MUTEX_PUT                                (65u)
#define TRACE_API_TX_QUEUE_CREATE                             (66u)
#define TRACE_API_TX_QUEUE_DELETE                             (67u)
#define TRACE_API_TX_QUEUE_FLUSH                              (68u)
#define TRACE_API_TX_QUEUE_FRONT_SEND                         (69u)
#define TRACE_API_TX_QUEUE_INFO_GET                           (70u)
#define TRACE_API_TX_QUEUE_PERFORMACE_INFO_GET                (71u)
#define TRACE_API_TX_QUEUE_PERFORMACE_SYSTEM_INFO_GET         (72u)
#define TRACE_API_TX_QUEUE_PRIORITIZE                         (73u)
#define TRACE_API_TX_QUEUE_RECEIVE                            (74u)
#define TRACE_API_TX_QUEUE_SEND                               (75u)
#define TRACE_API_TX_QUEUE_NOTIFY                             (76u)
#define TRACE_API_TX_SEMAPHORE_CEILING_PUT                    (77u)
#define TRACE_API_TX_SEMAPHORE_CREATE                         (78u)
#define TRACE_API_TX_SEMAPHORE_DELETE                         (79u)
#define TRACE_API_TX_SEMAPHORE_GET                            (80u)
#define TRACE_API_TX_SEMAPHORE_INFO_GET                       (81u)
#define TRACE_API_TX_SEMAPHORE_PERFORMANCE_INFO_GET           (82u)
#define TRACE_API_TX_SEMAPHORE_PERFORMANCE_SYSTEM_INFO_GET    (83u)
#define TRACE_API_TX_SEMAPHORE_PRIORITIZE                     (84u)
#define TRACE_API_TX_SEMAPHORE_PUT                            (85u)
#define TRACE_API_TX_SEMAPHORE_PUT_NOTIFY                     (86u)
#define TRACE_API_TX_THREAD_CREATE                            (87u)
#define TRACE_API_TX_THREAD_DELETE                            (88u)
#define TRACE_API_TX_THREAD_ENTRY_EXIT                        (89u)
#define TRACE_API_TX_THREAD_IDENTIFY                          (90u)
#define TRACE_API_TX_THREAD_INFO_GET                          (91u)
#define TRACE_API_TX_THREAD_PERFOMANCE_INFO_GET               (92u)
#define TRACE_API_TX_THREAD_PERFOMANCE_SYSTEM_INFO_GET        (93u)
#define TRACE_API_TX_THREAD_PREEMTION_CHANGE                  (94u)
#define TRACE_API_TX_THREAD_PRIORITY_CHANGE                   (95u)
#define TRACE_API_TX_THREAD_RELINQUISH                        (96u)
#define TRACE_API_TX_THREAD_RESET                             (97u)
#define TRACE_API_TX_THREAD_RESUME                            (98u)
#define TRACE_API_TX_THREAD_SLEEP                             (99u)
#define TRACE_API_TX_THREAD_STACK_ERROR_NOTIFY                (100u)
#define TRACE_API_TX_THREAD_SUSPEND                           (101u)
#define TRACE_API_TX_THREAD_TERMINATE                         (102u)
#define TRACE_API_TX_THREAD_TIME_SLICE_CHANGE                 (103u)
#define TRACE_API_TX_THREAD_WAIT_ABORT                        (104u)
#define TRACE_API_TX_TIME_GET                                 (105u)
#define TRACE_API_TX_TIME_SET                                 (106u)
#define TRACE_API_TX_TIMER_ACTIVATE                           (107u)
#define TRACE_API_TX_TIMER_CHANGE                             (108u)
#define TRACE_API_TX_TIMER_CREATE                             (109u)
#define TRACE_API_TX_TIMER_DEACTIVATE                         (110u)
#define TRACE_API_TX_TIMER_DELETE                             (111u)
#define TRACE_API_TX_TIMER_INFO_GET                           (112u)
#define TRACE_API_TX_TIMER_PERFORMANCE_INFO_GET               (113u)
#define TRACE_API_TX_TIMER_PERFORMANCE_SYSTEM_INFO_GET        (114u)
#define TRACE_API_TXE_BLOCK_ALLOCATE                          (115u)
#define TRACE_API_TXE_BLOCK_POOL_CREATE                       (116u)
#define TRACE_API_TXE_BLOCK_POOL_DELETE                       (117u)
#define TRACE_API_TXE_BLOCK_POOL_INFO_GET                     (118u)
#define TRACE_API_TXE_BLOCK_PRIORITIZE                        (119u)
#define TRACE_API_TXE_BLOCK_RELEASE                           (120u)
#define TRACE_API_TXE_BYTE_ALLOCATE                           (121u)
#define TRACE_API_TXE_BYTE_POOL_CREATE                        (122u)
#define TRACE_API_TXE_BYTE_POOL_DELETE                        (123u)
#define TRACE_API_TXE_BYTE_POOL_INFO_GET                      (124u)
#define TRACE_API_TXE_BYTE_PRIORITIZE                         (125u)
#define TRACE_API_TXE_BYTE_RELEASE                            (126u)
#define TRACE_API_TXE_EVENT_FLAGS_CREATE                      (127u)
#define TRACE_API_TXE_EVENT_FLAGS_DELETE                      (128u)
#define TRACE_API_TXE_EVENT_FLAGS_GET                         (129u)
#define TRACE_API_TXE_EVENT_FLAGS_INFO_GET                    (130u)
#define TRACE_API_TXE_EVENT_FLAGS_SET                         (131u)
#define TRACE_API_TXE_EVENT_FLAGS_NOTIFY                      (132u)
#define TRACE_API_TXE_MUTEX_CREATE                            (133u)
#define TRACE_API_TXE_MUTEX_DELETE                            (134u)
#define TRACE_API_TXE_MUTEX_GET                               (135u)
#define TRACE_API_TXE_MUTEX_INFO_GET                          (136u)
#define TRACE_API_TXE_MUTEX_PRIORITIZE                        (137u)
#define TRACE_API_TXE_MUTEX_PUT                               (138u)
#define TRACE_API_TXE_QUEUE_CREATE                            (139u)
#define TRACE_API_TXE_QUEUE_DELETE                            (140u)
#define TRACE_API_TXE_QUEUE_FLUSH                             (141u)
#define TRACE_API_TXE_QUEUE_FRONT_SEND                        (142u)
#define TRACE_API_TXE_QUEUE_INFO_GET                          (143u)
#define TRACE_API_TXE_QUEUE_PRIORITIZE                        (144u)
#define TRACE_API_TXE_QUEUE_RECEIVE                           (145u)
#define TRACE_API_TXE_QUEUE_SEND                              (146u)
#define TRACE_API_TXE_QUEUE_NOTIFY                            (147u)
#define TRACE_API_TXE_SEMAPHORE_CEILING_PUT                   (148u)
#define TRACE_API_TXE_SEMAPHORE_CREATE                        (149u)
#define TRACE_API_TXE_SEMAPHORE_DELETE                        (150u)
#define TRACE_API_TXE_SEMAPHORE_GET                           (151u)
#define TRACE_API_TXE_SEMAPHORE_INFO_GET                      (152u)
#define TRACE_API_TXE_SEMAPHORE_PRIORITIZE                    (153u)
#define TRACE_API_TXE_SEMAPHORE_PUT                           (154u)
#define TRACE_API_TXE_SEMAPHORE_PUT_NOTIFY                    (155u)
#define TRACE_API_TXE_THREAD_CREATE                           (156u)
#define TRACE_API_TXE_THREAD_DELETE                           (157u)
#define TRACE_API_TXE_THREAD_ENTRY_EXIT                       (158u)
#define TRACE_API_TXE_THREAD_INFO_GET                         (159u)
#define TRACE_API_TXE_THREAD_PREEMTION_CHANGE                 (160u)
#define TRACE_API_TXE_THREAD_PRIORITY_CHANGE                  (161u)
#define TRACE_API_TXE_THREAD_RELINQUISH                       (162u)
#define TRACE_API_TXE_THREAD_RESET                            (163u)
#define TRACE_API_TXE_THREAD_RESUME                           (164u)
#define TRACE_API_TXE_THREAD_SUSPEND                          (165u)
#define TRACE_API_TXE_THREAD_TERMINATE                        (166u)
#define TRACE_API_TXE_THREAD_TIME_SLICE_CHANGE                (167u)
#define TRACE_API_TXE_THREAD_WAIT_ABORT                       (168u)
#define TRACE_API_TXE_TIMER_ACTIVATE                          (169u)
#define TRACE_API_TXE_TIMER_CHANGE                            (170u)
#define TRACE_API_TXE_TIMER_CREATE                            (171u)
#define TRACE_API_TXE_TIMER_DEACTIVATE                        (172u)
#define TRACE_API_TXE_TIMER_DELETE                            (173u)
#define TRACE_API_TXE_TIMER_INFO_GET                          (174u)


#define TRACE_RECORD_VOID(a)                                  SEGGER_SYSVIEW_RecordVoid(a)
#define TRACE_RECORD_U32(a, b)                                SEGGER_SYSVIEW_RecordU32(a, b)
#define TRACE_RECORD_U32x2(a, b, c)                           SEGGER_SYSVIEW_RecordU32x2(a, b, c)
#define TRACE_RECORD_U32x3(a, b, c, d)                        SEGGER_SYSVIEW_RecordU32x3(a, b, c, d)
#define TRACE_RECORD_U32x4(a, b, c, d, e)                     SEGGER_SYSVIEW_RecordU32x4(a, b, c, d, e)
#define TRACE_RECORD_U32x5(a, b, c, d, e, f)                  SEGGER_SYSVIEW_RecordU32x5(a, b, c, d, e, f)
#define TRACE_RECORD_U32x6(a, b, c, d, e, f, g)               SEGGER_SYSVIEW_RecordU32x6(a, b, c, d, e, f, g)
#define TRACE_RECORD_U32x7(a, b, c, d, e, f, g, h)            SEGGER_SYSVIEW_RecordU32x7(a, b, c, d, e, f, g, h)
#define TRACE_RECORD_U32x8(a, b, c, d, e, f, g, h, i)         SEGGER_SYSVIEW_RecordU32x8(a, b, c, d, e, f, g, h, i)
#define TRACE_RECORD_U32x9(a, b, c, d, e, f, g, h, i, j)      SEGGER_SYSVIEW_RecordU32x9(a, b, c, d, e, f, g, h, i, j)
#define TRACE_RECORD_U32x10(a, b, c, d, e, f, g, h, i, j, k)  SEGGER_SYSVIEW_RecordU32x10(a, b, c, d, e, f, g, h, i, j, k)
#define TRACE_RECORD_ENTER_TIMER(a)                           SEGGER_SYSVIEW_RecordEnterTimer(a);
#define TRACE_RECORD_EXIT_TIMER()                             SEGGER_SYSVIEW_RecordExitTimer();
#define TRACE_RECORD_END_CALL(a)                              SEGGER_SYSVIEW_RecordEndCall(a)
#define TRACE_RECORD_END_CALL_U32(a, b)                       SEGGER_SYSVIEW_RecordEndCallU32(a, b)
#define TRACE_NAME_RESOURCE(a, b)                             SEGGER_SYSVIEW_NameResource(a, b);

#define TRACE_ON_TASK_START_READY(a)                          SEGGER_SYSVIEW_OnTaskStartReady(a);

#endif /* SEGGER_SEGGER_SYSVIEW_THREADX_H_ */
