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

//
// Module ports
//

// flags
#define ETX_MOD_DIR_RX      (1 << 0)
#define ETX_MOD_DIR_TX      (1 << 1)
#define ETX_MOD_DIR_TX_RX   (ETX_MOD_DIR_TX | ETX_MOD_DIR_RX)
#define ETX_MOD_FULL_DUPLEX (1 << 2)

#define ETX_MOD_TYPE_NONE   0
#define ETX_MOD_TYPE_TIMER  (1 << 0)
#define ETX_MOD_TYPE_SERIAL (1 << 1)

enum ModulePort : uint8_t {
  ETX_MOD_PORT_UART,      // UART on heartbeat pin
  ETX_MOD_PORT_TIMER,     // PPM or TX on CPPM pin
  ETX_MOD_PORT_SOFT_INV,  // TX soft-serial 
  ETX_MOD_PORT_SPORT,     // UART on S.Port
  ETX_MOD_PORT_SPORT_INV, // RX soft-serial sampled bit-by-bit via timer IRQ on S.PORT
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

  // this is specific to the driver above
  void* hw_def;

  // inverter support: only works on the following ports
  // - ETX_MOD_PORT_UART / ETX_MOD_PORT_SPORT
  void (*set_inverted)(uint8_t enable);

} etx_module_port_t;

typedef struct {
  const etx_module_port_t* const ports;
  void (*set_pwr)(uint8_t on);
  void (*set_bootcmd)(uint8_t on);
  const uint8_t n_ports;
} etx_module_t;

#define BEGIN_MODULES()                                 \
  extern const etx_module_t* const _modules[];          \
  const etx_module_t* const _modules[] = {

#define END_MODULES() };                        \
  extern const uint8_t _n_modules;              \
  const uint8_t _n_modules = DIM(_modules);

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

const etx_module_t* modulePortGetModuleDescription(uint8_t module);

const etx_module_port_t* modulePortFind(uint8_t module, uint8_t type,
                                        uint8_t port, uint8_t polarity,
                                        uint8_t direction);

void modulePortSetPower(uint8_t module, uint8_t enabled);

bool modulePortPowered(uint8_t module);

// Init module port with params (driver & context stored locally)
etx_module_state_t* modulePortInitSerial(uint8_t module, uint8_t port,
                                         const etx_serial_init* params,
                                         bool softserial_fallback);

// Init module port with params (driver & context stored locally)
etx_module_state_t* modulePortInitTimer(uint8_t module, uint8_t port,
                                        const etx_timer_config_t* cfg);

// De-init port and clear data
void modulePortDeInit(etx_module_state_t* st);

// De-init RX part
void modulePortDeInitRxPort(etx_module_state_t* st);

// Once initialized, retrieve module serial driver and context
etx_module_state_t* modulePortGetState(uint8_t module);

uint8_t modulePortGetModule(etx_module_state_t* st);

inline uint8_t modulePortGetType(const etx_module_driver_t& d) {
  return d.port->type;
}

inline const etx_serial_driver_t* modulePortGetSerialDrv(const etx_module_driver_t& d) {
  if (d.port == nullptr) return nullptr;
  return d.port->drv.serial;
}

inline const etx_timer_driver_t* modulePortGetTimerDrv(const etx_module_driver_t& d) {
  if (d.port == nullptr) return nullptr;
  return d.port->drv.timer;
}

inline void* modulePortGetCtx(const etx_module_driver_t& d) {
  return d.ctx;
}

bool modulePortIsPortUsedByModule(uint8_t module, uint8_t port);
bool modulePortIsPortUsed(uint8_t port);

int8_t modulePortGetModuleForPort(uint8_t port);

bool modulePortHasRx(uint8_t module);
