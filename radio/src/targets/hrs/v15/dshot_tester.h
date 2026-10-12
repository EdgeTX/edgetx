/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * V15 external-port DShot ESC tester.
 * J10: pin1=GND, pin2=+5V (PD7), pin3=DShot (UART8_TX / PJ8).
 *
 * Motor battery power must be supplied separately on the ESC.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#define DSHOT_TESTER_MODE_MANUAL 0
#define DSHOT_TESTER_MODE_AUTO 1
#define DSHOT_TESTER_MODE_STICK 2

enum {
  DSHOT_TESTER_PROTO_150 = 0,
  DSHOT_TESTER_PROTO_300 = 1,
  DSHOT_TESTER_PROTO_600 = 2,
  DSHOT_TESTER_PROTO_1200 = 3,
  DSHOT_TESTER_PROTO_COUNT
};

/** DShot special commands (1-47). 0 = motor stop. */
enum {
  DSHOT_CMD_MOTOR_STOP = 0,
  DSHOT_CMD_BEEP1 = 1,
  DSHOT_CMD_BEEP2 = 2,
  DSHOT_CMD_BEEP3 = 3,
  DSHOT_CMD_BEEP4 = 4,
  DSHOT_CMD_BEEP5 = 5,
};

#define DSHOT_TESTER_THROTTLE_MIN 0     // percent
#define DSHOT_TESTER_THROTTLE_MAX 100
#define DSHOT_TESTER_VALUE_STOP 0
#define DSHOT_TESTER_VALUE_THROTTLE_MIN 48
#define DSHOT_TESTER_VALUE_THROTTLE_MAX 2047

#define DSHOT_TESTER_OC_DELTA_MA 1000
/** Continuous DShot frame rate on the wire. */
#define DSHOT_TESTER_FRAME_HZ 1000

#if defined(__cplusplus)
extern "C" {
#endif

bool v15DshotTesterIsActive(void);
bool v15DshotTesterIsPowerOn(void);
bool v15DshotTesterOvercurrentFault(void);
int16_t v15DshotTesterGetCurrentMa(void);

void v15DshotTesterStart(void);
void v15DshotTesterStop(void);

void v15DshotTesterSetProtocol(uint8_t proto);
uint8_t v15DshotTesterGetProtocol(void);

void v15DshotTesterSetMode(uint8_t mode);
uint8_t v15DshotTesterGetMode(void);

/** Arm gate: when false, only DShot stop (0) is transmitted. */
void v15DshotTesterSetArmed(bool armed);
bool v15DshotTesterIsArmed(void);

/** Throttle request 0-100%. Output only when armed. */
void v15DshotTesterSetThrottlePercent(uint8_t pct);
uint8_t v15DshotTesterGetThrottlePercent(void);

/** Last 11-bit DShot value actually sent (0 / cmd / 48-2047). */
uint16_t v15DshotTesterGetOutputValue(void);

/** Queue a special command for a short burst, then resume throttle/stop. */
void v15DshotTesterSendCommand(uint16_t cmd);

void v15DshotTesterSetAutoMaxPercent(uint8_t pct);
uint8_t v15DshotTesterGetAutoMaxPercent(void);

void v15DshotTesterTask(void);
void v15DshotTesterMixerHook(void);

#if defined(__cplusplus)
}

void v15DshotTesterOpen(void);
#endif
