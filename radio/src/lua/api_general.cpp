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

#define LUA_LIB

#include <ctype.h>
#include <stdio.h>
#include "edgetx.h"
#include "stamp.h"
#include "lua_api.h"
#include "api_filesystem.h"
#include "hal/module_port.h"
#include "hal/adc_driver.h"
#include "hal/rotary_encoder.h"
#include "switches.h"
#include "input_mapping.h"
#if defined(LED_STRIP_GPIO)
#include "boards/generic_stm32/rgb_leds.h"
#endif


#if defined(LIBOPENUI)
  #include "libopenui.h"
  #include "api_colorlcd.h"
  #include "standalone_lua.h"
#endif

#include "telemetry/frsky.h"

#if defined(MULTIMODULE)
  #include "telemetry/multi.h"
#endif

#if defined(CROSSFIRE)
  #include "telemetry/crossfire.h"
#endif

#if defined(GHOST)
  #include "telemetry/ghost.h"
#endif

#if defined(SIMU)
  #define RADIO_VERSION FLAVOUR "-simu"
#else
  #define RADIO_VERSION FLAVOUR
#endif

#define VERSION_OSNAME "EdgeTX"

#define FIND_FIELD_DESC  0x01

// see strhelpers.cpp for pre-instantiation of function-template
// getSourceString() for this parametrization
static constexpr uint8_t maxSourceNameLength{16};

// Note:
// - luaRxFifo & luaReceiveData are used only for USB serial
// - otherwise, the AUX serial buffer is used directly
//
static Fifo<uint8_t, LUA_FIFO_SIZE>* luaRxFifo = nullptr;

static int luaRxFifoGetByte(void*, uint8_t* data)
{
  if (!luaRxFifo) return -1;
  return luaRxFifo->pop(*data);
}

void luaAllocRxFifo()
{
  if (!luaRxFifo) {
    auto fifo = new Fifo<uint8_t, LUA_FIFO_SIZE>();
    luaRxFifo = fifo;
    luaSetGetSerialByte(nullptr, luaRxFifoGetByte);
  }
}

void luaFreeRxFifo()
{
  auto fifo = luaRxFifo;
  luaSetGetSerialByte(nullptr, nullptr);
  luaRxFifo = nullptr;
  delete(fifo);
}

void luaReceiveData(uint8_t* buf, uint32_t len)
{
  if (luaRxFifo) {
    while(len--) luaRxFifo->push(*buf++);
  }
}

static void (*luaSendDataCb)(void*, uint8_t) = nullptr;
static void* luaSendDataCtx = nullptr;

void luaSetSendCb(void* ctx, void (*cb)(void*, uint8_t))
{
  luaSendDataCb = nullptr;
  luaSendDataCtx = ctx;
  luaSendDataCb = cb;
}

static int (*luaGetSerialByte)(void*, uint8_t*) = nullptr;
static void* luaGetSerialByteCtx = nullptr;

void luaSetGetSerialByte(void* ctx, int (*fct)(void*, uint8_t*))
{
  luaGetSerialByte = nullptr;
  luaGetSerialByteCtx = ctx;
  luaGetSerialByte = fct;
}

/*luadoc
@function getVersion()

Return OpenTX version

@retval string OpenTX version (ie "2.1.5")

@retval multiple values (available since 2.1.7):
 * (string) OpenTX version (ie "2.1.5")
 * (string) radio type: `x12s`, `x10`, `x9e`, `x9d+`, `x9d` or `x7`.
If running in simulator the "-simu" is added
 * (number) major version (ie 2 if version 2.1.5)
 * (number) minor version (ie 1 if version 2.1.5)
 * (number) revision number (ie 5 if version 2.1.5)
Since EdgeTX 2.4.0, sixth value added
 * (string) OS name (i.e. EdgeTX or nil if OpenTX)

@status current Introduced in 2.0.0, expanded in 2.1.7, radio type strings changed in 2.2.0, os name added in EdgeTX 2.4.0

### Example

This example also runs in OpenTX versions where the function returned only one value:

```lua
local function run(event)
  local ver, radio, maj, minor, rev, osname = getVersion()
  print("version: "..ver)
  if radio then print ("radio: "..radio) end
  if maj then print ("maj: "..maj) end
  if minor then print ("minor: "..minor) end
  if rev then print ("rev: "..rev) end
  if osname then print ("osname: "..osname) end
  return 1
end

return {  run=run }
```
Output of the above script in simulator:
```
version: 2.4.0
radio: tx16s-simu
maj: 2
minor: 4
rev: 0
osname: EdgeTX
```
*/
static int luaGetVersion(lua_State * L)
{
  lua_pushstring(L, VERSION);
  lua_pushstring(L, RADIO_VERSION);
  lua_pushinteger(L, VERSION_MAJOR);
  lua_pushinteger(L, VERSION_MINOR);
  lua_pushinteger(L, VERSION_REVISION);
  lua_pushstring(L, VERSION_OSNAME);
  return 6;
}

/*luadoc
@function getTime()

Return the time since the radio was started in multiple of 10ms

@retval number Number of 10ms ticks since the radio was started Example:
run time: 12.54 seconds, return value: 1254

The timer internally uses a 32-bit counter which is enough for 497 days so
overflows will not happen.

@status current Introduced in 2.0.0
*/
static int luaGetTime(lua_State * L)
{
  lua_pushinteger(L, get_tmr10ms());
  return 1;
}

void luaPushDateTime(lua_State * L, uint32_t year, uint32_t mon, uint32_t day,
                            uint32_t hour, uint32_t min, uint32_t sec)
{
  uint32_t hour12 = hour;

  if (hour == 0) {
    hour12 = 12;
  }
  else if (hour > 12) {
    hour12 = hour - 12;
  }
  lua_createtable(L, 0, 8);
  lua_pushtableinteger(L, "year", year);
  lua_pushtableinteger(L, "mon", mon);
  lua_pushtableinteger(L, "day", day);
  lua_pushtableinteger(L, "hour", hour);
  lua_pushtableinteger(L, "min", min);
  lua_pushtableinteger(L, "sec", sec);
  lua_pushtableinteger(L, "hour12", hour12);
  if (hour < 12) {
    lua_pushtablestring(L, "suffix", "am");
  }
  else {
    lua_pushtablestring(L, "suffix", "pm");
  }
}

/*luadoc
@function getDateTime()

Return current system date and time that is kept by the RTC unit

@retval table current date and time, table elements:
 * `year` (number) year
 * `mon` (number) month
 * `day` (number) day of month
 * `hour` (number) hours
 * `hour12` (number) hours in US format
 * `min` (number) minutes
 * `sec` (number) seconds
 * `suffix` (text) am or pm
*/
static int luaGetDateTime(lua_State * L)
{
  struct gtm utm;
  gettime(&utm);
  luaPushDateTime(L, utm.tm_year + TM_YEAR_BASE, utm.tm_mon + 1, utm.tm_mday, utm.tm_hour, utm.tm_min, utm.tm_sec);
  return 1;
}

/*luadoc
@function getRtcTime()

Return current RTC system date as unix timstamp (in seconds since 1. Jan 1970)

Please note the RTC timestamp is kept internally as a 32bit integer, which will overflow
in 2038.

@retval number Number of seconds elapsed since 1. Jan 1970
*/

#if defined(RTCLOCK)
static int luaGetRtcTime(lua_State * L)
{
  lua_pushinteger(L, g_rtcTime);
  return 1;
}
#endif

static void luaPushLatLon(lua_State* L, TelemetrySensor & telemetrySensor, TelemetryItem & telemetryItem)
/* result is lua table containing members ["lat"] and ["lon"] as lua_Number (doubles) in decimal degrees */
{
  lua_createtable(L, 0, 5);
  lua_pushtablenumber(L, "lat", telemetryItem.gps.latitude * 0.000001); // floating point multiplication is faster than division
  lua_pushtablenumber(L, "pilot-lat", telemetryItem.pilotLatitude * 0.000001);
  lua_pushtablenumber(L, "lon", telemetryItem.gps.longitude * 0.000001);
  lua_pushtablenumber(L, "pilot-lon", telemetryItem.pilotLongitude * 0.000001);

  int8_t delay = telemetryItem.getDelaySinceLastValue();
  if (delay >= 0)
    lua_pushtableinteger(L, "delay", delay);
}

static void luaPushTelemetryDateTime(lua_State* L, TelemetrySensor & telemetrySensor, TelemetryItem & telemetryItem)
{
  luaPushDateTime(L, telemetryItem.datetime.year, telemetryItem.datetime.month, telemetryItem.datetime.day,
                  telemetryItem.datetime.hour, telemetryItem.datetime.min, telemetryItem.datetime.sec);
}

static void luaPushCells(lua_State* L, TelemetrySensor & telemetrySensor, TelemetryItem & telemetryItem)
{
  if (telemetryItem.cells.count == 0)
    lua_pushinteger(L, (int)0); // returns zero if no cells
  else {
    lua_createtable(L, telemetryItem.cells.count, 0);
    for (int i = 0; i < telemetryItem.cells.count; i++) {
      lua_pushinteger(L, i + 1);
      lua_pushnumber(L, telemetryItem.cells.values[i].value * 0.01f);
      lua_settable(L, -3);
    }
  }
}

void luaGetValueAndPush(lua_State* L, int src)
{
  getvalue_t value = getValue(src); // ignored for GPS, DATETIME, and CELLS

  if (src >= MIXSRC_FIRST_TELEM && src <= MIXSRC_LAST_TELEM) {
    div_t qr = div(src-MIXSRC_FIRST_TELEM, 3);
    // telemetry values
    if (TELEMETRY_STREAMING() && telemetryItems[qr.quot].isAvailable()) {
      TelemetrySensor & telemetrySensor = g_model.telemetrySensors[qr.quot];
      switch (telemetrySensor.unit) {
        case UNIT_GPS:
          luaPushLatLon(L, telemetrySensor, telemetryItems[qr.quot]);
          break;
        case UNIT_DATETIME:
          luaPushTelemetryDateTime(L, telemetrySensor, telemetryItems[qr.quot]);
          break;
        case UNIT_TEXT:
          lua_pushstring(L, telemetryItems[qr.quot].text);
          break;
        case UNIT_CELLS:
          if (qr.rem == 0) {
            luaPushCells(L, telemetrySensor, telemetryItems[qr.quot]);
            break;
          }
          // deliberate no break here to properly return `Cels-` and `Cels+`
        default:
          if (telemetrySensor.prec > 0)
            lua_pushnumber(L, float(value)/telemetrySensor.getPrecDivisor());
          else
            lua_pushinteger(L, value);
          break;
      }
    }
    else {
      // telemetry not working, return zero for telemetry sources
      lua_pushinteger(L, (int)0);
    }
  }
  else if (src == MIXSRC_TX_VOLTAGE) {
    lua_pushnumber(L, float(value) * 0.1f);
  }
  #if defined(GVARS)
  else if(src >= MIXSRC_FIRST_GVAR && src <= MIXSRC_LAST_GVAR) {
   if(g_model.gvars[src - MIXSRC_FIRST_GVAR].prec)
     lua_pushnumber(L, float(value) * 0.1f);    // prec "0.0"
  else
     lua_pushinteger(L, value);                 // prec "0.-"
  } 
  #endif
  else {
    lua_pushinteger(L, value);
  }
}

struct LuaSingleField {
  uint16_t id;
  const char* name;
  const char* desc;
};

const LuaSingleField luaSingleFields[] = {
#if defined(IMU)
    {MIXSRC_TILT_X, "tiltx", "Tilt X"},
    {MIXSRC_TILT_Y, "tilty", "Tilt Y"},
#endif

#if defined(PCBHORUS)
    {MIXSRC_SPACEMOUSE_A, "sma", "SpaceMouse A"},
    {MIXSRC_SPACEMOUSE_B, "smb", "SpaceMouse B"},
    {MIXSRC_SPACEMOUSE_C, "smc", "SpaceMouse C"},
    {MIXSRC_SPACEMOUSE_D, "smd", "SpaceMouse D"},
    {MIXSRC_SPACEMOUSE_E, "sme", "SpaceMouse E"},
    {MIXSRC_SPACEMOUSE_F, "smf", "SpaceMouse F"},
#endif

    {MIXSRC_MIN, "min", "MIN"},
    {MIXSRC_MAX, "max", "MAX"},

    {MIXSRC_TX_VOLTAGE, "tx-voltage", "Transmitter battery voltage [volts]"},
    {MIXSRC_TX_TIME, "clock", "RTC clock [minutes from midnight]"},
};

// Legacy input names
// TODO: move to some HAL/driver functions
#include "lua_inputs.inc"

struct LuaMultipleField {
  uint16_t id;
  const char* name;
  const char* desc;
  uint8_t count;
};

// The list of Lua fields that have a range of values
const LuaMultipleField luaMultipleFields[] = {
    {MIXSRC_FIRST_INPUT, "input", "Input [I%d]", MAX_INPUTS},
    {MIXSRC_FIRST_LUA, "lua", "Lua mix output %d", MAX_SCRIPTS * MAX_SCRIPT_OUTPUTS},
    {MIXSRC_FIRST_LOGICAL_SWITCH, "ls", "Logical switch L%d", MAX_LOGICAL_SWITCHES},
    {MIXSRC_FIRST_TRAINER, "trn", "Trainer input %d", MAX_TRAINER_CHANNELS},
    {MIXSRC_FIRST_CH, "ch", "Channel CH%d", MAX_OUTPUT_CHANNELS},
    {MIXSRC_FIRST_GVAR, "gvar", "Global variable %d", MAX_GVARS},
    {MIXSRC_FIRST_TELEM, "telem", "Telemetry sensor %d", MAX_TELEMETRY_SENSORS},
    {MIXSRC_FIRST_TIMER, "timer", "Timer %d value [seconds]", MAX_TIMERS},
    {MIXSRC_FIRST_HELI, "cyc", "Cyclic %d", 3},
};

static bool _searchSingleFieldsByName(const char* name, LuaField& field,
                                unsigned int flags,
                                const LuaSingleField* fields, size_t n_fields)
{
  for (unsigned int n = 0; n < n_fields; ++n) {
    if (!strcmp(name, fields[n].name)) {
      field.id = fields[n].id;
      if (flags & FIND_FIELD_DESC) {
        strncpy(field.desc, fields[n].desc, sizeof(field.desc) - 1);
        field.desc[sizeof(field.desc) - 1] = '\0';
      } else {
        field.desc[0] = '\0';
      }
      return true;
    }
  }

  return false;
}

