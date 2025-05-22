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

File    : SEGGER_SYSVIEW_FreeRTOS.h
Purpose : Interface between FreeRTOS and SystemView.
          Tested with FreeRTOS V11.0.1
Revision: $Rev: 7745 $

Notes:
  (1) Include this file at the end of FreeRTOSConfig.h
*/

#ifndef SYSVIEW_FREERTOS_H
#define SYSVIEW_FREERTOS_H

#include "SEGGER_SYSVIEW.h"

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/
#ifndef portSTACK_GROWTH
  #define portSTACK_GROWTH              ( -1 )
#endif

#ifndef USE_LEGACY_TRACE_API
  #define USE_LEGACY_TRACE_API          ( 0 )
#endif

#ifndef TRACERETURN_ENABLE
  #define TRACERETURN_ENABLE            ( 1 )
#endif

#define SYSVIEW_FREERTOS_MAX_NOF_TASKS  8

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

#if ( USE_LEGACY_TRACE_API != 0 )
  #define apiID_OFFSET                              (32u)

  #define apiID_VTASKALLOCATEMPUREGIONS             (1u)
  #define apiID_VTASKDELETE                         (2u)
  #define apiID_VTASKDELAY                          (3u)
  #define apiID_VTASKDELAYUNTIL                     (4u)
  #define apiID_UXTASKPRIORITYGET                   (5u)
  #define apiID_UXTASKPRIORITYGETFROMISR            (6u)
  #define apiID_ETASKGETSTATE                       (7u)
  #define apiID_VTASKPRIORITYSET                    (8u)
  #define apiID_VTASKSUSPEND                        (9u)
  #define apiID_VTASKRESUME                         (10u)
  #define apiID_XTASKRESUMEFROMISR                  (11u)
  #define apiID_VTASKSTARTSCHEDULER                 (12u)
  #define apiID_VTASKENDSCHEDULER                   (13u)
  #define apiID_VTASKSUSPENDALL                     (14u)
  #define apiID_XTASKRESUMEALL                      (15u)
  #define apiID_XTASKGETTICKCOUNT                   (16u)
  #define apiID_XTASKGETTICKCOUNTFROMISR            (17u)
  #define apiID_UXTASKGETNUMBEROFTASKS              (18u)
  #define apiID_PCTASKGETTASKNAME                   (19u)
  #define apiID_UXTASKGETSTACKHIGHWATERMARK         (20u)
  #define apiID_VTASKSETAPPLICATIONTASKTAG          (21u)
  #define apiID_XTASKGETAPPLICATIONTASKTAG          (22u)
  #define apiID_VTASKSETTHREADLOCALSTORAGEPOINTER   (23u)
  #define apiID_PVTASKGETTHREADLOCALSTORAGEPOINTER  (24u)
  #define apiID_XTASKCALLAPPLICATIONTASKHOOK        (25u)
  #define apiID_XTASKGETIDLETASKHANDLE              (26u)
  #define apiID_UXTASKGETSYSTEMSTATE                (27u)
  #define apiID_VTASKLIST                           (28u)
  #define apiID_VTASKGETRUNTIMESTATS                (29u)
  #define apiID_XTASKGENERICNOTIFY                  (30u)
  #define apiID_XTASKGENERICNOTIFYFROMISR           (31u)
  #define apiID_XTASKNOTIFYWAIT                     (32u)
  #define apiID_VTASKNOTIFYGIVEFROMISR              (33u)
  #define apiID_ULTASKNOTIFYTAKE                    (34u)
  #define apiID_XTASKNOTIFYSTATECLEAR               (35u)
  #define apiID_XTASKGETCURRENTTASKHANDLE           (36u)
  #define apiID_VTASKSETTIMEOUTSTATE                (37u)
  #define apiID_XTASKCHECKFORTIMEOUT                (38u)
  #define apiID_VTASKMISSEDYIELD                    (39u)
  #define apiID_XTASKGETSCHEDULERSTATE              (40u)
  #define apiID_VTASKPRIORITYINHERIT                (41u)
  #define apiID_XTASKPRIORITYDISINHERIT             (42u)
  #define apiID_XTASKGENERICCREATE                  (43u)
  #define apiID_UXTASKGETTASKNUMBER                 (44u)
  #define apiID_VTASKSETTASKNUMBER                  (45u)
  #define apiID_VTASKSTEPTICK                       (46u)
  #define apiID_ETASKCONFIRMSLEEPMODESTATUS         (47u)
  #define apiID_XTIMERCREATE                        (48u)
  #define apiID_PVTIMERGETTIMERID                   (49u)
  #define apiID_VTIMERSETTIMERID                    (50u)
  #define apiID_XTIMERISTIMERACTIVE                 (51u)
  #define apiID_XTIMERGETTIMERDAEMONTASKHANDLE      (52u)
  #define apiID_XTIMERPENDFUNCTIONCALLFROMISR       (53u)
  #define apiID_XTIMERPENDFUNCTIONCALL              (54u)
  #define apiID_PCTIMERGETTIMERNAME                 (55u)
  #define apiID_XTIMERCREATETIMERTASK               (56u)
  #define apiID_XTIMERGENERICCOMMAND                (57u)
  #define apiID_XQUEUEGENERICSEND                   (58u)
  #define apiID_XQUEUEPEEKFROMISR                   (59u)
  #define apiID_XQUEUEGENERICRECEIVE                (60u)
  #define apiID_UXQUEUEMESSAGESWAITING              (61u)
  #define apiID_UXQUEUESPACESAVAILABLE              (62u)
  #define apiID_VQUEUEDELETE                        (63u)
  #define apiID_XQUEUEGENERICSENDFROMISR            (64u)
  #define apiID_XQUEUEGIVEFROMISR                   (65u)
  #define apiID_XQUEUERECEIVEFROMISR                (66u)
  #define apiID_XQUEUEISQUEUEEMPTYFROMISR           (67u)
  #define apiID_XQUEUEISQUEUEFULLFROMISR            (68u)
  #define apiID_UXQUEUEMESSAGESWAITINGFROMISR       (69u)
  #define apiID_XQUEUEALTGENERICSEND                (70u)
  #define apiID_XQUEUEALTGENERICRECEIVE             (71u)
  #define apiID_XQUEUECRSENDFROMISR                 (72u)
  #define apiID_XQUEUECRRECEIVEFROMISR              (73u)
  #define apiID_XQUEUECRSEND                        (74u)
  #define apiID_XQUEUECRRECEIVE                     (75u)
  #define apiID_XQUEUECREATEMUTEX                   (76u)
  #define apiID_XQUEUECREATECOUNTINGSEMAPHORE       (77u)
  #define apiID_XQUEUEGETMUTEXHOLDER                (78u)
  #define apiID_XQUEUETAKEMUTEXRECURSIVE            (79u)
  #define apiID_XQUEUEGIVEMUTEXRECURSIVE            (80u)
  #define apiID_VQUEUEADDTOREGISTRY                 (81u)
  #define apiID_VQUEUEUNREGISTERQUEUE               (82u)
  #define apiID_XQUEUEGENERICCREATE                 (83u)
  #define apiID_XQUEUECREATESET                     (84u)
  #define apiID_XQUEUEADDTOSET                      (85u)
  #define apiID_XQUEUEREMOVEFROMSET                 (86u)
  #define apiID_XQUEUESELECTFROMSET                 (87u)
  #define apiID_XQUEUESELECTFROMSETFROMISR          (88u)
  #define apiID_XQUEUEGENERICRESET                  (89u)
  #define apiID_VLISTINITIALISE                     (90u)
  #define apiID_VLISTINITIALISEITEM                 (91u)
  #define apiID_VLISTINSERT                         (92u)
  #define apiID_VLISTINSERTEND                      (93u)
  #define apiID_UXLISTREMOVE                        (94u)
  #define apiID_XEVENTGROUPCREATE                   (95u)
  #define apiID_XEVENTGROUPWAITBITS                 (96u)
  #define apiID_XEVENTGROUPCLEARBITS                (97u)
  #define apiID_XEVENTGROUPCLEARBITSFROMISR         (98u)
  #define apiID_XEVENTGROUPSETBITS                  (99u)
  #define apiID_XEVENTGROUPSETBITSFROMISR           (100u)
  #define apiID_XEVENTGROUPSYNC                     (101u)
  #define apiID_XEVENTGROUPGETBITSFROMISR           (102u)
  #define apiID_VEVENTGROUPDELETE                   (103u)
  #define apiID_UXEVENTGROUPGETNUMBER               (104u)
  #define apiID_XSTREAMBUFFERCREATE                 (105u)
  #define apiID_VSTREAMBUFFERDELETE                 (106u)
  #define apiID_XSTREAMBUFFERRESET                  (107u)
  #define apiID_XSTREAMBUFFERSEND                   (108u)
  #define apiID_XSTREAMBUFFERSENDFROMISR            (109u)
  #define apiID_XSTREAMBUFFERRECEIVE                (110u)
  #define apiID_XSTREAMBUFFERRECEIVEFROMISR         (111u)
  #define apiID_XQUEUESETSEND                       (112u)
#endif

#define apiID_XQUEUEGENERICRESET                       (150u)
#define apiID_XQUEUEGENERICCREATESTATIC                (151u)
#define apiID_XQUEUEGENERICCREATE                      (152u)
#define apiID_XQUEUECREATEMUTEX                        (153u)
#define apiID_XQUEUECREATEMUTEXSTATIC                  (154u)
#define apiID_XQUEUEGETMUTEXHOLDER                     (155u)
#define apiID_XQUEUEGETMUTEXHOLDERFROMISR              (156u)
#define apiID_XQUEUEGIVEMUTEXRECURSIVE                 (157u)
#define apiID_XQUEUETAKEMUTEXRECURSIVE                 (158u)
#define apiID_XQUEUECREATECOUNTINGSEMAPHORESTATIC      (159u)
#define apiID_XQUEUECREATECOUNTINGSEMAPHORE            (160u)
#define apiID_XQUEUEGENERICSEND                        (161u)
#define apiID_XQUEUEGENERICSENDFROMISR                 (162u)
#define apiID_XQUEUEGIVEFROMISR                        (163u)
#define apiID_XQUEUERECEIVE                            (164u)
#define apiID_XQUEUESEMAPHORETAKE                      (165u)
#define apiID_XQUEUEPEEK                               (166u)
#define apiID_XQUEUERECEIVEFROMISR                     (167u)
#define apiID_XQUEUEPEEKFROMISR                        (168u)
#define apiID_UXQUEUEMESSAGESWAITING                   (169u)
#define apiID_UXQUEUESPACESAVAILABLE                   (170u)
#define apiID_UXQUEUEMESSAGESWAITINGFROMISR            (171u)
#define apiID_VQUEUEDELETE                             (172u)
#define apiID_UXQUEUEGETQUEUENUMBER                    (173u)
#define apiID_VQUEUESETQUEUENUMBER                     (174u)
#define apiID_UCQUEUEGETQUEUETYPE                      (175u)
#define apiID_XQUEUEISQUEUEEMPTYFROMISR                (176u)
#define apiID_XQUEUEISQUEUEFULLFROMISR                 (177u)
#define apiID_XQUEUECRSEND                             (178u)
#define apiID_XQUEUECRRECEIVE                          (179u)
#define apiID_XQUEUECRSENDFROMISR                      (180u)
#define apiID_XQUEUECRRECEIVEFROMISR                   (181u)
#define apiID_VQUEUEADDTOREGISTRY                      (182u)
#define apiID_PCQUEUEGETNAME                           (183u)
#define apiID_VQUEUEUNREGISTERQUEUE                    (184u)
#define apiID_VQUEUEWAITFORMESSAGERESTRICTED           (185u)
#define apiID_XQUEUECREATESET                          (186u)
#define apiID_XQUEUEADDTOSET                           (187u)
#define apiID_XQUEUEREMOVEFROMSET                      (188u)
#define apiID_XQUEUESELECTFROMSET                      (189u)
#define apiID_XQUEUESELECTFROMSETFROMISR               (190u)

#define apiID_XTASKCREATESTATIC                        (191u)
#define apiID_XTASKCREATERESTRICTEDSTATIC              (192u)
#define apiID_XTASKCREATERESTRICTED                    (193u)
#define apiID_XTASKCREATE                              (194u)
#define apiID_VTASKDELETE                              (195u)
#define apiID_XTASKDELAYUNTIL                          (196u)
#define apiID_VTASKDELAY                               (197u)
#define apiID_ETASKGETSTATE                            (198u)
#define apiID_UXTASKPRIORITYGET                        (199u)
#define apiID_UXTASKPRIORITYGETFROMISR                 (200u)
#define apiID_VTASKPRIORITYSET                         (201u)
#define apiID_VTASKSUSPEND                             (202u)
#define apiID_VTASKRESUME                              (203u)
#define apiID_XTASKRESUMEFROMISR                       (204u)
#define apiID_VTASKSTARTSCHEDULER                      (205u)
#define apiID_VTASKENDSCHEDULER                        (206u)
#define apiID_VTASKSUSPENDALL                          (207u)
#define apiID_XTASKRESUMEALL                           (208u)
#define apiID_XTASKGETTICKCOUNT                        (209u)
#define apiID_XTASKGETTICKCOUNTFROMISR                 (210u)
#define apiID_UXTASKGETNUMBEROFTASKS                   (211u)
#define apiID_PCTASKGETNAME                            (212u)
#define apiID_XTASKGETHANDLE                           (213u)
#define apiID_UXTASKGETSYSTEMSTATE                     (214u)
#define apiID_XTASKGETIDLETASKHANDLE                   (215u)
#define apiID_VTASKSTEPTICK                            (216u)
#define apiID_XTASKCATCHUPTICKS                        (217u)
#define apiID_XTASKABORTDELAY                          (218u)
#define apiID_XTASKINCREMENTTICK                       (219u)
#define apiID_VTASKSETAPPLICATIONTASKTAG               (220u)
#define apiID_XTASKGETAPPLICATIONTASKTAG               (221u)
#define apiID_XTASKGETAPPLICATIONTASKTAGFROMISR        (222u)
#define apiID_XTASKCALLAPPLICATIONTASKHOOK             (223u)
#define apiID_VTASKPLACEONEVENTLIST                    (225u)
#define apiID_VTASKPLACEONUNORDEREDEVENTLIST           (226u)
#define apiID_VTASKPLACEONEVENTLISTRESTRICTED          (227u)
#define apiID_XTASKREMOVEFROMEVENTLIST                 (228u)
#define apiID_VTASKREMOVEFROMUNORDEREDEVENTLIST        (229u)
#define apiID_VTASKSETTIMEOUTSTATE                     (230u)
#define apiID_VTASKINTERNALSETTIMEOUTSTATE             (231u)
#define apiID_XTASKCHECKFORTIMEOUT                     (232u)
#define apiID_VTASKMISSEDYIELD                         (233u)
#define apiID_UXTASKGETTASKNUMBER                      (234u)
#define apiID_VTASKSETTASKNUMBER                       (235u)
#define apiID_ETASKCONFIRMSLEEPMODESTATUS              (236u)
#define apiID_VTASKSETTHREADLOCALSTORAGEPOINTER        (237u)
#define apiID_PVTASKGETTHREADLOCALSTORAGEPOINTER       (238u)
#define apiID_VTASKALLOCATEMPUREGIONS                  (239u)
#define apiID_VTASKGETINFO                             (240u)
#define apiID_UXTASKGETSTACKHIGHWATERMARK2             (241u)
#define apiID_UXTASKGETSTACKHIGHWATERMARK              (242u)
#define apiID_PXTASKGETSTACKSTART                      (243u)
#define apiID_XTASKGETCURRENTTASKHANDLE                (244u)
#define apiID_XTASKGETSCHEDULERSTATE                   (245u)
#define apiID_XTASKPRIORITYINHERIT                     (246u)
#define apiID_XTASKPRIORITYDISINHERIT                  (247u)
#define apiID_VTASKPRIORITYDISINHERITAFTERTIMEOUT      (248u)
#define apiID_VTASKENTERCRITICAL                       (249u)
#define apiID_VTASKEXITCRITICAL                        (250u)
#define apiID_VTASKLIST                                (251u)
#define apiID_VTASKGETRUNTIMESTATS                     (252u)
#define apiID_UXTASKRESETEVENTITEMVALUE                (253u)
#define apiID_PVTASKINCREMENTMUTEXHELDCOUNT            (254u)
#define apiID_ULTASKGENERICNOTIFYTAKE                  (255u)
#define apiID_XTASKGENERICNOTIFYWAIT                   (256u)
#define apiID_XTASKGENERICNOTIFY                       (257u)
#define apiID_XTASKGENERICNOTIFYFROMISR                (258u)
#define apiID_VTASKGENERICNOTIFYGIVEFROMISR            (259u)
#define apiID_XTASKGENERICNOTIFYSTATECLEAR             (260u)
#define apiID_ULTASKGENERICNOTIFYVALUECLEAR            (261u)
#define apiID_ULTASKGETIDLERUNTIMECOUNTER              (262u)
#define apiID_ULTASKGETIDLERUNTIMEPERCENT              (263u)
#define apiID_XTIMERCREATETIMERTASK                    (264u)
#define apiID_XTIMERCREATE                             (265u)
#define apiID_XTIMERCREATESTATIC                       (266u)
#define apiID_XTIMERGENERICCOMMAND                     (267u)
#define apiID_XTIMERGETTIMERDAEMONTASKHANDLE           (268u)
#define apiID_XTIMERGETPERIOD                          (269u)
#define apiID_VTIMERSETRELOADMODE                      (270u)
#define apiID_XTIMERGETRELOADMODE                      (271u)
#define apiID_XTIMERGETEXPIRYTIME                      (272u)
#define apiID_PCTIMERGETNAME                           (273u)
#define apiID_XTIMERISTIMERACTIVE                      (274u)
#define apiID_PVTIMERGETTIMERID                        (275u)
#define apiID_VTIMERSETTIMERID                         (276u)
#define apiID_XTIMERPENDFUNCTIONCALLFROMISR            (277u)
#define apiID_XTIMERPENDFUNCTIONCALL                   (278u)
#define apiID_UXTIMERGETTIMERNUMBER                    (279u)
#define apiID_VTIMERSETTIMERNUMBER                     (280u)

