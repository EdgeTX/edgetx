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

#include "module_port.h"
#include "dataconstants.h" // NUM_MODULES

#include <string.h>

extern const etx_module_port_t _module_ports[];
extern const uint8_t _n_module_ports;

static etx_module_state_t _module_states[MAX_MODULES];

// extmoduleGetSerialPort
#include "extmodule_serial_driver.h"

void modulePortInit()
{
  memset(_module_states, 0, sizeof(_module_states));
}

static void modulePortClear(etx_module_state_t* st)
{
  memset(st, 0, sizeof(etx_module_state_t));
}

static void _init_serial_driver(etx_module_driver_t* d, const etx_module_port_t* port,
                                const etx_serial_init* params)
{
  auto drv = port->drv.serial;
  d->ctx = drv->init(port->hw_def, params);
  d->port = port;
}

static void _init_timer_driver(etx_module_driver_t* d, const etx_module_port_t* port,
                               const etx_timer_config_t* cfg)
{
  auto drv = port->drv.timer;
  d->ctx = drv->init(port->hw_def, cfg);
  d->port = port;
}

static const etx_module_port_t* _find_port(uint8_t type, uint8_t port)
{
  uint8_t n_ports = _n_module_ports;
  const etx_module_port_t* p = _module_ports;
  const etx_module_port_t* found_port = nullptr;

  while(n_ports > 0) {

    if (p->type == type && p->port == port) {
      found_port = p;
      break;
    }

    ++p; --n_ports;
  }

  return found_port;
}

etx_module_state_t* modulePortInitSerial(uint8_t moduleIdx, uint8_t port, uint8_t dir,
                                         const etx_serial_init* params)
{
  if (moduleIdx >= NUM_MODULES) return nullptr;

  const etx_module_port_t* found_port = _find_port(ETX_MOD_TYPE_SERIAL, port);
  if (!found_port) return nullptr;

  // TODO: serial port as external module
  // if ((moduleIdx != EXTERNAL_MODULE) || (port != ETX_MOD_PORT_EXTERNAL_UART) ||
  //     !extmoduleGetSerialPort(drv, hw_def))
  
  auto state = &(_module_states[moduleIdx]);

  const uint8_t duplex = ETX_MOD_DIR_TX | ETX_MOD_DIR_RX;
  dir &= duplex;

  if (dir == duplex) {
    _init_serial_driver(&state->tx, found_port, params);
    state->rx = state->tx;
  } else if (dir == ETX_MOD_DIR_TX) {
    _init_serial_driver(&state->tx, found_port, params);
  } else if (dir == ETX_MOD_DIR_RX) {
    _init_serial_driver(&state->rx, found_port, params);
  }

  return state;
}

etx_module_state_t* modulePortInitTimer(uint8_t moduleIdx, uint8_t port,
                                        const etx_timer_config_t* cfg)
{
  if (moduleIdx >= NUM_MODULES) return nullptr;
  
  const etx_module_port_t* found_port = _find_port(ETX_MOD_TYPE_TIMER, port);
  if (!found_port) return nullptr;

  auto state = &(_module_states[moduleIdx]);
  _init_timer_driver(&state->tx, found_port, cfg);

  return state;
}

static void _deinit_driver(etx_module_driver_t* d)
{
  auto p = d->port;
  auto ctx = d->ctx;
    
  if (p->type == ETX_MOD_TYPE_SERIAL) {
    auto drv = p->drv.serial;
    drv->deinit(ctx);
  } else if (p->type == ETX_MOD_TYPE_TIMER) {
    auto drv = p->drv.timer;
    drv->deinit(ctx);
  }
}

void modulePortDeInit(etx_module_state_t* st)
{
  if (st->tx.port != nullptr) {
    _deinit_driver(&st->tx);
  }

  if (st->rx.port != nullptr && st->rx.port != st->tx.port) {
    _deinit_driver(&st->rx);
  }

  modulePortClear(st);
}

etx_module_state_t* modulePortGetState(uint8_t moduleIdx)
{
  if (moduleIdx >= NUM_MODULES) return nullptr;
  return &(_module_states[moduleIdx]);
}

uint8_t modulePortGetModule(etx_module_state_t* st)
{
  return st - _module_states;
}