/**
  Return field data for a given field name
*/
bool luaFindFieldByName(const char * name, LuaField & field, unsigned int flags)
{
  auto len = strlen(name);
  strncpy(field.name, name, sizeof(field.name) - 1);
  field.name[sizeof(field.name) - 1] = '\0';

  // hardware specific inputs
  if (_searchSingleFieldsByName(name, field, flags, _lua_inputs, DIM(_lua_inputs)))
    return true;
  
  // well known single fields
  if (_searchSingleFieldsByName(name, field, flags, luaSingleFields, DIM(luaSingleFields)))
    return true;

  // check switches from 'sa' to 'sz'
  // TODO: does not work with function switches!
  if (len == 2 && name[0] == 's' && name[1] >= 'a' && name[1] <= 'z') {
    auto c = name[1] - 'a' + 'A';
    auto sw_idx = switchLookupIdx(c);
    if (sw_idx >= 0) {
      field.id = MIXSRC_FIRST_SWITCH + sw_idx;
      if (flags & FIND_FIELD_DESC) {
        snprintf(field.desc, sizeof(field.desc)-1, "Switch %c", c);
        field.desc[sizeof(field.desc)-1] = '\0';
      } else {
        field.desc[0] = '\0';
      }
      return true;
    }
  }
  
  // search in multiples
  for (unsigned int n=0; n<DIM(luaMultipleFields); ++n) {
    const char * fieldName = luaMultipleFields[n].name;
    unsigned int fieldLen = strlen(fieldName);
    if (!strncmp(name, fieldName, fieldLen)) {
      unsigned int index;
      if (len >= fieldLen + 1 && isdigit(name[fieldLen])) {
        index = name[fieldLen] - '0';
        if (len >= fieldLen + 2 && isdigit(name[fieldLen + 1])) {
          index = 10 * index + (name[fieldLen + 1] - '0');
        }
        index -= 1;
      }
      else {
        continue;
      }
      if (index < luaMultipleFields[n].count) {
        if(luaMultipleFields[n].id == MIXSRC_FIRST_TELEM) {
          index *= 3;
          if (name[len - 1] == '-')
            index += 1;
          else if (name[len - 1] == '+')
            index += 2;
        }
        field.id = luaMultipleFields[n].id + index;
        if (flags & FIND_FIELD_DESC) {
          snprintf(field.desc, sizeof(field.desc)-1, luaMultipleFields[n].desc, index+1);
          field.desc[sizeof(field.desc)-1] = '\0';
        }
        else {
          field.desc[0] = '\0';
        }
        return true;
      }
    }
  }

  // search in telemetry
  field.desc[0] = '\0';
  for (int i = 0; i < MAX_TELEMETRY_SENSORS; i++) {
    if (isTelemetryFieldAvailable(i)) {
      const char* sensorName = g_model.telemetrySensors[i].label;
      int len = strnlen(sensorName, TELEM_LABEL_LEN);
      if (!strncmp(sensorName, name, len)) {
        if (name[len] == '\0') {
          field.id = MIXSRC_FIRST_TELEM + 3 * i;
          field.desc[0] = '\0';
          return true;
        } else if (name[len] == '-' && name[len + 1] == '\0') {
          field.id = MIXSRC_FIRST_TELEM + 3 * i + 1;
          field.desc[0] = '\0';
          return true;
        } else if (name[len] == '+' && name[len + 1] == '\0') {
          field.id = MIXSRC_FIRST_TELEM + 3 * i + 2;
          field.desc[0] = '\0';
          return true;
        }
      }
    }
  }

  return false;  // not found
}

static bool _searchSingleFieldsById(int id, LuaField& field,
                                unsigned int flags,
                                const LuaSingleField* fields, size_t n_fields)
{
  for (unsigned int n = 0; n < n_fields; ++n) {
    if (id == fields[n].id) {
      strncpy(field.name, fields[n].name, sizeof(field.name) - 1);
      if (flags & FIND_FIELD_DESC) {
        strncpy(field.desc, fields[n].desc, sizeof(field.desc) - 1);
        field.desc[sizeof(field.desc) - 1] = '\0';
      }
      return true;
    }
  }

  return false;
}

// Return field data for a given field id
bool luaFindFieldById(int id, LuaField & field, unsigned int flags)
{
  field.id = id;
  field.name[sizeof(field.name) - 1] = '\0';
  field.desc[0] = '\0';

  // hardware specific inputs
  if (_searchSingleFieldsById(id, field, flags, _lua_inputs, DIM(_lua_inputs)))
    return true;
  
  // well known single fields
  if (_searchSingleFieldsById(id, field, flags, luaSingleFields, DIM(luaSingleFields)))
    return true;

  // search in multiples
  for (unsigned int n = 0; n < DIM(luaMultipleFields); ++n) {
    int index = id - luaMultipleFields[n].id;
    if (0 <= index && index < luaMultipleFields[n].count) {
      int index2 = 0;
      if(luaMultipleFields[n].id == MIXSRC_FIRST_TELEM) {
        index2 = index % 3;
        index /= 3;
      }
      switch (index2) {
        case 0:
          snprintf(field.name, sizeof(field.name), "%s%i", luaMultipleFields[n].name, index + 1);
          break;
        case 1:
          snprintf(field.name, sizeof(field.name), "%s%i-", luaMultipleFields[n].name, index + 1);
          break;
        case 2:
          snprintf(field.name, sizeof(field.name), "%s%i+", luaMultipleFields[n].name, index + 1);
      }
      if (flags & FIND_FIELD_DESC)
        snprintf(field.desc, sizeof(field.desc), luaMultipleFields[n].desc, index + 1);
      return true;
    }
  }

  // search in telemetry
  for (int i = 0; i < MAX_TELEMETRY_SENSORS; i++) {
    if (isTelemetryFieldAvailable(i)) {
      int index = id - (MIXSRC_FIRST_TELEM + 3 * i);
      if (0 <= index && index < 3) {
        const char* sensorName = g_model.telemetrySensors[i].label;
        switch (index) {
          case 0:
            snprintf(field.name, sizeof(field.name), "%s", sensorName);
            break;
          case 1:
            snprintf(field.name, sizeof(field.name), "%s-", sensorName);
            break;
          case 2:
            snprintf(field.name, sizeof(field.name), "%s+", sensorName);
        }
      }
    }
  }

  return false;  // not found
}

/*luadoc
@function getFieldInfo(source)

Return detailed information about field (source)

The list of valid sources is available:

| OpenTX Version | Radio |
|----------------|-------|
| 2.0 | [all](http://downloads-20.open-tx.org/firmware/lua_fields.txt) |
| 2.1 | [X9D and X9D+](http://downloads-21.open-tx.org/firmware/lua_fields_taranis.txt), [X9E](http://downloads-21.open-tx.org/firmware/lua_fields_taranis_x9e.txt) |
| 2.2 | [X9D and X9D+](http://downloads.open-tx.org/2.2/release/firmware/lua_fields_x9d.txt), [X9E](http://downloads.open-tx.org/2.2/release/firmware/lua_fields_x9e.txt), [Horus](http://downloads.open-tx.org/2.2/release/firmware/lua_fields_x12s.txt) |
| 2.3 | [X9D and X9D+](http://downloads.open-tx.org/2.3/release/firmware/lua_fields_x9d.txt), [X9E](http://downloads.open-tx.org/2.3/release/firmware/lua_fields_x9e.txt), [X7](http://downloads.open-tx.org/2.3/release/firmware/lua_fields_x7.txt), [Horus](http://downloads.open-tx.org/2.3/release/firmware/lua_fields_x12s.txt) |

@param source can be an index (number) (which was obtained by `getFieldInfo` or `getSourceIndex`) or a name (string) of the source.

@retval table information about requested field, table elements:
 * `id`   (number) field identifier
 * `name` (string) field name
 * `desc` (string) field description
 * `unit` (number) unit identifier [Full list](../appendix/units.html)

@retval nil the requested field was not found

@status current Introduced in 2.0.8, 'unit' field added in 2.2.0, and argument also can be an index number as of 2.6.0
*/
static int luaGetFieldInfo(lua_State * L)
{
  bool found;
  LuaField field;

  if (lua_type(L, 1) == LUA_TNUMBER)
    found = luaFindFieldById(luaL_checkinteger(L, 1), field, FIND_FIELD_DESC);
  else
    found = luaFindFieldByName(luaL_checkstring(L, 1), field, FIND_FIELD_DESC);

  if (found) {
    lua_newtable(L);
    lua_pushtableinteger(L, "id", field.id);
    lua_pushtablestring(L, "name", field.name);
    lua_pushtablestring(L, "desc", field.desc);
    if (field.id >= MIXSRC_FIRST_TELEM && field.id <= MIXSRC_LAST_TELEM) {
      TelemetrySensor & telemetrySensor = g_model.telemetrySensors[(int)((field.id-MIXSRC_FIRST_TELEM)/3)];
      lua_pushtableinteger(L, "unit", telemetrySensor.unit);
    }
    return 1;
  }
  return 0;
}

/*luadoc
@function getValue(source)

Returns the value of a source.

The list of fixed sources:

| OpenTX Version | Radio |
|----------------|-------|
| 2.0 | [all](http://downloads-20.open-tx.org/firmware/lua_fields.txt) |
| 2.1 | [X9D and X9D+](http://downloads-21.open-tx.org/firmware/lua_fields_taranis.txt), [X9E](http://downloads-21.open-tx.org/firmware/lua_fields_taranis_x9e.txt) |
| 2.2 | [X9D and X9D+](http://downloads.open-tx.org/2.2/release/firmware/lua_fields_x9d.txt), [X9E](http://downloads.open-tx.org/2.2/release/firmware/lua_fields_x9e.txt), [Horus](http://downloads.open-tx.org/2.2/release/firmware/lua_fields_x12s.txt) |
| 2.3 | [X9D and X9D+](http://downloads.open-tx.org/2.3/release/firmware/lua_fields_x9d.txt), [X9E](http://downloads.open-tx.org/2.3/release/firmware/lua_fields_x9e.txt), [X7](http://downloads.open-tx.org/2.3/release/firmware/lua_fields_x7.txt), [Horus](http://downloads.open-tx.org/2.3/release/firmware/lua_fields_x12s.txt) |

In OpenTX 2.1.x the telemetry sources no longer have a predefined name.
To get a telemetry value simply use it's sensor name. For example:
 * Altitude sensor has a name "Alt"
 * to get the current altitude use the source "Alt"
 * to get the minimum altitude use the source "Alt-", to get the maximum use "Alt+"

@param source can be an index (number) (which was obtained by `getFieldInfo` or `getSourceIndex`) or a name (string) of the source.

@retval value current source value (number). Zero is returned for:
 * non-existing sources
 * for all telemetry source when the telemetry stream is not received
 * far all non allowed sensors while FAI MODE is active

@retval table GPS position is returned in a table:
 * `lat` (number) latitude, positive is North
 * `lon` (number) longitude, positive is East
 * `pilot-lat` (number) pilot latitude, positive is North
 * `pilot-lon` (number) pilot longitude, positive is East

@retval table GPS date/time, see getDateTime()

@retval table Cells are returned in a table
(except where no cells were detected in which
case the returned value is 0):
 * table has one item for each detected cell:
  * key (number) cell number (1 to number of cells)
  * value (number) current cell voltage

@status current Introduced in 2.0.0, changed in 2.1.0, `Cels+` and
`Cels-` added in 2.1.9

@notice Getting a value by its numerical identifier is faster then by its name.
While `Cels` sensor returns current values of all cells in a table, a `Cels+` or
`Cels-` will return a single value - the maximum or minimum Cels value.
*/
static int luaGetValue(lua_State * L)
{
  int src = MIXSRC_NONE;
  if (lua_isnumber(L, 1)) {
    src = luaL_checkinteger(L, 1);
  }
  else {
    // convert from field name to its id
    const char *name = luaL_checkstring(L, 1);
    LuaField field;
    bool found = luaFindFieldByName(name, field);
    if (found) {
      src = field.id;
    }
  }
  luaGetValueAndPush(L, src);
  return 1;
}

/*luadoc
@function getSourceValue(source)

Returns the value of a source. Superseeds getValue.

The list of fixed sources:

| OpenTX Version | Radio |
|----------------|-------|
| 2.0 | [all](http://downloads-20.open-tx.org/firmware/lua_fields.txt) |
| 2.1 | [X9D and X9D+](http://downloads-21.open-tx.org/firmware/lua_fields_taranis.txt), [X9E](http://downloads-21.open-tx.org/firmware/lua_fields_taranis_x9e.txt) |
| 2.2 | [X9D and X9D+](http://downloads.open-tx.org/2.2/release/firmware/lua_fields_x9d.txt), [X9E](http://downloads.open-tx.org/2.2/release/firmware/lua_fields_x9e.txt), [Horus](http://downloads.open-tx.org/2.2/release/firmware/lua_fields_x12s.txt) |
| 2.3 | [X9D and X9D+](http://downloads.open-tx.org/2.3/release/firmware/lua_fields_x9d.txt), [X9E](http://downloads.open-tx.org/2.3/release/firmware/lua_fields_x9e.txt), [X7](http://downloads.open-tx.org/2.3/release/firmware/lua_fields_x7.txt), [Horus](http://downloads.open-tx.org/2.3/release/firmware/lua_fields_x12s.txt) |

In OpenTX 2.1.x the telemetry sources no longer have a predefined name.
To get a telemetry value simply use it's sensor name. For example:
 * Altitude sensor has a name "Alt"
 * to get the current altitude use the source "Alt"
 * to get the minimum altitude use the source "Alt-", to get the maximum use "Alt+"

@param source can be an index (number) (which was obtained by `getFieldInfo` or `getSourceIndex`) or a name (string) of the source.

@retval value current source value (number), or last known telemetry item value.

value is nil for non-existing sources. all non-allowed sensors while FAI MODE is active, or if a telemetry item value has never been received

value is a table for GPS position:

 * `lat` (number) latitude, positive is North
 * `lon` (number) longitude, positive is East
 * `pilot-lat` (number) pilot latitude, positive is North
 * `pilot-lon` (number) pilot longitude, positive is East

value is a table for date/time, see getDateTime()

value is a table for battery cells(except where no cells were detected in which case the returned value is 0):
 * table has one item for each detected cell:
  * key (number) cell number (1 to number of cells)
  * value (number) current cell voltage

@retval isCurrent is true for telemetry sources that are within the "Sensor Lost" duration and telemetry is streaming . Always true for non-telemetry items.

@retval isFresh is true for telemetry sources which have been recently updated and telemetry is streaming. Always true for non-telemetry items.

@status current Introduced in 2.0.0, changed in 2.1.0, `Cels+` and `Cels-` added in 2.1.9

@notice Getting a value by its numerical identifier is much faster than by its name.
While `Cels` sensor returns current values of all cells in a table, a `Cels+` or
`Cels-` will return a single value - the maximum or minimum Cels value.
*/

static int luaGetSourceValue(lua_State * L)
{
  // Get source id
  int src = MIXSRC_NONE;
  if (lua_isnumber(L, 1)) {
    src = luaL_checkinteger(L, 1);
  }
  else {
    // convert from field name to its id
    const char *name = luaL_checkstring(L, 1);
    LuaField field;
    bool found = luaFindFieldByName(name, field);
    if (found) {
      src = field.id;
    }
  }

  // Get source value. Ignored for GPS, DATETIME, and CELLS
  bool valid = true;
  getvalue_t value = getValue(src, &valid);

  if (!valid)
  {
    return 0;
  }

  if (src >= MIXSRC_FIRST_TELEM && src <= MIXSRC_LAST_TELEM) {
    div_t qr = div(src-MIXSRC_FIRST_TELEM, 3);
    // telemetry values
    if (telemetryItems[qr.quot].isAvailable()) {
      TelemetrySensor & telemetrySensor = g_model.telemetrySensors[qr.quot];
      switch (telemetrySensor.unit) {
        case UNIT_GPS:
          luaPushLatLon(L, telemetrySensor, telemetryItems[qr.quot]);
          break;
        case UNIT_DATETIME:
          luaPushTelemetryDateTime(L, telemetrySensor, telemetryItems[qr.quot]);
          break;
        case UNIT_TEXT:
          lua_pushstring(L, telemetryItems[qr.quot].text);
          break;
        case UNIT_CELLS:
          if (qr.rem == 0) {
            // Return nil if there are no cells
            if (telemetryItems[qr.quot].cells.count == 0) {
              lua_pushnil(L);
              lua_pushboolean(L, false);
              lua_pushboolean(L, false);
              return 3;
            }
            luaPushCells(L, telemetrySensor, telemetryItems[qr.quot]);
            break;
          }
          // deliberate no break here to properly return `Cels-` and `Cels+`
        default:
          if (telemetrySensor.prec > 0)
            lua_pushnumber(L, float(value)/telemetrySensor.getPrecDivisor());
          else
            lua_pushinteger(L, value);
          break;
      }
      lua_pushboolean(L, !telemetryItems[qr.quot].isOld());
      lua_pushboolean(L, telemetryItems[qr.quot].isFresh());
    }
    else { // telemetry is not available
      return 0;
    }
  }
  else if (src == MIXSRC_TX_VOLTAGE) {
    lua_pushnumber(L, float(value) * 0.1f);
    lua_pushboolean(L, true);
    lua_pushboolean(L, true);
  }
  else {
    lua_pushinteger(L, value);
    lua_pushboolean(L, true);
    lua_pushboolean(L, true);
  }
  return 3;
}

