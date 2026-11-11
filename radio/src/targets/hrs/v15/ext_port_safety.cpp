/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * Probe EXT jack signal-pin voltage before CHIP_FUN connects it to the MCU.
 * Hardware: BAT-S2 (HC138 Y1) is the signal-entry sense when CHIP_FUN is low.
 */

#include "ext_port_safety.h"

#if defined(RADIO_V15) && !defined(SIMU)

#include "board.h"
#include "delays_driver.h"
#include "hal/gpio.h"
#include "stm32_gpio.h"

#include <cstdio>

#if defined(MODULE_BATTERY_SENSOR)
#include "csd203_driver.h"
#endif

namespace {

/** HC138 Y1 = BAT-S2 = EXT signal-entry sense (not cell/system channels). */
constexpr uint8_t SIG_SENSE_MUX_CH = 1;
constexpr uint32_t MUX_SETTLE_US = 5000;

bool g_blocked = false;
uint16_t g_lastMv = 0;
char g_msg[72] = "EXT signal unsafe";

void driveFun(bool uartPath)
{
#if defined(CHIP_FUN_GPIO)
  gpio_init(CHIP_FUN_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  // CHIP_FUN: 0 = sense/battery (MCU disconnected), 1 = UART/PWM to MCU
  if (uartPath) gpio_set(CHIP_FUN_GPIO);
  else gpio_clear(CHIP_FUN_GPIO);
#endif
}

void drivePower(bool on)
{
  gpio_init(SERVO_TESTER_PWR_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  if (on) gpio_set(SERVO_TESTER_PWR_GPIO);
  else gpio_clear(SERVO_TESTER_PWR_GPIO);
}

void selectMux(uint8_t channel)
{
#if defined(CHIP_CS1_GPIO)
  gpio_init(CHIP_CS1_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_init(CHIP_CS2_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_init(CHIP_CS3_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_write(CHIP_CS1_GPIO, (channel & 0x01) ? 1 : 0);
  gpio_write(CHIP_CS2_GPIO, (channel & 0x02) ? 1 : 0);
  gpio_write(CHIP_CS3_GPIO, (channel & 0x04) ? 1 : 0);
#else
  (void)channel;
#endif
}

void setBlocked(uint16_t mv, bool senseFail)
{
  g_blocked = true;
  g_lastMv = mv;
  if (senseFail) {
    snprintf(g_msg, sizeof(g_msg), "EXT voltage sense failed");
  } else {
    snprintf(g_msg, sizeof(g_msg), "EXT signal unsafe (%u.%02uV)",
             (unsigned)(mv / 1000u), (unsigned)((mv % 1000u) / 10u));
  }
}

bool readSigSenseMv(uint16_t& mv)
{
  selectMux(SIG_SENSE_MUX_CH);
  delay_us(MUX_SETTLE_US);

  // Same cadence as batsenser: a current read helps the ADC refresh bus voltage.
  int16_t dummyMa = 0;
  (void)csd203DriverReadCurrent(dummyMa);
  delay_us(MUX_SETTLE_US);

  if (!csd203DriverReadVoltage(mv)) return false;

  // Discard first sample after mux change; take a second reading.
  delay_us(MUX_SETTLE_US);
  (void)csd203DriverReadCurrent(dummyMa);
  delay_us(MUX_SETTLE_US);
  return csd203DriverReadVoltage(mv);
}

}  // namespace

bool v15ExtPortProbeSignalSafe(uint16_t* voltageMv)
{
  g_blocked = false;
  g_lastMv = 0;

  driveFun(false);
  drivePower(false);
  delay_us(500);

#if !defined(MODULE_BATTERY_SENSOR)
  setBlocked(0, true);
  if (voltageMv) *voltageMv = 0;
  return false;
#else
  if (!csd203DriverEnsureReady()) {
    csd203DriverInit();
    if (!csd203DriverEnsureReady()) {
      setBlocked(0, true);
      if (voltageMv) *voltageMv = 0;
      return false;
    }
  }

  uint16_t mv = 0;
  if (!readSigSenseMv(mv)) {
    selectMux(7);
    driveFun(false);
    setBlocked(0, true);
    if (voltageMv) *voltageMv = 0;
    return false;
  }

  // Leave mux idle; FUN stays disconnected.
  selectMux(7);
  driveFun(false);

  if (voltageMv) *voltageMv = mv;
  g_lastMv = mv;

  if (mv > EXT_PORT_SAFE_VOLTAGE_MAX_MV) {
    setBlocked(mv, false);
    return false;
  }

  g_blocked = false;
  snprintf(g_msg, sizeof(g_msg), "EXT OK %u.%02uV",
           (unsigned)(mv / 1000u), (unsigned)((mv % 1000u) / 10u));
  return true;
#endif
}

bool v15ExtPortTryOpenSignalPath(void)
{
  uint16_t mv = 0;
  if (!v15ExtPortProbeSignalSafe(&mv)) {
    driveFun(false);
    return false;
  }

  // Safe — connect MCU signal path. Caller enables +5V if needed.
  driveFun(true);
#if defined(CHIP_CS1_GPIO)
  // Keep HC138 on Y7 so cell mux switches stay off while UART/PWM path is active.
  selectMux(7);
#endif
  g_blocked = false;
  return true;
}

void v15ExtPortEnsureSignalPathOpen(void)
{
  driveFun(true);
#if defined(CHIP_CS1_GPIO)
  selectMux(7);
#endif
}

void v15ExtPortCloseSignalPath(void)
{
  driveFun(false);
#if defined(CHIP_CS1_GPIO)
  selectMux(7);
#endif
}

bool v15ExtPortIsBlocked(void) { return g_blocked; }
uint16_t v15ExtPortLastVoltageMv(void) { return g_lastMv; }
const char* v15ExtPortBlockedMessage(void) { return g_msg; }

#else  // !RADIO_V15 || SIMU

bool v15ExtPortProbeSignalSafe(uint16_t* voltageMv)
{
  if (voltageMv) *voltageMv = 0;
  return true;
}
bool v15ExtPortTryOpenSignalPath(void) { return true; }
void v15ExtPortEnsureSignalPathOpen(void) {}
void v15ExtPortCloseSignalPath(void) {}
bool v15ExtPortIsBlocked(void) { return false; }
uint16_t v15ExtPortLastVoltageMv(void) { return 0; }
const char* v15ExtPortBlockedMessage(void) { return ""; }

#endif
