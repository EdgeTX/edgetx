# Dynamic Array Allocation for EdgeTX ModelData

## Context

The `ModelData` struct used fixed-size arrays and bit-packed cross-references that wasted memory on simple models while limiting power users. This project addresses both problems through arena-based dynamic allocation and structured source/switch references.

---

## Completed Work

### Phase 1: Accessor Abstraction

All ModelData array access routed through accessor functions. No direct `g_model.mixData[i]` etc. remains in non-test code.

- `mixAddress(idx)`, `expoAddress(idx)`, `curveHeaderAddress(idx)`, `curvePointsBase()`, `lswAddress(idx)`, `customFnAddress(idx)`
- Centralized `expos.cpp`, `customfn.cpp` (insert/delete/copy/move)
- `getExpoCount()` / `updateExpoCount()` paralleling `getMixCount()`

### Phase 2: Arena Allocator

Six arrays removed from ModelData and stored in a shared arena. **ModelData shrunk from ~6.5 KB to ~2.5 KB.**

Key components:
- **`ModelArena`** class: contiguous buffer with section-based layout, `insertSlot`/`deleteSlot`, `uint32_t` capacity/offsets
- **YAML `YDT_EXTERN_ARRAY`**: tree walker redirects data access to arena via `get_ptr` callbacks
- **`CUST_EXTERN_ARRAY`** macro + `generate_yaml.py` support
- **RTC backup**: arena included in `RamBackupUncompressed`
- **Label operations**: `patchModelYamlLabels()` for direct YAML text patching
- Arena initialized with max-size layout (backward-compatible with old static arrays)

### Phase 3b: Structured Source/Switch References

Replaced the monolithic `MixSources`/`SwitchSources` enums and 10-bit packed fields with 32-bit structured types. **Fixes the H7 telemetry sensor overflow bug and removes all source/switch index limits.**

Three new types (`sourceref.h`):

```cpp
struct SourceRef {        // replaces srcRaw:10 (MixSources enum)
    uint8_t  type;        // SOURCE_TYPE_NONE, SOURCE_TYPE_INPUT, SOURCE_TYPE_STICK, ...
    uint8_t  flags;       // SOURCE_FLAG_INVERTED (replaces negative srcRaw encoding)
    uint16_t index;       // 0-65535 within type
};

struct SwitchRef {        // replaces swtch:10 (SwitchSources enum)
    uint8_t  type;        // SWITCH_TYPE_NONE, SWITCH_TYPE_SWITCH, SWITCH_TYPE_LOGICAL, ...
    uint8_t  flags;       // SWITCH_FLAG_INVERTED (replaces negative swtch encoding)
    uint16_t index;       // 0-65535 within type
};

struct ValueOrSource {    // replaces SourceNumVal (11-bit packed)
    int16_t  value;       // numeric value, or source index when isSource=1
    uint8_t  isSource;    // 0 = numeric, 1 = source reference
    uint8_t  srcType;     // SourceType (when isSource=1)
};
```

All three are 4 bytes, 32-bit word-aligned.

**Structs migrated:**

| Struct | Old size | New size | Fields changed |
|--------|----------|----------|----------------|
| MixData | 20 B | 35 B | srcRaw→SourceRef, swtch→SwitchRef, weight→ValueOrSource, offset→ValueOrSource |
| ExpoData | 18 B | 33 B | srcRaw→SourceRef, swtch→SwitchRef, weight→ValueOrSource, offset→ValueOrSource |
| CurveRef | 2 B | 6 B | type→uint8_t, value→ValueOrSource |
| LogicalSwitchData | 9 B | 14 B | v1/v3 widened to int16_t, andsw→SwitchRef |
| CustomFunctionData | 11 B | 14 B | swtch→SwitchRef, func widened to uint8_t |
| FlightModeData | +2 B | +2 B | swtch→SwitchRef (removed spare:6) |
| TimerData | +3 B | +3 B | swtch→SwitchRef (restructured bit-fields) |
| SwashRingData | 8 B | 17 B | collectiveSource/aileronSource/elevatorSource→SourceRef |
| ScriptDataInput | 2 B | 4 B | source→SourceRef (union with int16_t value) |
| ModuleData.crsf | unchanged | unchanged | crsfArmingTrigger→SwitchRef (within union) |
| RadioData | +198 B | +198 B | backlightSrc/volumeSrc→SourceRef, customFn[64] +3 each |

