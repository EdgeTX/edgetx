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

Bridge function usages remain in two boundary categories (YAML, Lua) plus some unconverted GUI/infrastructure code.

### Permanent boundaries

#### YAML format translation (~20 usages)
`yaml_datastructs_funcs.cpp` — `r_logicSw`/`w_logicSw`, `r_customFn`/`w_customFn`, `r_mixSrcRawEx`/`w_mixSrcRawEx`, `r_swtchSrc`/`w_swtchSrc` use bridges to translate between YAML integer format and SourceRef/SwitchRef. These are the format boundary and remain until YAML format changes.

#### Lua API (~30 usages)
`lua/api_model.cpp`, `lua/api_general.cpp`, `lua/api_stdlcd.cpp`, `lua/api_colorlcd.cpp`, `lua/interface.cpp`, `lua/lua_lvgl_widget.cpp`. SourceRef/SwitchRef are exposed as 32-bit integers to Lua. Bridge functions at this boundary are acceptable.

### Remaining conversion targets

#### source_t telemetry fields (8 usages)
`FrSkyBarData.source`, `FrSkyLineData.sources[]` still use `source_t` (uint16_t). Display code bridges via `sourceRefToMixSrc()`.

Files: `model_display.cpp` (128x64, 212x64), `view_telemetry.cpp` (128x64, 212x64).

#### Logical switch display (6 usages)
`model_logical_switches.cpp` (all 3 UI variants) and `view_logical_switches.cpp` use `sourceRefToMixSrc()` to get integer source values for range-based display logic.

#### strhelpers iteration (3 usages)
`getValueOrSrcVarString`, `getSwitchIndex`, `getSourceString` backward-compat paths still bridge internally.

Files: `strhelpers.cpp`.

#### getMixSrcRange (1 usage)
`edgetx.cpp` calls `getMixSrcRange(sourceRefToMixSrc(source), ...)`. Needs native SourceRef overload.

#### checkIncDecMovedSwitch (1 usage)
`navigation/common.cpp` still has the legacy `checkIncDecMovedSwitch(swsrc_t)` using `switchRefToSwSrc`. Dead code — auto-switch is now handled natively in `checkIncDecSwitch`.

#### myeeprom.h declarations (5 usages)
Bridge function declarations + `CFN_SWITCH` macro. Can be cleaned up once Lua API is converted.

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
- ✅ 3b.16: Removed legacy navigation popup system (onSourceLongEnterPress, onSwitchLongEnterPress, showPopupMenus, checkMovedInput, checkIncDecSelection)
- ✅ 3b.17: Removed INCDEC_SOURCE/SWITCH flags, CHECK_INCDEC_SWITCH macros, isSourceAvailableInt
- ✅ 3b.18: Removed dead checkIncDec(int, ..., srcMin, srcMax) overload
- ✅ 3b.19: Simplified navigation_9x/navigation_x7 checkIncDec to standalone (no source/switch handling)

### Next steps

#### 3b.20 YAML serialization update
The generated YAML struct definitions still reference old field bit widths. Need custom read/write for SourceRef/SwitchRef/ValueOrSource and regeneration of all 22 yaml_datastructs_*.cpp files.

#### 3b.21 Lua API conversion
SourceRef and SwitchRef are 4 bytes = a 32-bit integer, which Lua supports natively. The Lua API can pack/unpack these directly:

```cpp
uint32_t sourceRefToInt(SourceRef ref);  // memcpy or union
SourceRef intToSourceRef(uint32_t v);
```

No bridge functions needed. Lua scripts use the same type+index addressing as the firmware. The old `MIXSRC_*` / `SWSRC_*` Lua constants become pre-computed 32-bit SourceRef/SwitchRef values.

Files: `lua/api_model.cpp`, `lua/api_general.cpp`, `lua/interface.cpp`, `lua/lua_lvgl_widget.cpp`, `lua/api_stdlcd.cpp`, `lua/api_colorlcd.cpp`. Also update Lua constant definitions.

#### 3b.22 Remaining GUI bridge elimination
Convert telemetry display, logical switch display, and strhelpers to use SourceRef/SwitchRef natively without bridging through MixSources/SwitchSources integers.

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

2. **Bridge functions**: `sourceRefToMixSrc()`, `switchRefToSwSrc()` etc. remain for YAML and Lua API boundaries. Non-boundary usage reduced to ~20 sites (telemetry display, logical switch display, strhelpers, getMixSrcRange).

3. **modelslist `updateModelCell()`**: reads temp models via `readModelYaml()` with arena save/restore. Could be replaced with header-only read.

4. **`curveEnd[]` parallel array**: sized `MAX_CURVES` (32), should match `MAX_CURVES_HARD` (64).

5. **Dead code**: RESOLVED — removed `legacyToValueOrSource()`, `checkIncDecMovedSwitch()`, `stopsSwitch`, `CATEGORY_END`, and unused `valueOrSourceToLegacy` extern declarations.
