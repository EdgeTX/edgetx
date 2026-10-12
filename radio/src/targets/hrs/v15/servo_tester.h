/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * V15 external-port multi-function servo tester.
 * J10: pin1=GND, pin2=+5V (PD7), pin3=PWM (UART8_TX / PJ8).
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#define SERVO_TESTER_MODE_MANUAL 0
#define SERVO_TESTER_MODE_AUTO 1
#define SERVO_TESTER_MODE_STICK 2

/** Absolute hardware pulse limits (Std / Wide / Digi / Heli760). */
#define SERVO_TESTER_PULSE_ABS_MIN_US 400
#define SERVO_TESTER_PULSE_ABS_MAX_US 2500
#define SERVO_TESTER_PULSE_DEFAULT_US 1500

/** Default type = Standard hobby servo. */
#define SERVO_TESTER_PULSE_MIN_US 1000
#define SERVO_TESTER_PULSE_MAX_LIMIT_US 2000

/** Frame period limits (µs). Pulse must stay below period. */
#define SERVO_TESTER_PERIOD_MIN_US 1600   // allows ~560 Hz (Heli 760)
#define SERVO_TESTER_PERIOD_MAX_US 30000  // ~33 Hz
#define SERVO_TESTER_PERIOD_US 20000      // 50 Hz default

/** Trip if current rises by this much (mA) vs previous sample or baseline. */
#define SERVO_TESTER_OC_DELTA_MA 1000

/** Built-in signal profiles for common servo classes. */
enum {
  SERVO_TESTER_TYPE_STD = 0,     // 1000–2000 µs
  SERVO_TESTER_TYPE_WIDE = 1,    // 500–2500 µs (3D / wide travel)
  SERVO_TESTER_TYPE_DIGI = 2,    // 900–2100 µs (typical digital)
  SERVO_TESTER_TYPE_HELI760 = 3, // 450–1050 µs @ 760µs center (KST etc.)
  SERVO_TESTER_TYPE_COUNT
};

enum {
  SERVO_TESTER_RATE_50 = 0,    // 20000 µs
  SERVO_TESTER_RATE_100 = 1,   // 10000 µs
  SERVO_TESTER_RATE_200 = 2,   // 5000 µs
  SERVO_TESTER_RATE_560 = 3,   // ~1786 µs (Heli 760µs protocol)
  SERVO_TESTER_RATE_COUNT
};

#if defined(__cplusplus)
extern "C" {
#endif

bool v15ServoTesterIsActive(void);
bool v15ServoTesterIsPowerOn(void);
bool v15ServoTesterOvercurrentFault(void);
int16_t v15ServoTesterGetCurrentMa(void);

void v15ServoTesterStart(void);
void v15ServoTesterStop(void);

void v15ServoTesterSetPulseUs(uint16_t pulseUs);
uint16_t v15ServoTesterGetPulseUs(void);

void v15ServoTesterSetPulseRange(uint16_t minUs, uint16_t maxUs);
uint16_t v15ServoTesterGetPulseMinUs(void);
uint16_t v15ServoTesterGetPulseMaxUs(void);

void v15ServoTesterSetPeriodUs(uint16_t periodUs);
uint16_t v15ServoTesterGetPeriodUs(void);
uint16_t v15ServoTesterGetFrameHz(void);

void v15ServoTesterSetType(uint8_t type);
uint8_t v15ServoTesterGetType(void);

void v15ServoTesterSetRate(uint8_t rate);
uint8_t v15ServoTesterGetRate(void);

void v15ServoTesterSetMode(uint8_t mode);
uint8_t v15ServoTesterGetMode(void);

void v15ServoTesterSetStick(uint8_t stick);
uint8_t v15ServoTesterGetStick(void);

void v15ServoTesterSetAutoSpeed(uint16_t usPerFrame);
uint16_t v15ServoTesterGetAutoSpeed(void);

/** Call from UI / main loop (~10–50 Hz) — OC monitor only; stick runs in mixer. */
void v15ServoTesterTask(void);

/**
 * Call once per mixer cycle after channelOutputs / sticks are fresh.
 * Stick-mode pulse must not depend on LVGL refresh rate.
 */
void v15ServoTesterMixerHook(void);

#if defined(__cplusplus)
}

void v15ServoTesterOpen(void);
#endif