**Bridge functions** (temporary, until all code uses structured types natively):
- `sourceRefToMixSrc()` / `switchRefToSwSrc()` in `mixer.cpp` — convert new→old for `getValue()`/`getSwitch()`/GUI functions that still expect enum integers
- `mixSrcToSourceRef()` / `swSrcToSwitchRef()` in `mixer.cpp` — reverse, declared in `myeeprom.h` for global access
- `valueOrSourceToLegacy()` / `legacyToValueOrSource()` — bridge ValueOrSource↔SourceNumVal packed format

**Files updated:** mixer.cpp, mixes.cpp, expos.cpp, model_init.cpp, curves.cpp, edgetx.cpp, switches.cpp, timers.cpp, all GUI editors (colorlcd + 212x64 + 128x64), Lua API (api_model.cpp, interface.cpp), telemetry/crossfire.cpp, pulses/crossfire.cpp, yaml_datastructs_funcs.cpp, all test files.

**Current arena element sizes** (updated):

| Array | Element size | Per-radio max | Hard cap |
|-------|-------------|---------------|----------|
| mixData | 35 B | 64 | 128 |
| expoData | 33 B | 64 | 128 |
| curves | 4 B | 32 | 64 |
| points | 1 B | 512 | 1024 |
| logicalSw | 18 B | 64 | 64 |
| customFn | 14 B | 64 | 64 |

Default arena usage: 64×35 + 64×33 + 32×4 + 512 + 64×18 + 64×14 = **6,976 B** (up from 4,352 due to larger structs).

Platform arena sizes need updating:

| Platform | MODEL_ARENA_SIZE | Default used | Status |
|----------|-----------------|-------------|--------|
| STM32F4 | 4096 B | 6,976 B | **Needs increase** |
| STM32H7 | 8192 B | 6,976 B | **Needs increase** |
| Companion/Sim | 65536 B | 6,976 B | OK |

---

## Remaining Work (Phase 3b cleanup)

### Phase 3b status: GUI migration COMPLETE

All GUI and infrastructure code uses SourceRef/SwitchRef/ValueOrSource natively. **Zero non-boundary bridge calls remain.**

Bridge functions (`sourceRefToMixSrc`, `mixSrcToSourceRef`, `switchRefToSwSrc`, `swSrcToSwitchRef`) are now used exclusively at format boundaries:

#### YAML format boundary (~20 usages)
`yaml_datastructs_funcs.cpp` — `r_logicSw`/`w_logicSw`, `r_customFn`/`w_customFn`, `r_mixSrcRawEx`/`w_mixSrcRawEx`, `r_swtchSrc`/`w_swtchSrc`. These translate between YAML's integer source/switch format and SourceRef/SwitchRef structs.

#### Lua API boundary (~32 usages)
`lua/api_model.cpp`, `lua/api_general.cpp`, `lua/api_stdlcd.cpp`, `lua/api_colorlcd.cpp`, `lua/interface.cpp`, `lua/lua_lvgl_widget.cpp`, `strhelpers.cpp` (`getSwitchIndex`/`getSourceIndex`). Lua exposes sources/switches as `MIXSRC_*`/`SWSRC_*` integers for backward compatibility.