#define apiID_VLISTINITIALISE                          (281u)
#define apiID_VLISTINITIALISEITEM                      (282u)
#define apiID_VLISTINSERTEND                           (283u)
#define apiID_VLISTINSERT                              (284u)
#define apiID_UXLISTREMOVE                             (285u)

#define apiID_XCOROUTINECREATE                         (286u)
#define apiID_VCOROUTINEADDTODELAYEDLIST               (287u)
#define apiID_VCOROUTINESCHEDULE                       (288u)
#define apiID_XCOROUTINEREMOVEFROMEVENTLIST            (289u)

#define apiID_XEVENTGROUPCREATESTATIC                  (290u)
#define apiID_XEVENTGROUPCREATE                        (291u)
#define apiID_XEVENTGROUPSYNC                          (292u)
#define apiID_XEVENTGROUPWAITBITS                      (293u)
#define apiID_XEVENTGROUPCLEARBITS                     (294u)
#define apiID_XEVENTGROUPCLEARBITSFROMISR              (295u)
#define apiID_XEVENTGROUPGETBITSFROMISR                (296u)
#define apiID_XEVENTGROUPSETBITS                       (297u)
#define apiID_VEVENTGROUPDELETE                        (298u)
#define apiID_VEVENTGROUPSETBITSCALLBACK               (299u)
#define apiID_VEVENTGROUPCLEARBITSCALLBACK             (300u)
#define apiID_XEVENTGROUPSETBITSFROMISR                (301u)
#define apiID_UXEVENTGROUPGETNUMBER                    (302u)
#define apiID_VEVENTGROUPSETNUMBER                     (303u)

#define apiID_XSTREAMBUFFERGENERICCREATE               (304u)
#define apiID_XSTREAMBUFFERGENERICCREATESTATIC         (305u)
#define apiID_VSTREAMBUFFERDELETE                      (306u)
#define apiID_XSTREAMBUFFERRESET                       (307u)
#define apiID_XSTREAMBUFFERSETTRIGGERLEVEL             (308u)
#define apiID_XSTREAMBUFFERSPACESAVAILABLE             (309u)
#define apiID_XSTREAMBUFFERBYTESAVAILABLE              (310u)
#define apiID_XSTREAMBUFFERSEND                        (311u)
#define apiID_XSTREAMBUFFERSENDFROMISR                 (312u)
#define apiID_XSTREAMBUFFERRECEIVE                     (313u)
#define apiID_XSTREAMBUFFERNEXTMESSAGELENGTHBYTES      (314u)
#define apiID_XSTREAMBUFFERRECEIVEFROMISR              (315u)
#define apiID_XSTREAMBUFFERISEMPTY                     (316u)
#define apiID_XSTREAMBUFFERISFULL                      (317u)
#define apiID_XSTREAMBUFFERSENDCOMPLETEDFROMISR        (318u)
#define apiID_XSTREAMBUFFERRECEIVECOMPLETEDFROMISR     (319u)
#define apiID_UXSTREAMBUFFERGETSTREAMBUFFERNUMBER      (320u)
#define apiID_VSTREAMBUFFERSETSTREAMBUFFERNUMBER       (321u)
#define apiID_UCSTREAMBUFFERGETSTREAMBUFFERTYPE        (322u)

#define apiID_XTIMERGENERICCOMMANDFROMISR              (323u)
#define apiID_XTIMERGENERICCOMMANDFROMTASK             (324u)

#define apiID_VTASKCOREAFFINITYSET                     (325u)
#define apiID_VTASKCOREAFFINITYGET                     (326u)
#define apiID_VTASKPREEMPTIONDISABLE                   (327u)
#define apiID_VTASKPREEMPTIONENABLE                    (328u)
#define apiID_VTASKYIELDWITHINAPI                      (329u)
#define apiID_VTASKENTERCRITICALFROMISR                (330u)
#define apiID_VTASKEXITCRITICALFROMISR                 (331u)
#define apiID_ULTASKGETRUNTIMECOUNTER                  (332u)


#define traceSTART()                                                            SEGGER_SYSVIEW_Conf()

#define traceAPI_xQueueGenericReset(xQueue, xNewQueue)                                                                SEGGER_SYSVIEW_RecordU32x2(apiID_XQUEUEGENERICRESET, SEGGER_SYSVIEW_ShrinkId((U32)xQueue), (U32)xNewQueue)
#define traceAPI_xQueueGenericCreateStatic(uxQueueLength, uxItemSize, pucQueueStorage, pxStaticQueue, ucQueueType)    SEGGER_SYSVIEW_RecordU32x5(apiID_XQUEUEGENERICCREATESTATIC, (U32)uxQueueLength, (U32)uxItemSize, SEGGER_SYSVIEW_ShrinkId((U32)pucQueueStorage), SEGGER_SYSVIEW_ShrinkId((U32)pxStaticQueue),(U32)ucQueueType)
#define traceAPI_xQueueGenericCreate(uxQueueLength, uxItemSize, ucQueueType)                                          SEGGER_SYSVIEW_RecordU32x3(apiID_XQUEUEGENERICCREATE, (U32)uxQueueLength,(U32)uxItemSize,(U32)ucQueueType)
#define traceAPI_xQueueCreateMutex(ucQueueType)                                                                       SEGGER_SYSVIEW_RecordU32  (apiID_XQUEUECREATEMUTEX, (U32)ucQueueType)
#define traceAPI_xQueueCreateMutexStatic(ucQueueType, pxStaticQueue)                                                  SEGGER_SYSVIEW_RecordU32x2(apiID_XQUEUECREATEMUTEXSTATIC, (U32)ucQueueType, SEGGER_SYSVIEW_ShrinkId((U32)pxStaticQueue))
#define traceAPI_xQueueGetMutexHolder(xSemaphore)                                                                     SEGGER_SYSVIEW_RecordU32  (apiID_XQUEUEGETMUTEXHOLDER, SEGGER_SYSVIEW_ShrinkId((U32)xSemaphore))
#define traceAPI_xQueueGetMutexHolderFromISR(xSemaphore)                                                              SEGGER_SYSVIEW_RecordU32  (apiID_XQUEUEGETMUTEXHOLDERFROMISR, SEGGER_SYSVIEW_ShrinkId((U32)xSemaphore))
#define traceAPI_xQueueGiveMutexRecursive(xMutex)                                                                     SEGGER_SYSVIEW_RecordU32  (apiID_XQUEUEGIVEMUTEXRECURSIVE, SEGGER_SYSVIEW_ShrinkId((U32)xMutex))
#define traceAPI_xQueueTakeMutexRecursive(xMutex, xTicksToWait)                                                       SEGGER_SYSVIEW_RecordU32x2(apiID_XQUEUETAKEMUTEXRECURSIVE, SEGGER_SYSVIEW_ShrinkId((U32)xMutex),(U32)xTicksToWait)
#define traceAPI_xQueueCreateCountingSemaphoreStatic(uxMaxCount, uxInitialCount, pxStaticQueue)                       SEGGER_SYSVIEW_RecordU32x3(apiID_XQUEUECREATECOUNTINGSEMAPHORESTATIC, (U32)uxMaxCount, (U32)uxInitialCount, SEGGER_SYSVIEW_ShrinkId((U32)pxStaticQueue))
#define traceAPI_xQueueCreateCountingSemaphore(uxMaxCount, uxInitialCount)                                            SEGGER_SYSVIEW_RecordU32x2(apiID_XQUEUECREATECOUNTINGSEMAPHORE, (U32)uxMaxCount, (U32)uxInitialCount)
#define traceAPI_xQueueGenericSend(xQueue, pvItemToQueue, xTicksToWait, xCopyPosition)                                SEGGER_SYSVIEW_RecordU32x4(apiID_XQUEUEGENERICSEND, SEGGER_SYSVIEW_ShrinkId((U32)xQueue), SEGGER_SYSVIEW_ShrinkId((U32)pvItemToQueue), (U32)xTicksToWait, (U32)xCopyPosition)
#define traceAPI_xQueueGenericSendFromISR(xQueue, pvItemToQueue, pxHigherPriorityTaskWoken, xCopyPosition)            SEGGER_SYSVIEW_RecordU32x4 (apiID_XQUEUEGENERICSENDFROMISR, SEGGER_SYSVIEW_ShrinkId((U32)xQueue), SEGGER_SYSVIEW_ShrinkId((U32)pvItemToQueue), SEGGER_SYSVIEW_ShrinkId((U32)pxHigherPriorityTaskWoken), (U32)xCopyPosition)
#define traceAPI_xQueueGiveFromISR(xQueue, pxHigherPriorityTaskWoken)                                                 SEGGER_SYSVIEW_RecordU32x2(apiID_XQUEUEGIVEFROMISR, SEGGER_SYSVIEW_ShrinkId((U32)xQueue), SEGGER_SYSVIEW_ShrinkId((U32)pxHigherPriorityTaskWoken))
#define traceAPI_xQueueReceive(xQueue, pvBuffer, xTicksToWait)                                                        SEGGER_SYSVIEW_RecordU32x3(apiID_XQUEUERECEIVE, SEGGER_SYSVIEW_ShrinkId((U32)xQueue), SEGGER_SYSVIEW_ShrinkId((U32)pvBuffer), (U32)xTicksToWait)
#define traceAPI_xQueueSemaphoreTake(xQueue, xTicksToWait)                                                            SEGGER_SYSVIEW_RecordU32x2(apiID_XQUEUESEMAPHORETAKE, SEGGER_SYSVIEW_ShrinkId((U32)xQueue), (U32)xTicksToWait)
#define traceAPI_xQueuePeek(xQueue, pvBuffer, xTicksToWait)                                                           SEGGER_SYSVIEW_RecordU32x3(apiID_XQUEUEPEEK, SEGGER_SYSVIEW_ShrinkId((U32)xQueue), SEGGER_SYSVIEW_ShrinkId((U32)pvBuffer), (U32)xTicksToWait)
#define traceAPI_xQueueReceiveFromISR(xQueue, pvBuffer, pxHigherPriorityTaskWoken)                                    SEGGER_SYSVIEW_RecordU32x3(apiID_XQUEUERECEIVEFROMISR, SEGGER_SYSVIEW_ShrinkId((U32)xQueue), SEGGER_SYSVIEW_ShrinkId((U32)pvBuffer), (U32)pxHigherPriorityTaskWoken)
#define traceAPI_xQueuePeekFromISR(xQueue, pvBuffer)                                                                  SEGGER_SYSVIEW_RecordU32x2(apiID_XQUEUEPEEKFROMISR, SEGGER_SYSVIEW_ShrinkId((U32)xQueue), SEGGER_SYSVIEW_ShrinkId((U32)pvBuffer))
#define traceAPI_uxQueueMessagesWaiting(xQueue)                                                                       SEGGER_SYSVIEW_RecordU32  (apiID_UXQUEUEMESSAGESWAITING, SEGGER_SYSVIEW_ShrinkId((U32)xQueue))
#define traceAPI_uxQueueSpacesAvailable(xQueue)                                                                       SEGGER_SYSVIEW_RecordU32  (apiID_UXQUEUESPACESAVAILABLE, SEGGER_SYSVIEW_ShrinkId((U32)xQueue))
#define traceAPI_uxQueueMessagesWaitingFromISR(xQueue)                                                                SEGGER_SYSVIEW_RecordU32  (apiID_UXQUEUEMESSAGESWAITINGFROMISR, SEGGER_SYSVIEW_ShrinkId((U32)xQueue))
#define traceAPI_vQueueDelete(xQueue)                                                                                 SEGGER_SYSVIEW_RecordU32  (apiID_VQUEUEDELETE, SEGGER_SYSVIEW_ShrinkId((U32)xQueue))
#define traceAPI_uxQueueGetQueueNumber(xQueue)                                                                        SEGGER_SYSVIEW_RecordU32  (apiID_UXQUEUEGETQUEUENUMBER, SEGGER_SYSVIEW_ShrinkId((U32)xQueue))
#define traceAPI_vQueueSetQueueNumber(xQueue, uxQueueNumber)                                                          SEGGER_SYSVIEW_RecordU32x2(apiID_VQUEUESETQUEUENUMBER, SEGGER_SYSVIEW_ShrinkId((U32)xQueue), (U32)uxQueueNumber)
#define traceAPI_ucQueueGetQueueType(xQueue)                                                                          SEGGER_SYSVIEW_RecordU32  (apiID_UCQUEUEGETQUEUETYPE, SEGGER_SYSVIEW_ShrinkId((U32)xQueue))
#define traceAPI_xQueueIsQueueEmptyFromISR(xQueue)                                                                    SEGGER_SYSVIEW_RecordU32  (apiID_XQUEUEISQUEUEEMPTYFROMISR, SEGGER_SYSVIEW_ShrinkId((U32)xQueue))
#define traceAPI_xQueueIsQueueFullFromISR(xQueue)                                                                     SEGGER_SYSVIEW_RecordU32  (apiID_XQUEUEISQUEUEFULLFROMISR, SEGGER_SYSVIEW_ShrinkId((U32)xQueue))
#define traceAPI_xQueueCRSend(xQueue, pvItemToQueue, xTicksToWait)                                                    SEGGER_SYSVIEW_RecordU32x3(apiID_XQUEUECRSEND, SEGGER_SYSVIEW_ShrinkId((U32)xQueue), SEGGER_SYSVIEW_ShrinkId((U32)pvItemToQueue), (U32)xTicksToWait)
#define traceAPI_xQueueCRReceive(xQueue, pvBuffer, xTicksToWait)                                                      SEGGER_SYSVIEW_RecordU32x3(apiID_XQUEUECRRECEIVE, SEGGER_SYSVIEW_ShrinkId((U32)xQueue), SEGGER_SYSVIEW_ShrinkId((U32)pvBuffer), (U32)xTicksToWait)
#define traceAPI_xQueueCRSendFromISR(xQueue, pvItemToQueue, xCoRoutinePreviouslyWoken)                                SEGGER_SYSVIEW_RecordU32x3(apiID_XQUEUECRSENDFROMISR, SEGGER_SYSVIEW_ShrinkId((U32)xQueue), SEGGER_SYSVIEW_ShrinkId((U32)pvItemToQueue), (U32)xCoRoutinePreviouslyWoken)
#define traceAPI_xQueueCRReceiveFromISR(xQueue, pvBuffer, pxCoRoutineWoken)                                           SEGGER_SYSVIEW_RecordU32x3(apiID_XQUEUECRRECEIVEFROMISR, SEGGER_SYSVIEW_ShrinkId((U32)xQueue), SEGGER_SYSVIEW_ShrinkId((U32)pvBuffer), (U32)pxCoRoutineWoken)
#define traceAPI_vQueueAddToRegistry(xQueue, pcQueueName)                                                             SEGGER_SYSVIEW_RecordU32x2(apiID_VQUEUEADDTOREGISTRY, SEGGER_SYSVIEW_ShrinkId((U32)xQueue), SEGGER_SYSVIEW_ShrinkId((U32)pcQueueName))
#define traceAPI_pcQueueGetName(xQueue)                                                                               SEGGER_SYSVIEW_RecordU32  (apiID_PCQUEUEGETNAME, SEGGER_SYSVIEW_ShrinkId((U32)xQueue))
#define traceAPI_vQueueUnregisterQueue(xQueue)                                                                        SEGGER_SYSVIEW_RecordU32  (apiID_VQUEUEUNREGISTERQUEUE, SEGGER_SYSVIEW_ShrinkId((U32)xQueue))
#define traceAPI_vQueueWaitForMessageRestricted(xQueue, xTicksToWait, xWaitIndefinitely)                              SEGGER_SYSVIEW_RecordU32x3(apiID_VQUEUEWAITFORMESSAGERESTRICTED, SEGGER_SYSVIEW_ShrinkId((U32)xQueue), (U32)xTicksToWait, (U32)xWaitIndefinitely)
#define traceAPI_xQueueCreateSet(uxEventQueueLength)                                                                  SEGGER_SYSVIEW_RecordU32  (apiID_XQUEUECREATESET, (U32)uxEventQueueLength)
#define traceAPI_xQueueAddToSet(xQueueOrSemaphore, xQueueSet)                                                         SEGGER_SYSVIEW_RecordU32x2(apiID_XQUEUEADDTOSET, SEGGER_SYSVIEW_ShrinkId((U32)xQueueOrSemaphore), SEGGER_SYSVIEW_ShrinkId((U32)xQueueSet))
#define traceAPI_xQueueRemoveFromSet(xQueueOrSemaphore, xQueueSet)                                                    SEGGER_SYSVIEW_RecordU32x2(apiID_XQUEUEREMOVEFROMSET, SEGGER_SYSVIEW_ShrinkId((U32)xQueueOrSemaphore), SEGGER_SYSVIEW_ShrinkId((U32)xQueueSet))
#define traceAPI_xQueueSelectFromSet(xQueueSet, xTicksToWait)                                                         SEGGER_SYSVIEW_RecordU32x2(apiID_XQUEUESELECTFROMSET, SEGGER_SYSVIEW_ShrinkId((U32)xQueueSet), (U32)xTicksToWait)
#define traceAPI_xQueueSelectFromSetFromISR(xQueueSet)                                                                SEGGER_SYSVIEW_RecordU32  (apiID_XQUEUESELECTFROMSETFROMISR, SEGGER_SYSVIEW_ShrinkId((U32)xQueueSet))

