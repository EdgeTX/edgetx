/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "serial.h"
#include "board.h"
#include "debug.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "hal/serial_port.h"
#include "hal/usb_driver.h"

#if defined(CONFIGURABLE_MODULE_PORT) and !defined(BOOT)
  #include "hal/module_port.h"
  #include "tasks/mixer_task.h"
#endif

#if !defined(BOOT)
  #include "edgetx.h"
  #include "lua/lua_api.h"
#else
  #include "dataconstants.h"
#endif

#if defined(CROSSFIRE)
  #include "telemetry/crossfire.h"
#endif

#if defined(DEBUG_SEGGER_RTT)
  #include "thirdparty/Segger_RTT/RTT/SEGGER_RTT.h"
#endif

#define PRINTF_BUFFER_SIZE    128

static void (*dbg_serial_putc)(void*, uint8_t) = nullptr;
static void* dbg_serial_ctx = nullptr;

void (*dbgSerialGetSendCb())(void*, uint8_t)
{
  return dbg_serial_putc;
}

void* dbgSerialGetSendCbCtx()
{
  return dbg_serial_ctx;
}

void dbgSerialSetSendCb(void* ctx, void (*cb)(void*, uint8_t))
{
  dbg_serial_putc = nullptr;
  dbg_serial_ctx = ctx;
  dbg_serial_putc = cb;
}

#if defined(DEBUG_SEGGER_RTT)

extern "C" void dbgSerialPutc(char c)
{
  SEGGER_RTT_Write(0, (const void *)&c, 1);
}

extern "C" void dbgSerialPrintf(const char * format, ...)
{
  va_list arglist;
  char tmp[PRINTF_BUFFER_SIZE+1];

  va_start(arglist, format);
  vsnprintf(tmp, PRINTF_BUFFER_SIZE, format, arglist);
  tmp[PRINTF_BUFFER_SIZE] = '\0';
  va_end(arglist);

  const char *t = tmp;
  while (*t) {
    SEGGER_RTT_Write(0, (const void*)t++, 1);
  }
}
#else

extern "C" void dbgSerialPutc(char c)
{
  auto _putc = dbg_serial_putc;
  auto _ctx = dbg_serial_ctx;
  if (_putc) _putc(_ctx, c);
}

extern "C" void dbgSerialPrintf(const char * format, ...)
{
  va_list arglist;
  char tmp[PRINTF_BUFFER_SIZE+1];

  // no need to do anything if we don't have an output
  if (!dbg_serial_putc) return;

  va_start(arglist, format);
  vsnprintf(tmp, PRINTF_BUFFER_SIZE, format, arglist);
  tmp[PRINTF_BUFFER_SIZE] = '\0';
  va_end(arglist);

  const char *t = tmp;
  while (*t && dbg_serial_putc) {
    dbg_serial_putc(dbg_serial_ctx, *t++);
  }
}
#endif

extern "C" void dbgSerialCrlf()
{
  dbgSerialPutc('\r');
  dbgSerialPutc('\n');
}

int8_t getSerialPortMode(uint8_t port_nr)
{
#if !defined(BOOT)
  if (port_nr < MAX_SERIAL_PORTS) {
    auto cfg = g_eeGeneral.serialPort;
    cfg >>= port_nr * SERIAL_CONF_BITS_PER_PORT;
    return cfg & SERIAL_CONF_MODE_MASK;
  }
#endif

  return UART_MODE_NONE;
}

static bool getSerialPower(uint8_t port_nr)
{
#if !defined(BOOT)
  if (port_nr < MAX_SERIAL_PORTS) {
    auto cfg = g_eeGeneral.serialPort;
    cfg >>= (port_nr * SERIAL_CONF_BITS_PER_PORT);
    return cfg & (1 << SERIAL_CONF_POWER_BIT);
  }
#endif

  return false;
}

struct SerialPortState
{
  uint8_t                  mode;
  const etx_serial_port_t* port;
  void*                    usart_ctx;
};

