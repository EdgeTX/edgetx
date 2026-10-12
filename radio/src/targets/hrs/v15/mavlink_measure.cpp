/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * MAVLink measure on V15 J10 (PJ8 half-duplex UART8).
 * Auto-tries baud ÃÂ polarity until valid MAVLink frames lock.
 */

#include "mavlink_measure.h"
#include "ext_port_safety.h"
#include "logic_measure.h"

#if defined(RADIO_V15) && !defined(SIMU)

#include "board.h"
#include "edgetx.h"
#include "hal/gpio.h"
#undef UNUSED
#include "hal/serial_driver.h"
#include "pwm_measure.h"
#include "ppm_measure.h"
#include "sbus_measure.h"
#include "crsf_measure.h"
#include "sumd_measure.h"
#include "dji_rs_measure.h"
#include "serial.h"
#include "servo_tester.h"
#include "dshot_tester.h"
#include "auto_measure.h"
#include "stm32_gpio.h"
#include "stm32_serial_driver.h"
#include "timers_driver.h"

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(MODULE_BATTERY_SENSOR)
#include "batsenser.h"
#include "csd203_driver.h"
#endif

namespace {

constexpr tmr10ms_t OC_SETTLE_10MS = 20;
constexpr tmr10ms_t OC_CHECK_PERIOD_10MS = 2;

constexpr uint8_t MAV1_STX = 0xFE;
constexpr uint8_t MAV2_STX = 0xFD;
constexpr uint8_t MAV1_HDR_LEN = 6;
constexpr uint8_t MAV2_HDR_LEN = 10;
constexpr uint8_t MAV_CRC_LEN = 2;
// Must be >255: was uint8_t 280 Ã¢Â?truncated to 24 and broke parsing.
constexpr uint16_t MAV_PARSE_MAX = 320;
constexpr uint8_t SOFT_LOCK_FRAMES = 3;
// 460800 flood + snprintf/memmove in IDLE Ã¢Â?WDT "Ã§Â´Â§Ã¦ÂÂ¥Ã¦Â¨Â¡Ã¥Â¼?. Cap ISR work.
constexpr uint32_t MAV_MAX_BYTES_IDLE = 128;
constexpr uint32_t MAV_MAX_BYTES_TASK = 384;
constexpr uint8_t MAV_MAX_FRAMES_IDLE = 1;
constexpr uint8_t MAV_MAX_FRAMES_TASK = 4;

// ELRS MAVLink is fixed 460800; keep slower bauds for generic FC links.
constexpr uint32_t BAUD_CANDIDATES[] = {460800u, 115200u, 57600u};
constexpr uint8_t BAUD_COUNT = sizeof(BAUD_CANDIDATES) / sizeof(BAUD_CANDIDATES[0]);

// Half-duplex RX on J10 pin3 (UART8_TX / PJ8).
static const stm32_usart_t mavlinkMeasUsart = {
    .USARTx = AUX_SERIAL_USART,
    .txGPIO = AUX_SERIAL_TX_GPIO,
    .rxGPIO = GPIO_UNDEF,
    .IRQn = AUX_SERIAL_USART_IRQn,
    .IRQ_Prio = 7,
    .txDMA = nullptr,
    .txDMA_Stream = 0,
    .txDMA_Channel = 0,
    .rxDMA = AUX_SERIAL_DMA_RX,
    .rxDMA_Stream = AUX_SERIAL_DMA_RX_STREAM,
    .rxDMA_Channel = AUX_SERIAL_DMA_RX_CHANNEL,
};

DEFINE_STM32_SERIAL_PORT(MavlinkMeas, mavlinkMeasUsart, 1024, 8);

volatile bool g_active = false;
volatile bool g_haveSignal = false;
volatile tmr10ms_t g_lastFrame10ms = 0;
volatile uint32_t g_lastFrameUs = 0;
volatile uint32_t g_framePeriodUs = 0;

volatile uint8_t g_versionRaw = 0;
volatile uint8_t g_sysIdRaw = 0;
volatile uint8_t g_compIdRaw = 0;
volatile uint32_t g_msgIdRaw = 0;
volatile uint8_t g_payloadLenRaw = 0;
volatile uint8_t g_polarityUi = MAVLINK_MEASURE_POL_SEARCHING;
volatile uint32_t g_baudUi = 0;

uint8_t g_version = 0;
uint8_t g_sysId = 0;
uint8_t g_compId = 0;
uint32_t g_msgId = 0;
uint8_t g_polarity = MAVLINK_MEASURE_POL_SEARCHING;
uint32_t g_baudrate = 0;
uint16_t g_msgHz = 0;

void* g_uartCtx = nullptr;
volatile bool g_configLocked = false;
volatile uint8_t g_tryPolarity = ETX_Pol_Normal;
volatile uint8_t g_tryBaudIdx = 0;
tmr10ms_t g_configTryStart = 0;

uint8_t g_parseBuf[MAV_PARSE_MAX] = {};
uint16_t g_parseLen = 0;

uint8_t g_softFrameCount = 0;
bool g_crcLocked = false;

uint8_t g_hbType = 0;
uint8_t g_hbAutopilot = 0;
uint8_t g_hbBaseMode = 0;
uint8_t g_hbSystemStatus = 0;
uint32_t g_hbCustomMode = 0;
bool g_haveHb = false;

bool g_haveSys = false;
uint16_t g_sysVoltMv = 0;
int16_t g_sysCurrentCA = 0;  // 10 mA units
int8_t g_sysBattPct = -1;
uint16_t g_sysLoad = 0;

bool g_haveBatt = false;
uint16_t g_battVoltMv = 0;
int16_t g_battCurrentCA = 0;
int8_t g_battPct = -1;

bool g_haveAtt = false;
float g_attRoll = 0.f;
float g_attPitch = 0.f;
float g_attYaw = 0.f;

bool g_haveGps = false;
uint8_t g_gpsFix = 0;
uint8_t g_gpsSats = 0;
int32_t g_gpsLat = 0;
int32_t g_gpsLon = 0;
int32_t g_gpsAltMm = 0;

bool g_havePos = false;
int32_t g_posRelAltMm = 0;
uint16_t g_posHdgCdeg = 0;

bool g_haveHud = false;
float g_hudAirspeed = 0.f;
float g_hudGroundspeed = 0.f;
int16_t g_hudHeading = 0;
uint16_t g_hudThrottle = 0;
float g_hudAlt = 0.f;
float g_hudClimb = 0.f;

bool g_haveRc = false;
uint8_t g_rcCount = 0;
uint8_t g_rcRssi = 0;
uint16_t g_rcCh[4] = {};

bool g_haveRadio = false;
uint8_t g_radRssi = 0;
uint8_t g_radRemRssi = 0;
uint8_t g_radTxbuf = 0;
int8_t g_radNoise = 0;
int8_t g_radRemNoise = 0;

uint16_t g_seenMask = 0;  // bit0=HB ... bit8=BAT

char g_lineText[MAVLINK_MEASURE_MAX_LINES][MAVLINK_MEASURE_LINE_TEXT_LEN] = {};
uint8_t g_lineCount = 0;
volatile bool g_linesDirty = false;

bool g_powerOn = false;
bool g_overcurrentFault = false;
bool g_ocBaselineValid = false;
tmr10ms_t g_ocSettleUntil = 0;
tmr10ms_t g_lastOcCheckTick = 0;
int16_t g_ocBaselineMa = 0;
int16_t g_lastCurrentMa = 0;
int16_t g_currentMa = 0;

int g_savedAuxMode = UART_MODE_NONE;
bool g_auxTakenOver = false;

uint16_t crcAccumulate(uint8_t data, uint16_t crcAccum)
{
  uint8_t tmp = (uint8_t)(data ^ (uint8_t)(crcAccum & 0xff));
  tmp ^= (uint8_t)(tmp << 4);
  crcAccum = (uint16_t)((crcAccum >> 8) ^ ((uint16_t)tmp << 8) ^ ((uint16_t)tmp << 3) ^
                        ((uint16_t)tmp >> 4));
  return crcAccum;
}

uint16_t crcX25(const uint8_t* buf, uint32_t len, uint16_t start = 0xFFFF)
{
  uint16_t crc = start;
  for (uint32_t i = 0; i < len; ++i) {
    crc = crcAccumulate(buf[i], crc);
  }
  return crc;
}

uint8_t crcExtraForMsgId(uint32_t msgId)
{
  switch (msgId) {
    case 0: return 50;     // HEARTBEAT
    case 1: return 124;    // SYS_STATUS
    case 24: return 24;    // GPS_RAW_INT
    case 30: return 39;    // ATTITUDE
    case 33: return 104;   // GLOBAL_POSITION_INT
    case 65: return 118;   // RC_CHANNELS
    case 74: return 20;    // VFR_HUD
    case 109: return 185;  // RADIO_STATUS
    case 147: return 154;  // BATTERY_STATUS
    default: return 0;
  }
}

uint16_t rdU16(const uint8_t* p)
{
  return (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
}

int16_t rdI16(const uint8_t* p) { return (int16_t)rdU16(p); }

uint32_t rdU32(const uint8_t* p)
{
  return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) |
         ((uint32_t)p[3] << 24);
}

int32_t rdI32(const uint8_t* p) { return (int32_t)rdU32(p); }

float rdF32(const uint8_t* p)
{
  float f = 0.f;
  memcpy(&f, p, sizeof(f));
  return f;
}

const char* mavStateName(uint8_t s)
{
  switch (s) {
    case 0: return "UNINIT";
    case 1: return "BOOT";
    case 2: return "CAL";
    case 3: return "STANDBY";
    case 4: return "ACTIVE";
    case 5: return "CRITICAL";
    case 6: return "EMERGENCY";
    case 7: return "POWEROFF";
    case 8: return "FLTTERM";
    default: return "?";
  }
}

const char* gpsFixName(uint8_t fix)
{
  switch (fix) {
    case 0: return "noGPS";
    case 1: return "noFix";
    case 2: return "2D";
    case 3: return "3D";
    case 4: return "DGPS";
    case 5: return "RTK-F";
    case 6: return "RTK-F";
    default: return "fix";
  }
}

const char* msgShortName(uint32_t id)
{
  switch (id) {
    case 0: return "HB";
    case 1: return "SYS";
    case 24: return "GPS";
    case 30: return "ATT";
    case 33: return "POS";
    case 65: return "RC";
    case 74: return "HUD";
    case 109: return "RAD";
    case 147: return "BAT";
    default: return nullptr;
  }
}

bool isMavlinkStx(uint8_t b) { return b == MAV1_STX || b == MAV2_STX; }

void setExtSignalPath(bool connect)
{
#if defined(CHIP_FUN_GPIO)
  gpio_init(CHIP_FUN_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  if (connect) {
    gpio_set(CHIP_FUN_GPIO);
  } else {
    gpio_clear(CHIP_FUN_GPIO);
  }
#endif
#if defined(CHIP_CS1_GPIO)
  if (connect) {
    gpio_write(CHIP_CS1_GPIO, 1);
    gpio_write(CHIP_CS2_GPIO, 1);
    gpio_write(CHIP_CS3_GPIO, 1);
  }
#endif
}

void setPortPower(bool on)
{
  gpio_init(SERVO_TESTER_PWR_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  if (on) {
    gpio_set(SERVO_TESTER_PWR_GPIO);
  } else {
    gpio_clear(SERVO_TESTER_PWR_GPIO);
  }
}

void takeOverAuxPort()
{
  if (g_auxTakenOver) return;
  g_savedAuxMode = serialGetMode(SP_AUX1);
  if (g_savedAuxMode != UART_MODE_NONE) {
    serialInit(SP_AUX1, UART_MODE_NONE);
  }
  g_auxTakenOver = true;
}

void restoreAuxPort()
{
  if (!g_auxTakenOver) return;
  g_auxTakenOver = false;
  serialInit(SP_AUX1, g_savedAuxMode);
  setPortPower(false);
}

void tripOvercurrent()
{
  if (g_overcurrentFault) return;

  g_overcurrentFault = true;
  g_powerOn = false;
  g_haveSignal = false;
  g_configLocked = false;
  g_polarityUi = MAVLINK_MEASURE_POL_SEARCHING;
  g_baudUi = 0;

  if (g_uartCtx) {
    STM32SerialDriver.deinit(g_uartCtx);
    g_uartCtx = nullptr;
  }

  gpio_init(SERVO_TESTER_PWM_GPIO, GPIO_IN_PD, GPIO_PIN_SPEED_LOW);
  setExtSignalPath(false);
  setPortPower(false);
}

void checkOvercurrent()
{
  if (!g_powerOn || g_overcurrentFault) return;

#if !defined(MODULE_BATTERY_SENSOR) || defined(SIMU)
  return;
#else
  const tmr10ms_t now = get_tmr10ms();
  if ((tmr10ms_t)(now - g_lastOcCheckTick) < OC_CHECK_PERIOD_10MS) return;
  g_lastOcCheckTick = now;

  int16_t raw = 0;
  if (!csd203DriverEnsureReady() || !csd203DriverReadCurrent(raw)) return;

  const int16_t ima = (raw < 0) ? static_cast<int16_t>(-raw) : raw;
  g_currentMa = ima;

  if (!g_ocBaselineValid) {
    if (static_cast<int32_t>(ima) - static_cast<int32_t>(g_lastCurrentMa) >=
        MAVLINK_MEASURE_OC_DELTA_MA) {
      tripOvercurrent();
      return;
    }
    g_lastCurrentMa = ima;
    if (static_cast<int32_t>(now - g_ocSettleUntil) >= 0) {
      g_ocBaselineMa = ima;
      g_ocBaselineValid = true;
    }
    return;
  }

  if (static_cast<int32_t>(ima) - static_cast<int32_t>(g_lastCurrentMa) >=
      MAVLINK_MEASURE_OC_DELTA_MA) {
    tripOvercurrent();
    return;
  }
  if (static_cast<int32_t>(ima) - static_cast<int32_t>(g_ocBaselineMa) >=
      MAVLINK_MEASURE_OC_DELTA_MA) {
    tripOvercurrent();
    return;
  }
  g_lastCurrentMa = ima;
#endif
}

void addLine(const char* fmt, ...)
{
  if (g_lineCount >= MAVLINK_MEASURE_MAX_LINES) return;
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(g_lineText[g_lineCount], MAVLINK_MEASURE_LINE_TEXT_LEN, fmt, ap);
  va_end(ap);
  ++g_lineCount;
}

void formatLatLon(char* out, size_t outLen, int32_t v)
{
  const bool neg = v < 0;
  uint32_t a = (uint32_t)(neg ? -v : v);
  const uint32_t deg = a / 10000000U;
  const uint32_t frac = (a % 10000000U) / 100U;  // 5 decimals
  snprintf(out, outLen, "%s%lu.%05lu", neg ? "-" : "", (unsigned long)deg,
           (unsigned long)frac);
}

void buildDisplayLines()
{
  g_lineCount = 0;

  const char* lastName = msgShortName(g_msgIdRaw);
  if (lastName) {
    addLine("Link v%u  %s pl%u  %uHz  CRC %s", (unsigned)g_versionRaw, lastName,
            (unsigned)g_payloadLenRaw, (unsigned)g_msgHz, g_crcLocked ? "OK" : "soft");
  } else {
    addLine("Link v%u  id%lu pl%u  %uHz  CRC %s", (unsigned)g_versionRaw,
            (unsigned long)g_msgIdRaw, (unsigned)g_payloadLenRaw, (unsigned)g_msgHz,
            g_crcLocked ? "OK" : "soft");
  }

  addLine("sys/comp %u/%u", (unsigned)g_sysIdRaw, (unsigned)g_compIdRaw);

  if (g_seenMask) {
    char seen[40];
    unsigned n = 0;
    seen[0] = '\0';
    const char* tags[] = {"HB", "SYS", "GPS", "ATT", "POS", "RC", "HUD", "RAD", "BAT"};
    for (unsigned i = 0; i < 9; ++i) {
      if (!(g_seenMask & (1u << i))) continue;
      if (n + 4 >= sizeof(seen)) break;
      if (n) seen[n++] = ' ';
      seen[n++] = tags[i][0];
      seen[n++] = tags[i][1];
      if (tags[i][2]) seen[n++] = tags[i][2];
      seen[n] = '\0';
    }
    addLine("Seen %s", seen);
  }

  if (g_haveRadio && !g_haveHb && !g_haveSys && !g_haveAtt) {
    addLine("ELRS radio only Ã¢Â?need FC for HB/GPS");
  }

  if (g_haveHb) {
    const bool armed = (g_hbBaseMode & 0x80) != 0;
    addLine("HB %s  %s", armed ? "ARMED" : "DISARM", mavStateName(g_hbSystemStatus));
    addLine("type%u ap%u mode%lu", (unsigned)g_hbType, (unsigned)g_hbAutopilot,
            (unsigned long)g_hbCustomMode);
  }

  if (g_haveSys || g_haveBatt) {
    const uint16_t mv = g_haveBatt ? g_battVoltMv : g_sysVoltMv;
    const int16_t ca = g_haveBatt ? g_battCurrentCA : g_sysCurrentCA;
    const int8_t pct = g_haveBatt ? g_battPct : g_sysBattPct;
    const int voltsX10 = (int)((mv + 50) / 100);
    const int ampsX10 = (int)ca;
    if (pct >= 0) {
      addLine("Batt %d.%dV  %d.%dA  %d%%", voltsX10 / 10, abs(voltsX10 % 10),
              ampsX10 / 10, abs(ampsX10 % 10), (int)pct);
    } else {
      addLine("Batt %d.%dV  %d.%dA", voltsX10 / 10, abs(voltsX10 % 10), ampsX10 / 10,
              abs(ampsX10 % 10));
    }
  }

  if (g_haveSys && g_sysLoad > 0) {
    addLine("CPU load %u%%", (unsigned)((g_sysLoad + 5) / 10));
  }

  if (g_haveAtt) {
    const int r = (int)lroundf(g_attRoll * 57.29578f);
    const int p = (int)lroundf(g_attPitch * 57.29578f);
    const int y = (int)lroundf(g_attYaw * 57.29578f);
    addLine("Att R%+d  P%+d  Y%+d", r, p, y);
  }

  if (g_haveHud) {
    const int altX10 = (int)lroundf(g_hudAlt * 10.f);
    const int spdX10 = (int)lroundf(g_hudGroundspeed * 10.f);
    const int climbX10 = (int)lroundf(g_hudClimb * 10.f);
    addLine("HUD alt %d.%dm  spd %d.%d", altX10 / 10, abs(altX10 % 10), spdX10 / 10,
            abs(spdX10 % 10));
    if (g_lineCount < MAVLINK_MEASURE_MAX_LINES) {
      addLine("HUD hdg %d  thr %u%%  cl %d.%d", (int)g_hudHeading, (unsigned)g_hudThrottle,
              climbX10 / 10, abs(climbX10 % 10));
    }
  }

  if (g_haveGps) {
    addLine("GPS %s  %u sat  alt %dm", gpsFixName(g_gpsFix), (unsigned)g_gpsSats,
            (int)(g_gpsAltMm / 1000));
    if (g_lineCount < MAVLINK_MEASURE_MAX_LINES) {
      char latBuf[16];
      char lonBuf[16];
      formatLatLon(latBuf, sizeof(latBuf), g_gpsLat);
      formatLatLon(lonBuf, sizeof(lonBuf), g_gpsLon);
      addLine("Lat %s Lon %s", latBuf, lonBuf);
    }
  }

  if (g_havePos) {
    if (g_haveHud) {
      addLine("Pos rel %dm", (int)(g_posRelAltMm / 1000));
    } else {
      addLine("Pos rel %dm  hdg %u", (int)(g_posRelAltMm / 1000),
              (unsigned)((g_posHdgCdeg + 50) / 100));
    }
  }

  if (g_haveRc) {
    addLine("RC %uch rssi%u  %u %u %u %u", (unsigned)g_rcCount, (unsigned)g_rcRssi,
            (unsigned)g_rcCh[0], (unsigned)g_rcCh[1], (unsigned)g_rcCh[2],
            (unsigned)g_rcCh[3]);
  }

  if (g_haveRadio) {
    addLine("Radio rssi%u/%u tx%u n%d/%d", (unsigned)g_radRssi, (unsigned)g_radRemRssi,
            (unsigned)g_radTxbuf, (int)g_radNoise, (int)g_radRemNoise);
  }
}

void decodePayload(uint32_t msgId, const uint8_t* payload, uint8_t payloadLen)
{
  // MAVLink v2 truncates trailing zero bytes Ã¢Â?pad before reading fields.
  uint8_t pl[64];
  memset(pl, 0, sizeof(pl));
  if (payload && payloadLen > 0) {
    const uint8_t n = (payloadLen > sizeof(pl)) ? (uint8_t)sizeof(pl) : payloadLen;
    memcpy(pl, payload, n);
  }
  // Treat as full canonical length for offset reads after padding.
  const uint8_t len = (payloadLen < 64) ? 64 : payloadLen;
  (void)len;

  // Payloads use MAVLink wire field order (sorted by type size).
  switch (msgId) {
    case 0:  // HEARTBEAT (9)
      if (payloadLen >= 8) {
        g_hbCustomMode = rdU32(pl);
        g_hbType = pl[4];
        g_hbAutopilot = pl[5];
        g_hbBaseMode = pl[6];
        g_hbSystemStatus = pl[7];
        g_haveHb = true;
        g_seenMask |= (1u << 0);
      }
      break;

    case 1:  // SYS_STATUS (31)
      if (payloadLen >= 19) {
        g_sysLoad = rdU16(pl + 12);
        g_sysVoltMv = rdU16(pl + 14);
        g_sysCurrentCA = rdI16(pl + 16);
        g_sysBattPct = (int8_t)pl[18];
        g_haveSys = true;
        g_seenMask |= (1u << 1);
      }
      break;

    case 24:  // GPS_RAW_INT (30+)
      if (payloadLen >= 29) {
        g_gpsLat = rdI32(pl + 8);
        g_gpsLon = rdI32(pl + 12);
        g_gpsAltMm = rdI32(pl + 16);
        g_gpsFix = pl[28];
        g_gpsSats = pl[29];
        g_haveGps = true;
        g_seenMask |= (1u << 2);
      }
      break;

    case 30:  // ATTITUDE (28)
      if (payloadLen >= 16) {
        g_attRoll = rdF32(pl + 4);
        g_attPitch = rdF32(pl + 8);
        g_attYaw = rdF32(pl + 12);
        g_haveAtt = true;
        g_seenMask |= (1u << 3);
      }
      break;

    case 33:  // GLOBAL_POSITION_INT (28)
      if (payloadLen >= 28) {
        g_posRelAltMm = rdI32(pl + 16);
        g_posHdgCdeg = rdU16(pl + 26);
        g_havePos = true;
        g_seenMask |= (1u << 4);
      }
      break;

    case 65:  // RC_CHANNELS
      if (payloadLen >= 12) {
        g_rcCh[0] = rdU16(pl + 4);
        g_rcCh[1] = rdU16(pl + 6);
        g_rcCh[2] = rdU16(pl + 8);
        g_rcCh[3] = rdU16(pl + 10);
        if (payloadLen >= 41) g_rcCount = pl[40];
        if (payloadLen >= 42) g_rcRssi = pl[41];
        g_haveRc = true;
        g_seenMask |= (1u << 5);
      }
      break;

    case 74:  // VFR_HUD (20, floats first on wire)
      if (payloadLen >= 20) {
        g_hudAirspeed = rdF32(pl);
        g_hudGroundspeed = rdF32(pl + 4);
        g_hudAlt = rdF32(pl + 8);
        g_hudClimb = rdF32(pl + 12);
        g_hudHeading = rdI16(pl + 16);
        g_hudThrottle = rdU16(pl + 18);
        g_haveHud = true;
        g_seenMask |= (1u << 6);
      }
      break;

    case 109:  // RADIO_STATUS (9, often truncated after remnoise=0)
      // Wire: rxerrors u16, fixed u16, rssi, remrssi, txbuf, noise, remnoise
      if (payloadLen >= 5) {
        g_radRssi = pl[4];
        g_radRemRssi = pl[5];
        g_radTxbuf = pl[6];
        g_radNoise = (int8_t)pl[7];
        g_radRemNoise = (int8_t)pl[8];
        g_haveRadio = true;
        g_seenMask |= (1u << 7);
      }
      break;

    case 147:  // BATTERY_STATUS
      if (payloadLen >= 12) {
        g_battVoltMv = rdU16(pl + 10);
        if (payloadLen >= 32) g_battCurrentCA = rdI16(pl + 30);
        if (payloadLen >= 36) g_battPct = (int8_t)pl[35];
        if (g_battVoltMv != 0 && g_battVoltMv != 0xFFFF) g_haveBatt = true;
        g_seenMask |= (1u << 8);
      }
      break;

    default:
      break;
  }
}

bool isKnownTelemMsg(uint32_t msgId)
{
  switch (msgId) {
    case 0:
    case 1:
    case 24:
    case 30:
    case 33:
    case 65:
    case 74:
    case 109:
    case 147:
      return true;
    default:
      return false;
  }
}

void publishFrame(uint8_t version, uint8_t sysId, uint8_t compId, uint32_t msgId,
                  const uint8_t* payload, uint8_t payloadLen, bool crcOk)
{
  g_versionRaw = version;
  g_sysIdRaw = sysId;
  g_compIdRaw = compId;
  g_msgIdRaw = msgId;
  g_payloadLenRaw = payloadLen;

  // Prefer CRC-OK; also soft-decode known telem msgs (ELRS RADIO_STATUS is often
  // zero-truncated and some links soft-lock before CRC_EXTRA locks).
  const bool softTelem = !crcOk && isKnownTelemMsg(msgId) && payloadLen >= 5;
  if (payload && payloadLen > 0 && (crcOk || softTelem)) {
    decodePayload(msgId, payload, payloadLen);
  }

  const uint32_t nowUs = timersGetUsTick();
  if (g_lastFrameUs != 0) {
    const uint32_t dt = nowUs - g_lastFrameUs;
    if (dt >= 2000U && dt <= 500000U) {
      g_framePeriodUs = dt;
    }
  }
  g_lastFrameUs = nowUs;
  g_lastFrame10ms = get_tmr10ms();

  if (crcOk) {
    g_crcLocked = true;
    g_softFrameCount = 0;
    g_haveSignal = true;
  } else {
    if (g_softFrameCount < 255) g_softFrameCount++;
    if (g_softFrameCount >= SOFT_LOCK_FRAMES) {
      g_haveSignal = true;
    }
  }

  // UI text built in Task Ã¢Â?never snprintf from IDLE/IRQ path.
  g_linesDirty = true;

  if (!g_configLocked && g_haveSignal) {
    g_configLocked = true;
    g_baudUi = BAUD_CANDIDATES[g_tryBaudIdx];
    g_polarityUi = (g_tryPolarity == ETX_Pol_Inverted) ? MAVLINK_MEASURE_POL_INVERTED
                                                       : MAVLINK_MEASURE_POL_NORMAL;
  }
}

bool verifyFrameCrc(const uint8_t* frame, uint16_t totalLen, uint8_t version,
                    uint32_t* outMsgId)
{
  if (totalLen < 8) return false;

  if (version == 1) {
    const uint8_t payloadLen = frame[1];
    const uint16_t expect = (uint16_t)(MAV1_HDR_LEN + payloadLen + MAV_CRC_LEN);
    if (totalLen != expect) return false;
    const uint32_t msgId = frame[5];
    if (outMsgId) *outMsgId = msgId;

    const uint16_t crcRx = (uint16_t)(frame[totalLen - 2] | ((uint16_t)frame[totalLen - 1] << 8));
    uint16_t crcCalc = crcX25(&frame[1], (uint32_t)(MAV1_HDR_LEN - 1 + payloadLen));
    const uint8_t extra = crcExtraForMsgId(msgId);
    if (extra) crcCalc = crcAccumulate(extra, crcCalc);
    return crcRx == crcCalc;
  }

  const uint8_t payloadLen = frame[1];
  const uint16_t expect = (uint16_t)(MAV2_HDR_LEN + payloadLen + MAV_CRC_LEN);
  if (totalLen != expect) return false;
  const uint32_t msgId =
      (uint32_t)frame[7] | ((uint32_t)frame[8] << 8) | ((uint32_t)frame[9] << 16);
  if (outMsgId) *outMsgId = msgId;

  const uint16_t crcRx = (uint16_t)(frame[totalLen - 2] | ((uint16_t)frame[totalLen - 1] << 8));
  uint16_t crcCalc = crcX25(&frame[1], (uint32_t)(MAV2_HDR_LEN - 1 + payloadLen));
  const uint8_t extra = crcExtraForMsgId(msgId);
  if (extra) crcCalc = crcAccumulate(extra, crcCalc);
  return crcRx == crcCalc;
}

bool parseHeader(const uint8_t* frame, uint16_t bufLen, uint8_t* outVersion,
                 uint32_t* outMsgId, uint8_t* outPayloadLen, uint16_t* outFrameLen)
{
  if (!frame || bufLen < 8) return false;

  if (frame[0] == MAV1_STX) {
    if (bufLen < MAV1_HDR_LEN) return false;
    const uint8_t payloadLen = frame[1];
    const uint16_t frameLen = (uint16_t)(MAV1_HDR_LEN + payloadLen + MAV_CRC_LEN);
    if (outVersion) *outVersion = 1;
    if (outMsgId) *outMsgId = frame[5];
    if (outPayloadLen) *outPayloadLen = payloadLen;
    if (outFrameLen) *outFrameLen = frameLen;
    return true;
  }

  if (frame[0] == MAV2_STX) {
    if (bufLen < MAV2_HDR_LEN) return false;
    const uint8_t payloadLen = frame[1];
    const uint16_t frameLen = (uint16_t)(MAV2_HDR_LEN + payloadLen + MAV_CRC_LEN);
    if (outVersion) *outVersion = 2;
    if (outMsgId) {
      *outMsgId = (uint32_t)frame[7] | ((uint32_t)frame[8] << 8) | ((uint32_t)frame[9] << 16);
    }
    if (outPayloadLen) *outPayloadLen = payloadLen;
    if (outFrameLen) *outFrameLen = frameLen;
    return true;
  }

  return false;
}

void dropParsePrefix(uint16_t n)
{
  if (n == 0) return;
  if (n >= g_parseLen) {
    g_parseLen = 0;
    return;
  }
  const uint16_t remain = (uint16_t)(g_parseLen - n);
  memmove(g_parseBuf, g_parseBuf + n, remain);
  g_parseLen = remain;
}

void consumeParseBuffer(uint8_t maxFrames)
{
  uint8_t frames = 0;
  while (g_parseLen >= 8 && frames < maxFrames) {
    if (!isMavlinkStx(g_parseBuf[0])) {
      uint16_t i = 1;
      while (i < g_parseLen && !isMavlinkStx(g_parseBuf[i])) ++i;
      dropParsePrefix(i);
      continue;
    }

    uint8_t version = 0;
    uint32_t msgId = 0;
    uint8_t payloadLen = 0;
    uint16_t totalLen = 0;
    if (!parseHeader(g_parseBuf, g_parseLen, &version, &msgId, &payloadLen, &totalLen)) {
      dropParsePrefix(1);
      continue;
    }

    if (totalLen > MAV_PARSE_MAX || totalLen < 8) {
      dropParsePrefix(1);
      continue;
    }

    if (g_parseLen < totalLen) return;

    const uint8_t hdrLen = (version == 1) ? MAV1_HDR_LEN : MAV2_HDR_LEN;
    const uint8_t sysId = (version == 1) ? g_parseBuf[3] : g_parseBuf[5];
    const uint8_t compId = (version == 1) ? g_parseBuf[4] : g_parseBuf[6];

    const bool crcOk = verifyFrameCrc(g_parseBuf, totalLen, version, &msgId);
    const uint8_t* payload = &g_parseBuf[hdrLen];
    publishFrame(version, sysId, compId, msgId, payload, payloadLen, crcOk);
    ++frames;

    dropParsePrefix(totalLen);
  }
}

void pollUartBytes(uint32_t maxBytes, uint8_t maxFrames)
{
  if (!g_active || g_overcurrentFault || !g_powerOn || !g_uartCtx) return;
  if (!STM32SerialDriver.getByte) return;

  uint8_t b = 0;
  uint32_t n = 0;
  while (n < maxBytes && STM32SerialDriver.getByte(g_uartCtx, &b) > 0) {
    ++n;
    if (g_parseLen < MAV_PARSE_MAX) {
      g_parseBuf[g_parseLen++] = b;
    } else {
      dropParsePrefix(MAV_PARSE_MAX / 2);
      g_parseBuf[g_parseLen++] = b;
    }
  }

  consumeParseBuffer(maxFrames);
}

void onMavlinkIdle(void*)
{
  // Keep USART IDLE short Ã¢Â?Task finishes residual bytes/frames + UI text.
  pollUartBytes(MAV_MAX_BYTES_IDLE, MAV_MAX_FRAMES_IDLE);
}

void stopUart()
{
  if (g_uartCtx) {
    STM32SerialDriver.deinit(g_uartCtx);
    g_uartCtx = nullptr;
  }
  g_parseLen = 0;
}

bool startUart(uint32_t baudrate, uint8_t polarity)
{
  stopUart();

  etx_serial_init params = {
      .baudrate = baudrate,
      .encoding = ETX_Encoding_8N1,
      .direction = ETX_Dir_RX,
      .polarity = polarity,
  };

  g_uartCtx = STM32SerialDriver.init(REF_STM32_SERIAL_PORT(MavlinkMeas), &params);
  if (!g_uartCtx) return false;

  if (STM32SerialDriver.setIdleCb) {
    STM32SerialDriver.setIdleCb(g_uartCtx, onMavlinkIdle, nullptr);
  }
  return true;
}

void beginConfigSearch(uint8_t baudIdx, uint8_t polarity)
{
  g_configLocked = false;
  g_tryBaudIdx = baudIdx % BAUD_COUNT;
  g_tryPolarity = polarity;
  g_configTryStart = get_tmr10ms();
  g_polarityUi = MAVLINK_MEASURE_POL_SEARCHING;
  g_baudUi = 0;
  g_haveSignal = false;
  g_crcLocked = false;
  g_softFrameCount = 0;
  g_lastFrameUs = 0;
  g_parseLen = 0;
  startUart(BAUD_CANDIDATES[g_tryBaudIdx], g_tryPolarity);
}

void advanceConfigSearch()
{
  if (g_tryPolarity == ETX_Pol_Normal) {
    g_tryPolarity = ETX_Pol_Inverted;
  } else {
    g_tryPolarity = ETX_Pol_Normal;
    g_tryBaudIdx = (uint8_t)((g_tryBaudIdx + 1) % BAUD_COUNT);
  }
  g_configTryStart = get_tmr10ms();
  g_parseLen = 0;
  g_softFrameCount = 0;
  startUart(BAUD_CANDIDATES[g_tryBaudIdx], g_tryPolarity);
}

void copyLiveToDisplay()
{
  g_version = g_versionRaw;
  g_sysId = g_sysIdRaw;
  g_compId = g_compIdRaw;
  g_msgId = g_msgIdRaw;
  g_polarity = g_polarityUi;
  g_baudrate = g_baudUi;
  if (g_framePeriodUs > 0) {
    g_msgHz = (uint16_t)((1000000U + (g_framePeriodUs / 2U)) / g_framePeriodUs);
  }
  if (g_linesDirty) {
    g_linesDirty = false;
    buildDisplayLines();
  }
}

}  // namespace

bool v15MavlinkMeasureIsActive(void) { return g_active; }
bool v15MavlinkMeasureHasSignal(void) { return g_haveSignal && !g_overcurrentFault; }
bool v15MavlinkMeasureIsPowerOn(void) { return g_powerOn && !g_overcurrentFault; }
bool v15MavlinkMeasureOvercurrentFault(void) { return g_overcurrentFault; }
int16_t v15MavlinkMeasureGetCurrentMa(void) { return g_currentMa; }

uint8_t v15MavlinkMeasureGetPolarity(void) { return g_polarity; }
uint32_t v15MavlinkMeasureGetBaudrate(void) { return g_baudrate; }
uint8_t v15MavlinkMeasureGetVersion(void) { return v15MavlinkMeasureHasSignal() ? g_version : 0; }
uint8_t v15MavlinkMeasureGetSysId(void) { return v15MavlinkMeasureHasSignal() ? g_sysId : 0; }
uint8_t v15MavlinkMeasureGetCompId(void) { return v15MavlinkMeasureHasSignal() ? g_compId : 0; }
uint32_t v15MavlinkMeasureGetMsgId(void) { return v15MavlinkMeasureHasSignal() ? g_msgId : 0; }
uint16_t v15MavlinkMeasureGetMsgHz(void) { return v15MavlinkMeasureHasSignal() ? g_msgHz : 0; }

uint8_t v15MavlinkMeasureGetLineCount(void)
{
  return v15MavlinkMeasureHasSignal() ? g_lineCount : 0;
}

uint8_t v15MavlinkMeasureGetLineText(uint8_t index, char* buf, uint8_t buflen)
{
  if (!buf || buflen == 0 || index >= g_lineCount) return 0;
  const char* src = g_lineText[index];
  uint8_t n = 0;
  while (src[n] && n + 1 < buflen) {
    buf[n] = src[n];
    ++n;
  }
  buf[n] = '\0';
  return n;
}

void v15MavlinkMeasureStart(void)
{
  if (g_active) return;
  if (v15ServoTesterIsActive()) return;
  if (v15PwmMeasureIsActive()) return;
  if (v15PpmMeasureIsActive()) return;
  if (v15SbusMeasureIsActive()) return;
  if (v15CrsfMeasureIsActive()) return;
  if (v15DshotTesterIsActive()) return;
  if (v15SumdMeasureIsActive()) return;
  if (v15DjiRsMeasureIsActive()) return;
  if (v15LogicMeasureIsActive()) return;
  if (v15AutoMeasureIsActive() && !v15AutoMeasureIsStartingChild()) return;

  takeOverAuxPort();

  memset(g_lineText, 0, sizeof(g_lineText));
  g_lineCount = 0;
  g_versionRaw = 0;
  g_sysIdRaw = 0;
  g_compIdRaw = 0;
  g_msgIdRaw = 0;
  g_version = 0;
  g_sysId = 0;
  g_compId = 0;
  g_msgId = 0;
  g_msgHz = 0;
  g_framePeriodUs = 0;
  g_lastFrameUs = 0;
  g_lastFrame10ms = get_tmr10ms();
  g_crcLocked = false;
  g_softFrameCount = 0;
  g_linesDirty = false;

  g_haveHb = g_haveSys = g_haveBatt = g_haveAtt = false;
  g_haveGps = g_havePos = g_haveHud = g_haveRc = g_haveRadio = false;
  g_seenMask = 0;
  g_sysBattPct = g_battPct = -1;
  g_hbCustomMode = 0;
  memset(g_rcCh, 0, sizeof(g_rcCh));

  g_overcurrentFault = false;
  g_ocBaselineValid = false;
  g_ocBaselineMa = 0;
  g_lastCurrentMa = 0;
  g_currentMa = 0;
  g_ocSettleUntil = get_tmr10ms() + OC_SETTLE_10MS;
  g_lastOcCheckTick = 0;

  if (v15AutoMeasureIsStartingChild() && v15AutoMeasureIsPowerOn()) {
    v15ExtPortEnsureSignalPathOpen();
  } else if (!v15ExtPortTryOpenSignalPath()) {
    restoreAuxPort();
    return;
  }

  g_active = true;
  g_powerOn = true;
  setPortPower(true);

  beginConfigSearch(0, ETX_Pol_Normal);
}

void v15MavlinkMeasureStop(void)
{
  if (!g_active) return;

  g_active = false;
  g_powerOn = false;
  g_haveSignal = false;
  g_configLocked = false;
  g_overcurrentFault = false;

  stopUart();
  gpio_init(SERVO_TESTER_PWM_GPIO, GPIO_IN_PD, GPIO_PIN_SPEED_LOW);

  if (v15AutoMeasureIsActive()) {
    g_auxTakenOver = false;
  } else {
    setExtSignalPath(false);
    setPortPower(false);
    restoreAuxPort();
#if defined(MODULE_BATTERY_SENSOR)
    v15BatterySensorResumeAfterExtPort();
#endif
  }
}

void v15MavlinkMeasureTask(void)
{
  if (!g_active) return;

  checkOvercurrent();
  if (g_overcurrentFault) return;

  pollUartBytes(MAV_MAX_BYTES_TASK, MAV_MAX_FRAMES_TASK);

  const tmr10ms_t now = get_tmr10ms();

  if (g_haveSignal &&
      (tmr10ms_t)(now - g_lastFrame10ms) >= MAVLINK_MEASURE_SIGNAL_TIMEOUT_10MS) {
    g_haveSignal = false;
    g_msgHz = 0;
    beginConfigSearch(g_tryBaudIdx, g_tryPolarity);
    return;
  }

  if (!g_configLocked && g_uartCtx) {
    if ((tmr10ms_t)(now - g_configTryStart) >= MAVLINK_MEASURE_POLARITY_TRY_10MS) {
      advanceConfigSearch();
    }
  }

  if (g_haveSignal) {
    copyLiveToDisplay();
  }
}

#else  // !RADIO_V15 || SIMU

bool v15MavlinkMeasureIsActive(void) { return false; }
bool v15MavlinkMeasureHasSignal(void) { return false; }
bool v15MavlinkMeasureIsPowerOn(void) { return false; }
bool v15MavlinkMeasureOvercurrentFault(void) { return false; }
int16_t v15MavlinkMeasureGetCurrentMa(void) { return 0; }
uint8_t v15MavlinkMeasureGetPolarity(void) { return MAVLINK_MEASURE_POL_SEARCHING; }
uint32_t v15MavlinkMeasureGetBaudrate(void) { return 0; }
uint8_t v15MavlinkMeasureGetVersion(void) { return 0; }
uint8_t v15MavlinkMeasureGetSysId(void) { return 0; }
uint8_t v15MavlinkMeasureGetCompId(void) { return 0; }
uint32_t v15MavlinkMeasureGetMsgId(void) { return 0; }
uint16_t v15MavlinkMeasureGetMsgHz(void) { return 0; }
uint8_t v15MavlinkMeasureGetLineCount(void) { return 0; }
uint8_t v15MavlinkMeasureGetLineText(uint8_t, char*, uint8_t) { return 0; }
void v15MavlinkMeasureStart(void) {}
void v15MavlinkMeasureStop(void) {}
void v15MavlinkMeasureTask(void) {}

#endif  // RADIO_V15