#define traceAPI_xTaskCreateStatic(pxTaskCode, pcName, ulStackDepth, pvParameters, uxPriority, puxStackBuffer, pxTaskBuffer)                             SEGGER_SYSVIEW_RecordU32x7(apiID_XTASKCREATESTATIC, SEGGER_SYSVIEW_ShrinkId((U32)pxTaskCode), SEGGER_SYSVIEW_ShrinkId((U32)pcName), (U32)ulStackDepth, SEGGER_SYSVIEW_ShrinkId((U32)pvParameters), (U32)uxPriority, SEGGER_SYSVIEW_ShrinkId((U32)puxStackBuffer), SEGGER_SYSVIEW_ShrinkId((U32)pxTaskBuffer))
#define traceAPI_xTaskCreateRestrictedStatic(pxTaskDefinition, pxCreatedTask)                                                                            SEGGER_SYSVIEW_RecordU32x2(apiID_XTASKCREATERESTRICTEDSTATIC, SEGGER_SYSVIEW_ShrinkId((U32)pxTaskDefinition), SEGGER_SYSVIEW_ShrinkId((U32)pxCreatedTask))
#define traceAPI_xTaskCreateRestricted(pxTaskDefinition, pxCreatedTask)                                                                                  SEGGER_SYSVIEW_RecordU32x2(apiID_XTASKCREATERESTRICTED, SEGGER_SYSVIEW_ShrinkId((U32)pxTaskDefinition), SEGGER_SYSVIEW_ShrinkId((U32)pxCreatedTask))
#define traceAPI_xTaskCreate(pxTaskCode, pcName, usStackDepth, pvParameters, uxPriority, pxCreatedTask)                                                  SEGGER_SYSVIEW_RecordU32x6(apiID_XTASKCREATE, SEGGER_SYSVIEW_ShrinkId((U32)pxTaskCode), SEGGER_SYSVIEW_ShrinkId((U32)pcName), (U32)usStackDepth, SEGGER_SYSVIEW_ShrinkId((U32)pvParameters), (U32)uxPriority, SEGGER_SYSVIEW_ShrinkId((U32)pxCreatedTask))
#define traceAPI_vTaskDelete(xTaskToDelete)                                                                                                              SEGGER_SYSVIEW_RecordU32  (apiID_VTASKDELETE, SEGGER_SYSVIEW_ShrinkId((U32)xTaskToDelete))
#define traceAPI_xTaskDelayUntil(pxPreviousWakeTime, xTimeIncrement)                                                                                     SEGGER_SYSVIEW_RecordU32x2(apiID_XTASKDELAYUNTIL, SEGGER_SYSVIEW_ShrinkId((U32)pxPreviousWakeTime), (U32)xTimeIncrement)
#define traceAPI_vTaskDelay(xTicksToDelay)                                                                                                               SEGGER_SYSVIEW_RecordU32  (apiID_VTASKDELAY, (U32)xTicksToDelay)
#define traceAPI_eTaskGetState(xTask)                                                                                                                    SEGGER_SYSVIEW_RecordU32  (apiID_ETASKGETSTATE, SEGGER_SYSVIEW_ShrinkId((U32)xTask))
#define traceAPI_uxTaskPriorityGet(xTask)                                                                                                                SEGGER_SYSVIEW_RecordU32  (apiID_UXTASKPRIORITYGET, SEGGER_SYSVIEW_ShrinkId((U32)xTask))
#define traceAPI_uxTaskPriorityGetFromISR(xTask)                                                                                                         SEGGER_SYSVIEW_RecordU32  (apiID_UXTASKPRIORITYGETFROMISR, SEGGER_SYSVIEW_ShrinkId((U32)xTask))
#define traceAPI_vTaskPrioritySet(xTask, uxNewPriority)                                                                                                  SEGGER_SYSVIEW_RecordU32x2(apiID_VTASKPRIORITYSET, SEGGER_SYSVIEW_ShrinkId((U32)xTask), (U32)uxNewPriority)
#define traceAPI_vTaskSuspend(xTaskToSuspend)                                                                                                            SEGGER_SYSVIEW_RecordU32  (apiID_VTASKSUSPEND, SEGGER_SYSVIEW_ShrinkId((U32)xTaskToSuspend))
#define traceAPI_vTaskResume(xTaskToResume)                                                                                                              SEGGER_SYSVIEW_RecordU32  (apiID_VTASKRESUME, SEGGER_SYSVIEW_ShrinkId((U32)xTaskToResume))
#define traceAPI_xTaskResumeFromISR(xTaskToResume)                                                                                                       SEGGER_SYSVIEW_RecordU32  (apiID_XTASKRESUMEFROMISR, SEGGER_SYSVIEW_ShrinkId((U32)xTaskToResume))
#define traceAPI_vTaskStartScheduler()                                                                                                                   SEGGER_SYSVIEW_RecordVoid (apiID_VTASKSTARTSCHEDULER)
#define traceAPI_vTaskEndScheduler()                                                                                                                     SEGGER_SYSVIEW_RecordVoid (apiID_VTASKENDSCHEDULER)
#define traceAPI_vTaskSuspendAll()                                                                                                                       SEGGER_SYSVIEW_RecordVoid (apiID_VTASKSUSPENDALL)
#define traceAPI_xTaskResumeAll()                                                                                                                        SEGGER_SYSVIEW_RecordVoid (apiID_XTASKRESUMEALL)
#define traceAPI_xTaskGetTickCount()                                                                                                                     SEGGER_SYSVIEW_RecordVoid (apiID_XTASKGETTICKCOUNT)
#define traceAPI_xTaskGetTickCountFromISR()                                                                                                              SEGGER_SYSVIEW_RecordVoid (apiID_XTASKGETTICKCOUNTFROMISR)
#define traceAPI_uxTaskGetNumberOfTasks()                                                                                                                SEGGER_SYSVIEW_RecordVoid (apiID_UXTASKGETNUMBEROFTASKS)
#define traceAPI_pcTaskGetName(xTaskToQuery)                                                                                                             SEGGER_SYSVIEW_RecordU32  (apiID_PCTASKGETNAME, SEGGER_SYSVIEW_ShrinkId((U32)xTaskToQuery))
#define traceAPI_xTaskGetHandle(pcNameToQuery)                                                                                                           SEGGER_SYSVIEW_RecordU32  (apiID_XTASKGETHANDLE, SEGGER_SYSVIEW_ShrinkId((U32)pcNameToQuery))
#define traceAPI_uxTaskGetSystemState(pxTaskStatusArray, uxArraySize, pulTotalRunTime)                                                                   SEGGER_SYSVIEW_RecordU32x3(apiID_UXTASKGETSYSTEMSTATE, SEGGER_SYSVIEW_ShrinkId((U32)pxTaskStatusArray), (U32)uxArraySize, SEGGER_SYSVIEW_ShrinkId((U32)pulTotalRunTime))
#define traceAPI_xTaskGetIdleTaskHandle()                                                                                                                SEGGER_SYSVIEW_RecordVoid (apiID_XTASKGETIDLETASKHANDLE)
#define traceAPI_vTaskStepTick(xTicksToJump)                                                                                                             SEGGER_SYSVIEW_RecordU32  (apiID_VTASKSTEPTICK, (U32)xTicksToJump)
#define traceAPI_xTaskCatchUpTicks(xTicksToCatchUp)                                                                                                      SEGGER_SYSVIEW_RecordU32  (apiID_XTASKCATCHUPTICKS, (U32)xTicksToCatchUp)
#define traceAPI_xTaskAbortDelay(xTask)                                                                                                                  SEGGER_SYSVIEW_RecordU32  (apiID_XTASKABORTDELAY, SEGGER_SYSVIEW_ShrinkId((U32)xTask))
#define traceAPI_xTaskIncrementTick()                                                                                                                    SEGGER_SYSVIEW_RecordVoid (apiID_XTASKINCREMENTTICK)
#define traceAPI_vTaskSetApplicationTaskTag(xTask, pxHookFunction)                                                                                       SEGGER_SYSVIEW_RecordU32x2(apiID_VTASKSETAPPLICATIONTASKTAG, SEGGER_SYSVIEW_ShrinkId((U32)xTask), SEGGER_SYSVIEW_ShrinkId((U32)pxHookFunction))
#define traceAPI_xTaskGetApplicationTaskTag(xTask)                                                                                                       SEGGER_SYSVIEW_RecordU32  (apiID_XTASKGETAPPLICATIONTASKTAG, SEGGER_SYSVIEW_ShrinkId((U32)xTask))
#define traceAPI_xTaskGetApplicationTaskTagFromISR(xTask)                                                                                                SEGGER_SYSVIEW_RecordU32  (apiID_XTASKGETAPPLICATIONTASKTAGFROMISR, SEGGER_SYSVIEW_ShrinkId((U32)xTask))
#define traceAPI_xTaskCallApplicationTaskHook(xTask, pvParameter)                                                                                        SEGGER_SYSVIEW_RecordU32x2(apiID_XTASKCALLAPPLICATIONTASKHOOK, SEGGER_SYSVIEW_ShrinkId((U32)xTask), SEGGER_SYSVIEW_ShrinkId((U32)pvParameter))
#define traceAPI_vTaskPlaceOnEventList(pxEventList, xTicksToWait)                                                                                        SEGGER_SYSVIEW_RecordU32x2(apiID_VTASKPLACEONEVENTLIST, SEGGER_SYSVIEW_ShrinkId((U32)pxEventList), (U32)xTicksToWait)
#define traceAPI_vTaskPlaceOnUnorderedEventList(pxEventList, xItemValue, xTicksToWait)                                                                   SEGGER_SYSVIEW_RecordU32x3(apiID_VTASKPLACEONUNORDEREDEVENTLIST, SEGGER_SYSVIEW_ShrinkId((U32)pxEventList), (U32)xItemValue, (U32)xTicksToWait)
#define traceAPI_vTaskPlaceOnEventListRestricted(pxEventList, xTicksToWait, xWaitIndefinitely)                                                           SEGGER_SYSVIEW_RecordU32x3(apiID_VTASKPLACEONEVENTLISTRESTRICTED, SEGGER_SYSVIEW_ShrinkId((U32)pxEventList), (U32)xTicksToWait, (U32)xWaitIndefinitely)
#define traceAPI_xTaskRemoveFromEventList(pxEventList)                                                                                                   SEGGER_SYSVIEW_RecordU32  (apiID_XTASKREMOVEFROMEVENTLIST, SEGGER_SYSVIEW_ShrinkId((U32)pxEventList))
#define traceAPI_vTaskRemoveFromUnorderedEventList(pxEventListItem, xItemValue)                                                                          SEGGER_SYSVIEW_RecordU32x2(apiID_VTASKREMOVEFROMUNORDEREDEVENTLIST, SEGGER_SYSVIEW_ShrinkId((U32)pxEventListItem), (U32)xItemValue)
#define traceAPI_vTaskSetTimeOutState(pxTimeOut)                                                                                                         SEGGER_SYSVIEW_RecordU32  (apiID_VTASKSETTIMEOUTSTATE, SEGGER_SYSVIEW_ShrinkId((U32)pxTimeOut))
#define traceAPI_vTaskInternalSetTimeOutState(pxTimeOut)                                                                                                 SEGGER_SYSVIEW_RecordU32  (apiID_VTASKINTERNALSETTIMEOUTSTATE, SEGGER_SYSVIEW_ShrinkId((U32)pxTimeOut))
#define traceAPI_xTaskCheckForTimeOut(pxTimeOut, pxTicksToWait)                                                                                          SEGGER_SYSVIEW_RecordU32x2(apiID_XTASKCHECKFORTIMEOUT, SEGGER_SYSVIEW_ShrinkId((U32)pxTimeOut), SEGGER_SYSVIEW_ShrinkId((U32)pxTicksToWait))
#define traceAPI_vTaskMissedYield()                                                                                                                      SEGGER_SYSVIEW_RecordVoid (apiID_VTASKMISSEDYIELD)
#define traceAPI_uxTaskGetTaskNumber(xTask)                                                                                                              SEGGER_SYSVIEW_RecordU32  (apiID_UXTASKGETTASKNUMBER, SEGGER_SYSVIEW_ShrinkId((U32)xTask))
#define traceAPI_vTaskSetTaskNumber(xTask, uxHandle)                                                                                                     SEGGER_SYSVIEW_RecordU32x2(apiID_VTASKSETTASKNUMBER, SEGGER_SYSVIEW_ShrinkId((U32)xTask), (U32)uxHandle)
#define traceAPI_eTaskConfirmSleepModeStatus()                                                                                                           SEGGER_SYSVIEW_RecordVoid (apiID_ETASKCONFIRMSLEEPMODESTATUS)
#define traceAPI_vTaskSetThreadLocalStoragePointer(xTaskToSet, xIndex, pvValue)                                                                          SEGGER_SYSVIEW_RecordU32x3(apiID_VTASKSETTHREADLOCALSTORAGEPOINTER, SEGGER_SYSVIEW_ShrinkId((U32)xTaskToSet), (U32)xIndex, SEGGER_SYSVIEW_ShrinkId((U32)pvValue))
#define traceAPI_pvTaskGetThreadLocalStoragePointer(xTaskToQuery, xIndex)                                                                                SEGGER_SYSVIEW_RecordU32x2(apiID_PVTASKGETTHREADLOCALSTORAGEPOINTER, SEGGER_SYSVIEW_ShrinkId((U32)xTaskToQuery), (U32)xIndex)
#define traceAPI_vTaskAllocateMPURegions(xTaskToModify, xRegions)                                                                                        SEGGER_SYSVIEW_RecordU32x2(apiID_VTASKALLOCATEMPUREGIONS, SEGGER_SYSVIEW_ShrinkId((U32)xTaskToModify), SEGGER_SYSVIEW_ShrinkId((U32)xRegions))
#define traceAPI_vTaskGetInfo(xTask, pxTaskStatus, xGetFreeStackSpace, eState)                                                                           SEGGER_SYSVIEW_RecordU32x4(apiID_VTASKGETINFO, SEGGER_SYSVIEW_ShrinkId((U32)xTask), SEGGER_SYSVIEW_ShrinkId((U32)pxTaskStatus), (U32)xGetFreeStackSpace, (U32)eState)
#define traceAPI_uxTaskGetStackHighWaterMark2(xTask)                                                                                                     SEGGER_SYSVIEW_RecordU32  (apiID_UXTASKGETSTACKHIGHWATERMARK2, SEGGER_SYSVIEW_ShrinkId((U32)xTask))
#define traceAPI_uxTaskGetStackHighWaterMark(xTask)                                                                                                      SEGGER_SYSVIEW_RecordU32  (apiID_UXTASKGETSTACKHIGHWATERMARK, SEGGER_SYSVIEW_ShrinkId((U32)xTask))
#define traceAPI_pxTaskGetStackStart(xTask)                                                                                                              SEGGER_SYSVIEW_RecordU32  (apiID_PXTASKGETSTACKSTART, SEGGER_SYSVIEW_ShrinkId((U32)xTask))
#define traceAPI_xTaskGetCurrentTaskHandle()                                                                                                             SEGGER_SYSVIEW_RecordVoid (apiID_XTASKGETCURRENTTASKHANDLE)
#define traceAPI_xTaskGetSchedulerState()                                                                                                                SEGGER_SYSVIEW_RecordVoid (apiID_XTASKGETSCHEDULERSTATE)
#define traceAPI_xTaskPriorityInherit(pxMutexHolder)                                                                                                     SEGGER_SYSVIEW_RecordU32  (apiID_XTASKPRIORITYINHERIT, SEGGER_SYSVIEW_ShrinkId((U32)pxMutexHolder))
#define traceAPI_xTaskPriorityDisinherit(pxMutexHolder)                                                                                                  SEGGER_SYSVIEW_RecordU32  (apiID_XTASKPRIORITYDISINHERIT,SEGGER_SYSVIEW_ShrinkId((U32)pxMutexHolder))
#define traceAPI_vTaskPriorityDisinheritAfterTimeout(pxMutexHolder, uxHighestPriorityWaitingTask)                                                        SEGGER_SYSVIEW_RecordU32x2(apiID_VTASKPRIORITYDISINHERITAFTERTIMEOUT, SEGGER_SYSVIEW_ShrinkId((U32)pxMutexHolder), (U32)uxHighestPriorityWaitingTask)
#define traceAPI_vTaskEnterCritical()                                                                                                                    SEGGER_SYSVIEW_RecordVoid (apiID_VTASKENTERCRITICAL)
#define traceAPI_vTaskExitCritical()                                                                                                                     SEGGER_SYSVIEW_RecordVoid (apiID_VTASKEXITCRITICAL)
#define traceAPI_vTaskList(pcWriteBuffer)                                                                                                                SEGGER_SYSVIEW_RecordU32  (apiID_VTASKLIST, SEGGER_SYSVIEW_ShrinkId((U32)pcWriteBuffer))
#define traceAPI_vTaskGetRunTimeStats(pcWriteBuffer)                                                                                                     SEGGER_SYSVIEW_RecordU32  (apiID_VTASKGETRUNTIMESTATS, SEGGER_SYSVIEW_ShrinkId((U32)pcWriteBuffer))
#define traceAPI_uxTaskResetEventItemValue()                                                                                                             SEGGER_SYSVIEW_RecordVoid (apiID_UXTASKRESETEVENTITEMVALUE)
#define traceAPI_pvTaskIncrementMutexHeldCount()                                                                                                         SEGGER_SYSVIEW_RecordVoid (apiID_PVTASKINCREMENTMUTEXHELDCOUNT)
#define traceAPI_ulTaskGenericNotifyTake(uxIndexToWaitOn, xClearCountOnExit, xTicksToWait)                                                               SEGGER_SYSVIEW_RecordU32x3(apiID_ULTASKGENERICNOTIFYTAKE, (U32)uxIndexToWaitOn, (U32)xClearCountOnExit, (U32)xTicksToWait)
#define traceAPI_xTaskGenericNotifyWait(uxIndexToWaitOn, ulBitsToClearOnEntry, ulBitsToClearOnExit, pulNotificationValue, xTicksToWait)                  SEGGER_SYSVIEW_RecordU32x5(apiID_XTASKGENERICNOTIFYWAIT, (U32)uxIndexToWaitOn, (U32)ulBitsToClearOnEntry, (U32)ulBitsToClearOnExit, (U32)pulNotificationValue, (U32)xTicksToWait)
#define traceAPI_xTaskGenericNotify(xTaskToNotify, uxIndexToNotify, ulValue, eAction, pulPreviousNotificationValue)                                      SEGGER_SYSVIEW_RecordU32x5(apiID_XTASKGENERICNOTIFY, SEGGER_SYSVIEW_ShrinkId((U32)xTaskToNotify), (U32)uxIndexToNotify, (U32)ulValue, (U32)eAction, (U32)pulPreviousNotificationValue)
#define traceAPI_xTaskGenericNotifyFromISR(xTaskToNotify, uxIndexToNotify, ulValue, eAction, pulPreviousNotificationValue, pxHigherPriorityTaskWoken)    SEGGER_SYSVIEW_RecordU32x6(apiID_XTASKGENERICNOTIFYFROMISR, SEGGER_SYSVIEW_ShrinkId((U32)xTaskToNotify), (U32)uxIndexToNotify, (U32)ulValue, (U32)eAction, SEGGER_SYSVIEW_ShrinkId((U32)pulPreviousNotificationValue), SEGGER_SYSVIEW_ShrinkId((U32)pxHigherPriorityTaskWoken))
#define traceAPI_vTaskGenericNotifyGiveFromISR(xTaskToNotify, uxIndexToNotify, pxHigherPriorityTaskWoken)                                                SEGGER_SYSVIEW_RecordU32x3(apiID_VTASKGENERICNOTIFYGIVEFROMISR, SEGGER_SYSVIEW_ShrinkId((U32)xTaskToNotify), (U32)uxIndexToNotify, SEGGER_SYSVIEW_ShrinkId((U32)pxHigherPriorityTaskWoken))
#define traceAPI_xTaskGenericNotifyStateClear(xTask, uxIndexToClear)                                                                                     SEGGER_SYSVIEW_RecordU32x2(apiID_XTASKGENERICNOTIFYSTATECLEAR, SEGGER_SYSVIEW_ShrinkId((U32)xTask), (U32)uxIndexToClear)
#define traceAPI_ulTaskGenericNotifyValueClear(xTask, uxIndexToClear, ulBitsToClear)                                                                     SEGGER_SYSVIEW_RecordU32x3(apiID_ULTASKGENERICNOTIFYVALUECLEAR, SEGGER_SYSVIEW_ShrinkId((U32)xTask), (U32)uxIndexToClear, (U32)ulBitsToClear)
#define traceAPI_ulTaskGetIdleRunTimeCounter()                                                                                                           SEGGER_SYSVIEW_RecordVoid (apiID_ULTASKGETIDLERUNTIMECOUNTER)
#define traceAPI_ulTaskGetIdleRunTimePercent()                                                                                                           SEGGER_SYSVIEW_RecordVoid (apiID_ULTASKGETIDLERUNTIMEPERCENT)