static SerialPortState serialPortStates[MAX_SERIAL_PORTS];

static SerialPortState* getSerialPortState(uint8_t port_nr)
{
  if (port_nr >= MAX_SERIAL_PORTS) return nullptr;
  return &serialPortStates[port_nr];
}

// TODO: replace serialSetupCallBacks & serialSetupPort
//       with usage based handlers.
//
static void serialSetCallBacks(int mode, void* ctx, const etx_serial_port_t* port)
{
  void (*sendByte)(void*, uint8_t) = nullptr;
  int (*getByte)(void*, uint8_t*) = nullptr;
  void (*setRxCb)(void*, void (*)(uint8_t*, uint32_t)) = nullptr;

  const etx_serial_driver_t* drv = nullptr;
  if (port && ctx) {
    drv = port->uart;
    if (drv) {
      sendByte = drv->sendByte;
      getByte = drv->getByte;
      setRxCb = drv->setReceiveCb;
    }
  }

  // prevent compiler warnings
  (void)sendByte;
  (void)getByte;
  (void)setRxCb;

  switch(mode) {
#if defined(DEBUG)
  case UART_MODE_DEBUG:
    // TODO: should we handle 2 outputs?
    dbgSerialSetSendCb(ctx, sendByte);
    break;
#endif

#if !defined(BOOT)
#if defined(LUA)
  case UART_MODE_LUA:
    luaSetSendCb(ctx, sendByte);
    if (getByte) {
      luaSetGetSerialByte(ctx, getByte);
    } else if (setRxCb) {
      luaAllocRxFifo();
      setRxCb(ctx, luaReceiveData);
    } else {
      luaFreeRxFifo();
    }
    break;
#endif

  case UART_MODE_SBUS_TRAINER:
    sbusSetReceiveCtx(ctx, drv);
    if (drv && drv->setIdleCb) {
      drv->setIdleCb(ctx, sbusAuxFrameReceived, nullptr);
    }
    break;

  case UART_MODE_TELEMETRY:
    // telemetrySetGetByte(ctx, getByte);

    // TODO: setRxCb (see MODE_LUA)
    //       de we really need telemetry
    //       input over USB VCP?
    break;

  case UART_MODE_TELEMETRY_MIRROR:
    telemetrySetMirrorCb(ctx, sendByte);
    break;

#if defined(CLI) && !defined(SIMU)
  case UART_MODE_CLI:
    cliSetSerialDriver(ctx, drv);
    break;
#endif

#if defined(INTERNAL_GPS)
  case UART_MODE_GPS:
    gpsSetSerialDriver(ctx, drv, GPS_PROTOCOL_AUTO);
    break;
#endif

#if defined(SPACEMOUSE)
  case UART_MODE_SPACEMOUSE:
    spacemouseSetSerialDriver(ctx, drv);
    break;
#endif

#if defined(CONFIGURABLE_MODULE_PORT) and !defined(BOOT)
  case UART_MODE_EXT_MODULE:
    if (port && !ctx) { // de-init
      etx_module_port_t mod_port;
      memset(&mod_port, 0, sizeof(mod_port));
      auto mod_st = modulePortGetState(EXTERNAL_MODULE);
      if (mod_st && mod_st->tx.port &&
          mod_st->tx.port->hw_def == port->hw_def) {
        // port is in use, let's stop it
        mixerTaskStop();
        pulsesStop();
        pulsesStopModule(EXTERNAL_MODULE);
        modulePortConfigExtra(&mod_port);
        pulsesStart();
        mixerTaskStart();
      } else {
        modulePortConfigExtra(&mod_port);
      }
    }
    break;
#endif

#endif
  }
}

