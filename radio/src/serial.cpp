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

#include "opentx.h"
#include "serial.h"
#include <stdarg.h>
#include <stdio.h>
#include "aux_serial_driver.h"
#include "hal/serial_port.h"
//#include "cli.h"

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
  // Avoid overwriting debug port config
  if (cb && dbg_serial_putc) return;

  dbg_serial_putc = nullptr;
  dbg_serial_ctx = ctx;
  dbg_serial_putc = cb;
}

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

extern "C" void dbgSerialCrlf()
{
  dbgSerialPutc('\r');
  dbgSerialPutc('\n');
}

static int getSerialPortMode(uint8_t port_nr)
{
#if !defined(BOOT)
  if (port_nr == 0) {
    return g_eeGeneral.auxSerialMode;
  }

// TODO: change in datastruct_private.h
#if defined(PCBHORUS) || defined(PCBNV14)
  if (port_nr == 1) {
    return g_eeGeneral.aux2SerialMode;
  }
#endif
#endif

  return UART_MODE_NONE;
}

struct SerialPortState
{
  uint8_t                  mode;
  const etx_serial_port_t* port;
  void*                    usart_ctx;
};

static SerialPortState serialPortStates[MAX_SERIAL_PORTS];

static SerialPortState* getSerialPortState(int port_nr)
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
  if (port) {
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
      setRxCb(ctx, luaReceiveData);
    }
    break;
#endif

#if defined(SBUS_TRAINER)
  case UART_MODE_SBUS_TRAINER:
    sbusSetAuxGetByte(ctx, getByte);
    // TODO: setRxCb (see MODE_LUA)
    break;
#endif

  case UART_MODE_TELEMETRY:
    telemetrySetGetByte(ctx, getByte);
    // TODO: setRxCb (see MODE_LUA)
    //       de we really need telemetry
    //       input over USB VCP?
    break;

  case UART_MODE_TELEMETRY_MIRROR:
    telemetrySetMirrorCb(ctx, sendByte);
    break;

#if defined(CLI)
  case UART_MODE_CLI:
    cliSetSerialDriver(ctx, drv);
    break;
#endif
#endif
  }
}

static void serialSetupPort(int mode, etx_serial_init& params, bool& power_required)
{
  switch (mode) {

#if defined(DEBUG) || defined(CLI)
  case UART_MODE_DEBUG:
    params.baudrate = DEBUG_BAUDRATE;
    break;
#endif

#if !defined(BOOT)
  case UART_MODE_TELEMETRY_MIRROR:
    // TODO: query telemetry baudrate / add setting for module
    power_required = true;
#if defined(CROSSFIRE)
    if (modelTelemetryProtocol() == PROTOCOL_TELEMETRY_CROSSFIRE) {
      params.baudrate = CROSSFIRE_TELEM_MIRROR_BAUDRATE;
      break;
    }
#endif
    params.baudrate = FRSKY_TELEM_MIRROR_BAUDRATE;
    break;

  case UART_MODE_TELEMETRY:
    if (modelTelemetryProtocol() == PROTOCOL_TELEMETRY_FRSKY_D_SECONDARY) {
      params.baudrate = FRSKY_D_BAUDRATE;
      params.rx_enable = true;
      power_required = true;
    }
    break;

  case UART_MODE_SBUS_TRAINER:
    params.baudrate = SBUS_BAUDRATE;
    params.word_length = ETX_WordLength_9;
    params.parity = ETX_Parity_Even;
    params.stop_bits = ETX_StopBits_Two;
    params.rx_enable = true;
    power_required = true;
    break;

#if defined(LUA)
  case UART_MODE_LUA:
    params.baudrate = LUA_DEFAULT_BAUDRATE;
    params.rx_enable = true;
    power_required = true;
    break;
#endif
#endif
  }
}

void serialInit(int port_nr, int mode)
{
  auto state = getSerialPortState(port_nr);
  if (!state) return;

  const etx_serial_port_t* port = nullptr;
  if (port_nr != SP_VCP) {
    port = auxSerialGetPort(port_nr);
  }
#if defined(USB_SERIAL)
  else {
    port = &UsbSerialPort;
  }
#endif
  if (!port) return;

  if (state->port) {
    auto drv = state->port->uart;
    if (drv->deinit) {
      drv->deinit(state->usart_ctx);
    }
    if (state->mode != 0) {
      // Clear callbacks
      serialSetCallBacks(state->mode, nullptr, nullptr);
    }
    memset(state, 0, sizeof(SerialPortState));
  }

  etx_serial_init params = {
    .baudrate = 0,
    .parity = ETX_Parity_None,
    .stop_bits = ETX_StopBits_One,
    .word_length = ETX_WordLength_8,
    .rx_enable = false,
    .on_receive = nullptr,
    .on_error = nullptr,
  };

  bool power_required = false;
  serialSetupPort(mode, params, power_required);
    
  if (params.baudrate != 0) {
    state->mode = mode;
    state->port = port;
    state->usart_ctx = port->uart->init(&params);

    // Set power on/off
    if (port->set_pwr) {
      port->set_pwr(power_required);
    }

    // Update callbacks once the port is setup
    serialSetCallBacks(mode, state->usart_ctx, state->port);
  }
}

void initSerialPorts()
{
  memset(serialPortStates, 0, sizeof(serialPortStates));
  
  for (int port_nr = 0; port_nr < MAX_AUX_SERIAL; port_nr++) {
    auto mode = getSerialPortMode(port_nr);
    serialInit(port_nr, mode);
  }
}

uint8_t serialGetMode(int port_nr)
{
  auto state = getSerialPortState(port_nr);
  if (!state) return UART_MODE_NONE;
  return state->mode;
}

uint8_t serialTracesEnabled(int port_nr)
{
#if defined(DEBUG)
  return serialGetMode(port_nr) == UART_MODE_DEBUG;
#else
  return false;
#endif
}

void serialStop(int port_nr)
{
  auto state = getSerialPortState(port_nr);
  if (!state) return;

  if (state->port) {
    auto port = state->port;
    auto drv = port->uart;
    if (drv && drv->deinit) {
      drv->deinit(state->usart_ctx);
    }
    if (state->mode != 0) {
      // Clear callbacks
      serialSetCallBacks(state->mode, nullptr, nullptr);
    }
    if (port->set_pwr) {
      // Power OFF
      port->set_pwr(0);
    }
    memset(state, 0, sizeof(SerialPortState));
  }
}

void serialPutc(int port_nr, uint8_t c)
{
  auto state = getSerialPortState(port_nr);
  if (!state) return;

  if (state->port) {
    state->port->uart->sendByte(state->usart_ctx, c);
  }  
}