#define traceAPI_vTaskCoreAffinitySet(xTask, uxCoreAffinityMask)                                                                                         SEGGER_SYSVIEW_RecordU32x2(apiID_VTASKCOREAFFINITYSET, SEGGER_SYSVIEW_ShrinkId((U32)xTask), (U32)uxCoreAffinityMask)
#define traceAPI_vTaskCoreAffinityGet(xTask)                                                                                                             SEGGER_SYSVIEW_RecordU32  (apiID_VTASKCOREAFFINITYGET, SEGGER_SYSVIEW_ShrinkId((U32)xTask))
#define traceAPI_vTaskPreemptionDisable(xTask)                                                                                                           SEGGER_SYSVIEW_RecordU32  (apiID_VTASKPREEMPTIONDISABLE, SEGGER_SYSVIEW_ShrinkId((U32)xTask))
#define traceAPI_vTaskPreemptionEnable(xTask)                                                                                                            SEGGER_SYSVIEW_RecordU32  (apiID_VTASKPREEMPTIONENABLE, SEGGER_SYSVIEW_ShrinkId((U32)xTask))
#define traceAPI_vTaskYieldWithinAPI()                                                                                                                   SEGGER_SYSVIEW_RecordVoid (apiID_VTASKYIELDWITHINAPI)
#define traceAPI_vTaskEnterCriticalFromISR()                                                                                                             SEGGER_SYSVIEW_RecordVoid (apiID_VTASKENTERCRITICALFROMISR)
#define traceAPI_vTaskExitCriticalFromISR(uxSavedInterruptStatus)                                                                                        SEGGER_SYSVIEW_RecordU32  (apiID_VTASKEXITCRITICALFROMISR, (U32)uxSavedInterruptStatus)
#define traceAPI_ulTaskGetRunTimeCounter(xTask)                                                                                                          SEGGER_SYSVIEW_RecordU32  (apiID_ULTASKGETRUNTIMECOUNTER, SEGGER_SYSVIEW_ShrinkId((U32)xTask))

#define traceAPI_xTimerCreateTimerTask()                                                                                            SEGGER_SYSVIEW_RecordVoid (apiID_XTIMERCREATETIMERTASK)
#define traceAPI_xTimerCreate(pcTimerName, xTimerPeriodInTicks, xAutoReload, pvTimerID, pxCallbackFunction)                         SEGGER_SYSVIEW_RecordU32x5(apiID_XTIMERCREATE, SEGGER_SYSVIEW_ShrinkId((U32)pcTimerName), (U32)xTimerPeriodInTicks, (U32)xAutoReload, SEGGER_SYSVIEW_ShrinkId((U32)pvTimerID), SEGGER_SYSVIEW_ShrinkId((U32)pxCallbackFunction))
#define traceAPI_xTimerCreateStatic(pcTimerName, xTimerPeriodInTicks, xAutoReload, pvTimerID, pxCallbackFunction, pxTimerBuffer)    SEGGER_SYSVIEW_RecordU32x6(apiID_XTIMERCREATESTATIC, SEGGER_SYSVIEW_ShrinkId((U32)pcTimerName), (U32)xTimerPeriodInTicks, (U32)xAutoReload, SEGGER_SYSVIEW_ShrinkId((U32)pvTimerID), SEGGER_SYSVIEW_ShrinkId((U32)pxCallbackFunction), SEGGER_SYSVIEW_ShrinkId((U32)pxTimerBuffer))
#define traceAPI_xTimerGenericCommand(xTimer, xCommandID, xOptionalValue, pxHigherPriorityTaskWoken, xTicksToWait)                  SEGGER_SYSVIEW_RecordU32x5(apiID_XTIMERGENERICCOMMAND, SEGGER_SYSVIEW_ShrinkId((U32)xTimer), (U32)xCommandID, (U32)xOptionalValue, SEGGER_SYSVIEW_ShrinkId((U32)pxHigherPriorityTaskWoken), (U32)xTicksToWait)
#define traceAPI_xTimerGenericCommandFromTask(xTimer, xCommandID, xOptionalValue, pxHigherPriorityTaskWoken, xTicksToWait)          SEGGER_SYSVIEW_RecordU32x5(apiID_XTIMERGENERICCOMMANDFROMTASK, SEGGER_SYSVIEW_ShrinkId((U32)xTimer), (U32)xCommandID, (U32)xOptionalValue, SEGGER_SYSVIEW_ShrinkId((U32)pxHigherPriorityTaskWoken), (U32)xTicksToWait)
#define traceAPI_xTimerGenericCommandFromISR(xTimer, xCommandID, xOptionalValue, pxHigherPriorityTaskWoken, xTicksToWait)           SEGGER_SYSVIEW_RecordU32x5(apiID_XTIMERGENERICCOMMANDFROMISR, SEGGER_SYSVIEW_ShrinkId((U32)xTimer), (U32)xCommandID, (U32)xOptionalValue, SEGGER_SYSVIEW_ShrinkId((U32)pxHigherPriorityTaskWoken), (U32)xTicksToWait)
#define traceAPI_xTimerGetTimerDaemonTaskHandle()                                                                                   SEGGER_SYSVIEW_RecordVoid (apiID_XTIMERGETTIMERDAEMONTASKHANDLE)
#define traceAPI_xTimerGetPeriod(xTimer)                                                                                            SEGGER_SYSVIEW_RecordU32  (apiID_XTIMERGETPERIOD, SEGGER_SYSVIEW_ShrinkId((U32)xTimer))
#define traceAPI_vTimerSetReloadMode(xTimer, xAutoReload)                                                                           SEGGER_SYSVIEW_RecordU32x2(apiID_VTIMERSETRELOADMODE, SEGGER_SYSVIEW_ShrinkId((U32)xTimer), (U32)xAutoReload)
#define traceAPI_xTimerGetReloadMode(xTimer)                                                                                        SEGGER_SYSVIEW_RecordU32  (apiID_XTIMERGETRELOADMODE, SEGGER_SYSVIEW_ShrinkId((U32)xTimer))
#define traceAPI_xTimerGetExpiryTime(xTimer)                                                                                        SEGGER_SYSVIEW_RecordU32  (apiID_XTIMERGETEXPIRYTIME, SEGGER_SYSVIEW_ShrinkId((U32)xTimer))
#define traceAPI_pcTimerGetName(xTimer)                                                                                             SEGGER_SYSVIEW_RecordU32  (apiID_PCTIMERGETNAME, SEGGER_SYSVIEW_ShrinkId((U32)xTimer))
#define traceAPI_xTimerIsTimerActive(xTimer)                                                                                        SEGGER_SYSVIEW_RecordU32  (apiID_XTIMERISTIMERACTIVE, SEGGER_SYSVIEW_ShrinkId((U32)xTimer))
#define traceAPI_pvTimerGetTimerID(xTimer)                                                                                          SEGGER_SYSVIEW_RecordU32  (apiID_PVTIMERGETTIMERID, SEGGER_SYSVIEW_ShrinkId((U32)xTimer))
#define traceAPI_vTimerSetTimerID(xTimer, pvNewID)                                                                                  SEGGER_SYSVIEW_RecordU32x2(apiID_VTIMERSETTIMERID, SEGGER_SYSVIEW_ShrinkId((U32)xTimer), SEGGER_SYSVIEW_ShrinkId((U32)pvNewID))
#define traceAPI_xTimerPendFunctionCallFromISR(xFunctionToPend, pvParameter1, ulParameter2, pxHigherPriorityTaskWoken)              SEGGER_SYSVIEW_RecordU32x4(apiID_XTIMERPENDFUNCTIONCALLFROMISR, SEGGER_SYSVIEW_ShrinkId((U32)xFunctionToPend), SEGGER_SYSVIEW_ShrinkId((U32)pvParameter1), (U32)ulParameter2, SEGGER_SYSVIEW_ShrinkId((U32)pxHigherPriorityTaskWoken))
#define traceAPI_xTimerPendFunctionCall(xFunctionToPend, pvParameter1, ulParameter2, xTicksToWait)                                  SEGGER_SYSVIEW_RecordU32x4(apiID_XTIMERPENDFUNCTIONCALL, SEGGER_SYSVIEW_ShrinkId((U32)xFunctionToPend), SEGGER_SYSVIEW_ShrinkId((U32)pvParameter1), (U32)ulParameter2, (U32)xTicksToWait)
#define traceAPI_uxTimerGetTimerNumber(xTimer)                                                                                      SEGGER_SYSVIEW_RecordU32  (apiID_UXTIMERGETTIMERNUMBER, SEGGER_SYSVIEW_ShrinkId((U32)xTimer))
#define traceAPI_vTimerSetTimerNumber(xTimer, uxTimerNumber)                                                                        SEGGER_SYSVIEW_RecordU32x2(apiID_VTIMERSETTIMERNUMBER, SEGGER_SYSVIEW_ShrinkId((U32)xTimer), (U32)uxTimerNumber)

#define traceAPI_vListInitialise(pxList)                  SEGGER_SYSVIEW_RecordU32  (apiID_VLISTINITIALISE, SEGGER_SYSVIEW_ShrinkId((U32)pxList))
#define traceAPI_vListInitialiseItem(pxItem)              SEGGER_SYSVIEW_RecordU32  (apiID_VLISTINITIALISEITEM, SEGGER_SYSVIEW_ShrinkId((U32)pxItem))
#define traceAPI_vListInsertEnd(pxList, pxNewListItem)    SEGGER_SYSVIEW_RecordU32x2(apiID_VLISTINSERTEND, SEGGER_SYSVIEW_ShrinkId((U32)pxList), SEGGER_SYSVIEW_ShrinkId((U32)pxNewListItem))
#define traceAPI_vListInsert(pxList, pxNewListItem)       SEGGER_SYSVIEW_RecordU32x2(apiID_VLISTINSERT, SEGGER_SYSVIEW_ShrinkId((U32)pxList), SEGGER_SYSVIEW_ShrinkId((U32)pxNewListItem))
#define traceAPI_uxListRemove(pxItemToRemove)             SEGGER_SYSVIEW_RecordU32  (apiID_UXLISTREMOVE, SEGGER_SYSVIEW_ShrinkId((U32)pxItemToRemove))

#define traceAPI_xCoRoutineCreate(pxCoRoutineCode, uxPriority, uxIndex)    SEGGER_SYSVIEW_RecordU32x3(apiID_XCOROUTINECREATE, SEGGER_SYSVIEW_ShrinkId((U32)pxCoRoutineCode), (U32)uxPriority, (U32)uxIndex)
#define traceAPI_vCoRoutineAddToDelayedList(xTicksToDelay, pxEventList)    SEGGER_SYSVIEW_RecordU32x2(apiID_VCOROUTINEADDTODELAYEDLIST, (U32)xTicksToDelay, SEGGER_SYSVIEW_ShrinkId((U32)pxEventList))
#define traceAPI_vCoRoutineSchedule()                                      SEGGER_SYSVIEW_RecordVoid (apiID_VCOROUTINESCHEDULE)
#define traceAPI_xCoRoutineRemoveFromEventList(pxEventList)                SEGGER_SYSVIEW_RecordU32  (apiID_XCOROUTINEREMOVEFROMEVENTLIST, SEGGER_SYSVIEW_ShrinkId((U32)pxEventList))