static void serialSetupPort(int mode, etx_serial_init& params)
{
  switch (mode) {

#if defined(DEBUG) || defined(CLI)
  case UART_MODE_DEBUG:
  case UART_MODE_CLI:
    params.baudrate = DEBUG_BAUDRATE;
    break;
#endif

#if !defined(BOOT)
  case UART_MODE_TELEMETRY_MIRROR:
    // TODO: query telemetry baudrate / add setting for module
#if defined(CROSSFIRE)
    if (isModuleCrossfire(EXTERNAL_MODULE) || isModuleCrossfire(INTERNAL_MODULE)) {
      params.baudrate = CROSSFIRE_TELEM_MIRROR_BAUDRATE;
      break;
    }
#endif
    params.baudrate = FRSKY_TELEM_MIRROR_BAUDRATE;
    break;

  case UART_MODE_TELEMETRY:
    if (isModulePPM(EXTERNAL_MODULE) &&
        g_model.telemetryProtocol == PROTOCOL_TELEMETRY_FRSKY_D_SECONDARY) {
      params.baudrate = FRSKY_D_BAUDRATE;
      params.direction = ETX_Dir_RX;
    }
    break;

  case UART_MODE_SBUS_TRAINER:
    params.baudrate = SBUS_BAUDRATE;
    params.encoding = ETX_Encoding_8E2,
    params.direction = ETX_Dir_RX;
    break;

#if defined(LUA)
  case UART_MODE_LUA:
    params.baudrate = LUA_DEFAULT_BAUDRATE;
    params.direction = ETX_Dir_TX_RX;
    break;
#endif

#if defined(INTERNAL_GPS)
  case UART_MODE_GPS:
    params.baudrate = GPS_USART_BAUDRATE;
    params.direction = ETX_Dir_TX_RX;
    break;
#endif

#if defined(SPACEMOUSE)
  case UART_MODE_SPACEMOUSE:
    params.baudrate = SPACEMOUSE_BAUDRATE;
    params.direction = ETX_Dir_TX_RX;
    break;
#endif

#endif // BOOT
  }
}

const etx_serial_port_t* serialGetPort(uint8_t port_nr)
{
  const etx_serial_port_t* port = nullptr;
  if (port_nr != SP_VCP) {
    port = auxSerialGetPort(port_nr);
  }
#if defined(USB_SERIAL)
  else {
    port = &UsbSerialPort;
  }
#endif
  return port;
}

static void serialSetPowerState(uint8_t port_nr)
{
    const etx_serial_port_t* port = serialGetPort(port_nr);
    if (!port) return;

    if (port->set_pwr) {
      port->set_pwr(getSerialPower(port_nr));
    }
}

#if !defined(BOOT)
void serialSetPower(uint8_t port_nr, bool enabled)
{
  if (port_nr >= MAX_SERIAL_PORTS) return;
  uint32_t pwr = (enabled ? 1 : 0) << SERIAL_CONF_POWER_BIT;
  uint32_t pwr_mask = (1 << SERIAL_CONF_POWER_BIT) << port_nr * SERIAL_CONF_BITS_PER_PORT;
  g_eeGeneral.serialPort = (g_eeGeneral.serialPort & ~pwr_mask) |
                           (pwr << port_nr * SERIAL_CONF_BITS_PER_PORT);

  serialSetPowerState(port_nr);
}
#endif

uint32_t serialGetBaudrate(uint8_t port_nr)
{
  auto state = getSerialPortState(port_nr);
  if (!state || !state->port || !state->usart_ctx)
    return 0;

  auto port = state->port;
  if (!port->uart || !port->uart->getBaudrate) return 0;

  return port->uart->getBaudrate(state->usart_ctx);
}

void serialSetBaudrate(uint8_t port_nr, uint32_t baudrate)
{
  auto state = getSerialPortState(port_nr);
  if (!state || !state->port || !state->usart_ctx)
    return;

  auto port = state->port;
  if (!port->uart || !port->uart->setBaudrate) return;

  port->uart->setBaudrate(state->usart_ctx, baudrate);
}

int serialGetModePort(int mode)
{
  for (int p = 0; p < MAX_SERIAL_PORTS; p++) {
    if (serialGetMode(p) == mode) return p;
  }
  return -1;  
}