/*luadoc
@function getRotEncSpeed()

Return rotary encoder current speed

@retval number in list: ROTENC_LOWSPEED, ROTENC_MIDSPEED, ROTENC_HIGHSPEED
        returns ROTENC_LOWSPEED on radio without rotary encoder

@status current Introduced in 2.3.10
*/
static int luaGetRotEncSpeed(lua_State * L)
{
  lua_pushinteger(L, max(rotaryEncoderGetAccel(), (int8_t)1));
  return 1;
}

/*luadoc
@function getRotEncMode()

Return rotary encoder mode

@retval number in list: Normal = 0, Both V and H inverted = 1, V-N = 2, V-A = 3
  return 0 on radio without rotary encoder

@status current Introduced in 2.8.0
*/
static int luaGetRotEncMode(lua_State * L)
{
#if defined(ROTARY_ENCODER_NAVIGATION) && !defined(USE_HATS_AS_KEYS)
  lua_pushinteger(L, g_eeGeneral.rotEncMode);
#else
  lua_pushinteger(L, 0);
#endif
  return 1;
}

/*luadoc
@function sportTelemetryPop()

Pops a received SPORT packet from the queue. Please note that only packets using a data ID within 0x5000 to 0x50FF
(frame ID == 0x10), as well as packets with a frame ID equal 0x32 (regardless of the data ID) will be passed to
the LUA telemetry receive queue.

@retval nil queue does not contain any (or enough) bytes to form a whole packet

@retval multiple returns 4 values:
 * sensor ID (number)
 * frame ID (number)
 * data ID (number)
 * value (number)

@status current Introduced in 2.2.0
*/
static int luaSportTelemetryPop(lua_State * L)
{
  if (!luaInputTelemetryFifo) {
    luaInputTelemetryFifo = new Fifo<uint8_t, LUA_TELEMETRY_INPUT_FIFO_SIZE>();
    if (!luaInputTelemetryFifo) {
      return 0;
    }
  }

  if (luaInputTelemetryFifo->size() >= sizeof(SportTelemetryPacket)) {
    SportTelemetryPacket packet;
    for (uint8_t i=0; i<sizeof(packet); i++) {
      luaInputTelemetryFifo->pop(packet.raw[i]);
    }
    lua_pushinteger(L, packet.physicalId);
    lua_pushinteger(L, packet.primId);
    lua_pushinteger(L, packet.dataId);
    lua_pushinteger(L, packet.value);
    return 4;
  }

  return 0;
}

#define BIT(x, index) (((x) >> index) & 0x01)
uint8_t getDataId(uint8_t physicalId)
{
  uint8_t result = physicalId;
  result += (BIT(physicalId, 0) ^ BIT(physicalId, 1) ^ BIT(physicalId, 2)) << 5;
  result += (BIT(physicalId, 2) ^ BIT(physicalId, 3) ^ BIT(physicalId, 4)) << 6;
  result += (BIT(physicalId, 0) ^ BIT(physicalId, 2) ^ BIT(physicalId, 4)) << 7;
  return result;
}

/*luadoc
@function sportTelemetryPush()

This functions allows for sending SPORT telemetry data toward the receiver,
and more generally, to anything connected SPORT bus on the receiver or transmitter.

When called without parameters, it will only return the status of the output buffer without sending anything.

@param sensorId  physical sensor ID

@param frameId   frame ID

@param dataId    data ID

@param value     value

@retval boolean  data queued in output buffer or not.

@retval nil      incorrect telemetry protocol.

@status current Introduced in 2.2.0, retval nil added in 2.3.4
*/

static bool _supports_sport(uint8_t module)
{
  return IS_NATIVE_FRSKY_PROTOCOL(module) ||
    (isModuleMultimodule(module) &&
     (IS_D16_MULTI(module) || IS_R9_MULTI(module)));
}

static int luaSportTelemetryPush(lua_State * L)
{
  bool extmod = _supports_sport(EXTERNAL_MODULE);
  bool intmod = _supports_sport(INTERNAL_MODULE);
  
  // dirty hack until 2 simultanous protocols are supported
  if (!extmod && !intmod) {
    lua_pushnil(L);
    return 1;
  }

  if (lua_gettop(L) == 0) {
    lua_pushboolean(L, outputTelemetryBuffer.isAvailable());
    return 1;
  }
  else if (lua_gettop(L) > int(sizeof(SportTelemetryPacket))) {
    lua_pushboolean(L, false);
    return 1;
  }

  uint16_t dataId = luaL_checkinteger(L, 3);

  if (outputTelemetryBuffer.isAvailable()) {
    for (uint8_t i=0; i<MAX_TELEMETRY_SENSORS; i++) {
      TelemetrySensor & sensor = g_model.telemetrySensors[i];
      if (sensor.id == dataId) {
        if (sensor.frskyInstance.rxIndex == TELEMETRY_ENDPOINT_SPORT) {
          SportTelemetryPacket packet;
          packet.physicalId = getDataId(luaL_checkinteger(L, 1));
          packet.primId = luaL_checkinteger(L, 2);
          packet.dataId = dataId;
          packet.value = luaL_checkinteger(L, 4);
          outputTelemetryBuffer.pushSportPacketWithBytestuffing(packet);
        }
        else {
          outputTelemetryBuffer.sport.physicalId = getDataId(luaL_checkinteger(L, 1));
          outputTelemetryBuffer.sport.primId = luaL_checkinteger(L, 2);
          outputTelemetryBuffer.sport.dataId = dataId;
          outputTelemetryBuffer.sport.value = luaL_checkinteger(L, 4);
        }
        outputTelemetryBuffer.setDestination(sensor.frskyInstance.rxIndex);
        lua_pushboolean(L, true);
        return 1;
      }
    }

    // sensor not found, we send the frame to the SPORT line
    {
      SportTelemetryPacket packet;
      packet.physicalId = getDataId(luaL_checkinteger(L, 1));
      packet.primId = luaL_checkinteger(L, 2);
      packet.dataId = dataId;
      packet.value = luaL_checkinteger(L, 4);
      outputTelemetryBuffer.pushSportPacketWithBytestuffing(packet);
#if defined(PXX2) && defined(HARDWARE_EXTERNAL_MODULE)
      uint8_t destination = (intmod ? INTERNAL_MODULE : EXTERNAL_MODULE);
      outputTelemetryBuffer.setDestination(isModulePXX2(destination)
                                           ? (destination << 2)
                                           : TELEMETRY_ENDPOINT_SPORT);
#else
      outputTelemetryBuffer.setDestination(TELEMETRY_ENDPOINT_SPORT);
#endif
      lua_pushboolean(L, true);
      return 1;
    }
  }

  lua_pushboolean(L, false);
  return 1;
}

#if defined(PXX2)
/*luadoc
@function accessTelemetryPush()

This functions allows for sending SPORT / ACCESS telemetry data toward the receiver,
and more generally, to anything connected SPORT bus on the receiver or transmitter.

When called without parameters, it will only return the status of the output buffer without sending anything.

@param module    module index (0 = internal, 1 = external)

@param rxUid     receiver index

@param sensorId  physical sensor ID

@param frameId   frame ID

@param dataId    data ID

@param value     value

@retval boolean  data queued in output buffer or not.

@status current Introduced in 2.3

*/

bool getDefaultAccessDestination(uint8_t & destination)
{
  for (uint8_t i=0; i<MAX_TELEMETRY_SENSORS; i++) {
    TelemetrySensor & sensor = g_model.telemetrySensors[i];
    if (sensor.type == TELEM_TYPE_CUSTOM) {
      TelemetryItem sensorItem = telemetryItems[i];
      if (sensorItem.isFresh()) {
        destination = sensor.frskyInstance.rxIndex;
        return true;
      }
    }
  }
  return false;
}

static int luaAccessTelemetryPush(lua_State * L)
{
  if (lua_gettop(L) == 0) {
    lua_pushboolean(L, outputTelemetryBuffer.isAvailable());
    return 1;
  }

  if (outputTelemetryBuffer.isAvailable()) {
    int8_t module = luaL_checkinteger(L, 1);
    uint8_t rxUid = luaL_checkinteger(L, 2);
    uint8_t destination;

    if (module < 0) {
      if (!getDefaultAccessDestination(destination)) {
        lua_pushboolean(L, false);
        return 1;
      }
    }
    else {
      destination = (module << 2) + rxUid;
    }

    outputTelemetryBuffer.sport.physicalId = getDataId(luaL_checkinteger(L, 3));
    outputTelemetryBuffer.sport.primId = luaL_checkinteger(L, 4);
    outputTelemetryBuffer.sport.dataId = luaL_checkinteger(L, 5);
    outputTelemetryBuffer.sport.value = luaL_checkinteger(L, 6);
    outputTelemetryBuffer.setDestination(destination);
    lua_pushboolean(L, true);
    return 1;
  }

  lua_pushboolean(L, false);
  return 1;
}
#endif

#if defined(CROSSFIRE)
/*luadoc
@function crossfireTelemetryPop()

Pops a received Crossfire Telemetry packet from the queue.

@retval nil queue does not contain any (or enough) bytes to form a whole packet

@retval multiple returns 2 values:
 * command (number)
 * packet (table) data bytes

@status current Introduced in 2.2.0
*/
static int luaCrossfireTelemetryPop(lua_State * L)
{
  if (!luaInputTelemetryFifo) {
    luaInputTelemetryFifo = new Fifo<uint8_t, LUA_TELEMETRY_INPUT_FIFO_SIZE>();
    if (!luaInputTelemetryFifo) {
      return 0;
    }
  }

  uint8_t length = 0, data = 0;
  if (luaInputTelemetryFifo->probe(length) && luaInputTelemetryFifo->size() >= uint32_t(length)) {
    // length value includes the length field
    luaInputTelemetryFifo->pop(length);
    luaInputTelemetryFifo->pop(data); // command
    lua_pushinteger(L, data);
    lua_newtable(L);
    for (uint8_t i=1; i<length-1; i++) {
      luaInputTelemetryFifo->pop(data);
      lua_pushinteger(L, i);
      lua_pushinteger(L, data);
      lua_settable(L, -3);
    }
    return 2;
  }

  return 0;
}