#define traceAPI_xEventGroupCreateStatic(pxEventGroupBuffer)                                                       SEGGER_SYSVIEW_RecordU32  (apiID_XEVENTGROUPCREATESTATIC, SEGGER_SYSVIEW_ShrinkId((U32)pxEventGroupBuffer))
#define traceAPI_xEventGroupCreate()                                                                               SEGGER_SYSVIEW_RecordVoid (apiID_XEVENTGROUPCREATE)
#define traceAPI_xEventGroupSync(xEventGroup, uxBitsToSet, uxBitsToWaitFor, xTicksToWait)                          SEGGER_SYSVIEW_RecordU32x4(apiID_XEVENTGROUPSYNC, SEGGER_SYSVIEW_ShrinkId((U32)xEventGroup), (U32)uxBitsToSet, (U32)uxBitsToWaitFor, (U32)xTicksToWait)
#define traceAPI_xEventGroupWaitBits(xEventGroup, uxBitsToWaitFor, xClearOnExit, xWaitForAllBits, xTicksToWait)    SEGGER_SYSVIEW_RecordU32x5(apiID_XEVENTGROUPWAITBITS, SEGGER_SYSVIEW_ShrinkId((U32)xEventGroup), (U32)uxBitsToWaitFor, (U32)xClearOnExit, (U32)xWaitForAllBits, (U32)xTicksToWait)
#define traceAPI_xEventGroupClearBits(xEventGroup, uxBitsToClear)                                                  SEGGER_SYSVIEW_RecordU32x2(apiID_XEVENTGROUPCLEARBITS, SEGGER_SYSVIEW_ShrinkId((U32)xEventGroup), (U32)uxBitsToClear)
#define traceAPI_xEventGroupClearBitsFromISR(xEventGroup, uxBitsToClear)                                           SEGGER_SYSVIEW_RecordU32x2(apiID_XEVENTGROUPCLEARBITSFROMISR, SEGGER_SYSVIEW_ShrinkId((U32)xEventGroup), (U32)uxBitsToClear)
#define traceAPI_xEventGroupGetBitsFromISR(xEventGroup)                                                            SEGGER_SYSVIEW_RecordU32  (apiID_XEVENTGROUPGETBITSFROMISR, SEGGER_SYSVIEW_ShrinkId((U32)xEventGroup))
#define traceAPI_xEventGroupSetBits(xEventGroup, uxBitsToSet)                                                      SEGGER_SYSVIEW_RecordU32x2(apiID_XEVENTGROUPSETBITS, SEGGER_SYSVIEW_ShrinkId((U32)xEventGroup), (U32)uxBitsToSet)
#define traceAPI_vEventGroupDelete(xEventGroup)                                                                    SEGGER_SYSVIEW_RecordU32  (apiID_VEVENTGROUPDELETE, SEGGER_SYSVIEW_ShrinkId((U32)xEventGroup))
#define traceAPI_vEventGroupSetBitsCallback(pvEventGroup, ulBitsToSet)                                             SEGGER_SYSVIEW_RecordU32x2(apiID_VEVENTGROUPSETBITSCALLBACK, SEGGER_SYSVIEW_ShrinkId((U32)pvEventGroup), (U32)ulBitsToSet)
#define traceAPI_vEventGroupClearBitsCallback(pvEventGroup, ulBitsToClear)                                         SEGGER_SYSVIEW_RecordU32x2(apiID_VEVENTGROUPCLEARBITSCALLBACK, SEGGER_SYSVIEW_ShrinkId((U32)pvEventGroup), (U32)ulBitsToClear)
#define traceAPI_xEventGroupSetBitsFromISR(xEventGroup, uxBitsToSet, pxHigherPriorityTaskWoken)                    SEGGER_SYSVIEW_RecordU32x3(apiID_XEVENTGROUPSETBITSFROMISR, SEGGER_SYSVIEW_ShrinkId((U32)xEventGroup), (U32)uxBitsToSet, SEGGER_SYSVIEW_ShrinkId((U32)pxHigherPriorityTaskWoken))
#define traceAPI_uxEventGroupGetNumber(xEventGroup)                                                                SEGGER_SYSVIEW_RecordU32  (apiID_UXEVENTGROUPGETNUMBER, SEGGER_SYSVIEW_ShrinkId((U32)xEventGroup))
#define traceAPI_vEventGroupSetNumber(xEventGroup, uxEventGroupNumber)                                             SEGGER_SYSVIEW_RecordU32x2(apiID_VEVENTGROUPSETNUMBER, SEGGER_SYSVIEW_ShrinkId((U32)xEventGroup), (U32)uxEventGroupNumber)

#define traceAPI_xStreamBufferGenericCreate(xBufferSizeBytes, xTriggerLevelBytes, xIsMessageBuffer, pxSendCompletedCallback, pxReceiveCompletedCallback)                                                            SEGGER_SYSVIEW_RecordU32x5(apiID_XSTREAMBUFFERGENERICCREATE, (U32)xBufferSizeBytes, (U32)xTriggerLevelBytes, (U32)xIsMessageBuffer, SEGGER_SYSVIEW_ShrinkId((U32)pxSendCompletedCallback), SEGGER_SYSVIEW_ShrinkId((U32)pxReceiveCompletedCallback))
#define traceAPI_xStreamBufferGenericCreateStatic(xBufferSizeBytes, xTriggerLevelBytes, xIsMessageBuffer, pucStreamBufferStorageArea, pxStaticStreamBuffer, pxSendCompletedCallback, pxReceiveCompletedCallback)    SEGGER_SYSVIEW_RecordU32x7(apiID_XSTREAMBUFFERGENERICCREATESTATIC, (U32)xBufferSizeBytes, (U32)xTriggerLevelBytes, (U32)xIsMessageBuffer, SEGGER_SYSVIEW_ShrinkId((U32)pucStreamBufferStorageArea), SEGGER_SYSVIEW_ShrinkId((U32)pxStaticStreamBuffer), SEGGER_SYSVIEW_ShrinkId((U32)pxSendCompletedCallback), SEGGER_SYSVIEW_ShrinkId((U32)pxReceiveCompletedCallback))
#define traceAPI_vStreamBufferDelete(xStreamBuffer)                                                                                                                                                                 SEGGER_SYSVIEW_RecordU32  (apiID_VSTREAMBUFFERDELETE, SEGGER_SYSVIEW_ShrinkId((U32)xStreamBuffer))
#define traceAPI_xStreamBufferReset(xStreamBuffer)                                                                                                                                                                  SEGGER_SYSVIEW_RecordU32  (apiID_XSTREAMBUFFERRESET, SEGGER_SYSVIEW_ShrinkId((U32)xStreamBuffer))
#define traceAPI_xStreamBufferSetTriggerLevel(xStreamBuffer, xTriggerLevel)                                                                                                                                         SEGGER_SYSVIEW_RecordU32x2(apiID_XSTREAMBUFFERSETTRIGGERLEVEL, SEGGER_SYSVIEW_ShrinkId((U32)xStreamBuffer), (U32)xTriggerLevel)
#define traceAPI_xStreamBufferSpacesAvailable(xStreamBuffer)                                                                                                                                                        SEGGER_SYSVIEW_RecordU32  (apiID_XSTREAMBUFFERSPACESAVAILABLE, SEGGER_SYSVIEW_ShrinkId((U32)xStreamBuffer))
#define traceAPI_xStreamBufferBytesAvailable(xStreamBuffer)                                                                                                                                                         SEGGER_SYSVIEW_RecordU32  (apiID_XSTREAMBUFFERBYTESAVAILABLE, SEGGER_SYSVIEW_ShrinkId((U32)xStreamBuffer))
#define traceAPI_xStreamBufferSend(xStreamBuffer, pvTxData, xDataLengthBytes, xTicksToWait)                                                                                                                         SEGGER_SYSVIEW_RecordU32x4(apiID_XSTREAMBUFFERSEND, SEGGER_SYSVIEW_ShrinkId((U32)xStreamBuffer), SEGGER_SYSVIEW_ShrinkId((U32)pvTxData), (U32)xDataLengthBytes, (U32)xTicksToWait)
#define traceAPI_xStreamBufferSendFromISR(xStreamBuffer, pvTxData, xDataLengthBytes, pxHigherPriorityTaskWoken)                                                                                                     SEGGER_SYSVIEW_RecordU32x4(apiID_XSTREAMBUFFERSENDFROMISR, SEGGER_SYSVIEW_ShrinkId((U32)xStreamBuffer), SEGGER_SYSVIEW_ShrinkId((U32)pvTxData), (U32)xDataLengthBytes, SEGGER_SYSVIEW_ShrinkId((U32)pxHigherPriorityTaskWoken))
#define traceAPI_xStreamBufferReceive(xStreamBuffer, pvRxData, xBufferLengthBytes, xTicksToWait)                                                                                                                    SEGGER_SYSVIEW_RecordU32x4(apiID_XSTREAMBUFFERRECEIVE, SEGGER_SYSVIEW_ShrinkId((U32)xStreamBuffer), SEGGER_SYSVIEW_ShrinkId((U32)pvRxData), (U32)xBufferLengthBytes, (U32)xTicksToWait)
#define traceAPI_xStreamBufferNextMessageLengthBytes(xStreamBuffer)                                                                                                                                                 SEGGER_SYSVIEW_RecordU32  (apiID_XSTREAMBUFFERNEXTMESSAGELENGTHBYTES, SEGGER_SYSVIEW_ShrinkId((U32)xStreamBuffer))
#define traceAPI_xStreamBufferReceiveFromISR(xStreamBuffer, pvRxData, xBufferLengthBytes, pxHigherPriorityTaskWoken)                                                                                                SEGGER_SYSVIEW_RecordU32x4(apiID_XSTREAMBUFFERRECEIVEFROMISR, SEGGER_SYSVIEW_ShrinkId((U32)xStreamBuffer), SEGGER_SYSVIEW_ShrinkId((U32)pvRxData), (U32)xBufferLengthBytes, SEGGER_SYSVIEW_ShrinkId((U32)pxHigherPriorityTaskWoken))
#define traceAPI_xStreamBufferIsEmpty(xStreamBuffer)                                                                                                                                                                SEGGER_SYSVIEW_RecordU32  (apiID_XSTREAMBUFFERISEMPTY, SEGGER_SYSVIEW_ShrinkId((U32)xStreamBuffer))
#define traceAPI_xStreamBufferIsFull(xStreamBuffer)                                                                                                                                                                 SEGGER_SYSVIEW_RecordU32  (apiID_XSTREAMBUFFERISFULL, SEGGER_SYSVIEW_ShrinkId((U32)xStreamBuffer))
#define traceAPI_xStreamBufferSendCompletedFromISR(xStreamBuffer,pxHigherPriorityTaskWoken)                                                                                                                         SEGGER_SYSVIEW_RecordU32x2(apiID_XSTREAMBUFFERSENDCOMPLETEDFROMISR, SEGGER_SYSVIEW_ShrinkId((U32)xStreamBuffer),SEGGER_SYSVIEW_ShrinkId((U32)pxHigherPriorityTaskWoken))
#define traceAPI_xStreamBufferReceiveCompletedFromISR(xStreamBuffer, pxHigherPriorityTaskWoken)                                                                                                                     SEGGER_SYSVIEW_RecordU32x2(apiID_XSTREAMBUFFERRECEIVECOMPLETEDFROMISR, SEGGER_SYSVIEW_ShrinkId((U32)xStreamBuffer), SEGGER_SYSVIEW_ShrinkId((U32)pxHigherPriorityTaskWoken))
#define traceAPI_uxStreamBufferGetStreamBufferNumber(xStreamBuffer)                                                                                                                                                 SEGGER_SYSVIEW_RecordU32  (apiID_UXSTREAMBUFFERGETSTREAMBUFFERNUMBER, SEGGER_SYSVIEW_ShrinkId((U32)xStreamBuffer))
#define traceAPI_vStreamBufferSetStreamBufferNumber()                                                                                                                                                               SEGGER_SYSVIEW_RecordVoid (apiID_VSTREAMBUFFERSETSTREAMBUFFERNUMBER)
#define traceAPI_ucStreamBufferGetStreamBufferType(xStreamBuffer)                                                                                                                                                   SEGGER_SYSVIEW_RecordU32  (apiID_UCSTREAMBUFFERGETSTREAMBUFFERTYPE, SEGGER_SYSVIEW_ShrinkId((U32)xStreamBuffer))

