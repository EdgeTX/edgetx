/*
 * Copyright (C) EdgeTx
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

#pragma once

#include <stdint.h>
#include "serial_driver.h"
#include "timer_driver.h"
#include "definitions.h"

// Module ports
//
// - internal module:
//   - timer based ("pulses") (w/ telemetry on S.PORT)
//   - serial based (some pxx1) (W/ telemetry on S.PORT)
//   - bidir serial (pxx2, crsf, multi)
//
// - external module:
//   - TX on PPM pin + RX on S.PORT (PXX1, MPM, DSM)
//   - TX on PPM pin + RX on HEARTBEAT (UART / soft-serial)
//
//   - RX/TX on half-duplex S.PORT
//
// More generic:
// - TX:
//   - timer based (PXX1 pulses, PPM)
//   - serial based (UART or soft-serial)

// TODOs:
// - handle power on/off
// - mixer scheduler off on de-init?

// flags
#define ETX_MOD_DIR_TX      (1 << 0)
#define ETX_MOD_DIR_RX      (1 << 1)
#define ETX_MOD_DIR_TX_RX   (ETX_MOD_DIR_TX | ETX_MOD_DIR_RX)
#define ETX_MOD_FULL_DUPLEX (1 << 2)

#define ETX_MOD_TYPE_NONE   0
#define ETX_MOD_TYPE_TIMER  (1 << 0)
#define ETX_MOD_TYPE_SERIAL (1 << 1)

// this corresponds to internal + external + S.PORT
#define MAX_OPEN_MODULE_PORTS 3

enum ModulePort : uint8_t {
  ETX_MOD_PORT_INTERNAL_UART,
  ETX_MOD_PORT_INTERNAL_TIMER,
  ETX_MOD_PORT_INTERNAL_SOFT_INV,
  ETX_MOD_PORT_EXTERNAL_UART,
  ETX_MOD_PORT_EXTERNAL_TIMER,
  ETX_MOD_PORT_EXTERNAL_SOFT_INV,
  ETX_MOD_PORT_SPORT,
  ETX_MOD_PORT_SPORT_INV,
  ETX_MOD_PORT_MAX
};

//
// A module port is a pin (single UART RX or TX; PPM pin)
// or pin pair (UART RX/TX)
//
typedef struct {

  // enum ModulePort
  uint8_t port;
  
  // ETX_MOD_TYPE_xx
  uint8_t type;

  // flags bitfield ETX_MOD_DIR_xx
  uint8_t dir_flags;

  union {
    const etx_timer_driver_t  *timer;
    const etx_serial_driver_t *serial;
  } drv;

  void* hw_def;

} etx_module_port_t;

#define BEGIN_MODULE_PORTS()                                            \
  extern const etx_module_port_t _module_ports[];                       \
  const etx_module_port_t _module_ports[] = {

#define END_MODULE_PORTS() };                           \
  extern const uint8_t _n_module_ports;                 \
  const uint8_t _n_module_ports = DIM(_module_ports);

typedef struct {
  const etx_module_port_t* port;
  void* ctx;
} etx_module_driver_t;

typedef struct {

  etx_module_driver_t tx;
  etx_module_driver_t rx;

  void* user_data;

} etx_module_state_t;

// Init the internal module port data
void modulePortInit();

#if defined(CONFIGURABLE_MODULE_PORT)
// Configure an extra port
void modulePortConfigExtra(const etx_module_port_t* port);
#endif

const etx_module_port_t* modulePortFind(uint8_t type, uint8_t port);

// Init module port with params (driver & context stored locally)
etx_module_state_t* modulePortInitSerial(uint8_t moduleIdx, uint8_t port,
                                         const etx_serial_init* params);

// Init module port with params (driver & context stored locally)
etx_module_state_t* modulePortInitTimer(uint8_t moduleIdx, uint8_t port,
                                        const etx_timer_config_t* cfg);

// De-init port and clear data
void modulePortDeInit(etx_module_state_t* st);

// Once initialized, retrieve module serial driver and context
etx_module_state_t* modulePortGetState(uint8_t moduleIdx);

uint8_t modulePortGetModule(etx_module_state_t* st);

inline uint8_t modulePortGetType(const etx_module_driver_t& d) {
  return d.port->type;
}

inline const etx_serial_driver_t* modulePortGetSerialDrv(const etx_module_driver_t& d) {
  return d.port->drv.serial;
}

inline const etx_timer_driver_t* modulePortGetTimerDrv(const etx_module_driver_t& d) {
  return d.port->drv.timer;
}

inline void* modulePortGetCtx(const etx_module_driver_t& d) {
  return d.ctx;
}