/*luadoc
@function crossfireTelemetryPush()

This functions allows for sending telemetry data toward the TBS Crossfire link.

When called without parameters, it will only return the status of the output buffer without sending anything.

@param command command

@param data table of data bytes

@retval boolean  data queued in output buffer or not.

@retval nil      incorrect telemetry protocol.

@status current Introduced in 2.2.0, retval nil added in 2.3.4
*/
static int luaCrossfireTelemetryPush(lua_State* L)
{
  bool external =
      (moduleState[EXTERNAL_MODULE].protocol == PROTOCOL_CHANNELS_CROSSFIRE);
  bool internal =
      (moduleState[INTERNAL_MODULE].protocol == PROTOCOL_CHANNELS_CROSSFIRE);

  if (!internal && !external) {
    lua_pushnil(L);
    return 1;
  }

  if (lua_gettop(L) == 0) {
    lua_pushboolean(L, outputTelemetryBuffer.isAvailable());
  } else if (lua_gettop(L) > TELEMETRY_OUTPUT_BUFFER_SIZE) {
    lua_pushboolean(L, false);
    return 1;
  } else if (outputTelemetryBuffer.isAvailable()) {
    uint8_t command = luaL_checkinteger(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    uint8_t length = luaL_len(L, 2);

    outputTelemetryBuffer.pushByte(MODULE_ADDRESS);

    // LENGTH
    if (command == COMMAND_ID) {
      // 1(COMMAND) + length(data) + 1(CRC_BA) + 1(CRC_D5)
      outputTelemetryBuffer.pushByte(3 + length);
    } else {
      // 1(COMMAND) + length(data) + 1(CRC_D5)
      outputTelemetryBuffer.pushByte(2 + length);
    }

    // COMMAND
    outputTelemetryBuffer.pushByte(command);

    // PAYLOAD
    for (int i = 0; i < length; i++) {
      lua_rawgeti(L, 2, i + 1);
      outputTelemetryBuffer.pushByte(luaL_checkinteger(L, -1));
    }

    // CRC
    if (command == COMMAND_ID) {
      // 1 byte CRC8_BA (counted from COMMAND byte)
      outputTelemetryBuffer.pushByte(
          crc8_BA(outputTelemetryBuffer.data + 2, 1 + length));
      // 1 byte CRC8_D5 (counted from COMMAND byte including CRC8_BA byte)
      outputTelemetryBuffer.pushByte(
          crc8(outputTelemetryBuffer.data + 2, 2 + length));
    } else {
      // 1 byte CRC8_D5 (counted from COMMAND byte)
      outputTelemetryBuffer.pushByte(
          crc8(outputTelemetryBuffer.data + 2, 1 + length));
    }

    outputTelemetryBuffer.setDestination(internal ? 0 : TELEMETRY_ENDPOINT_SPORT);
    lua_pushboolean(L, true);
  } else {
    lua_pushboolean(L, false);
  }
  return 1;
}
#endif

#if defined(GHOST)
/*luadoc
@function ghostTelemetryPop()

Pops a received Ghost Telemetry packet from the queue.

@retval nil queue does not contain any (or enough) bytes to form a whole packet

@retval multiple returns 2 values:
 * type (number)
 * packet (table) data bytes

@status current Introduced in 2.7.0
*/
static int luaGhostTelemetryPop(lua_State * L)
{
  if (!luaInputTelemetryFifo) {
    luaInputTelemetryFifo = new Fifo<uint8_t, LUA_TELEMETRY_INPUT_FIFO_SIZE>();
    if (!luaInputTelemetryFifo) {
      return 0;
    }
  }

  uint8_t length = 0, data = 0;
  if (luaInputTelemetryFifo->probe(length) && luaInputTelemetryFifo->size() >= uint32_t(length)) {
    // length value includes type(1B), payload, crc(1B)
    luaInputTelemetryFifo->pop(length);
    luaInputTelemetryFifo->pop(data); // type
    lua_pushinteger(L, data);          // return type
    lua_newtable(L);
    for (uint8_t i=0; i<length-2; i++) {
      luaInputTelemetryFifo->pop(data);
      lua_pushinteger(L, i + 1);
      lua_pushinteger(L, data);
      lua_settable(L, -3);
    }
    return 2;
  }

  return 0;
}

/*luadoc
@function ghostTelemetryPush()

This functions allows for sending telemetry data toward the Ghost link.

When called without parameters, it will only return the status of the output buffer without sending anything.

@param command command

@param data table of data bytes

@retval boolean  data queued in output buffer or not.

@retval nil      incorrect telemetry protocol.

@status current Introduced in 2.7.0
*/
static int luaGhostTelemetryPush(lua_State * L)
{
  bool extmod = (moduleState[EXTERNAL_MODULE].protocol == PROTOCOL_CHANNELS_GHOST);
  if (!extmod) {
    lua_pushnil(L);
    return 1;
  }

  if (lua_gettop(L) == 0) {
    lua_pushboolean(L, outputTelemetryBuffer.isAvailable());
  }
  else if (lua_gettop(L) > TELEMETRY_OUTPUT_BUFFER_SIZE ) {
    lua_pushboolean(L, false);
    return 1;
  }
  else if (outputTelemetryBuffer.isAvailable()) {
    uint8_t type = luaL_checkinteger(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    uint8_t length = luaL_len(L, 2);              // payload length

    if( length > 10 ) {                           // max 10B payload
      lua_pushboolean(L, false);
      return 1;
    }

    // Ghost frames are fixed 14B:
    // address(1B) + len (1B) + type(1B) + payload(10B) + crc(1B)
    // -> address + len up-front are inserted later
    outputTelemetryBuffer.pushByte(type);         // type (1B)
    int i = 0;
    for (; i < length; i++) {                     // data, max 10B
      lua_rawgeti(L, 2, i + 1);
      outputTelemetryBuffer.pushByte(luaL_checkinteger(L, -1));
    }
    for (; i < 10; i++) {                         // fill zeroes to frame size
      outputTelemetryBuffer.pushByte(0);
    }
    // CRC over type (1B) + payload (10B)
    outputTelemetryBuffer.pushByte(crc8(outputTelemetryBuffer.data, 11 ));
    outputTelemetryBuffer.setDestination(TELEMETRY_ENDPOINT_SPORT);
    lua_pushboolean(L, true);
  }
  else {
    lua_pushboolean(L, false);
  }
  return 1;
}
#endif

/*luadoc
@function getRAS()

Return the RAS value or nil if no valid hardware found

@retval number representing RAS value. Value bellow 0x33 (51 decimal) are all ok, value above 0x33 indicate a hardware antenna issue.
This is just a hardware pass/fail measure and does not represent the quality of the radio link

@notice RAS was called SWR in the past

@status current Introduced in 2.2.0
*/
static int luaGetRAS(lua_State * L)
{
  if (isRasValueValid()) {
    lua_pushinteger(L, telemetryData.swrInternal.value());
  }
  else {
    lua_pushnil(L);
  }
  return 1;
}

/*luadoc
@function getTxGPS()

Return the internal GPS position or nil if no valid hardware found

@retval table representing the current radio position
 * `lat` (number) internal GPS latitude, positive is North
 * `lon` (number) internal GPS longitude, positive is East
 * `numsat` (number) current number of sats locked in by the GPS sensor
 * `fix` (boolean) fix status
 * `alt` (number) internal GPS altitude in 0.1m
 * `speed` (number) internal GPSspeed in 0.1m/s
 * `heading`  (number) internal GPS ground course estimation in degrees * 10
 * `hdop` (number)  internal GPS horizontal dilution of precision

@status current Introduced in 2.2.2
*/
static int luaGetTxGPS(lua_State * L)
{
#if defined(INTERNAL_GPS)
  lua_createtable(L, 0, 8);
  lua_pushtablenumber(L, "lat", gpsData.latitude * 0.000001);
  lua_pushtablenumber(L, "lon", gpsData.longitude * 0.000001);
  lua_pushtableinteger(L, "numsat", gpsData.numSat);
  lua_pushtableinteger(L, "alt", gpsData.altitude);
  lua_pushtableinteger(L, "speed", gpsData.speed);
  lua_pushtableinteger(L, "heading", gpsData.groundCourse);
  lua_pushtableinteger(L, "hdop", gpsData.hdop);
  if (gpsData.fix)
    lua_pushtableboolean(L, "fix", true);
  else
    lua_pushtableboolean(L, "fix", false);
#else
    lua_pushnil(L);
#endif
  return 1;
}


/*luadoc
@function getFlightMode(mode)

Return flight mode data.

@param mode (number) flight mode number to return (0 - 8). If mode parameter
is not specified (or contains invalid value), then the current flight mode data is returned.

@retval multiple returns 2 values:
 * (number) (current) flight mode number (0 - 8)
 * (string) (current) flight mode name

@status current Introduced in 2.1.7
*/
static int luaGetFlightMode(lua_State * L)
{
  int mode = luaL_optinteger(L, 1, -1);
  if (mode < 0 || mode >= MAX_FLIGHT_MODES) {
    mode = mixerCurrentFlightMode;
  }
  lua_pushinteger(L, mode);
  char name[sizeof(g_model.flightModeData[0].name)+1];
  strncpy(name, g_model.flightModeData[mode].name, sizeof(g_model.flightModeData[0].name));
  name[sizeof(g_model.flightModeData[0].name)] = '\0';
  lua_pushstring(L, name);
  return 2;
}

/*luadoc
@function playFile(filename [, volume])

Play a file from the SD card

@param filename (string) full path to wav file (i.e. “/SOUNDS/en/system/tada.wav”)
Introduced in 2.1.0: If you use a relative path, the current language is appended
to the path (example: for English language: `/SOUNDS/en` is appended)

@param volume (number):
 - (1..5) override radio settings Wav volume for the duration of file
 - omitting the parameter uses radio settings Wav volume

@retval none 

@status current Introduced in 2.0.0, changed in 2.1.0, changed in 2.10

// targets: BW, COLOR
//
// EXAMPLES:
// playFile("armed.wav", 5) -- play file armed.wav, use Wav volume 2
// playFile("armed.wav", 1) -- play file armed.wav, use Wav volume 1
// playFile("armed.wav")		-- play file armed.wav, use radio settings Wav volume
*/
static int luaPlayFile(lua_State * L)
{
  const char * filename = luaL_checkstring(L, 1);
  int volume = luaL_optinteger(L, 2, USE_SETTINGS_VOLUME);

  if(volume != USE_SETTINGS_VOLUME)
    volume = limit(-2, volume-3, 2);  // (rescale 1..5) to internal format and limit to (-2..2)

  if (filename[0] != '/') {
    // relative sound file path - use current language dir for absolute path
    char file[AUDIO_FILENAME_MAXLEN+1];
    char * str = getAudioPath(file);
    strncpy(str, filename, AUDIO_FILENAME_MAXLEN - (str-file));
    file[AUDIO_FILENAME_MAXLEN] = 0;
    audioQueue.playFile(file, 0, 0, volume);
  }
  else {
    audioQueue.playFile(filename, 0, 0, volume);
  }
  return 0;
}

/*luadoc
@function playNumber(value, unit [, attributes [, volume]])

Play a numerical value (text to speech)

@param value (number) number to play. Value is interpreted as integer.

@param unit (number) unit identifier [Full list]((../appendix/units.html))

@param attributes (unsigned number) possible values:
 * `0 or not present` plays integral part of the number (for a number 123 it plays 123)
 * `PREC1` plays a number with one decimal place (for a number 123 it plays 12.3)
 * `PREC2` plays a number with two decimal places (for a number 123 it plays 1.23)
 
 @param volume (number):
 - (1..5) override radio settings Wav volume for the duration of file
 - omitting the parameter uses radio settings Wav volume

@retval none 

@status current Introduced in 2.0.0, changed in 2.10

// targets: BW, COLOR
//
// EXAMPLES:
// playNumber(123, 3, 0, 5) -- play number 123, unit mAh, use Wav volume 5
// playNumber(123, 3, 0, 1) -- play number 123, unit mAh, use Wav volume 1
// playNumber(123, 3, 0)    -- play number 123, unit mAh, use radio settings Wav volume
*/

static int luaPlayNumber(lua_State * L)
{
  int number = luaL_checkinteger(L, 1);
  int unit = luaL_checkinteger(L, 2);
  unsigned int att = luaL_optinteger(L, 3, 0);
  int volume = luaL_optinteger(L, 4, USE_SETTINGS_VOLUME);

  if(volume != USE_SETTINGS_VOLUME)
    volume = limit(-2, volume-3, 2);  // (rescale 1..5) to internal format and limit to (-2..2)

  playNumber(number, unit, att, 0, volume);
  return 0;
}

/*luadoc
@function playDuration(duration [, hourFormat [, volume]])

Play a time value (text to speech)

@param duration (number) number of seconds to play. Only integral part is used.

@param hourFormat (number):
 * `0 or not present` play format: minutes and seconds.
 * `!= 0` play format: hours, minutes and seconds.
 * 
@param volume (number):
 - (1..5) override radio settings Wav volume for the duration of file
 - omitting the parameter uses radio settings Wav volume

@retval none 

@status current Introduced in 2.1.0, changed in 2.10

// targets: BW, COLOR
//
// EXAMPLES:
// playDuration(101, 0, 5) -- play duration 101s, seconds format, use Wav volume 5
// playDuration(101, 0, 1) -- play duration 101s, seconds format, use Wav volume 1
// playDuration(101, 1)    -- play duration 101s, hour format, use radio settings Wav volume
*/

static int luaPlayDuration(lua_State * L)
{
  int duration = luaL_checkinteger(L, 1);
  bool playTime = (luaL_optinteger(L, 2, 0) != 0);
  int volume = luaL_optinteger(L, 3, USE_SETTINGS_VOLUME);

  if(volume != USE_SETTINGS_VOLUME)
    volume = limit(-2, volume-3, 2);  // (rescale 1..5) to internal format and limit to (-2..2)

  playDuration(duration, playTime ? PLAY_TIME : 0, 0, volume);
  return 0;
}

/*luadoc
@function playTone(frequency, duration, pause [, flags [, freqIncr [, volume]]])

Play a tone

@param frequency (number) tone frequency in Hz (from 150 to 15000)

@param duration (number) length of the tone in milliseconds

@param pause (number) length of the silence after the tone in milliseconds

@param flags (number):
 * `0 or not present` play with normal priority.
 * `PLAY_BACKGROUND` play in background (built in vario function uses this context)
 * `PLAY_NOW` play immediately

@param freqIncr (number) positive number increases the tone pitch (frequency with time),
negative number decreases it. The frequency changes every 10 milliseconds, the change is `freqIncr * 10Hz`.
The valid range is from -127 to 127.

@param volume (number):
 - (1..5) override radio settings Beep volume for the duration of file
 - omitting the parameter uses radio settings Beep volume

@retval none 

@status current Introduced in 2.1.0, changed in 2.10

// targets: BW, COLOR
//
// EXAMPLES:
// playTone(2550, 160, 20, 3, -10, 5) -- play tone, use Beep volume 5
// playTone(2550, 160, 20, 3, -10, 1) -- play tone, use Beep volume 1
// playTone(2550, 160, 20, 3, -10)		-- play tone, use radio settings Beep volume
*/
static int luaPlayTone(lua_State * L)
{
  int frequency = luaL_checkinteger(L, 1);
  int length = luaL_checkinteger(L, 2);
  int pause = luaL_checkinteger(L, 3);
  int flags = luaL_optinteger(L, 4, 0);
  int freqIncr = luaL_optinteger(L, 5, 0);
  int volume = luaL_optinteger(L, 6, USE_SETTINGS_VOLUME);

  if(volume != USE_SETTINGS_VOLUME)
    volume = limit(-2, volume-3, 2);  // (rescale 1..5) to internal format and limit to (-2..2)

  audioQueue.playTone(frequency, length, pause, flags, freqIncr, volume);
  return 0;
}

/*luadoc
@name screenshot

@description Takes a screenshot, which is saved to the SCREENSHOTS folder on the radio SD card.

@syntax screenshot()

@return none

@notes This command is currently not rate limited, so repeated frequent calls will slow down the UI and can even freeze the entire radio, so should be used with care. 

@target [BW]
@target [GS]
@target [COLOR]

@status current Introduced in 2.11
*/
static int luaScreenshot(lua_State * L)
{
  UNUSED(L);
  writeScreenshot();
  return 0;
}

/*luadoc
@function playHaptic(duration, pause [, flags])

Generate haptic feedback

@param duration (number) length of the haptic feedback in milliseconds

@param pause (number) length of the silence after haptic feedback in milliseconds

@param flags (number):
 * `0 or not present` play with normal priority
 * `PLAY_NOW` play immediately

@status current Introduced in 2.2.0
*/
static int luaPlayHaptic(lua_State * L)
{
#if defined(HAPTIC)
  int length = luaL_checkinteger(L, 1);
  int pause = luaL_checkinteger(L, 2);
  int flags = luaL_optinteger(L, 3, 0);
  haptic.play(length, pause, flags);
#else
  UNUSED(L);
#endif
  return 0;
}

/*luadoc
@function killEvents(key)

Stops key state machine. See [Key Events](../key_events.md) for the detailed description.

@param key (number) key to be killed, can also include event type (only the key part is used)

@status current Introduced in 2.0.0

*/
static int luaKillEvents(lua_State * L)
{
#if defined(KEYS_GPIO_REG_MENU)
  #define IS_MASKABLE(key)                                      \
    ((key) != KEY_EXIT && (key) != KEY_ENTER &&                 \
     ((scriptInternalData[0].reference == SCRIPT_STANDALONE) || \
      (key) != KEY_PAGEDN))
#else
  #define IS_MASKABLE(key) ((key) != KEY_EXIT && (key) != KEY_ENTER)
#endif

  event_t key = EVT_KEY_MASK(luaL_checkinteger(L, 1));
  // prevent killing maskable keys (only in telemetry scripts)
  // TODO add which type of script is running before lua_resume()
  if (IS_MASKABLE(key)) {
    killEvents(key);
    luaEmptyEventBuffer();
   }
  return 0;
}

#if LCD_DEPTH > 1 && !defined(COLORLCD)
/*luadoc
@function GREY()

Returns gray value which can be used in LCD functions

@retval (number) a value that represents amount of *greyness* (from 0 to 15)

@notice Only available on Taranis

@status current Introduced in 2.0.13
*/
static int luaGrey(lua_State * L)
{
  int index = luaL_checkinteger(L, 1);
  lua_pushinteger(L, GREY(index));
  return 1;
}
#endif

/*luadoc
@function getGeneralSettings()

Returns (some of) the general radio settings

@retval table with elements:
 * `battWarn` (number) radio battery range - warning value
 * `battMin` (number) radio battery range - minimum value
 * `battMax` (number) radio battery range - maximum value
 * `imperial` (number) set to a value different from 0 if the radio is set to the
 IMPERIAL units
 * `language` (string) radio language (used for menus)
 * `voice` (string) voice language (used for speech)
 * `gtimer` (number) radio global timer in seconds (does not include current session)

@status current Introduced in 2.0.6, `imperial` added in TODO,
`language` and `voice` added in 2.2.0, gtimer added in 2.2.2.

*/
static int luaGetGeneralSettings(lua_State * L)
{
  lua_newtable(L);
  lua_pushtablenumber(L, "battWarn", (g_eeGeneral.vBatWarn) * 0.1f);
  lua_pushtablenumber(L, "battMin", (90+g_eeGeneral.vBatMin) * 0.1f);
  lua_pushtablenumber(L, "battMax", (120+g_eeGeneral.vBatMax) * 0.1f);
  lua_pushtableinteger(L, "imperial", g_eeGeneral.imperial);
  lua_pushtablestring(L, "language", TRANSLATIONS);
  lua_pushtablestring(L, "voice", currentLanguagePack->id);
  lua_pushtableinteger(L, "gtimer", g_eeGeneral.globalTimer);
  return 1;
}

/*luadoc
@function getGlobalTimer()

Returns radio timers

@retval table with elements:
* `gtimer` (number) radio global timer in seconds
* `session` (number) radio session in seconds
* `ttimer` (number) radio throttle timer in seconds
* `tptimer` (number) radio throttle percent timer in seconds

@status current Introduced added in 2.3.0.

*/
static int luaGetGlobalTimer(lua_State * L)
{
  lua_newtable(L);
  lua_pushtableinteger(L, "total", g_eeGeneral.globalTimer + sessionTimer);
  lua_pushtableinteger(L, "session", sessionTimer);
  lua_pushtableinteger(L, "throttle", s_timeCumThr);
  lua_pushtableinteger(L, "throttlepct", s_timeCum16ThrP/16);
  return 1;
}

#if defined(ENABLE_LUA_POPUP_INPUT)
/*luadoc
@function popupInput(title, event, input, min, max)

Raises a pop-up on screen that allows uses input

@param title (string) text to display

@param event (number) the event variable that is passed in from the
Run function (key pressed)

@param input (number) value that can be adjusted by the +/­- keys

@param min  (number) min value that input can reach (by pressing the -­ key)

@param max  (number) max value that input can reach

@retval number result of the input adjustment

@retval "OK" user pushed ENT key

@retval "CANCEL" user pushed EXIT key

@notice Use only from stand-alone and telemetry scripts.

@status current Introduced in 2.0.0
*/
static int luaPopupInput(lua_State * L)
{
  event_t event = luaL_checkinteger(L, 2);
  warningInputValue = luaL_checkinteger(L, 3);
  warningInputValueMin = luaL_checkinteger(L, 4);
  warningInputValueMax = luaL_checkinteger(L, 5);
  warningText = luaL_checkstring(L, 1);
  warningType = WARNING_TYPE_INPUT;

  runPopupWarning(event);

  if (warningResult) {
    warningResult = 0;
    lua_pushstring(L, "OK");
  }
  else if (!warningText) {
    lua_pushstring(L, "CANCEL");
  }
  else {
    lua_pushinteger(L, warningInputValue);
  }
  warningText = NULL;
  return 1;
}
#endif

/*luadoc
@function popupWarning(title, event)

Raises a pop-up on screen that shows a warning

@param title (string) text to display

@param event (number) the event variable that is passed in from the
Run function (key pressed)

@retval "CANCEL" user pushed EXIT key

@notice Use only from stand-alone and telemetry scripts.

@status current Introduced in 2.2.0
*/
static int luaPopupWarning(lua_State * L)
{
#if defined(COLORLCD)
  const char* warningText = nullptr;
  uint8_t     warningType = 0;
  bool        warningResult = false;
#endif

  event_t event = luaL_checkinteger(L, 2);
  warningText = luaL_checkstring(L, 1);
  warningType = WARNING_TYPE_ASTERISK;

#if defined(COLORLCD)
  if (StandaloneLuaWindow::instance()->displayPopup(event, warningType, warningText,
                                                    nullptr, warningResult)) {
    warningText = nullptr;
  }
#else
  runPopupWarning(event);
#endif

  if (!warningText) {
    lua_pushstring(L, "CANCEL");
  }
  else {
    warningText = NULL;
    lua_pushnil(L);
  }
  return 1;
}

/*luadoc
@function popupConfirmation(title, event) deprecated, please replace by
@function popupConfirmation(title, message, event)

Raises a pop-up on screen that asks for confirmation

@param title (string) title to display

@param message (string) text to display

@param event (number) the event variable that is passed in from the
Run function (key pressed)

@retval "CANCEL" user pushed EXIT key

@notice Use only from stand-alone and telemetry scripts.

@status current Introduced in 2.2.0, changed to (title, message, event) in 2.3.8
*/
static int luaPopupConfirmation(lua_State * L)
{
#if defined(COLORLCD)
  // necessary to run the LUA pop-ups
  uint8_t     warningType     = 0;
  bool        warningResult   = false;
  const char* warningText     = nullptr;
  const char* warningInfoText = nullptr;
#endif

  warningType = WARNING_TYPE_CONFIRM;
  event_t event;

  if (lua_isnone(L, 3)) {
    // only two args: deprecated mode
    warningText = luaL_checkstring(L, 1);
    event = luaL_checkinteger(L, 2);
  }
  else {
    warningText = luaL_checkstring(L, 1);
    warningInfoText = luaL_checkstring(L, 2);
    event = luaL_optinteger(L, 3, 0);
  }

#if defined(COLORLCD)
  if (StandaloneLuaWindow::instance()->displayPopup(event, warningType, warningText,
                                                    warningInfoText, warningResult)) {
    warningText = nullptr;
  }
#else
  runPopupWarning(event);
#endif

  if (!warningText) {
    lua_pushstring(L, warningResult ? "OK" : "CANCEL");
  }
  else {
    warningText = nullptr;
    lua_pushnil(L);
  }

  return 1;
}

/*luadoc
@function defaultStick(channel)

Get stick that is assigned to a channel. See Default Channel Order in General Settings.

@param channel (number) channel number (0 means CH1)

@retval number Stick assigned to this channel (from 0 to 3)

@status current Introduced in 2.0.0
*/
static int luaDefaultStick(lua_State * L)
{
  uint8_t channel = luaL_checkinteger(L, 1);
  lua_pushinteger(L, inputMappingChannelOrder(channel));
  return 1;
}

/*luadoc
@function setTelemetryValue(id, subID, instance, value [, unit [, precision [, name]]])

@param id Id of the sensor, valid range is from 0 to 0xFFFF

@param subID subID of the sensor, usually 0, valid range is from 0 to 7

@param instance instance of the sensor (SensorID), valid range is from 0 to 0xFF

@param value fed to the sensor

@param unit unit of the sensor [Full list](../../appendix/units.html)

@param precision the precision of the sensor
 * `0 or not present` no decimal precision.
 * `!= 0` value is divided by 10^precision, e.g. value=1000, prec=2 => 10.00.

@param name (string) Name of the sensor if it does not yet exist (4 chars).
 * `not present` Name defaults to the Id.
 * `present` Sensor takes name of the argument. Argument must have name surrounded by quotes: e.g., "Name"

@retval true, if the sensor was just added. In this case the value is ignored (subsequent call will set the value)

@notice All three parameters `id`, `subID` and `instance` can't be zero at the same time. At least one of them
must be different from zero.

@status current Introduced in 2.2.0
*/
static int luaSetTelemetryValue(lua_State * L)
{
  uint16_t id = luaL_checkinteger(L, 1);
  uint8_t subId = luaL_checkinteger(L, 2) & 0x7;
  uint8_t instance = luaL_checkinteger(L, 3);
  int32_t value = luaL_checkinteger(L, 4);
  uint32_t unit = luaL_optinteger(L, 5, 0);
  uint32_t prec = luaL_optinteger(L, 6, 0);

  char name_buf[TELEM_LABEL_LEN]; // 4
  const char* name = luaL_optstring(L, 7, NULL);
  if (name == NULL || strlen(name) > 0) {
    name_buf[0] = hex2char((id & 0xf000) >> 12);
    name_buf[1] = hex2char((id & 0x0f00) >> 8);
    name_buf[2] = hex2char((id & 0x00f0) >> 4);
    name_buf[3] = hex2char((id & 0x000f) >> 0);
  }
  if (id | subId | instance) {
    int index = setTelemetryValue(PROTOCOL_TELEMETRY_LUA, id, subId, instance,
                                  value, unit, prec);
    if (index >= 0) {
      TelemetrySensor &telemetrySensor = g_model.telemetrySensors[index];
      telemetrySensor.id = id;
      telemetrySensor.subId = subId;
      telemetrySensor.instance = instance;
      telemetrySensor.init(name ? name: name_buf, unit, prec);
      
      storageDirty(EE_MODEL);
      
      lua_pushboolean(L, true);
    } else {
      lua_pushboolean(L, false);
    }
    return 1;
  }
  lua_pushboolean(L, false);
  return 1;
}

/*luadoc
@function defaultChannel(stick)

Get channel assigned to stick. See Default Channel Order in General Settings

@param stick (number) stick number (from 0 to 3)

@retval number channel assigned to this stick (from 0 to 3)

@retval nil stick not found

@status current Introduced in 2.0.0
*/
static int luaDefaultChannel(lua_State * L)
{
  uint8_t stick = luaL_checkinteger(L, 1);
  for (int i = 0; i < adcGetMaxInputs(ADC_INPUT_MAIN); i++) {
    int tmp = inputMappingChannelOrder(i);
    if (tmp == stick) {
      lua_pushinteger(L, i);
      return 1;
    }
  }
  lua_pushnil(L);
  return 1;
}

/*luadoc
@function flushAudio()

flushes audio queue

@status experimental
*/
static int luaFlushAudio(lua_State * L)
{
  audioQueue.flush();
  return 0;
}

/*luadoc
@function getRSSI()

Get RSSI value as well as low and critical RSSI alarm levels (in dB)

@retval rssi RSSI value (0 if no link)

@retval alarm_low Configured low RSSI alarm level

@retval alarm_crit Configured critical RSSI alarm level

@status current Introduced in 2.2.0
*/
static int luaGetRSSI(lua_State * L)
{
  if (TELEMETRY_STREAMING())
    lua_pushinteger(L, min((uint8_t)99, TELEMETRY_RSSI()));
  else
    lua_pushinteger(L, 0);
  lua_pushinteger(L, g_model.rfAlarms.warning);
  lua_pushinteger(L, g_model.rfAlarms.critical);
  return 3;
}

/*luadoc
@function loadScript(file [, mode], [,env])

Load a Lua script file. This is similar to Lua's own [loadfile()](https://www.lua.org/manual/5.2/manual.html#pdf-loadfile)
API method, but it uses OpenTx's optional pre-compilation feature to save memory and time during load.

Return values are same as from Lua API loadfile() method: If the script was loaded w/out errors
then the loaded script (or "chunk") is returned as a function. Otherwise, returns nil plus the error message.

@param file (string) Full path and file name of script. The file extension is optional and ignored (see `mode` param to control
  which extension will be used). However, if an extension is specified, it should be ".lua" (or ".luac"), otherwise it is treated
  as part of the file name and the .lua/.luac will be appended to that.

@param mode (string) (optional) Controls whether to force loading the text (.lua) or pre-compiled binary (.luac)
  version of the script. By default ETX will load the newest version and compile a new binary if necessary (overwriting any
  existing .luac version of the same script, and stripping some debug info like line numbers).
  You can use `mode` to control the loading behavior more specifically. Possible values are:
   * `b` only binary.
   * `t` only text.
   * `T` (default on simulator) prefer text but load binary if that is the only version available.
   * `bt` (default on radio) either binary or text, whichever is newer (binary preferred when timestamps are equal).
   * Add `x` to avoid automatic compilation of source file to .luac version.
       Eg: "tx", "bx", or "btx".
   * Add `c` to force compilation of source file to .luac version (even if existing version is newer than source file).
       Eg: "tc" or "btc" (forces "t", overrides "x").
   * Add `d` to keep extra debug info in the compiled binary.
       Eg: "td", "btd", or "tcd" (no effect with just "b" or with "x").

@notice
  Note that you will get an error if you specify `mode` as "b" or "t" and that specific version of the file does not exist (eg. no .luac file when "b" is used).
  Also note that `mode` is NOT passed on to Lua's loader function, so unlike with loadfile() the actual file content is not checked (as if no mode or "bt" were passed to loadfile()).

@param env (integer) See documentation for Lua function loadfile().

@retval function The loaded script, or `nil` if there was an error (e.g. file not found or syntax error).

@retval string Error message(s), if any. Blank if no error occurred.

@status current Introduced in 2.2.0

### Example

```lua
  fun, err = loadScript("/SCRIPTS/FUNCTIONS/print.lua")
  if (fun ~= nil) then
     fun("Hello from loadScript()")
  else
     print(err)
  end
```

*/
static int luaLoadScript(lua_State * L)
{
  // this function is replicated pretty much verbatim from luaB_loadfile() and load_aux() in lbaselib.c
  const char *fname = luaL_optstring(L, 1, NULL);
  const char *mode = luaL_optstring(L, 2, NULL);
  int env = (!lua_isnone(L, 3) ? 3 : 0);  // 'env' index or 0 if no 'env'
  lua_settop(L, 0);
  if (fname != NULL && luaLoadScriptFileToState(L, fname , mode) == SCRIPT_OK) {
    if (env != 0) {  // 'env' parameter?
      lua_pushvalue(L, env);  // environment for loaded function
      if (!lua_setupvalue(L, -2, 1))  // set it as 1st upvalue
        lua_pop(L, 1);  // remove 'env' if not used by previous call
    }
    return 1;
  }
  else {
    // error (message should be on top of the stack)
    if (!lua_isstring(L, -1)) {
      // probably didn't find a file or had some other error before luaL_loadfile() was run
      lua_pushfstring(L, "loadScript(\"%s\", \"%s\") error: File not found", (fname != NULL ? fname : "nul"), (mode != NULL ? mode : "bt"));
    }
    lua_pushnil(L);
    lua_insert(L, -2);  // move nil before error message
    return 2;  // return nil plus error message
  }
}

/*luadoc
@function getUsage()

Get percent of already used Lua instructions in current script execution cycle.

@retval usage (number) a value from 0 to 100 (percent)

@status current Introduced in 2.2.1
*/
static int luaGetUsage(lua_State * L)
{
#if defined(COLORLCD)
  if (luaLvglManager && luaLvglManager->useLvglLayout()) {
    lua_pushinteger(L, luaLvglManager->refreshInstructionsPercent);
  } else {
    lua_pushinteger(L, instructionsPercent);
  }
#else
  lua_pushinteger(L, instructionsPercent);
#endif
  return 1;
}

/*luadoc
@function getAvailableMemory()

Get available memory remaining in the Heap for Lua.

@retval usage (number) a value returned in b
*/
static int luaGetAvailableMemory(lua_State * L)
{
  lua_pushinteger(L, availableMemory());
  return 1;
}

/*luadoc
@function resetGlobalTimer([type])

 Resets the radio global timer to 0.

@param (optional) : if set to 'all', throttle ,throttle percent and session timers are reset too
                    if set to 'session', radio session timer is reset too
                    if set to 'ttimer', radio throttle timer is reset too
                    if set to  'tptimer', radio throttle percent timer is reset too

@status current Introduced in 2.2.2, param added in 2.3
*/
static int luaResetGlobalTimer(lua_State * L)
{
  size_t length;
  const char * option = luaL_optlstring(L, 1, "total", &length);
  if (!strcmp(option, "all")) {
    g_eeGeneral.globalTimer = 0;
    sessionTimer = 0;
    s_timeCumThr = 0;
    s_timeCum16ThrP = 0;
  }
  else if (!strcmp(option, "total")) {
    g_eeGeneral.globalTimer = 0;
    sessionTimer = 0;
  }
  else if (!strcmp(option, "session")) {
    sessionTimer = 0;
  }
  else if (!strcmp(option, "throttle")) {
    s_timeCumThr = 0;
  }
  else if (!strcmp(option, "throttlepct")) {
    s_timeCum16ThrP = 0;
  }
  storageDirty(EE_GENERAL);
  return 0;
}

/*luadoc
@function multiBuffer(address[,value])

This function reads/writes the Multi protocol buffer to interact with a protocol².

@param address to read/write in the buffer
@param (optional): value to write in the buffer

@retval buffer value (number)

@status current Introduced in 2.3.2
*/
#if defined(MULTIMODULE)
uint8_t * Multi_Buffer = nullptr;

static int luaMultiBuffer(lua_State * L)
{
  uint8_t address = luaL_checkinteger(L, 1);
  if (!Multi_Buffer)
    Multi_Buffer = (uint8_t *) malloc(MULTI_BUFFER_SIZE);

  if (!Multi_Buffer || address >= MULTI_BUFFER_SIZE) {
    lua_pushinteger(L, 0);
    return 0;
  }
  uint16_t value = luaL_optinteger(L, 2, 0x100);
  if (value < 0x100) {
    Multi_Buffer[address] = value;
  }
  lua_pushinteger(L, Multi_Buffer[address]);
  return 1;
}
#endif

/*luadoc
@function setSerialBaudrate(baudrate)
@param baudrate Desired baurate

Set baudrate for serial port(s) affected to LUA

@status current Introduced in 2.3.12
*/
static int luaSetSerialBaudrate(lua_State * L)
{
  int port_nr = serialGetModePort(UART_MODE_LUA);
  if (port_nr < 0) return 0;

  uint32_t baudrate = luaL_checkunsigned(L, 1);
  serialSetBaudrate(port_nr, baudrate);

  return 0;
}

/*luadoc
@function serialWrite(str)
@param str (string) String to be written to the serial port.

Writes a string to the serial port. The string is allowed to contain any character, including 0.

@status current Introduced in 2.3.10
*/
static int luaSerialWrite(lua_State * L)
{
  const char * str = luaL_checkstring(L, 1);
  size_t len = lua_rawlen(L, 1);

  if (!str || len < 1)
    return 0;

  auto _sendCb = luaSendDataCb;
  auto _ctx = luaSendDataCtx;

  if (_sendCb) {
    size_t wr_len = len;
    const char* p = str;
    while(wr_len--) _sendCb(_ctx, *p++);
  }

  return 0;
}

/*luadoc
@function serialRead([num])
@param num (optional): maximum number of bytes to read.
                       If non-zero, serialRead will read up to num characters from the buffer.
                       If 0 or left out, serialRead will read up to and including the first newline character or the end of the buffer.
                       Note that the returned string may not end in a newline if this character is not present in the buffer.

@retval str string. Empty if no new characters were available.

Reads characters from the serial port. The string is allowed to contain any character, including 0.

@status current Introduced in 2.3.8
*/
static int luaSerialRead(lua_State * L)
{
  int num = luaL_optinteger(L, 1, 0);

  uint8_t str[LUA_FIFO_SIZE];
  uint8_t *p = str;

  auto _getByte = luaGetSerialByte;
  auto _ctx = luaGetSerialByteCtx;

  if (_getByte) {
    while (_getByte(_ctx, p) > 0) {
      p++;  // increment only when pop was successful
      if (p - str >= LUA_FIFO_SIZE) {
        // buffer full
        break;
      }
      if (num == 0) {
        if (*(p - 1) == '\n' || *(p - 1) == '\r') {
          // found newline
          break;
        }
      }
      else if (p - str >= num) {
        // requested number of characters reached
        break;
      }
    }
  }
  lua_pushlstring(L, (const char*)str, p - str);

  return 1;
}

#if defined(SWSERIALPOWER) && !defined(SIMU)
/*luadoc
@function serialGetPower(port_nr)

@param port_nr: valid values are only 0 and 1 on radios that have SWSERIALPOWER defined
                0 - first serial port, e.g. on TX16S AUX1
                1 - second serial port, e.g. on TX16S AUX2

@retval value: true for power enabled, false for power disabled.

@retval nil the serial port power control not available on this radio

@status current Introduced in 2.9.0

*/
static int luaSerialGetPower(lua_State* L)
{
  uint8_t port_nr = luaL_checkinteger(L, 1) & 0x3;

  #if defined(AUX_SERIAL)
    if (port_nr == SP_AUX1)
    {
      bool res = serialGetPower(SP_AUX1);
      lua_pushboolean(L, res);
      return 1;
    }
  #endif
  #if defined(AUX2_SERIAL)
    if (port_nr == SP_AUX2)
    {
      bool res = serialGetPower(SP_AUX2);
      lua_pushboolean(L, res);
      return 1;
    }
  #endif
  return 0;
}

/*luadoc
@function serialSetPower(port_nr, value)

@param port_nr: valid values are only 0 and 1 on radios that have SWSERIALPOWER defined
                0 - first serial port, e.g. on TX16S AUX1
                1 - second serial port, e.g. on TX16S AUX2

@param value: 0 - disable power
              1 - enable power

@retval success: true/false.

@status current Introduced in 2.9.0

*/
static int luaSerialSetPower(lua_State* L)
{
  uint8_t port_nr = luaL_checkinteger(L, 1) & 0x3;
  uint8_t value = luaL_checkinteger(L, 2) & 0x3;

  if (value < 2)
  {
  #if defined(AUX_SERIAL)
    if (port_nr == SP_AUX1)
    {
      serialSetPower(SP_AUX1, value);
      lua_pushboolean(L, true);
      return 1;
    }
  #endif
  #if defined(AUX2_SERIAL)
    if (port_nr == SP_AUX2)
    {
      serialSetPower(SP_AUX2, value);
      lua_pushboolean(L, true);
      return 1;
    }
  #endif
  }
  lua_pushboolean(L, false);
  return 1;
}
#endif

#if defined(COLORLCD)
static int shmVar[16] = {0};

/*luadoc
@function setShmVar(id, value)

@param id: integer between 1 and 16 identifying the shared memory variable.

@param value: integer. The value of the shared memory variable.

Sets the value of a shared memory variable that can be used for passing data between Lua widgets and other Lua scripts.

@status current Introduced in 2.6

@notice Only available on radios with color display
*/

static int luaSetShmVar(lua_State * L)
{
  int id = luaL_checkinteger(L, 1);
  int value = luaL_checkinteger(L, 2);

  if (1 <= id && id <= 16)
    shmVar[id - 1] = value;

  return 0;
}

/*luadoc
@function getShmVar(id)

@param id: integer between 1 and 16 identifying the shared memory variable.

@retval value: integer. The value of the shared memory variable.

Gets the value of a shared memory variable that can be used for passing data between Lua widgets and other Lua scripts.

@status current Introduced in 2.6

@notice Only available on radios with color display
*/

static int luaGetShmVar(lua_State * L)
{
  int id = luaL_checkinteger(L, 1);

  if (1 <= id && id <= 16)
    lua_pushinteger(L, shmVar[id - 1]);
  else
    lua_pushnil(L);

  return 1;
}
#endif

/*luadoc
@function setStickySwitch(id, value)

@param id: integer identifying the sticky logical switch (zero for LS1 etc.).

@param value: true/false. The new value of the sticky logical switch.

@retval bufferFull: true/false. This function sends a message from Lua to the logical switch processor
via a buffer with eight slots that are read 10 times per second. If the buffer is full, then a true value
is returned and no messages was sent (i.e. the switch was not changed).

Sets the value of a sticky logical switch.

@status current Introduced in 2.6
*/

#if (MAX_LOGICAL_SWITCHES != 64)
#warning "The following code assumes that MAX_LOGICAL_SWITCHES == 64!"
#endif

static int luaSetStickySwitch(lua_State * L)
{
  int id = luaL_checkinteger(L, 1);
  bool value = lua_toboolean(L, 2);

  uint8_t msg = (1 << 6);       // This bit is always set to have a non-zero value
  if (value) msg |= (1 << 7);
  msg |= (id & 0x3F);

  lua_pushboolean(L, luaSetStickySwitchBuffer.write(msg));
  return 1;
}

/*luadoc
@function getLogicalSwitchValue(id)

@param id: integer identifying the logical switch (zero for LS1 etc.).

@retval value: true/false.

Reads the value of a logical switch.

@status current Introduced in 2.6
*/

static int luaGetLogicalSwitchValue(lua_State * L)
{
  int id = luaL_checkinteger(L, 1);

  if (id >= 0 && id < MAX_LOGICAL_SWITCHES)
    lua_pushboolean(L, getSwitch(SWSRC_FIRST_LOGICAL_SWITCH + id));
  else
    lua_pushnil(L);
  return 1;
}

/*luadoc
@function getSwitchIndex(positionName)

@param positionName: string naming a switch position as it is shown on radio menus where you can select a switch. Notice that many names have
special characters in them like arrow up/down etc.

@retval value: integer. The switchIndex, which can be used as input for `getSwitchName(switchIndex)` and `getSwitchValue(switchIndex)`. Also corresponds
to the fields in the table returned by `model.getLogicalSwitch(switch)` identifying switches.

@status current Introduced in 2.6
*/

static int luaGetSwitchIndex(lua_State * L)
{
  const char * name = luaL_checkstring(L, 1);
  bool negate = false;
  bool found = false;
  swsrc_t idx;

  if (name[0] == '!') {
    name++;
    negate = true;
  }

  for (idx = SWSRC_NONE; idx < SWSRC_COUNT; idx++) {
    if (isSwitchAvailable(idx, ModelCustomFunctionsContext)) {
      char* s = getSwitchPositionName(idx);
      if (!strncasecmp(s, name, 31)) {
        found = true;
        break;
      }
    }
  }

  if (found) {
    if (negate)
      idx = -idx;
    lua_pushinteger(L, idx);
  }
  else
    lua_pushnil(L);

  return 1;
}

/*luadoc
@function getSwitchName(switchIndex)

@param switchIndex: integer identifying a switch as returned by `getSwitchIndex(positionName)` or fields in the table returned by
`model.getLogicalSwitch(switch)` identifying switches.

@retval value: string naming the switch position as it is shown on radio menus where a switch can be chosen.

@status current Introduced in 2.6
*/

static int luaGetSwitchName(lua_State * L)
{
  swsrc_t idx = luaL_checkinteger(L, 1);
  if (idx > -SWSRC_COUNT && idx < SWSRC_COUNT && isSwitchAvailable(idx, ModelCustomFunctionsContext)) {
    char* name = getSwitchPositionName(idx);
    lua_pushstring(L, name);
  }
  else
    lua_pushnil(L);
  return 1;
}

/*luadoc
@function getSwitchValue(switchIndex)

@param switchIndex: integer identifying a switch as returned by `getSwitchIndex(positionName)` or fields in the table returned by
`model.getLogicalSwitch(switch)` identifying switches.

@retval value: true/false. The value of the switch.

@status current Introduced in 2.6
*/

static int luaGetSwitchValue(lua_State * L)
{
  swsrc_t idx = luaL_checkinteger(L, 1);
  if (idx > -SWSRC_COUNT && idx < SWSRC_COUNT && isSwitchAvailable(idx, ModelCustomFunctionsContext))
    lua_pushboolean(L, getSwitch(idx));
  else
    lua_pushnil(L);
  return 1;
}

/*luadoc
@function switches([first[, last]])

@param first: the first switch index. If `nil` or omitted, the first available switch is used.

@param last: the last switch index. If `nil` or omitted, the last available switch is used.

This is an iterator function over switch positions. `for switchIndex, switchName in switches() do ...` will iterate over all available switch positions.

@status current Introduced in 2.6
*/

static int luaNextSwitch(lua_State * L)
{
  swsrc_t last = luaL_checkinteger(L, 1);
  swsrc_t idx = luaL_checkinteger(L, 2);

  while (++idx <= last) {
    if (isSwitchAvailable(idx, ModelCustomFunctionsContext)) {
      char* name = getSwitchPositionName(idx);
      lua_pushinteger(L, idx);
      lua_pushstring(L, name);
      return 2;
    }
  }

  lua_pushnil(L);
  return 1;
}

static int luaSwitches(lua_State * L)
{
  swsrc_t first;
  swsrc_t last;

  if (lua_isnumber(L, 1)) {
    first = luaL_checkinteger(L, 1) - 1;
    if (first < SWSRC_FIRST - 1)
      first = SWSRC_FIRST - 1;
  } else
    first = SWSRC_FIRST - 1;

  if (lua_isnumber(L, 2)) {
    last = luaL_checkinteger(L, 2);
    if (last > SWSRC_LAST)
      last = SWSRC_LAST;
  } else
    last = SWSRC_LAST;

  lua_pushcfunction(L, luaNextSwitch);
  lua_pushinteger(L, last);
  lua_pushinteger(L, first);
  return 3;
}

/*luadoc
@function getSourceIndex(sourceName)

@param sourceName: string naming a value source as it is shown on radio menus where you can select it. Notice that many names have special characters in them.

@retval sourceIndex: integer. The source index, which can be used as input for `getSourceName(sourceIndex)`, `getValue(sourceIndex)`, and `getFieldInfo(sourceIndex)`.

@notice the source names shown on the screen are not the same as the names used by `getFieldInfo` and `getValue`. But the indices are the same, so `getValue(index)` will work with the indices obtained here.
This function is rather time consuming, and should not be used repeatedly in a script, if it can be avoided.

@status current Introduced in 2.6
*/

static int luaGetSourceIndex(lua_State* const L)
{
  const char* const name = luaL_checkstring(L, 1);
  bool found = false;
  mixsrc_t idx;

  for (idx = MIXSRC_NONE; idx <= MIXSRC_LAST_TELEM; idx++) {
    if (isSourceAvailable(idx)) {
      char srcName[maxSourceNameLength];
      getSourceString(srcName, idx);
      if (!strncasecmp(srcName, name)) {
        found = true;
        break;
      }
    }
  }

  if (found)
    lua_pushinteger(L, idx);
  else
    lua_pushnil(L);

  return 1;
}

/*luadoc
@function getSourceName(sourceIndex)

@param sourceIndex: integer identifying a value source as returned by `getSourceIndex(sourceName)` or the `id` field in the table returned by `getFieldInfo`.

@retval sourceName: string naming the value source as it is shown on radio menus where a source can be chosen.

@notice the source names shown on the screen are not the same as the names used by `getFieldInfo` and `getValue`. But the indices are the same, so `getValue(index)` will work with the indices used here.

@status current Introduced in 2.6
*/

static int luaGetSourceName(lua_State * L)
{
  mixsrc_t idx = luaL_checkinteger(L, 1);
  if (idx <= MIXSRC_LAST_TELEM && isSourceAvailable(idx)) {
    char srcName[maxSourceNameLength];
    getSourceString(srcName, idx);
    lua_pushstring(L, srcName);
  } else {
    lua_pushnil(L);
  }
  return 1;
}

/*luadoc
@function sources([first[, last]])

@param first: the first source index. If `nil` or omitted, the first available source is used.

@param last: the last soure index. If `nil` or omitted, the last available source is used.

This is an iterator function over value sources. `for sourceIndex, sourceName in sources() do ...` will iterate over all available value sources.

@status current Introduced in 2.6
*/

static int luaNextSource(lua_State * L)
{
  mixsrc_t last = luaL_checkinteger(L, 1);
  mixsrc_t idx = luaL_checkinteger(L, 2);

  while (++idx <= last) {
    if (isSourceAvailable(idx)) {
      char srcName[maxSourceNameLength];
      getSourceString(srcName, idx);
      lua_pushinteger(L, idx);
      lua_pushstring(L, srcName);
      return 2;
    }
  }

  lua_pushnil(L);
  return 1;
}

static int luaSources(lua_State * L)
{
  mixsrc_t first;
  mixsrc_t last;

  if (lua_isnumber(L, 1))
    first = luaL_checkinteger(L, 1) - 1;
  else
    first = MIXSRC_NONE - 1;

  if (lua_isnumber(L, 2)) {
    last = luaL_checkinteger(L, 2);
    if (last > INPUTSRC_LAST)
      last = INPUTSRC_LAST;
  } else
    last = INPUTSRC_LAST;

  lua_pushcfunction(L, luaNextSource);
  lua_pushinteger(L, last);
  lua_pushinteger(L, first);
  return 3;
}

/*luadoc
@function getOutputValue(outputIndex)

@param outputIndex: integer identifying the output channel number 0 for CH1, up to MAX_OUTPUT_CHANNELS - 1.

@retval value current output value (number). Zero is returned for:
 * non-existing outputs

@status current Introduced in 2.8.0
*/
static int luaGetOutputValue(lua_State * L)
{
  mixsrc_t idx = luaL_checkinteger(L, 1);
  if (idx < MAX_OUTPUT_CHANNELS) {           // mixsrc_t is unsigned, no need to check for <0
    lua_pushinteger(L, channelOutputs[idx]);
  } else {
    lua_pushinteger(L, 0);
  }
  return 1;
}

/*luadoc
@function getTrainerStatus()

@retval value current output value (number).
 0 - Not Connected
 1 - Connected
 2 - Disconnected
 3 - Reconnected

@status current Introduced in 2.9.0
*/
static int luaGetTrainerStatus(lua_State * L)
{
  extern uint8_t trainerStatus;
  lua_pushinteger(L, trainerStatus);
  return 1;
}

#if defined(LED_STRIP_GPIO)
/*luadoc
@function setRGBLedColor(id, rvalue, bvalue, cvalue)

@param id: integer identifying a led in the led chain

@param rvalue: interger, value of red channel

@param gvalue: interger, value of green channel

@param bvalue: interger, value of blue channel

@status current Introduced in 2.10
*/

static int luaSetRgbLedColor(lua_State * L)
{
  uint8_t id = luaL_checkunsigned(L, 1);
  uint8_t r = luaL_checkunsigned(L, 2);
  uint8_t g = luaL_checkunsigned(L, 3);
  uint8_t b = luaL_checkunsigned(L, 4);

  rgbSetLedColor(id, r, g, b);

  return 1;
}

/*luadoc
@function applyRGBLedColors()

 Apply RGB led colors previously defined by setRGBLedColor

@status current Introduced in 2.10
*/

static int luaApplyRGBLedColors(lua_State * L)
{

  rgbLedColorApply();

  return 1;
}

#endif

#define KEY_EVENTS(xxx, yyy)                                    \
  { "EVT_"#xxx"_FIRST", LRO_NUMVAL(EVT_KEY_FIRST(yyy)) },       \
  { "EVT_"#xxx"_BREAK", LRO_NUMVAL(EVT_KEY_BREAK(yyy)) },       \
  { "EVT_"#xxx"_LONG",  LRO_NUMVAL(EVT_KEY_LONG(yyy)) },        \
  { "EVT_"#xxx"_REPT",  LRO_NUMVAL(EVT_KEY_REPT(yyy)) },

LROT_BEGIN(etxlib, NULL, 0)
  LROT_FUNCENTRY( getTime, luaGetTime )
  LROT_FUNCENTRY( getDateTime, luaGetDateTime )
#if defined(RTCLOCK)
  LROT_FUNCENTRY( getRtcTime, luaGetRtcTime )
#endif
  LROT_FUNCENTRY( getVersion, luaGetVersion )
  LROT_FUNCENTRY( getGeneralSettings, luaGetGeneralSettings )
  LROT_FUNCENTRY( getGlobalTimer, luaGetGlobalTimer )
  LROT_FUNCENTRY( getRotEncSpeed, luaGetRotEncSpeed )
  LROT_FUNCENTRY( getRotEncMode, luaGetRotEncMode )
  LROT_FUNCENTRY( getValue, luaGetValue )
  LROT_FUNCENTRY( getOutputValue, luaGetOutputValue )
  LROT_FUNCENTRY( getSourceValue, luaGetSourceValue )
  LROT_FUNCENTRY( getTrainerStatus, luaGetTrainerStatus )
  LROT_FUNCENTRY( getRAS, luaGetRAS )
  LROT_FUNCENTRY( getTxGPS, luaGetTxGPS )
  LROT_FUNCENTRY( getFieldInfo, luaGetFieldInfo )
  LROT_FUNCENTRY( getSourceInfo, luaGetFieldInfo )
  LROT_FUNCENTRY( getFlightMode, luaGetFlightMode )
  LROT_FUNCENTRY( playFile, luaPlayFile )
  LROT_FUNCENTRY( playNumber, luaPlayNumber )
  LROT_FUNCENTRY( playDuration, luaPlayDuration )
  LROT_FUNCENTRY( playTone, luaPlayTone )
  LROT_FUNCENTRY( playHaptic, luaPlayHaptic )
  LROT_FUNCENTRY( flushAudio, luaFlushAudio )
  LROT_FUNCENTRY( screenshot, luaScreenshot )
#if defined(ENABLE_LUA_POPUP_INPUT)
  LROT_FUNCENTRY( popupInput, luaPopupInput )
#endif
  LROT_FUNCENTRY( popupWarning, luaPopupWarning )
  LROT_FUNCENTRY( popupConfirmation, luaPopupConfirmation )
  LROT_FUNCENTRY( defaultStick, luaDefaultStick )
  LROT_FUNCENTRY( defaultChannel, luaDefaultChannel )
  LROT_FUNCENTRY( getRSSI, luaGetRSSI )
  LROT_FUNCENTRY( killEvents, luaKillEvents )
  LROT_FUNCENTRY( loadScript, luaLoadScript )
  LROT_FUNCENTRY( getUsage, luaGetUsage )
  LROT_FUNCENTRY( getAvailableMemory, luaGetAvailableMemory )
  LROT_FUNCENTRY( resetGlobalTimer, luaResetGlobalTimer )
#if LCD_DEPTH > 1 && !defined(COLORLCD)
  LROT_FUNCENTRY( GREY, luaGrey )
#endif
#if defined(PXX2)
  LROT_FUNCENTRY( accessTelemetryPush, luaAccessTelemetryPush )
#endif
  LROT_FUNCENTRY( sportTelemetryPop, luaSportTelemetryPop )
  LROT_FUNCENTRY( sportTelemetryPush, luaSportTelemetryPush )
  LROT_FUNCENTRY( setTelemetryValue, luaSetTelemetryValue )
#if defined(CROSSFIRE)
  LROT_FUNCENTRY( crossfireTelemetryPop, luaCrossfireTelemetryPop )
  LROT_FUNCENTRY( crossfireTelemetryPush, luaCrossfireTelemetryPush )
#endif
#if defined(GHOST)
  LROT_FUNCENTRY( ghostTelemetryPop, luaGhostTelemetryPop )
  LROT_FUNCENTRY( ghostTelemetryPush, luaGhostTelemetryPush )
#endif
#if defined(MULTIMODULE)
  LROT_FUNCENTRY( multiBuffer, luaMultiBuffer )
#endif
  LROT_FUNCENTRY( setSerialBaudrate, luaSetSerialBaudrate )
  LROT_FUNCENTRY( serialWrite, luaSerialWrite )
  LROT_FUNCENTRY( serialRead, luaSerialRead )
#if defined(SWSERIALPOWER) && !defined(SIMU)
  LROT_FUNCENTRY( serialGetPower, luaSerialGetPower )
  LROT_FUNCENTRY( serialSetPower, luaSerialSetPower )
#endif
#if defined(COLORLCD)
  LROT_FUNCENTRY( setShmVar, luaSetShmVar )
  LROT_FUNCENTRY( getShmVar, luaGetShmVar )
#endif
  LROT_FUNCENTRY( setStickySwitch, luaSetStickySwitch )
  LROT_FUNCENTRY( getLogicalSwitchValue, luaGetLogicalSwitchValue )
  LROT_FUNCENTRY( getSwitchIndex, luaGetSwitchIndex )
  LROT_FUNCENTRY( getSwitchName, luaGetSwitchName )
  LROT_FUNCENTRY( getSwitchValue, luaGetSwitchValue )
  LROT_FUNCENTRY( switches, luaSwitches )
  LROT_FUNCENTRY( getSourceIndex, luaGetSourceIndex )
  LROT_FUNCENTRY( getSourceName, luaGetSourceName )
  LROT_FUNCENTRY( sources, luaSources )
#if defined(LED_STRIP_GPIO)
  LROT_FUNCENTRY(setRGBLedColor, luaSetRgbLedColor )
  LROT_FUNCENTRY(applyRGBLedColors, luaApplyRGBLedColors )
#endif
LROT_END(etxlib, NULL, 0)

LROT_BEGIN(etxcst, NULL, 0)
  // Constants
  LROT_NUMENTRY( FULLSCALE, RESX )
#if defined(COLORLCD)
  LROT_NUMENTRY( STDSIZE, FONT(STD) )
  LROT_NUMENTRY( XXLSIZE, FONT(XXL) )
  LROT_NUMENTRY( DBLSIZE, FONT(XL) )
  LROT_NUMENTRY( MIDSIZE, FONT(L) )
  LROT_NUMENTRY( SMLSIZE, FONT(XS) )
  LROT_NUMENTRY( TINSIZE, FONT(XXS) )
  LROT_NUMENTRY( BOLD, FONT(BOLD) )
  LROT_NUMENTRY( BLINK, BLINK )
  LROT_NUMENTRY( INVERS, INVERS )
  LROT_NUMENTRY( VCENTER, VCENTERED )
#else
  LROT_NUMENTRY( XXLSIZE, XXLSIZE )
  LROT_NUMENTRY( DBLSIZE, DBLSIZE )
  LROT_NUMENTRY( MIDSIZE, MIDSIZE )
  LROT_NUMENTRY( SMLSIZE, SMLSIZE )
  LROT_NUMENTRY( BOLD, BOLD )
  LROT_NUMENTRY( BLINK, BLINK )
  LROT_NUMENTRY( INVERS, INVERS )
#endif
  LROT_NUMENTRY( RIGHT, RIGHT )
  LROT_NUMENTRY( LEFT, LEFT )
  LROT_NUMENTRY( CENTER, CENTERED )
  LROT_NUMENTRY( PREC1, PREC1 )
  LROT_NUMENTRY( PREC2, PREC2 )
  LROT_NUMENTRY( VALUE, INPUT_TYPE_VALUE )
  LROT_NUMENTRY( SOURCE, INPUT_TYPE_SOURCE )
  LROT_NUMENTRY( REPLACE, MLTPX_REPL )
  LROT_NUMENTRY( MIXSRC_MIN, MIXSRC_MIN )
  LROT_NUMENTRY( MIXSRC_MAX, MIXSRC_MAX )
  LROT_NUMENTRY( MIXSRC_FIRST_INPUT, MIXSRC_FIRST_INPUT )
  #include "lua_mixsrc.inc"
  LROT_NUMENTRY( MIXSRC_CH1, MIXSRC_FIRST_CH )
  LROT_NUMENTRY( SWSRC_LAST, SWSRC_LAST_LOGICAL_SWITCH )
  LROT_NUMENTRY( SWITCH_COUNT, SWSRC_COUNT )
  LROT_NUMENTRY( MAX_SENSORS, MAX_TELEMETRY_SENSORS )

  LROT_NUMENTRY( MAX_OUTPUT_CHANNELS, MAX_OUTPUT_CHANNELS )
  LROT_NUMENTRY( LIMIT_EXT_PERCENT, LIMIT_EXT_PERCENT )
  LROT_NUMENTRY( LIMIT_STD_PERCENT, LIMIT_STD_PERCENT )

  LROT_NUMENTRY( LS_FUNC_NONE, LS_FUNC_NONE )
  LROT_NUMENTRY( LS_FUNC_VEQUAL, LS_FUNC_VEQUAL )
  LROT_NUMENTRY( LS_FUNC_VALMOSTEQUAL, LS_FUNC_VALMOSTEQUAL )
  LROT_NUMENTRY( LS_FUNC_VPOS, LS_FUNC_VPOS )
  LROT_NUMENTRY( LS_FUNC_VNEG, LS_FUNC_VNEG )
  LROT_NUMENTRY( LS_FUNC_APOS, LS_FUNC_APOS )
  LROT_NUMENTRY( LS_FUNC_ANEG, LS_FUNC_ANEG )
  LROT_NUMENTRY( LS_FUNC_AND, LS_FUNC_AND )
  LROT_NUMENTRY( LS_FUNC_OR, LS_FUNC_OR )
  LROT_NUMENTRY( LS_FUNC_XOR, LS_FUNC_XOR )
  LROT_NUMENTRY( LS_FUNC_EDGE, LS_FUNC_EDGE )
  LROT_NUMENTRY( LS_FUNC_EQUAL, LS_FUNC_EQUAL )
  LROT_NUMENTRY( LS_FUNC_GREATER, LS_FUNC_GREATER )
  LROT_NUMENTRY( LS_FUNC_LESS, LS_FUNC_LESS )
  LROT_NUMENTRY( LS_FUNC_DIFFEGREATER, LS_FUNC_DIFFEGREATER )
  LROT_NUMENTRY( LS_FUNC_ADIFFEGREATER, LS_FUNC_ADIFFEGREATER )
  LROT_NUMENTRY( LS_FUNC_TIMER, LS_FUNC_TIMER )
  LROT_NUMENTRY( LS_FUNC_STICKY, LS_FUNC_STICKY )

  LROT_NUMENTRY( FUNC_OVERRIDE_CHANNEL, FUNC_OVERRIDE_CHANNEL )
  LROT_NUMENTRY( FUNC_TRAINER, FUNC_TRAINER )
  LROT_NUMENTRY( FUNC_INSTANT_TRIM, FUNC_INSTANT_TRIM )
  LROT_NUMENTRY( FUNC_RESET, FUNC_RESET )
  LROT_NUMENTRY( FUNC_SET_TIMER, FUNC_SET_TIMER )
  LROT_NUMENTRY( FUNC_ADJUST_GVAR, FUNC_ADJUST_GVAR )
  LROT_NUMENTRY( FUNC_VOLUME, FUNC_VOLUME )
  LROT_NUMENTRY( FUNC_SET_FAILSAFE, FUNC_SET_FAILSAFE )
  LROT_NUMENTRY( FUNC_RANGECHECK, FUNC_RANGECHECK )
  LROT_NUMENTRY( FUNC_BIND, FUNC_BIND )
  LROT_NUMENTRY( FUNC_PLAY_SOUND, FUNC_PLAY_SOUND )
  LROT_NUMENTRY( FUNC_PLAY_TRACK, FUNC_PLAY_TRACK )
  LROT_NUMENTRY( FUNC_PLAY_VALUE, FUNC_PLAY_VALUE )
  LROT_NUMENTRY( FUNC_PLAY_SCRIPT, FUNC_PLAY_SCRIPT )
  LROT_NUMENTRY( FUNC_BACKGND_MUSIC, FUNC_BACKGND_MUSIC )
  LROT_NUMENTRY( FUNC_BACKGND_MUSIC_PAUSE, FUNC_BACKGND_MUSIC_PAUSE )
  LROT_NUMENTRY( FUNC_VARIO, FUNC_VARIO )
  LROT_NUMENTRY( FUNC_HAPTIC, FUNC_HAPTIC )
  LROT_NUMENTRY( FUNC_LOGS, FUNC_LOGS )
  LROT_NUMENTRY( FUNC_BACKLIGHT, FUNC_BACKLIGHT )
  LROT_NUMENTRY( FUNC_SCREENSHOT, FUNC_SCREENSHOT )
  LROT_NUMENTRY( FUNC_RACING_MODE, FUNC_RACING_MODE )
#if defined(FUNCTION_SWITCHES)
  LROT_NUMENTRY( FUNC_PUSH_CUST_SWITCH, FUNC_PUSH_CUST_SWITCH )
#endif
  LROT_NUMENTRY( FUNC_SET_SCREEN, FUNC_SET_SCREEN )
#if defined(COLORLCD)
  LROT_NUMENTRY( FUNC_DISABLE_TOUCH, FUNC_DISABLE_TOUCH )

  LROT_NUMENTRY( SHADOWED, SHADOWED )
  // ZoneType::Integer == INPUT_TYPE_VALUE - use VALUE in widget options
  // ZoneType::Source == INPUT_TYPE_SOURCE - use SOURCE in widget options
  LROT_NUMENTRY( COLOR, ZoneOption::Color )
  LROT_NUMENTRY( BOOL, ZoneOption::Bool )
  LROT_NUMENTRY( STRING, ZoneOption::String )
  LROT_NUMENTRY( TIMER, ZoneOption::Timer )
  LROT_NUMENTRY( TEXT_SIZE, ZoneOption::TextSize )
  LROT_NUMENTRY( ALIGNMENT, ZoneOption::Align )
  LROT_NUMENTRY( SWITCH, ZoneOption::Switch )
  LROT_NUMENTRY( SLIDER, ZoneOption::Slider )
  LROT_NUMENTRY( CHOICE, ZoneOption::Choice )
  LROT_NUMENTRY( FILE, ZoneOption::File )
  LROT_NUMENTRY( MENU_HEADER_HEIGHT, COLOR2FLAGS(EdgeTxStyles::MENU_HEADER_HEIGHT) )

  // Colors gui/colorlcd/colors.h
  LROT_NUMENTRY( COLOR_THEME_PRIMARY1, COLOR2FLAGS(COLOR_THEME_PRIMARY1_INDEX) )
  LROT_NUMENTRY( COLOR_THEME_PRIMARY2, COLOR2FLAGS(COLOR_THEME_PRIMARY2_INDEX) )
  LROT_NUMENTRY( COLOR_THEME_PRIMARY3, COLOR2FLAGS(COLOR_THEME_PRIMARY3_INDEX) )
  LROT_NUMENTRY( COLOR_THEME_SECONDARY1, COLOR2FLAGS(COLOR_THEME_SECONDARY1_INDEX) )
  LROT_NUMENTRY( COLOR_THEME_SECONDARY2, COLOR2FLAGS(COLOR_THEME_SECONDARY2_INDEX) )
  LROT_NUMENTRY( COLOR_THEME_SECONDARY3, COLOR2FLAGS(COLOR_THEME_SECONDARY3_INDEX) )
  LROT_NUMENTRY( COLOR_THEME_FOCUS, COLOR2FLAGS(COLOR_THEME_FOCUS_INDEX) )
  LROT_NUMENTRY( COLOR_THEME_EDIT, COLOR2FLAGS(COLOR_THEME_EDIT_INDEX) )
  LROT_NUMENTRY( COLOR_THEME_ACTIVE, COLOR2FLAGS(COLOR_THEME_ACTIVE_INDEX) )
  LROT_NUMENTRY( COLOR_THEME_WARNING, COLOR2FLAGS(COLOR_THEME_WARNING_INDEX) )
  LROT_NUMENTRY( COLOR_THEME_DISABLED, COLOR2FLAGS(COLOR_THEME_DISABLED_INDEX) )
  LROT_NUMENTRY( CUSTOM_COLOR, COLOR2FLAGS(CUSTOM_COLOR_INDEX) )

  // Old style theme color constants
  LROT_NUMENTRY( ALARM_COLOR, COLOR2FLAGS(COLOR_THEME_WARNING_INDEX) )
  LROT_NUMENTRY( BARGRAPH_BGCOLOR, COLOR2FLAGS(COLOR_THEME_SECONDARY3_INDEX) )
  LROT_NUMENTRY( BARGRAPH1_COLOR, COLOR2FLAGS(COLOR_THEME_SECONDARY1_INDEX) )
  LROT_NUMENTRY( BARGRAPH2_COLOR, COLOR2FLAGS(COLOR_THEME_SECONDARY2_INDEX) )
  LROT_NUMENTRY( CURVE_AXIS_COLOR, COLOR2FLAGS(COLOR_THEME_SECONDARY2_INDEX) )
  LROT_NUMENTRY( CURVE_COLOR, COLOR2FLAGS(COLOR_THEME_SECONDARY1_INDEX) )
  LROT_NUMENTRY( CURVE_CURSOR_COLOR, COLOR2FLAGS(COLOR_THEME_WARNING_INDEX) )
  LROT_NUMENTRY( HEADER_BGCOLOR, COLOR2FLAGS(COLOR_THEME_FOCUS_INDEX) )
  LROT_NUMENTRY( HEADER_COLOR, COLOR2FLAGS(COLOR_THEME_SECONDARY1_INDEX) )
  LROT_NUMENTRY( HEADER_CURRENT_BGCOLOR, COLOR2FLAGS(COLOR_THEME_FOCUS_INDEX) )
  LROT_NUMENTRY( HEADER_ICON_BGCOLOR, COLOR2FLAGS(COLOR_THEME_SECONDARY1_INDEX) )
  LROT_NUMENTRY( LINE_COLOR, COLOR2FLAGS(COLOR_THEME_PRIMARY3_INDEX) )
  LROT_NUMENTRY( MAINVIEW_GRAPHICS_COLOR, COLOR2FLAGS(COLOR_THEME_SECONDARY1_INDEX) )
  LROT_NUMENTRY( MAINVIEW_PANES_COLOR, COLOR2FLAGS(COLOR_THEME_PRIMARY2_INDEX) )
  LROT_NUMENTRY( MENU_TITLE_BGCOLOR, COLOR2FLAGS(COLOR_THEME_SECONDARY1_INDEX) )
  LROT_NUMENTRY( MENU_TITLE_COLOR, COLOR2FLAGS(COLOR_THEME_PRIMARY2_INDEX) )
  LROT_NUMENTRY( MENU_TITLE_DISABLE_COLOR, COLOR2FLAGS(COLOR_THEME_PRIMARY3_INDEX) )
  LROT_NUMENTRY( OVERLAY_COLOR, COLOR2FLAGS(COLOR_THEME_PRIMARY1_INDEX) )
  LROT_NUMENTRY( SCROLLBOX_COLOR, COLOR2FLAGS(COLOR_THEME_SECONDARY3_INDEX) )
  LROT_NUMENTRY( TEXT_BGCOLOR, COLOR2FLAGS(COLOR_THEME_SECONDARY3_INDEX) )
  LROT_NUMENTRY( TEXT_COLOR, COLOR2FLAGS(COLOR_THEME_SECONDARY1_INDEX) )
  LROT_NUMENTRY( TEXT_DISABLE_COLOR, COLOR2FLAGS(COLOR_THEME_DISABLED_INDEX) )
  LROT_NUMENTRY( TEXT_INVERTED_BGCOLOR, COLOR2FLAGS(COLOR_THEME_FOCUS_INDEX) )
  LROT_NUMENTRY( TEXT_INVERTED_COLOR, COLOR2FLAGS(COLOR_THEME_PRIMARY2_INDEX) )
  LROT_NUMENTRY( TITLE_BGCOLOR, COLOR2FLAGS(COLOR_THEME_SECONDARY1_INDEX) )
  LROT_NUMENTRY( TRIM_BGCOLOR, COLOR2FLAGS(COLOR_THEME_FOCUS_INDEX) )
  LROT_NUMENTRY( TRIM_SHADOW_COLOR, COLOR2FLAGS(COLOR_THEME_PRIMARY1_INDEX) )
  LROT_NUMENTRY( WARNING_COLOR, COLOR2FLAGS(COLOR_THEME_WARNING_INDEX) )

  // Literal colors
  LROT_NUMENTRY( BLACK, COLOR2FLAGS(COLOR_BLACK_INDEX) )
  LROT_NUMENTRY( WHITE, COLOR2FLAGS(COLOR_WHITE_INDEX) )
  LROT_NUMENTRY( LIGHTWHITE, RGB2FLAGS(0xEA, 0xEA, 0xEA) )
  LROT_NUMENTRY( YELLOW, RGB2FLAGS(0xFF, 0xFF, 0x00) )
  LROT_NUMENTRY( BLUE, RGB2FLAGS(0x00, 0x00, 0xFF) )
  LROT_NUMENTRY( DARKBLUE, RGB2FLAGS(0x00, 0x00, 0xA0) )
  LROT_NUMENTRY( GREY, RGB2FLAGS(0x60, 0x60, 0x60) )
  LROT_NUMENTRY( DARKGREY, RGB2FLAGS(0x40, 0x40, 0x40) )
  LROT_NUMENTRY( LIGHTGREY, RGB2FLAGS(0xC0, 0xC0, 0xC0) )
  LROT_NUMENTRY( RED, RGB2FLAGS(0xFF, 0x00, 0x00) )
  LROT_NUMENTRY( DARKRED, RGB2FLAGS(0xA0, 0x00, 0x00) )
  LROT_NUMENTRY( GREEN, RGB2FLAGS(0x00, 0xFF, 0x00) )
  LROT_NUMENTRY( DARKGREEN, RGB2FLAGS(0x00, 0xA0, 0x00) )
  LROT_NUMENTRY( LIGHTBROWN, RGB2FLAGS(0x9C, 0x6D, 0x20) )
  LROT_NUMENTRY( DARKBROWN, RGB2FLAGS(0x6A, 0x48, 0x10) )
  LROT_NUMENTRY( BRIGHTGREEN, RGB2FLAGS(0x00, 0xB4, 0x3C) )
  LROT_NUMENTRY( ORANGE, RGB2FLAGS(0xE5, 0x64, 0x1E) )

#else
  LROT_NUMENTRY( FIXEDWIDTH, FIXEDWIDTH )
#endif

  // Key events
  #include "lua_keys.inc"

// touch events
#if defined(HARDWARE_TOUCH)
  LROT_NUMENTRY( EVT_TOUCH_FIRST, EVT_TOUCH_FIRST )
  LROT_NUMENTRY( EVT_TOUCH_BREAK, EVT_TOUCH_BREAK )
  LROT_NUMENTRY( EVT_TOUCH_SLIDE, EVT_TOUCH_SLIDE )
  LROT_NUMENTRY( EVT_TOUCH_TAP, EVT_TOUCH_TAP )
#endif

// misc definitions
#if LCD_DEPTH > 1 && !defined(COLORLCD)
  LROT_NUMENTRY( FILL_WHITE, FILL_WHITE )
  LROT_NUMENTRY( GREY_DEFAULT, GREY_DEFAULT )
#endif

#if LCD_W <= 212
  LROT_NUMENTRY( FORCE, FORCE )
  LROT_NUMENTRY( ERASE, ERASE )
  LROT_NUMENTRY( ROUND, ROUND )
#endif

  LROT_NUMENTRY( SOLID, SOLID )
  LROT_NUMENTRY( DOTTED, DOTTED )
  LROT_NUMENTRY( LCD_W, LCD_W )
  LROT_NUMENTRY( LCD_H, LCD_H )
  LROT_NUMENTRY( PLAY_NOW, PLAY_NOW )
  LROT_NUMENTRY( PLAY_BACKGROUND, PLAY_BACKGROUND )
  LROT_NUMENTRY( TIMEHOUR, TIMEHOUR )
#if defined(LED_STRIP_GPIO)
  #if defined(RADIO_V16)
    LROT_NUMENTRY( LED_STRIP_LENGTH, LED_STRIP_LENGTH - 6)
  #else
    LROT_NUMENTRY( LED_STRIP_LENGTH, LED_STRIP_LENGTH )
  #endif
#endif
  LROT_NUMENTRY( UNIT_RAW, UNIT_RAW )
  LROT_NUMENTRY( UNIT_VOLTS, UNIT_VOLTS )
  LROT_NUMENTRY( UNIT_AMPS, UNIT_AMPS )
  LROT_NUMENTRY( UNIT_MILLIAMPS, UNIT_MILLIAMPS )
  LROT_NUMENTRY( UNIT_KTS, UNIT_KTS )
  LROT_NUMENTRY( UNIT_METERS_PER_SECOND, UNIT_METERS_PER_SECOND )
  LROT_NUMENTRY( UNIT_FEET_PER_SECOND, UNIT_FEET_PER_SECOND )
  LROT_NUMENTRY( UNIT_KMH, UNIT_KMH )
  LROT_NUMENTRY( UNIT_MPH, UNIT_MPH )
  LROT_NUMENTRY( UNIT_METERS, UNIT_METERS )
  LROT_NUMENTRY( UNIT_KM, UNIT_KM )
  LROT_NUMENTRY( UNIT_FEET, UNIT_FEET )
  LROT_NUMENTRY( UNIT_CELSIUS, UNIT_CELSIUS )
  LROT_NUMENTRY( UNIT_FAHRENHEIT, UNIT_FAHRENHEIT )
  LROT_NUMENTRY( UNIT_PERCENT, UNIT_PERCENT )
  LROT_NUMENTRY( UNIT_MAH, UNIT_MAH )
  LROT_NUMENTRY( UNIT_WATTS, UNIT_WATTS )
  LROT_NUMENTRY( UNIT_MILLIWATTS, UNIT_MILLIWATTS )
  LROT_NUMENTRY( UNIT_DB, UNIT_DB )
  LROT_NUMENTRY( UNIT_RPMS, UNIT_RPMS )
  LROT_NUMENTRY( UNIT_G, UNIT_G )
  LROT_NUMENTRY( UNIT_DEGREE, UNIT_DEGREE )
  LROT_NUMENTRY( UNIT_RADIANS, UNIT_RADIANS )
  LROT_NUMENTRY( UNIT_MILLILITERS, UNIT_MILLILITERS )
  LROT_NUMENTRY( UNIT_FLOZ, UNIT_FLOZ )
  LROT_NUMENTRY( UNIT_MILLILITERS_PER_MINUTE, UNIT_MILLILITERS_PER_MINUTE )
  LROT_NUMENTRY( UNIT_HERTZ, UNIT_HERTZ )
  LROT_NUMENTRY( UNIT_MS, UNIT_MS )
  LROT_NUMENTRY( UNIT_US, UNIT_US )
  LROT_NUMENTRY( UNIT_DBM, UNIT_DBM )
  LROT_NUMENTRY( UNIT_HOURS, UNIT_HOURS )
  LROT_NUMENTRY( UNIT_MINUTES, UNIT_MINUTES )
  LROT_NUMENTRY( UNIT_SECONDS, UNIT_SECONDS )
  LROT_NUMENTRY( UNIT_CELLS, UNIT_CELLS )
  LROT_NUMENTRY( UNIT_DATETIME, UNIT_DATETIME )
  LROT_NUMENTRY( UNIT_GPS, UNIT_GPS )
  LROT_NUMENTRY( UNIT_BITFIELD, UNIT_BITFIELD )
  LROT_NUMENTRY( UNIT_TEXT, UNIT_TEXT )

  LROT_NUMENTRY( AM_RDO, AM_RDO )
  LROT_NUMENTRY( AM_HID, AM_HID )
  LROT_NUMENTRY( AM_SYS, AM_SYS )
  LROT_NUMENTRY( AM_DIR, AM_DIR )
  LROT_NUMENTRY( AM_ARC, AM_ARC )
LROT_END(etxcst, NULL, 0)

// LUA strings cannot be encoded statically,
// use light user data instead
LROT_BEGIN(etxstr, NULL, 0)
  LROT_LUDENTRY( CHAR_RIGHT, STR_CHAR_RIGHT )
  LROT_LUDENTRY( CHAR_LEFT, STR_CHAR_LEFT )
  LROT_LUDENTRY( CHAR_UP, STR_CHAR_UP )
  LROT_LUDENTRY( CHAR_DOWN, STR_CHAR_DOWN )
  LROT_LUDENTRY( CHAR_DELTA, STR_CHAR_DELTA )
  LROT_LUDENTRY( CHAR_STICK, STR_CHAR_STICK )
  LROT_LUDENTRY( CHAR_POT, STR_CHAR_POT )
  LROT_LUDENTRY( CHAR_SLIDER, STR_CHAR_SLIDER )
  LROT_LUDENTRY( CHAR_SWITCH, STR_CHAR_SWITCH )
  LROT_LUDENTRY( CHAR_TRIM, STR_CHAR_TRIM )
  LROT_LUDENTRY( CHAR_INPUT, STR_CHAR_INPUT )
  LROT_LUDENTRY( CHAR_FUNCTION, STR_CHAR_FUNCTION )
  LROT_LUDENTRY( CHAR_CYC, STR_CHAR_CYC )
  LROT_LUDENTRY( CHAR_TRAINER, STR_CHAR_TRAINER )
  LROT_LUDENTRY( CHAR_CHANNEL, STR_CHAR_CHANNEL )
  LROT_LUDENTRY( CHAR_TELEMETRY, STR_CHAR_TELEMETRY )
  LROT_LUDENTRY( CHAR_LUA, STR_CHAR_LUA )
LROT_END(etxstr, NULL, 0)