#if ( TRACERETURN_ENABLE != 0 )
  #define traceRETURN_xQueueGenericReset(xReturn)                       SEGGER_SYSVIEW_RecordEndCallU32(apiID_XQUEUEGENERICRESET, (U32)xReturn)
  #define traceRETURN_xQueueGenericCreateStatic(pxNewQueue)             SEGGER_SYSVIEW_RecordEndCallU32(apiID_XQUEUEGENERICCREATESTATIC, SEGGER_SYSVIEW_ShrinkId((U32)pxNewQueue))
  #define traceRETURN_xQueueGenericCreate(pxNewQueue)                   SEGGER_SYSVIEW_RecordEndCallU32(apiID_XQUEUEGENERICCREATE, SEGGER_SYSVIEW_ShrinkId((U32)pxNewQueue))
  #define traceRETURN_xQueueCreateMutex(xNewQueue)                      SEGGER_SYSVIEW_RecordEndCallU32(apiID_XQUEUECREATEMUTEX, SEGGER_SYSVIEW_ShrinkId((U32)xNewQueue))
  #define traceRETURN_xQueueCreateMutexStatic(xNewQueue)                SEGGER_SYSVIEW_RecordEndCallU32(apiID_XQUEUECREATEMUTEXSTATIC, SEGGER_SYSVIEW_ShrinkId((U32)xNewQueue))
  #define traceRETURN_xQueueGetMutexHolder(pxReturn)                    SEGGER_SYSVIEW_RecordEndCallU32(apiID_XQUEUEGETMUTEXHOLDER, SEGGER_SYSVIEW_ShrinkId((U32)pxReturn))
  #define traceRETURN_xQueueGetMutexHolderFromISR(pxReturn)             SEGGER_SYSVIEW_RecordEndCallU32(apiID_XQUEUEGETMUTEXHOLDERFROMISR, SEGGER_SYSVIEW_ShrinkId((U32)pxReturn))
  #define traceRETURN_xQueueGiveMutexRecursive(xReturn)                 SEGGER_SYSVIEW_RecordEndCallU32(apiID_XQUEUEGIVEMUTEXRECURSIVE, (U32)xReturn)
  #define traceRETURN_xQueueTakeMutexRecursive(xReturn)                 SEGGER_SYSVIEW_RecordEndCallU32(apiID_XQUEUETAKEMUTEXRECURSIVE, (U32)xReturn)
  #define traceRETURN_xQueueCreateCountingSemaphoreStatic(xHandle)      SEGGER_SYSVIEW_RecordEndCallU32(apiID_XQUEUECREATECOUNTINGSEMAPHORESTATIC, SEGGER_SYSVIEW_ShrinkId((U32)xHandle))
  #define traceRETURN_xQueueCreateCountingSemaphore(xHandle)            SEGGER_SYSVIEW_RecordEndCallU32(apiID_XQUEUECREATECOUNTINGSEMAPHORE, SEGGER_SYSVIEW_ShrinkId((U32)xHandle))
  #define traceRETURN_xQueueGenericSend(xReturn)                        SEGGER_SYSVIEW_RecordEndCallU32(apiID_XQUEUEGENERICSEND, (U32)xReturn)
  #define traceRETURN_xQueueGenericSendFromISR(xReturn)                 SEGGER_SYSVIEW_RecordEndCallU32(apiID_XQUEUEGENERICSENDFROMISR, (U32)xReturn)
  #define traceRETURN_xQueueGiveFromISR(xReturn)                        SEGGER_SYSVIEW_RecordEndCallU32(apiID_XQUEUEGIVEFROMISR, (U32)xReturn)
  #define traceRETURN_xQueueReceive(xReturn)                            SEGGER_SYSVIEW_RecordEndCallU32(apiID_XQUEUERECEIVE, (U32)xReturn)
  #define traceRETURN_xQueueSemaphoreTake(xReturn)                      SEGGER_SYSVIEW_RecordEndCallU32(apiID_XQUEUESEMAPHORETAKE, (U32)xReturn)
  #define traceRETURN_xQueuePeek(xReturn)                               SEGGER_SYSVIEW_RecordEndCallU32(apiID_XQUEUEPEEK, (U32)xReturn)
  #define traceRETURN_xQueueReceiveFromISR(xReturn)                     SEGGER_SYSVIEW_RecordEndCallU32(apiID_XQUEUERECEIVEFROMISR, (U32)xReturn)
  #define traceRETURN_xQueuePeekFromISR(xReturn)                        SEGGER_SYSVIEW_RecordEndCallU32(apiID_XQUEUEPEEKFROMISR, (U32)xReturn)
  #define traceRETURN_uxQueueMessagesWaiting(uxReturn)                  SEGGER_SYSVIEW_RecordEndCallU32(apiID_UXQUEUEMESSAGESWAITING, (U32)uxReturn)
  #define traceRETURN_uxQueueSpacesAvailable(uxReturn)                  SEGGER_SYSVIEW_RecordEndCallU32(apiID_UXQUEUESPACESAVAILABLE, (U32)uxReturn)
  #define traceRETURN_uxQueueMessagesWaitingFromISR(uxReturn)           SEGGER_SYSVIEW_RecordEndCallU32(apiID_UXQUEUEMESSAGESWAITINGFROMISR, (U32)uxReturn)
  #define traceRETURN_vQueueDelete()                                    SEGGER_SYSVIEW_RecordEndCall   (apiID_VQUEUEDELETE)
  #define traceRETURN_uxQueueGetQueueNumber(uxQueueNumber)              SEGGER_SYSVIEW_RecordEndCallU32(apiID_UXQUEUEGETQUEUENUMBER, (U32)uxQueueNumber)
  #define traceRETURN_vQueueSetQueueNumber()                            SEGGER_SYSVIEW_RecordEndCall   (apiID_VQUEUESETQUEUENUMBER)
  #define traceRETURN_ucQueueGetQueueType(ucQueueType)                  SEGGER_SYSVIEW_RecordEndCallU32(apiID_UCQUEUEGETQUEUETYPE, (U32)ucQueueType)
  #define traceRETURN_xQueueIsQueueEmptyFromISR(xReturn)                SEGGER_SYSVIEW_RecordEndCallU32(apiID_XQUEUEISQUEUEEMPTYFROMISR, (U32)xReturn)
  #define traceRETURN_xQueueIsQueueFullFromISR(xReturn)                 SEGGER_SYSVIEW_RecordEndCallU32(apiID_XQUEUEISQUEUEFULLFROMISR, (U32)xReturn)
  #define traceRETURN_xQueueCRSend(xReturn)                             SEGGER_SYSVIEW_RecordEndCallU32(apiID_XQUEUECRSEND, (U32)xReturn)
  #define traceRETURN_xQueueCRReceive(xReturn)                          SEGGER_SYSVIEW_RecordEndCallU32(apiID_XQUEUECRRECEIVE, (U32)xReturn)
  #define traceRETURN_xQueueCRSendFromISR(xCoRoutinePreviouslyWoken)    SEGGER_SYSVIEW_RecordEndCallU32(apiID_XQUEUECRSENDFROMISR, (U32)xCoRoutinePreviouslyWoken)
  #define traceRETURN_xQueueCRReceiveFromISR(xReturn)                   SEGGER_SYSVIEW_RecordEndCallU32(apiID_XQUEUECRRECEIVEFROMISR, (U32)xReturn)
  #define traceRETURN_vQueueAddToRegistry()                             SEGGER_SYSVIEW_RecordEndCall   (apiID_VQUEUEADDTOREGISTRY)
  #define traceRETURN_pcQueueGetName(pcReturn)                          SEGGER_SYSVIEW_RecordEndCallU32(apiID_PCQUEUEGETNAME, SEGGER_SYSVIEW_ShrinkId((U32)pcReturn))
  #define traceRETURN_vQueueUnregisterQueue()                           SEGGER_SYSVIEW_RecordEndCall   (apiID_VQUEUEUNREGISTERQUEUE)
  #define traceRETURN_vQueueWaitForMessageRestricted()                  SEGGER_SYSVIEW_RecordEndCall   (apiID_VQUEUEWAITFORMESSAGERESTRICTED)
  #define traceRETURN_xQueueCreateSet(pxQueue)                          SEGGER_SYSVIEW_RecordEndCallU32(apiID_XQUEUECREATESET, SEGGER_SYSVIEW_ShrinkId((U32)pxQueue))
  #define traceRETURN_xQueueAddToSet(xReturn)                           SEGGER_SYSVIEW_RecordEndCallU32(apiID_XQUEUEADDTOSET, (U32)xReturn)
  #define traceRETURN_xQueueRemoveFromSet(xReturn)                      SEGGER_SYSVIEW_RecordEndCallU32(apiID_XQUEUEREMOVEFROMSET, (U32)xReturn)
  #define traceRETURN_xQueueSelectFromSet(xReturn)                      SEGGER_SYSVIEW_RecordEndCallU32(apiID_XQUEUESELECTFROMSET, SEGGER_SYSVIEW_ShrinkId((U32)xReturn))
  #define traceRETURN_xQueueSelectFromSetFromISR(xReturn)               SEGGER_SYSVIEW_RecordEndCallU32(apiID_XQUEUESELECTFROMSETFROMISR, SEGGER_SYSVIEW_ShrinkId((U32)xReturn))

  #define traceRETURN_xTaskCreateStatic(xReturn)                           SEGGER_SYSVIEW_RecordEndCallU32(apiID_XTASKCREATESTATIC, SEGGER_SYSVIEW_ShrinkId((U32)xReturn))
  #define traceRETURN_xTaskCreateRestrictedStatic(xReturn)                 SEGGER_SYSVIEW_RecordEndCallU32(apiID_XTASKCREATERESTRICTEDSTATIC, (U32)xReturn)
  #define traceRETURN_xTaskCreateRestricted(xReturn)                       SEGGER_SYSVIEW_RecordEndCallU32(apiID_XTASKCREATERESTRICTED, (U32)xReturn)
  #define traceRETURN_xTaskCreate(xReturn)                                 SEGGER_SYSVIEW_RecordEndCallU32(apiID_XTASKCREATE, (U32)xReturn)
  #define traceRETURN_vTaskDelete()                                        SEGGER_SYSVIEW_RecordEndCall   (apiID_VTASKDELETE)
  #define traceRETURN_xTaskDelayUntil(xShouldDelay)                        SEGGER_SYSVIEW_RecordEndCallU32(apiID_XTASKDELAYUNTIL, (U32)xShouldDelay)
  #define traceRETURN_vTaskDelay()                                         SEGGER_SYSVIEW_RecordEndCall   (apiID_VTASKDELAY)
  #define traceRETURN_eTaskGetState(eReturn)                               SEGGER_SYSVIEW_RecordEndCallU32(apiID_ETASKGETSTATE, (U32)eReturn)
  #define traceRETURN_uxTaskPriorityGet(uxReturn)                          SEGGER_SYSVIEW_RecordEndCallU32(apiID_UXTASKPRIORITYGET, (U32)uxReturn)
  #define traceRETURN_uxTaskPriorityGetFromISR(uxReturn)                   SEGGER_SYSVIEW_RecordEndCallU32(apiID_UXTASKPRIORITYGETFROMISR, (U32)uxReturn)
  #define traceRETURN_vTaskPrioritySet()                                   SEGGER_SYSVIEW_RecordEndCall   (apiID_VTASKPRIORITYSET)
  #define traceRETURN_vTaskSuspend()                                       SEGGER_SYSVIEW_RecordEndCall   (apiID_VTASKSUSPEND)
  #define traceRETURN_vTaskResume()                                        SEGGER_SYSVIEW_RecordEndCall   (apiID_VTASKRESUME)
  #define traceRETURN_xTaskResumeFromISR(xYieldRequired)                   SEGGER_SYSVIEW_RecordEndCallU32(apiID_XTASKRESUMEFROMISR, (U32)xYieldRequired)
  #define traceRETURN_vTaskStartScheduler()                                SEGGER_SYSVIEW_RecordEndCall   (apiID_VTASKSTARTSCHEDULER)
  #define traceRETURN_vTaskEndScheduler()                                  SEGGER_SYSVIEW_RecordEndCall   (apiID_VTASKENDSCHEDULER)
  #define traceRETURN_vTaskSuspendAll()                                    SEGGER_SYSVIEW_RecordEndCall   (apiID_VTASKSUSPENDALL)
  #define traceRETURN_xTaskResumeAll(xAlreadyYielded)                      SEGGER_SYSVIEW_RecordEndCallU32(apiID_XTASKRESUMEALL, (U32)xAlreadyYielded)
  #define traceRETURN_xTaskGetTickCount(xTicks)                            SEGGER_SYSVIEW_RecordEndCallU32(apiID_XTASKGETTICKCOUNT, (U32)xTicks)
  #define traceRETURN_xTaskGetTickCountFromISR(xReturn)                    SEGGER_SYSVIEW_RecordEndCallU32(apiID_XTASKGETTICKCOUNTFROMISR, (U32)xReturn)
  #define traceRETURN_uxTaskGetNumberOfTasks(uxCurrentNumberOfTasks)       SEGGER_SYSVIEW_RecordEndCallU32(apiID_UXTASKGETNUMBEROFTASKS, (U32)uxCurrentNumberOfTasks)
  #define traceRETURN_pcTaskGetName(pcTaskName)                            SEGGER_SYSVIEW_RecordEndCallU32(apiID_PCTASKGETNAME, SEGGER_SYSVIEW_ShrinkId((U32)pcTaskName))
  #define traceRETURN_xTaskGetHandle(pxTCB)                                SEGGER_SYSVIEW_RecordEndCallU32(apiID_XTASKGETHANDLE, SEGGER_SYSVIEW_ShrinkId((U32)pxTCB))
  #define traceRETURN_uxTaskGetSystemState(uxTask)                         SEGGER_SYSVIEW_RecordEndCallU32(apiID_UXTASKGETSYSTEMSTATE, (U32)uxTask)
  #define traceRETURN_xTaskGetIdleTaskHandle(xIdleTaskHandle)              SEGGER_SYSVIEW_RecordEndCallU32(apiID_XTASKGETIDLETASKHANDLE, SEGGER_SYSVIEW_ShrinkId((U32)xIdleTaskHandle))
  #define traceRETURN_vTaskStepTick()                                      SEGGER_SYSVIEW_RecordEndCall   (apiID_VTASKSTEPTICK)
  #define traceRETURN_xTaskCatchUpTicks(xYieldOccurred)                    SEGGER_SYSVIEW_RecordEndCallU32(apiID_XTASKCATCHUPTICKS, (U32)xYieldOccurred)
  #define traceRETURN_xTaskAbortDelay(xReturn)                             SEGGER_SYSVIEW_RecordEndCallU32(apiID_XTASKABORTDELAY, (U32)xReturn)
  #define traceRETURN_xTaskIncrementTick(xSwitchRequired)                  SEGGER_SYSVIEW_RecordEndCallU32(apiID_XTASKINCREMENTTICK, (U32)xSwitchRequired)
  #define traceRETURN_vTaskSetApplicationTaskTag()                         SEGGER_SYSVIEW_RecordEndCall   (apiID_VTASKSETAPPLICATIONTASKTAG)
  #define traceRETURN_xTaskGetApplicationTaskTag(xReturn)                  SEGGER_SYSVIEW_RecordEndCallU32(apiID_XTASKGETAPPLICATIONTASKTAG, SEGGER_SYSVIEW_ShrinkId((U32)xReturn))
  #define traceRETURN_xTaskGetApplicationTaskTagFromISR(xReturn)           SEGGER_SYSVIEW_RecordEndCallU32(apiID_XTASKGETAPPLICATIONTASKTAGFROMISR, SEGGER_SYSVIEW_ShrinkId((U32)xReturn))
  #define traceRETURN_xTaskCallApplicationTaskHook(xReturn)                SEGGER_SYSVIEW_RecordEndCallU32(apiID_XTASKCALLAPPLICATIONTASKHOOK, (U32)xReturn)
  #define traceRETURN_vTaskPlaceOnEventList()                              SEGGER_SYSVIEW_RecordEndCall   (apiID_VTASKPLACEONEVENTLIST)
  #define traceRETURN_vTaskPlaceOnUnorderedEventList()                     SEGGER_SYSVIEW_RecordEndCall   (apiID_VTASKPLACEONUNORDEREDEVENTLIST)
  #define traceRETURN_vTaskPlaceOnEventListRestricted()                    SEGGER_SYSVIEW_RecordEndCall   (apiID_VTASKPLACEONEVENTLISTRESTRICTED)
  #define traceRETURN_xTaskRemoveFromEventList(xReturn)                    SEGGER_SYSVIEW_RecordEndCallU32(apiID_XTASKREMOVEFROMEVENTLIST, (U32)xReturn)
  #define traceRETURN_vTaskRemoveFromUnorderedEventList()                  SEGGER_SYSVIEW_RecordEndCall   (apiID_VTASKREMOVEFROMUNORDEREDEVENTLIST)
  #define traceRETURN_vTaskSetTimeOutState()                               SEGGER_SYSVIEW_RecordEndCall   (apiID_VTASKSETTIMEOUTSTATE)
  #define traceRETURN_vTaskInternalSetTimeOutState()                       SEGGER_SYSVIEW_RecordEndCall   (apiID_VTASKINTERNALSETTIMEOUTSTATE )
  #define traceRETURN_xTaskCheckForTimeOut(xReturn)                        SEGGER_SYSVIEW_RecordEndCallU32(apiID_XTASKCHECKFORTIMEOUT, (U32)xReturn)
  #define traceRETURN_vTaskMissedYield()                                   SEGGER_SYSVIEW_RecordEndCall   (apiID_VTASKMISSEDYIELD)
  #define traceRETURN_uxTaskGetTaskNumber(uxReturn)                        SEGGER_SYSVIEW_RecordEndCallU32(apiID_UXTASKGETTASKNUMBER, (U32)uxReturn)
  #define traceRETURN_vTaskSetTaskNumber()                                 SEGGER_SYSVIEW_RecordEndCall   (apiID_VTASKSETTASKNUMBER)
  #define traceRETURN_eTaskConfirmSleepModeStatus(eReturn)                 SEGGER_SYSVIEW_RecordEndCallU32(apiID_ETASKCONFIRMSLEEPMODESTATUS, (U32)eReturn)
  #define traceRETURN_vTaskSetThreadLocalStoragePointer()                  SEGGER_SYSVIEW_RecordEndCall   (apiID_VTASKSETTHREADLOCALSTORAGEPOINTER)
  #define traceRETURN_pvTaskGetThreadLocalStoragePointer(pvReturn)         SEGGER_SYSVIEW_RecordEndCallU32(apiID_PVTASKGETTHREADLOCALSTORAGEPOINTER, SEGGER_SYSVIEW_ShrinkId((U32)pvReturn))
  #define traceRETURN_vTaskAllocateMPURegions()                            SEGGER_SYSVIEW_RecordEndCall   (apiID_VTASKALLOCATEMPUREGIONS)
  #define traceRETURN_vTaskGetInfo()                                       SEGGER_SYSVIEW_RecordEndCall   (apiID_VTASKGETINFO)
  #define traceRETURN_uxTaskGetStackHighWaterMark2(uxReturn)               SEGGER_SYSVIEW_RecordEndCallU32(apiID_UXTASKGETSTACKHIGHWATERMARK2, (U32)uxReturn)
  #define traceRETURN_uxTaskGetStackHighWaterMark(uxReturn)                SEGGER_SYSVIEW_RecordEndCallU32(apiID_UXTASKGETSTACKHIGHWATERMARK, (U32)uxReturn)
  #define traceRETURN_pxTaskGetStackStart(pxStack)                         SEGGER_SYSVIEW_RecordEndCallU32(apiID_PXTASKGETSTACKSTART, SEGGER_SYSVIEW_ShrinkId((U32)pxStack))
  #define traceRETURN_xTaskGetCurrentTaskHandle(xReturn)                   SEGGER_SYSVIEW_RecordEndCallU32(apiID_XTASKGETCURRENTTASKHANDLE, SEGGER_SYSVIEW_ShrinkId((U32)xReturn))
  #define traceRETURN_xTaskGetSchedulerState(xReturn)                      SEGGER_SYSVIEW_RecordEndCallU32(apiID_XTASKGETSCHEDULERSTATE, (U32)xReturn)
  #define traceRETURN_xTaskPriorityInherit(xReturn)                        SEGGER_SYSVIEW_RecordEndCallU32(apiID_XTASKPRIORITYINHERIT, (U32)xReturn)
  #define traceRETURN_xTaskPriorityDisinherit(xReturn)                     SEGGER_SYSVIEW_RecordEndCallU32(apiID_XTASKPRIORITYDISINHERIT, (U32)xReturn)
  #define traceRETURN_vTaskPriorityDisinheritAfterTimeout()                SEGGER_SYSVIEW_RecordEndCall   (apiID_VTASKPRIORITYDISINHERITAFTERTIMEOUT)
  #define traceRETURN_vTaskEnterCritical()                                 SEGGER_SYSVIEW_RecordEndCall   (apiID_VTASKENTERCRITICAL)
  #define traceRETURN_vTaskExitCritical()                                  SEGGER_SYSVIEW_RecordEndCall   (apiID_VTASKEXITCRITICAL)
  #define traceRETURN_vTaskList()                                          SEGGER_SYSVIEW_RecordEndCall   (apiID_VTASKLIST)
  #define traceRETURN_vTaskGetRunTimeStats()                               SEGGER_SYSVIEW_RecordEndCall   (apiID_VTASKGETRUNTIMESTATS)
  #define traceRETURN_uxTaskResetEventItemValue(uxReturn)                  SEGGER_SYSVIEW_RecordEndCallU32(apiID_UXTASKRESETEVENTITEMVALUE, (U32)uxReturn)
  #define traceRETURN_pvTaskIncrementMutexHeldCount(pxCurrentTCB)          SEGGER_SYSVIEW_RecordEndCallU32(apiID_PVTASKINCREMENTMUTEXHELDCOUNT, SEGGER_SYSVIEW_ShrinkId((U32)pxCurrentTCB))
  #define traceRETURN_ulTaskGenericNotifyTake(ulReturn)                    SEGGER_SYSVIEW_RecordEndCallU32(apiID_ULTASKGENERICNOTIFYTAKE, (U32)ulReturn)
  #define traceRETURN_xTaskGenericNotifyWait(xReturn)                      SEGGER_SYSVIEW_RecordEndCallU32(apiID_XTASKGENERICNOTIFYWAIT, (U32)xReturn)
  #define traceRETURN_xTaskGenericNotify(xReturn)                          SEGGER_SYSVIEW_RecordEndCallU32(apiID_XTASKGENERICNOTIFY, (U32)xReturn)
  #define traceRETURN_xTaskGenericNotifyFromISR(xReturn)                   SEGGER_SYSVIEW_RecordEndCallU32(apiID_XTASKGENERICNOTIFYFROMISR, (U32)xReturn)
  #define traceRETURN_vTaskGenericNotifyGiveFromISR()                      SEGGER_SYSVIEW_RecordEndCall   (apiID_VTASKGENERICNOTIFYGIVEFROMISR)
  #define traceRETURN_xTaskGenericNotifyStateClear(xReturn)                SEGGER_SYSVIEW_RecordEndCallU32(apiID_XTASKGENERICNOTIFYSTATECLEAR, (U32)xReturn)
  #define traceRETURN_ulTaskGenericNotifyValueClear(ulReturn)              SEGGER_SYSVIEW_RecordEndCallU32(apiID_ULTASKGENERICNOTIFYVALUECLEAR, (U32)ulReturn)
  #define traceRETURN_ulTaskGetIdleRunTimeCounter(ulReturn)                SEGGER_SYSVIEW_RecordEndCallU32(apiID_ULTASKGETIDLERUNTIMECOUNTER, (U32)ulReturn)
  #define traceRETURN_ulTaskGetIdleRunTimePercent(ulReturn)                SEGGER_SYSVIEW_RecordEndCallU32(apiID_ULTASKGETIDLERUNTIMEPERCENT, (U32)ulReturn)
  #define traceRETURN_vTaskCoreAffinitySet()                               SEGGER_SYSVIEW_RecordEndCall   (apiID_VTASKCOREAFFINITYSET)
  #define traceRETURN_vTaskCoreAffinityGet(uxCoreAffinityMask)             SEGGER_SYSVIEW_RecordEndCallU32(apiID_VTASKCOREAFFINITYGET, (U32)uxCoreAffinityMask)
  #define traceRETURN_vTaskPreemptionDisable()                             SEGGER_SYSVIEW_RecordEndCall   (apiID_VTASKPREEMPTIONDISABLE)
  #define traceRETURN_vTaskPreemptionEnable()                              SEGGER_SYSVIEW_RecordEndCall   (apiID_VTASKPREEMPTIONENABLE)
  #define traceRETURN_vTaskYieldWithinAPI()                                SEGGER_SYSVIEW_RecordEndCall   (apiID_VTASKYIELDWITHINAPI)
  #define traceRETURN_vTaskEnterCriticalFromISR(uxSavedInterruptStatus)    SEGGER_SYSVIEW_RecordEndCallU32(apiID_VTASKYIELDWITHINAPI. (U32)uxSavedInterruptStatus)
  #define traceRETURN_vTaskExitCriticalFromISR()                           SEGGER_SYSVIEW_RecordEndCall   (apiID_VTASKEXITCRITICALFROMISR)
  #define traceRETURN_ulTaskGetRunTimeCounter(ulRunTimeCounter)            SEGGER_SYSVIEW_RecordEndCallU32(apiID_ULTASKGETIDLERUNTIMECOUNTER, (U32)ulRunTimeCounter)

  #define traceRETURN_xTimerCreateTimerTask(xReturn)                      SEGGER_SYSVIEW_RecordEndCallU32(apiID_XTIMERCREATETIMERTASK, (U32)xReturn)
  #define traceRETURN_xTimerCreate(pxNewTimer)                            SEGGER_SYSVIEW_RecordEndCallU32(apiID_XTIMERCREATE, SEGGER_SYSVIEW_ShrinkId((U32)pxNewTimer))
  #define traceRETURN_xTimerCreateStatic(pxNewTimer)                      SEGGER_SYSVIEW_RecordEndCallU32(apiID_XTIMERCREATESTATIC, SEGGER_SYSVIEW_ShrinkId((U32)pxNewTimer))
  #define traceRETURN_xTimerGenericCommand(xReturn)                       SEGGER_SYSVIEW_RecordEndCallU32(apiID_XTIMERGENERICCOMMAND, (U32)xReturn)
  #define traceRETURN_xTimerGenericCommandFromTask(xReturn)               SEGGER_SYSVIEW_RecordEndCallU32(apiID_XTIMERGENERICCOMMANDFROMTASK, (U32)xReturn)
  #define traceRETURN_xTimerGenericCommandFromISR(xReturn)                SEGGER_SYSVIEW_RecordEndCallU32(apiID_XTIMERGENERICCOMMANDFROMISR, (U32)xReturn)
  #define traceRETURN_xTimerGetTimerDaemonTaskHandle(xTimerTaskHandle)    SEGGER_SYSVIEW_RecordEndCallU32(apiID_XTIMERGETTIMERDAEMONTASKHANDLE, SEGGER_SYSVIEW_ShrinkId((U32)xTimerTaskHandle))
  #define traceRETURN_xTimerGetPeriod(xTimerPeriodInTicks)                SEGGER_SYSVIEW_RecordEndCallU32(apiID_XTIMERGETPERIOD, (U32)xTimerPeriodInTicks)
  #define traceRETURN_vTimerSetReloadMode()                               SEGGER_SYSVIEW_RecordEndCall   (apiID_VTIMERSETRELOADMODE)
  #define traceRETURN_xTimerGetReloadMode(xReturn)                        SEGGER_SYSVIEW_RecordEndCallU32(apiID_XTIMERGETRELOADMODE, (U32)xReturn)
  #define traceRETURN_xTimerGetExpiryTime(xReturn)                        SEGGER_SYSVIEW_RecordEndCallU32(apiID_XTIMERGETEXPIRYTIME, (U32)xReturn)
  #define traceRETURN_pcTimerGetName(pcTimerName)                         SEGGER_SYSVIEW_RecordEndCallU32(apiID_PCTIMERGETNAME, SEGGER_SYSVIEW_ShrinkId((U32)pcTimerName))
  #define traceRETURN_xTimerIsTimerActive(xReturn)                        SEGGER_SYSVIEW_RecordEndCallU32(apiID_XTIMERISTIMERACTIVE, (U32)xReturn)
  #define traceRETURN_pvTimerGetTimerID(pvReturn)                         SEGGER_SYSVIEW_RecordEndCallU32(apiID_PVTIMERGETTIMERID, SEGGER_SYSVIEW_ShrinkId((U32)pvReturn))
  #define traceRETURN_vTimerSetTimerID()                                  SEGGER_SYSVIEW_RecordEndCall   (apiID_VTIMERSETTIMERID)
  #define traceRETURN_xTimerPendFunctionCallFromISR(xReturn)              SEGGER_SYSVIEW_RecordEndCallU32(apiID_XTIMERPENDFUNCTIONCALLFROMISR, (U32)xReturn)
  #define traceRETURN_xTimerPendFunctionCall(xReturn)                     SEGGER_SYSVIEW_RecordEndCallU32(apiID_XTIMERPENDFUNCTIONCALL, (U32)xReturn)
  #define traceRETURN_uxTimerGetTimerNumber(uxTimerNumber)                SEGGER_SYSVIEW_RecordEndCallU32(apiID_UXTIMERGETTIMERNUMBER, (U32)uxTimerNumber)
  #define traceRETURN_vTimerSetTimerNumber()                              SEGGER_SYSVIEW_RecordEndCall   (apiID_VTIMERSETTIMERNUMBER)

  #define traceRETURN_vListInitialise()                SEGGER_SYSVIEW_RecordEndCall(apiID_VLISTINITIALISE)
  #define traceRETURN_vListInitialiseItem()            SEGGER_SYSVIEW_RecordEndCall(apiID_VLISTINITIALISEITEM)
  #define traceRETURN_vListInsertEnd()                 SEGGER_SYSVIEW_RecordEndCall(apiID_VLISTINSERTEND)
  #define traceRETURN_vListInsert()                    SEGGER_SYSVIEW_RecordEndCall(apiID_VLISTINSERT)
  #define traceRETURN_uxListRemove(uxNumberOfItems)    SEGGER_SYSVIEW_RecordEndCallU32(apiID_UXLISTREMOVE, (U32)uxNumberOfItems)

  #define traceRETURN_xCoRoutineCreate(xReturn)                 SEGGER_SYSVIEW_RecordEndCallU32(apiID_XCOROUTINECREATE, (U32)xReturn)
  #define traceRETURN_vCoRoutineAddToDelayedList()              SEGGER_SYSVIEW_RecordEndCall   (apiID_VCOROUTINEADDTODELAYEDLIST)
  #define traceRETURN_vCoRoutineSchedule()                      SEGGER_SYSVIEW_RecordEndCall   (apiID_VCOROUTINESCHEDULE)
  #define traceRETURN_xCoRoutineRemoveFromEventList(xReturn)    SEGGER_SYSVIEW_RecordEndCallU32(apiID_XCOROUTINEREMOVEFROMEVENTLIST, (U32)xReturn)

  #define traceRETURN_xEventGroupCreateStatic(pxEventBits)    SEGGER_SYSVIEW_RecordEndCallU32(apiID_XEVENTGROUPCREATESTATIC, SEGGER_SYSVIEW_ShrinkId((U32)pxEventBits))
  #define traceRETURN_xEventGroupCreate(pxEventBits)          SEGGER_SYSVIEW_RecordEndCallU32(apiID_XEVENTGROUPCREATE, SEGGER_SYSVIEW_ShrinkId((U32)pxEventBits))
  #define traceRETURN_xEventGroupSync(uxReturn)               SEGGER_SYSVIEW_RecordEndCallU32(apiID_XEVENTGROUPSYNC, (U32)uxReturn)
  #define traceRETURN_xEventGroupWaitBits(uxReturn)           SEGGER_SYSVIEW_RecordEndCallU32(apiID_XEVENTGROUPWAITBITS, (U32)uxReturn)
  #define traceRETURN_xEventGroupClearBits(uxReturn)          SEGGER_SYSVIEW_RecordEndCallU32(apiID_XEVENTGROUPCLEARBITS, (U32)uxReturn)
  #define traceRETURN_xEventGroupClearBitsFromISR(xReturn)    SEGGER_SYSVIEW_RecordEndCallU32(apiID_XEVENTGROUPCLEARBITSFROMISR, (U32)xReturn)
  #define traceRETURN_xEventGroupGetBitsFromISR(uxReturn)     SEGGER_SYSVIEW_RecordEndCallU32(apiID_XEVENTGROUPGETBITSFROMISR, (U32)uxReturn)
  #define traceRETURN_xEventGroupSetBits(uxEventBits)         SEGGER_SYSVIEW_RecordEndCallU32(apiID_XEVENTGROUPSETBITS, (U32)uxEventBits)
  #define traceRETURN_vEventGroupDelete()                     SEGGER_SYSVIEW_RecordEndCall   (apiID_VEVENTGROUPDELETE)
  #define traceRETURN_vEventGroupSetBitsCallback()            SEGGER_SYSVIEW_RecordEndCall   (apiID_VEVENTGROUPSETBITSCALLBACK)
  #define traceRETURN_vEventGroupClearBitsCallback()          SEGGER_SYSVIEW_RecordEndCall   (apiID_VEVENTGROUPCLEARBITSCALLBACK)
  #define traceRETURN_xEventGroupSetBitsFromISR(xReturn)      SEGGER_SYSVIEW_RecordEndCallU32(apiID_XEVENTGROUPSETBITSFROMISR, (U32)xReturn)
  #define traceRETURN_uxEventGroupGetNumber(xReturn)          SEGGER_SYSVIEW_RecordEndCallU32(apiID_UXEVENTGROUPGETNUMBER, (U32)xReturn)
  #define traceRETURN_vEventGroupSetNumber()                  SEGGER_SYSVIEW_RecordEndCall   (apiID_VEVENTGROUPSETNUMBER)

  #define traceRETURN_xStreamBufferGenericCreate(pucAllocatedMemory)               SEGGER_SYSVIEW_RecordEndCallU32(apiID_XSTREAMBUFFERGENERICCREATE, SEGGER_SYSVIEW_ShrinkId((U32)pucAllocatedMemory))
  #define traceRETURN_xStreamBufferGenericCreateStatic(xReturn)                    SEGGER_SYSVIEW_RecordEndCallU32(apiID_XSTREAMBUFFERGENERICCREATESTATIC, SEGGER_SYSVIEW_ShrinkId((U32)xReturn))
  #define traceRETURN_vStreamBufferDelete()                                        SEGGER_SYSVIEW_RecordEndCall   (apiID_VSTREAMBUFFERDELETE)
  #define traceRETURN_xStreamBufferReset(xReturn)                                  SEGGER_SYSVIEW_RecordEndCallU32(apiID_XSTREAMBUFFERRESET, (U32)xReturn)
  #define traceRETURN_xStreamBufferSetTriggerLevel(xReturn)                        SEGGER_SYSVIEW_RecordEndCallU32(apiID_XSTREAMBUFFERSETTRIGGERLEVEL, (U32)xReturn)
  #define traceRETURN_xStreamBufferSpacesAvailable(xSpace)                         SEGGER_SYSVIEW_RecordEndCallU32(apiID_XSTREAMBUFFERSPACESAVAILABLE, (U32)xSpace)
  #define traceRETURN_xStreamBufferBytesAvailable(xReturn)                         SEGGER_SYSVIEW_RecordEndCallU32(apiID_XSTREAMBUFFERBYTESAVAILABLE, (U32)xReturn)
  #define traceRETURN_xStreamBufferSend(xReturn)                                   SEGGER_SYSVIEW_RecordEndCallU32(apiID_XSTREAMBUFFERSEND, (U32)xReturn)
  #define traceRETURN_xStreamBufferSendFromISR(xReturn)                            SEGGER_SYSVIEW_RecordEndCallU32(apiID_XSTREAMBUFFERSENDFROMISR, (U32)xReturn)
  #define traceRETURN_xStreamBufferReceive(xReceivedLength)                        SEGGER_SYSVIEW_RecordEndCallU32(apiID_XSTREAMBUFFERRECEIVE, (U32)xReceivedLength)
  #define traceRETURN_xStreamBufferNextMessageLengthBytes(xReturn)                 SEGGER_SYSVIEW_RecordEndCallU32(apiID_XSTREAMBUFFERNEXTMESSAGELENGTHBYTES, (U32)xReturn)
  #define traceRETURN_xStreamBufferReceiveFromISR(xReceivedLength)                 SEGGER_SYSVIEW_RecordEndCallU32(apiID_XSTREAMBUFFERRECEIVEFROMISR, (U32)xReceivedLength)
  #define traceRETURN_xStreamBufferIsEmpty(xReturn)                                SEGGER_SYSVIEW_RecordEndCallU32(apiID_XSTREAMBUFFERISEMPTY, (U32)xReturn)
  #define traceRETURN_xStreamBufferIsFull(xReturn)                                 SEGGER_SYSVIEW_RecordEndCallU32(apiID_XSTREAMBUFFERISFULL, (U32)xReturn)
  #define traceRETURN_xStreamBufferSendCompletedFromISR(xReturn)                   SEGGER_SYSVIEW_RecordEndCallU32(apiID_XSTREAMBUFFERSENDCOMPLETEDFROMISR, (U32)xReturn)
  #define traceRETURN_xStreamBufferReceiveCompletedFromISR(xReturn)                SEGGER_SYSVIEW_RecordEndCallU32(apiID_XSTREAMBUFFERRECEIVECOMPLETEDFROMISR, (U32)xReturn)
  #define traceRETURN_uxStreamBufferGetStreamBufferNumber(uxStreamBufferNumber)    SEGGER_SYSVIEW_RecordEndCallU32(apiID_UXSTREAMBUFFERGETSTREAMBUFFERNUMBER, (U32)uxStreamBufferNumber)
  #define traceRETURN_vStreamBufferSetStreamBufferNumber()                         SEGGER_SYSVIEW_RecordEndCall   (apiID_VSTREAMBUFFERSETSTREAMBUFFERNUMBER)
  #define traceRETURN_ucStreamBufferGetStreamBufferType(ucFlags)                   SEGGER_SYSVIEW_RecordEndCallU32(apiID_UCSTREAMBUFFERGETSTREAMBUFFERTYPE, (U32)ucFlags)
