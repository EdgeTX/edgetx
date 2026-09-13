/*
 * Copyright (C) EdgeTX
 *
 * Widget Studio sim-only Lua module (simu target + WIDGET_STUDIO only).
 * Provides the low-level simulator controls the harness cannot reach through
 * the public Lua API:
 *   simu.setSwitch(name, state)  - physical switch -1/0/1
 *   simu.setAnalog(name, value)  - ADC input in 0..4096
 *   simu.armCapture(path)        - deterministic one-shot PNG of the next frame
 *   simu.reset()                 - full simulator reset (hot reload)
 *
 * Telemetry injection is intentionally NOT here: the public global
 * setTelemetryValue(id, subId, instance, value, unit, prec, name) already
 * registers real sensors through the real sensor registry, which is what the
 * harness needs for truthful getValue()/getFieldInfo()/CELLS behaviour.
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 */

#include <string.h>
#include <string>

#define LUA_LIB
#include "edgetx.h"
#include "lua_api.h"
#include "lua_states.h"
#include "switches.h"
#include "strhelpers.h"
#include "hal/adc_driver.h"

#if defined(COLORLCD) && defined(SIMU) && defined(WIDGET_STUDIO)

#include "targets/simu/simulib.h"

static int luaSimuSetSwitch(lua_State* L)
{
  const char* name = luaL_checkstring(L, 1);
  int state = luaL_checkinteger(L, 2);
  if (state < -1) state = -1;
  if (state > 1) state = 1;

  uint8_t count = getSwitchCount();
  for (uint8_t i = 0; i < count; i++) {
    char buf[16];
    const char* sname = getSwitchName(buf, i);
    if (sname && !strcmp(sname, name)) {
      simuSetSwitch(i, (int8_t)state);
      lua_pushboolean(L, true);
      return 1;
    }
  }

  lua_pushboolean(L, false);
  return 1;
}

static int luaSimuSetAnalog(lua_State* L)
{
  const char* name = luaL_checkstring(L, 1);
  int value = luaL_checkinteger(L, 2);
  if (value < 0) value = 0;
  if (value > 4096) value = 4096;

  int idx = adcGetInputIdx(name, strlen(name));
  if (idx < 0) {
    lua_pushboolean(L, false);
    return 1;
  }

  simuSetAnalogValue((uint8_t)idx, (uint16_t)value);
  lua_pushboolean(L, true);
  return 1;
}

static int luaSimuArmCapture(lua_State* L)
{
  const char* path = luaL_checkstring(L, 1);
  if (path[0] == '\0') {
    lua_pushboolean(L, false);
    return 1;
  }

  // Resolve a radio-style path (for example /SCREENSHOTS/x.png) through the
  // simulator's configured SD-card mapping.
  std::string real = simuFatfsGetRealPath(path);
  if (real.empty()) {
    lua_pushboolean(L, false);
    return 1;
  }
  simuCaptureArm(real.c_str());

  lua_pushboolean(L, true);
  return 1;
}

static int luaSimuReset(lua_State* L)
{
  // Deliberately async: the native simu main loop consumes the request between
  // frames, so this is safe to call from inside Lua.
  simuRequestReset();
  lua_pushboolean(L, true);
  return 1;
}

extern "C" {
LROT_BEGIN(simulib, NULL, 0)
  LROT_FUNCENTRY( setSwitch, luaSimuSetSwitch )
  LROT_FUNCENTRY( setAnalog, luaSimuSetAnalog )
  LROT_FUNCENTRY( armCapture, luaSimuArmCapture )
  LROT_FUNCENTRY( reset, luaSimuReset )
LROT_END(simulib, NULL, 0)
}

#endif  // COLORLCD && SIMU && WIDGET_STUDIO
