/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * V15 EXT Auto Measure — link → pulse probe → classify → lock child measure.
 * On signal loss from handoff child UI, return to link / re-probe.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

enum {
  AUTO_MEASURE_STATE_LINK = 0,
  AUTO_MEASURE_STATE_PROBE,
  AUTO_MEASURE_STATE_TRY,
  AUTO_MEASURE_STATE_LOCKED,
};

enum {
  AUTO_MEASURE_PROTO_NONE = 0,
  AUTO_MEASURE_PROTO_PWM,
  AUTO_MEASURE_PROTO_PPM,
  AUTO_MEASURE_PROTO_SBUS,
  AUTO_MEASURE_PROTO_CRSF,
  AUTO_MEASURE_PROTO_SUMD,
  AUTO_MEASURE_PROTO_DJI_RS,
  AUTO_MEASURE_PROTO_MAVLINK,
};

#if defined(__cplusplus)
extern "C" {
#endif

/** True while Auto Measure session owns the workflow. */
bool v15AutoMeasureIsActive(void);

/**
 * True only while Auto Measure is calling a child Start().
 * Child modules skip the "auto active" mutex check when this is set.
 */
bool v15AutoMeasureIsStartingChild(void);

/** True after Auto hands UI to a locked child measure dialog. */
bool v15AutoMeasureIsHandoff(void);

/** True while Auto Measure UI is driving the state machine Task. */
bool v15AutoMeasureIsUiAttached(void);

/**
 * While handoff: track sustained signal loss.
 * Returns true when child UI should stop and call ResumeSearch + Open.
 */
bool v15AutoMeasureWatchHandoffLoss(bool hasSignal);

uint8_t v15AutoMeasureGetState(void);
uint8_t v15AutoMeasureGetProtocol(void);
uint8_t v15AutoMeasureGetTryProtocol(void);

/** Human-readable status line for UI (static buffer). */
const char* v15AutoMeasureGetStatusText(void);

bool v15AutoMeasureOvercurrentFault(void);
bool v15AutoMeasureIsPowerOn(void);
int16_t v15AutoMeasureGetCurrentMa(void);

void v15AutoMeasureStart(void);
void v15AutoMeasureStop(void);
void v15AutoMeasureTask(void);

/**
 * Keep session + running child measure, stop Auto UI from driving Task.
 * Marks handoff so child UI can return to search on signal loss.
 */
void v15AutoMeasureBeginHandoff(void);

/** Re-attach Auto UI driver after handoff (session must still be active). */
void v15AutoMeasureReattachUi(void);

/**
 * Child measure lost signal: stop expecting handoff, reclaim EXT, back to LINK.
 * Caller should delete its dialog then call v15AutoMeasureOpen().
 */
void v15AutoMeasureResumeSearch(void);

#if defined(__cplusplus)
}

void v15AutoMeasureOpen(void);
#endif