#endif

#if ( USE_LEGACY_TRACE_API != 0 )
  #define traceTASK_NOTIFY_TAKE(uxIndexToWait)                                    SEGGER_SYSVIEW_RecordU32x2(apiID_OFFSET + apiID_ULTASKNOTIFYTAKE, xClearCountOnExit, xTicksToWait)
  #define traceTASK_DELAY()                                                       SEGGER_SYSVIEW_RecordU32  (apiID_OFFSET + apiID_VTASKDELAY, xTicksToDelay)
  #define traceTASK_DELAY_UNTIL(xTimeToWake)                                      SEGGER_SYSVIEW_RecordVoid (apiID_OFFSET + apiID_VTASKDELAYUNTIL)
  #define traceTASK_NOTIFY_GIVE_FROM_ISR(uxIndexToNotify)                         SEGGER_SYSVIEW_RecordU32x2(apiID_OFFSET + apiID_VTASKNOTIFYGIVEFROMISR, SEGGER_SYSVIEW_ShrinkId((U32)pxTCB), (U32)pxHigherPriorityTaskWoken)

  #define traceTASK_PRIORITY_INHERIT( pxTCB, uxPriority )                         SEGGER_SYSVIEW_RecordU32  (apiID_OFFSET + apiID_VTASKPRIORITYINHERIT, (U32)pxMutexHolder)
  #define traceTASK_RESUME( pxTCB )                                               SEGGER_SYSVIEW_RecordU32  (apiID_OFFSET + apiID_VTASKRESUME, SEGGER_SYSVIEW_ShrinkId((U32)pxTCB))
  #define traceINCREASE_TICK_COUNT( xTicksToJump )                                SEGGER_SYSVIEW_RecordU32  (apiID_OFFSET + apiID_VTASKSTEPTICK, xTicksToJump)
  #define traceTASK_SUSPEND( pxTCB )                                              SEGGER_SYSVIEW_RecordU32  (apiID_OFFSET + apiID_VTASKSUSPEND, SEGGER_SYSVIEW_ShrinkId((U32)pxTCB))
  #define traceTASK_PRIORITY_DISINHERIT( pxTCB, uxBasePriority )                  SEGGER_SYSVIEW_RecordU32  (apiID_OFFSET + apiID_XTASKPRIORITYDISINHERIT, (U32)pxMutexHolder)
  #define traceTASK_RESUME_FROM_ISR( pxTCB )                                      SEGGER_SYSVIEW_RecordU32  (apiID_OFFSET + apiID_XTASKRESUMEFROMISR, SEGGER_SYSVIEW_ShrinkId((U32)pxTCB))
  #define traceTASK_NOTIFY(uxIndexToNotify)                                       SEGGER_SYSVIEW_RecordU32x4(apiID_OFFSET + apiID_XTASKGENERICNOTIFY, SEGGER_SYSVIEW_ShrinkId((U32)pxTCB), ulValue, eAction, (U32)pulPreviousNotificationValue)
  #define traceTASK_NOTIFY_FROM_ISR(uxIndexToWait)                                SEGGER_SYSVIEW_RecordU32x5(apiID_OFFSET + apiID_XTASKGENERICNOTIFYFROMISR, SEGGER_SYSVIEW_ShrinkId((U32)pxTCB), ulValue, eAction, (U32)pulPreviousNotificationValue, (U32)pxHigherPriorityTaskWoken)
  #define traceTASK_NOTIFY_WAIT(uxIndexToWait)                                    SEGGER_SYSVIEW_RecordU32x4(apiID_OFFSET + apiID_XTASKNOTIFYWAIT, ulBitsToClearOnEntry, ulBitsToClearOnExit, (U32)pulNotificationValue, xTicksToWait)

  #define traceQUEUE_CREATE( pxNewQueue )                                         SEGGER_SYSVIEW_RecordU32x3(apiID_OFFSET + apiID_XQUEUEGENERICCREATE, uxQueueLength, uxItemSize, ucQueueType)
  #define traceQUEUE_DELETE( pxQueue )                                            SEGGER_SYSVIEW_RecordU32  (apiID_OFFSET + apiID_VQUEUEDELETE, SEGGER_SYSVIEW_ShrinkId((U32)pxQueue))
  #define traceQUEUE_PEEK( pxQueue )                                              SEGGER_SYSVIEW_RecordU32x4(apiID_OFFSET + apiID_XQUEUEGENERICRECEIVE, SEGGER_SYSVIEW_ShrinkId((U32)pxQueue), SEGGER_SYSVIEW_ShrinkId((U32)pvBuffer), xTicksToWait, 1)
  #define traceQUEUE_PEEK_FROM_ISR( pxQueue )                                     SEGGER_SYSVIEW_RecordU32x2(apiID_OFFSET + apiID_XQUEUEPEEKFROMISR, SEGGER_SYSVIEW_ShrinkId((U32)pxQueue), SEGGER_SYSVIEW_ShrinkId((U32)pvBuffer))
  #define traceQUEUE_PEEK_FROM_ISR_FAILED( pxQueue )                              SEGGER_SYSVIEW_RecordU32x2(apiID_OFFSET + apiID_XQUEUEPEEKFROMISR, SEGGER_SYSVIEW_ShrinkId((U32)pxQueue), SEGGER_SYSVIEW_ShrinkId((U32)pvBuffer))
  #define traceQUEUE_RECEIVE( pxQueue )                                           SEGGER_SYSVIEW_RecordU32x4(apiID_OFFSET + apiID_XQUEUEGENERICRECEIVE, SEGGER_SYSVIEW_ShrinkId((U32)pxQueue), SEGGER_SYSVIEW_ShrinkId((U32)0), xTicksToWait, 1)
  #define traceQUEUE_RECEIVE_FAILED( pxQueue )                                    SEGGER_SYSVIEW_RecordU32x4(apiID_OFFSET + apiID_XQUEUEGENERICRECEIVE, SEGGER_SYSVIEW_ShrinkId((U32)pxQueue), SEGGER_SYSVIEW_ShrinkId((U32)0), xTicksToWait, 1)
  #define traceQUEUE_SEMAPHORE_RECEIVE( pxQueue )                                 SEGGER_SYSVIEW_RecordU32x4(apiID_OFFSET + apiID_XQUEUEGENERICRECEIVE, SEGGER_SYSVIEW_ShrinkId((U32)pxQueue), SEGGER_SYSVIEW_ShrinkId((U32)0), xTicksToWait, 0)
  #define traceQUEUE_RECEIVE_FROM_ISR( pxQueue )                                  SEGGER_SYSVIEW_RecordU32x3(apiID_OFFSET + apiID_XQUEUERECEIVEFROMISR, SEGGER_SYSVIEW_ShrinkId((U32)pxQueue), SEGGER_SYSVIEW_ShrinkId((U32)pvBuffer), (U32)pxHigherPriorityTaskWoken)
  #define traceQUEUE_RECEIVE_FROM_ISR_FAILED( pxQueue )                           SEGGER_SYSVIEW_RecordU32x3(apiID_OFFSET + apiID_XQUEUERECEIVEFROMISR, SEGGER_SYSVIEW_ShrinkId((U32)pxQueue), SEGGER_SYSVIEW_ShrinkId((U32)pvBuffer), (U32)pxHigherPriorityTaskWoken)
  #define traceQUEUE_REGISTRY_ADD( xQueue, pcQueueName )                          SEGGER_SYSVIEW_RecordU32x2(apiID_OFFSET + apiID_VQUEUEADDTOREGISTRY, SEGGER_SYSVIEW_ShrinkId((U32)xQueue), (U32)pcQueueName)
  #if ( configUSE_QUEUE_SETS != 1 )
    #define traceQUEUE_SEND( pxQueue )                                            SEGGER_SYSVIEW_RecordU32x4(apiID_OFFSET + apiID_XQUEUEGENERICSEND, SEGGER_SYSVIEW_ShrinkId((U32)pxQueue), (U32)pvItemToQueue, xTicksToWait, xCopyPosition)
  #else
    #define traceQUEUE_SEND( pxQueue )                                            SEGGER_SYSVIEW_RecordU32(apiID_OFFSET + apiID_XQUEUESETSEND, SEGGER_SYSVIEW_ShrinkId((U32)pxQueue))
  #endif
  #define traceQUEUE_SEND_FAILED( pxQueue )                                       SEGGER_SYSVIEW_RecordU32x4(apiID_OFFSET + apiID_XQUEUEGENERICSEND, SEGGER_SYSVIEW_ShrinkId((U32)pxQueue), (U32)pvItemToQueue, xTicksToWait, xCopyPosition)
  #define traceQUEUE_SEND_FROM_ISR( pxQueue )                                     SEGGER_SYSVIEW_RecordU32x2(apiID_OFFSET + apiID_XQUEUEGENERICSENDFROMISR, SEGGER_SYSVIEW_ShrinkId((U32)pxQueue), (U32)pxHigherPriorityTaskWoken)
  #define traceQUEUE_SEND_FROM_ISR_FAILED( pxQueue )                              SEGGER_SYSVIEW_RecordU32x2(apiID_OFFSET + apiID_XQUEUEGENERICSENDFROMISR, SEGGER_SYSVIEW_ShrinkId((U32)pxQueue), (U32)pxHigherPriorityTaskWoken)
  #define traceSTREAM_BUFFER_CREATE( pxStreamBuffer, xIsMessageBuffer )           SEGGER_SYSVIEW_RecordU32x2(apiID_OFFSET + apiID_XSTREAMBUFFERCREATE, (U32)xIsMessageBuffer, (U32)pxStreamBuffer)
  #define traceSTREAM_BUFFER_CREATE_FAILED( xIsMessageBuffer )                    SEGGER_SYSVIEW_RecordU32x2(apiID_OFFSET + apiID_XSTREAMBUFFERCREATE, (U32)xIsMessageBuffer, 0u)
  #define traceSTREAM_BUFFER_DELETE( xStreamBuffer )                              SEGGER_SYSVIEW_RecordU32  (apiID_OFFSET + apiID_VSTREAMBUFFERDELETE, (U32)xStreamBuffer)
  #define traceSTREAM_BUFFER_RESET( xStreamBuffer )                               SEGGER_SYSVIEW_RecordU32  (apiID_OFFSET + apiID_XSTREAMBUFFERRESET, (U32)xStreamBuffer)
  #define traceSTREAM_BUFFER_SEND( xStreamBuffer, xBytesSent )                    SEGGER_SYSVIEW_RecordU32x2(apiID_OFFSET + apiID_XSTREAMBUFFERSEND, (U32)xStreamBuffer, (U32)xBytesSent)
  #define traceSTREAM_BUFFER_SEND_FAILED( xStreamBuffer )                         SEGGER_SYSVIEW_RecordU32x2(apiID_OFFSET + apiID_XSTREAMBUFFERSEND, (U32)xStreamBuffer, 0u)
  #define traceSTREAM_BUFFER_SEND_FROM_ISR( xStreamBuffer, xBytesSent )           SEGGER_SYSVIEW_RecordU32x2(apiID_OFFSET + apiID_XSTREAMBUFFERSENDFROMISR, (U32)xStreamBuffer, (U32)xBytesSent)
  #define traceSTREAM_BUFFER_RECEIVE( xStreamBuffer, xReceivedLength )            SEGGER_SYSVIEW_RecordU32x2(apiID_OFFSET + apiID_XSTREAMBUFFERRECEIVE, (U32)xStreamBuffer, (U32)xReceivedLength)
  #define traceSTREAM_BUFFER_RECEIVE_FAILED( xStreamBuffer )                      SEGGER_SYSVIEW_RecordU32x2(apiID_OFFSET + apiID_XSTREAMBUFFERRECEIVE, (U32)xStreamBuffer, 0u)
  #define traceSTREAM_BUFFER_RECEIVE_FROM_ISR( xStreamBuffer, xReceivedLength )   SEGGER_SYSVIEW_RecordU32x2(apiID_OFFSET + apiID_XSTREAMBUFFERRECEIVEFROMISR, (U32)xStreamBuffer, (U32)xReceivedLength)