### Completed cleanup items
- ✅ 3b.1: isSourceAvailable/isSwitchAvailable SourceRef overloads
- ✅ 3b.2: applyExpos parameter → SourceRef
- ✅ 3b.3: s_currSrcRaw → SourceRef
- ✅ 3b.4: LogicalSwitchData v1/v2 → LSValue union
- ✅ 3b.5: mixer internals (getSourceTrimValue, calcVolume/Backlight)
- ✅ 3b.6: throttle source accessors
- ✅ 3b.7: source_numberedit ValueOrSource constructor
- ✅ 3b.8: widget_settings memcpy storage
- ✅ 3b.9: backward-compat overloads removed (getSourceString, drawSource, drawSwitch, getSwitchPositionName, getSwitch)
- ✅ 3b.10: checkIncDecSource/checkIncDecSwitch with type bitmasks (native iteration, no bridge)
- ✅ 3b.11: Lua API plan (32-bit SourceRef↔integer)
- ✅ 3b.12: editCurveRef converted to native ValueOrSource
- ✅ 3b.13: LS/SF stdlcd editors converted to checkIncDecSource/Switch
- ✅ 3b.14: editValueOrSource for weight/offset (pointer-based, replaces editSrcVarFieldValue)
- ✅ 3b.15: Native auto-source/auto-switch popups in checkIncDecSource/Switch
- ✅ 3b.16: Removed legacy navigation popup system
- ✅ 3b.17: Removed INCDEC_SOURCE/SWITCH flags, CHECK_INCDEC_SWITCH macros, isSourceAvailableInt
- ✅ 3b.18: Removed dead checkIncDec(int, ..., srcMin, srcMax) overload
- ✅ 3b.19: Simplified navigation_9x/navigation_x7 checkIncDec
- ✅ 3b.20: drawSourceCustomValue/getSourceCustomValueString → SourceRef
- ✅ 3b.21: drawSourceValue → SourceRef
- ✅ 3b.22: getMixSrcRange(int) removed, SourceRef-only
- ✅ 3b.23: Telemetry display (model_display, view_telemetry) → SourceRef
- ✅ 3b.24: Logical switch display (all 3 UIs) → SourceRef
- ✅ 3b.25: drawTelemScreenDate → sensor index (no source_t)
- ✅ 3b.26: getValueOrSrcVarString → ValueOrSource (no legacy SourceNumVal)
- ✅ 3b.27: setWeight → ValueOrSource, valueOrSourceToLegacy removed
- ✅ 3b.28: All dead code removed (legacyToValueOrSource, checkIncDecMovedSwitch, stopsSwitch, CATEGORY_END)

### Next steps

#### 3b.29 YAML serialization update
The generated YAML struct definitions still reference old field bit widths. Need custom read/write for SourceRef/SwitchRef/ValueOrSource and regeneration of all 22 yaml_datastructs_*.cpp files.

#### 3b.30 Lua API conversion

Convert the entire Lua API from `MIXSRC_*`/`SWSRC_*` 16-bit integers to `SourceRef::toUint32()`/`SwitchRef::toUint32()` packed 32-bit values. This is the last bridge function consumer.

**Packing format** (already implemented in `sourceref.h`):
- `toUint32()`: `index | (flags << 16) | (type << 24)` — type in high byte
- `fromUint32()`: reverse
- Any non-NONE value packs to >= 0x01000000, distinguishable from legacy 10-bit values

**Step 1: Widen LuaField infrastructure**

| File | Change |
|------|--------|
| `lua/lua_api.h` | `LuaField.id`: `uint16_t` → `uint32_t` |
| `lua/api_general.cpp` | `LuaSingleField.id`: `uint16_t` → `uint32_t` |
| `lua/api_general.cpp` | `LuaMultipleField.id`: `uint16_t` → `uint32_t` |

**Step 2: Convert Lua field tables to use toUint32()**

`luaSingleFields[]` entries like `{MIXSRC_TILT_X, "tiltx", ...}` become
`{SourceRef_(SOURCE_TYPE_IMU, 0).toUint32(), "tiltx", ...}`.

`luaMultipleFields[]` entries like `{MIXSRC_FIRST_INPUT, "input", ..., MAX_INPUTS}` become
`{SourceRef_(SOURCE_TYPE_INPUT, 0).toUint32(), "input", ..., MAX_INPUTS}`.

The `id + offset` arithmetic in `luaFindFieldByName`/`luaFindFieldById` works because
`toUint32()` puts index in bits 0-15, so incrementing by 1 correctly advances the index.

**Step 3: Convert luaPushFieldValue()**

Replace `mixsrc_t src` parameter with `SourceRef`. Replace all `MIXSRC_*` range checks
with `ref.type == SOURCE_TYPE_*` dispatch. This function handles telemetry precision,
GVar precision, TX voltage scaling, etc.

**Step 4: Convert Lua constant globals**

`MIXSRC_Rud`, `MIXSRC_Thr`, etc. are generated from `lua_inputs.inc` (Jinja template
from hardware descriptions). Change the template to emit `toUint32()` packed values:
```
{ "MIXSRC_{{ li.lua }}", SourceRef_(SOURCE_TYPE_STICK, {{ loop.index0 }}).toUint32() },
```

Same for `SWSRC_*` constants in switch-related Lua globals.

**Step 5: Convert API functions**