void serialInit(uint8_t port_nr, int mode)
{
  auto state = getSerialPortState(port_nr);
  if (!state) return;

  const etx_serial_port_t* port = serialGetPort(port_nr);
  if (!port) return;

  if (state->port) {
    auto drv = state->port->uart;
    if (drv && drv->deinit && state->usart_ctx) {
      drv->deinit(state->usart_ctx);
    }
    if (state->mode != UART_MODE_NONE) {
      // Clear callbacks
      serialSetCallBacks(state->mode, nullptr, state->port);
    }
    memset(state, 0, sizeof(SerialPortState));
  }

#if defined(CONFIGURABLE_MODULE_PORT) and !defined(BOOT)
  if (mode == UART_MODE_EXT_MODULE) {
    etx_module_port_t mod_port = {
      .port = ETX_MOD_PORT_UART,
      .type = ETX_MOD_TYPE_SERIAL,
      .dir_flags = ETX_MOD_DIR_TX_RX,
      .drv = { .serial = port->uart },
      .hw_def = port->hw_def,
      .set_inverted = nullptr,
    };
    modulePortConfigExtra(&mod_port);
    state->mode = mode;
    state->port = port;
    return;
  }
#endif

  etx_serial_init params = {
    .baudrate = 0,
    .encoding = ETX_Encoding_8N1,
    .direction = ETX_Dir_TX,
    .polarity = ETX_Pol_Normal,
  };

  serialSetupPort(mode, params);

  if (mode == UART_MODE_NONE ) {
    // Even if port has no mode, port power needs to be set
    serialSetPowerState(port_nr);
    return;
  }

  if (!port || params.baudrate == 0 ||
      !port->uart || !port->uart->init)
    return;
  
  auto hw_def = port->hw_def;
  state->usart_ctx = port->uart->init(hw_def, &params);

  // init failed
  if (!state->usart_ctx) return;

  state->mode = mode;
  state->port = port;
        
  // Update callbacks once the port is setup
  serialSetCallBacks(mode, state->usart_ctx, state->port);

#if defined(SWSERIALPOWER)
  // Set power on/off
  if (port_nr < SP_VCP)
    serialSetPowerState(port_nr);
#endif
}

void initSerialPorts()
{
  for (uint8_t port_nr = 0; port_nr < MAX_AUX_SERIAL; port_nr++) {
    auto mode = getSerialPortMode(port_nr);
    serialInit(port_nr, mode);
  }
}

int serialGetMode(uint8_t port_nr)
{
  return getSerialPortMode(port_nr);
}

bool serialGetPower(uint8_t port_nr)
{
  return getSerialPower(port_nr);
}

#if !defined(BOOT)
void serialSetMode(uint8_t port_nr, int mode)
{
  if (port_nr >= MAX_SERIAL_PORTS) return;
  uint16_t m = mode & SERIAL_CONF_MODE_MASK;
  g_eeGeneral.serialPort =
      (g_eeGeneral.serialPort &
       ~(SERIAL_CONF_MODE_MASK << port_nr * SERIAL_CONF_BITS_PER_PORT)) |
      (m << port_nr * SERIAL_CONF_BITS_PER_PORT);
}
#endif

// uint8_t serialTracesEnabled(int port_nr)
// {
// #if defined(DEBUG)
//   return serialGetMode(port_nr) == UART_MODE_DEBUG;
// #else
//   return false;
// #endif
// }

void serialStop(uint8_t port_nr)
{
  auto state = getSerialPortState(port_nr);
  if (!state) return;

  if (state->port) {
    auto port = state->port;
    auto drv = port->uart;
    if (drv && drv->deinit) {
      drv->deinit(state->usart_ctx);
    }
    if (port->set_pwr) {
      // Power OFF
      port->set_pwr(0);
    }
    if (state->mode != 0) {
      // Clear callbacks
      serialSetCallBacks(state->mode, nullptr, nullptr);
    }
  }
  memset(state, 0, sizeof(SerialPortState));
}