#endif

  #define traceTASK_DELETE( pxTCB )                   {                                                                                                   \
                                                        SEGGER_SYSVIEW_RecordU32(apiID_VTASKDELETE, SEGGER_SYSVIEW_ShrinkId((U32)pxTCB));  \
                                                        SYSVIEW_DeleteTask((U32)pxTCB);                                                                   \
                                                      }


  #if( portSTACK_GROWTH < 0 )
  #define traceTASK_CREATE(pxNewTCB)                  if (pxNewTCB != NULL) {                                             \
                                                        SEGGER_SYSVIEW_OnTaskCreate((U32)pxNewTCB);                       \
                                                        SYSVIEW_AddTask((U32)pxNewTCB,                                    \
                                                                        &(pxNewTCB->pcTaskName[0]),                       \
                                                                        pxNewTCB->uxPriority,                             \
                                                                        (U32)pxNewTCB->pxStack,                           \
                                                                        ((U32)pxNewTCB->pxTopOfStack - (U32)pxNewTCB->pxStack) \
                                                                        );                                                \
                                                      }
  #else
  #define traceTASK_CREATE(pxNewTCB)                  if (pxNewTCB != NULL) {                                             \
                                                        SEGGER_SYSVIEW_OnTaskCreate((U32)pxNewTCB);                       \
                                                        SYSVIEW_AddTask((U32)pxNewTCB,                                    \
                                                                        &(pxNewTCB->pcTaskName[0]),                       \
                                                                        pxNewTCB->uxPriority,                             \
                                                                        (U32)pxNewTCB->pxStack,                           \
                                                                        (U32)(pxNewTCB->pxStack-pxNewTCB->pxTopOfStack)   \
                                                                        );                                                \
                                                      }
  #endif
  #define traceTASK_PRIORITY_SET(pxTask, uxNewPriority) {                                                                 \
                                                          SEGGER_SYSVIEW_RecordU32x2(apiID_VTASKPRIORITYSET, \
                                                                                     SEGGER_SYSVIEW_ShrinkId((U32)pxTCB), \
                                                                                     uxNewPriority                        \
                                                                                    );                                    \
                                                          SYSVIEW_UpdateTask((U32)pxTask,                                 \
                                                                             &(pxTask->pcTaskName[0]),                    \
                                                                             uxNewPriority,                               \
                                                                             (U32)pxTask->pxStack,                        \
                                                                             0                                            \
                                                                            );                                            \
                                                        }


//
// Define INCLUDE_xTaskGetIdleTaskHandle as 1 in FreeRTOSConfig.h to allow identification of Idle state.
//
#if ( INCLUDE_xTaskGetIdleTaskHandle == 1 )
  #define traceTASK_SWITCHED_IN()                   if(prvGetTCBFromHandle(NULL) == xIdleTaskHandles[0]) {              \
                                                      SEGGER_SYSVIEW_OnIdle();                                          \
                                                    } else {                                                            \
                                                      SEGGER_SYSVIEW_OnTaskStartExec((U32)pxCurrentTCB);                \
                                                    }
#else
  #define traceTASK_SWITCHED_IN()                   {                                                                   \
                                                      if (memcmp(pxCurrentTCB->pcTaskName, "IDLE", 5) != 0) {           \
                                                        SEGGER_SYSVIEW_OnTaskStartExec((U32)pxCurrentTCB);              \
                                                      } else {                                                          \
                                                        SEGGER_SYSVIEW_OnIdle();                                        \
                                                      }                                                                 \
                                                    }
#endif

#define traceMOVED_TASK_TO_READY_STATE(pxTCB)       SEGGER_SYSVIEW_OnTaskStartReady((U32)pxTCB)
#define traceREADDED_TASK_TO_READY_STATE(pxTCB)

#define traceMOVED_TASK_TO_DELAYED_LIST()           SEGGER_SYSVIEW_OnTaskStopReady((U32)pxCurrentTCB,  (1u << 2))
#define traceMOVED_TASK_TO_OVERFLOW_DELAYED_LIST()  SEGGER_SYSVIEW_OnTaskStopReady((U32)pxCurrentTCB,  (1u << 2))
#define traceMOVED_TASK_TO_SUSPENDED_LIST(pxTCB)    SEGGER_SYSVIEW_OnTaskStopReady((U32)pxTCB,         ((3u << 3) | 3))


#define traceISR_EXIT_TO_SCHEDULER()                SEGGER_SYSVIEW_RecordExitISRToScheduler()
#define traceISR_EXIT()                             SEGGER_SYSVIEW_RecordExitISR()
#define traceISR_ENTER()                            SEGGER_SYSVIEW_RecordEnterISR()

/*********************************************************************
*
*       API functions
*
**********************************************************************
*/
#ifdef __cplusplus
extern "C" {
#endif
void SYSVIEW_AddTask      (U32 xHandle, const char* pcTaskName, unsigned uxCurrentPriority, U32  pxStack, unsigned uStackHighWaterMark);
void SYSVIEW_UpdateTask   (U32 xHandle, const char* pcTaskName, unsigned uxCurrentPriority, U32 pxStack, unsigned uStackHighWaterMark);
void SYSVIEW_DeleteTask   (U32 xHandle);
void SYSVIEW_SendTaskInfo (U32 TaskID, const char* sName, unsigned Prio, U32 StackBase, unsigned StackSize);

#ifdef __cplusplus
}
#endif

#endif

/*************************** End of file ****************************/