| Function | File | Change |
|----------|------|--------|
| `getValue()` | `api_general.cpp` | Take `uint32_t` from Lua → `SourceRef::fromUint32()` → `getValue(SourceRef)` |
| `getFieldInfo()` | `api_general.cpp` | Return `toUint32()` in `field.id` |
| `getSourceName()` | `api_general.cpp` | Take `uint32_t` → `fromUint32()` |
| `getSwitchName()` | `api_general.cpp` | Take `uint32_t` → `fromUint32()` |
| `getSwitchValue()` | `api_general.cpp` | Take `uint32_t` → `fromUint32()` |
| `getSourceIndex()` | `strhelpers.cpp` | Return `toUint32()` (already done) |
| `getSwitchIndex()` | `strhelpers.cpp` | Return `toUint32()` (already done) |
| `drawSource()` | `api_stdlcd.cpp`, `api_colorlcd.cpp` | Take `uint32_t` → `fromUint32()` |
| `drawSwitch()` | `api_stdlcd.cpp`, `api_colorlcd.cpp` | Take `uint32_t` → `fromUint32()` |
| `model.getInput/setInput` | `api_model.cpp` | Source/switch fields use `toUint32()`/`fromUint32()` |
| `model.getMix/setMix` | `api_model.cpp` | Same |
| `model.getTimer/setTimer` | `api_model.cpp` | Switch field |
| `model.getFlightMode/setFlightMode` | `api_model.cpp` | Switch field |
| `model.getLogicalSwitch/setLogicalSwitch` | `api_model.cpp` | Source/switch v1/v2/andsw |
| `model.getCustomFunction` | `api_model.cpp` | Switch field |
| `model.getSwashRing/setSwashRing` | `api_model.cpp` | Source fields |

**Step 6: Convert lua_widget_factory.cpp**

`sourceValue()` / `switchValue()`: already call `getSourceIndex()`/`getSwitchIndex()` which
will return `toUint32()` values. Sentinel checks need updating (0 = NONE packed).

**Step 7: Convert lua_lvgl_widget.cpp**

SourceChoice/SwitchChoice lambdas: use `fromUint32()`/`toUint32()` instead of bridge functions.
Available handlers: same.

**Step 8: Convert interface.cpp**

`luaGetValueAndPush`: pass `toUint32()` packed value.

**Step 9: Update Lua tests**

`tests/lua.cpp`: Update `MIXSRC_Thr`, `SWSRC_FIRST_SWITCH` references. The test Lua scripts
use global constants which will be automatically updated by step 4.

**Step 10: Remove bridge functions**

Once all consumers are converted:
- Remove `sourceRefToMixSrc()` / `mixSrcToSourceRef()` from `mixer.cpp`
- Remove `switchRefToSwSrc()` / `swSrcToSwitchRef()` from `mixer.cpp`/`switches.cpp`
- Remove declarations from `myeeprom.h`
- Remove `MixSources` / `SwitchSources` enums if no longer referenced

**Key insight**: `toUint32()` puts `index` in bits 0-15, so `base_id + n` arithmetic
still works for the `luaMultipleFields` table — incrementing by 1 advances the source
index within the same type. This preserves the existing field lookup algorithm.

**Backward compatibility**: Lua scripts that use string names (`getSourceIndex("Rud")`,
`getValue("Thr")`) work unchanged. Scripts that hardcode numeric constants
(`getValue(MIXSRC_Thr)`) get the correct packed value from the updated Lua globals.
Scripts that hardcode raw integers (`getValue(4)`) will break — but this was
undocumented and fragile.

---

## Phase 4: Dynamic Arena Sizing

1. Populate `ModelDynData` counts after model load
2. Compact arena layout based on actual usage
3. Arena-aware insert/delete
4. Dynamic loop bounds in mixer hot path
5. GUI memory indicator
6. STM32F4 arena sizing resolution (now critical: default layout 6,208B > MODEL_ARENA_SIZE 4,096B)

---

## Phase 5 (Future): GVars + FlightModes

## Phase 6 (Future): Growable Arena

---

## Known Issues

1. **STM32F4 arena undersize**: default layout (6,976 B) exceeds `MODEL_ARENA_SIZE` (4096 B). Must be resolved before merging.

2. **Bridge functions**: RESOLVED for GUI/infrastructure. Bridges remain only at YAML and Lua API boundaries (~52 usages total). These are permanent until those formats change.

3. **modelslist `updateModelCell()`**: reads temp models via `readModelYaml()` with arena save/restore. Could be replaced with header-only read.

4. **`curveEnd[]` parallel array**: sized `MAX_CURVES` (32), should match `MAX_CURVES_HARD` (64).
