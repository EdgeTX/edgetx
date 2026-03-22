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
| logicalSw | 9 B | 64 | 64 |
| customFn | 11 B | 64 | 64 |

Default arena usage: 64×35 + 64×33 + 32×4 + 512 + 64×9 + 64×11 = **6,208 B** (up from 4,352 due to larger MixData/ExpoData).

Platform arena sizes need updating:

| Platform | MODEL_ARENA_SIZE | Default used | Status |
|----------|-----------------|-------------|--------|
| STM32F4 | 4096 B | 6,208 B | **Needs increase** |
| STM32H7 | 8192 B | 6,208 B | OK |
| Companion/Sim | 65536 B | 6,208 B | OK |

---

## Remaining Work

### Immediate: Fix STM32F4 arena size

The default arena layout (6,208 B) now exceeds `MODEL_ARENA_SIZE` on STM32F4 (4096 B). Options:
1. Increase STM32F4 arena to 8192 B (same as H7)
2. Enable dynamic arena sizing so simple models fit in 4096 B
3. Reduce per-radio MAX_MIXERS/MAX_EXPOS on F4 boards

### Immediate: Migrate remaining structs

These structs still use old 10-bit `swtch` fields:
- `LogicalSwitchData` — `v1:10`, `v3:10`, `andsw:10` (polymorphic: source or switch depending on func)
- `CustomFunctionData` — `swtch:10`
- `FlightModeData` — `swtch:10`
- `TimerData` — `swtch:10`
- `SwashRingData` — `collectiveSource:8`, `aileronSource:8`, `elevatorSource:8` (uint8_t MixSources)
- `RadioData` — `backlightSrc:10`, `volumeSrc:10`
- `ModuleData.crsf` — `crsfArmingTrigger:10`

### Immediate: Remove bridge functions

Once all structs are migrated, update `getValue()` and `getSwitch()` to accept `SourceRef`/`SwitchRef` directly instead of `mixsrc_t`/`swsrc_t`. Then remove `sourceRefToMixSrc()`, `switchRefToSwSrc()`, and the reverse bridges. The `MixSources`/`SwitchSources` enums become internal helpers, no longer a storage format.

### Immediate: Update YAML serialization

The generated YAML struct definitions still reference the old field layout. With SourceRef/SwitchRef/ValueOrSource, the YAML read/write functions need updating:
- New custom read/write for SourceRef (type+index format)
- New custom read/write for SwitchRef (type+index format)
- New custom read/write for ValueOrSource
- Regenerate all 22 yaml_datastructs_*.cpp files
- Backward compatibility: YAML reader should accept both old (enum name) and new (type+index) formats

---

## Phase 3b cleanup: Remaining bridge function elimination

The core runtime, display functions, and GUI widgets now use SourceRef/SwitchRef natively. Bridge functions (`sourceRefToMixSrc`, `mixSrcToSourceRef`, `switchRefToSwSrc`, `swSrcToSwitchRef`) are confined to internal infrastructure. The following items eliminate the remaining ~90 non-Lua bridge usages.

### 3b.1 Convert `isSourceAvailable`/`isSwitchAvailable` to SourceRef/SwitchRef

These filter functions are called from SourceChoice/SwitchChoice availability handlers, GUI editors, and `checkIncDecSource`/`checkIncDecSwitch`. Currently take `mixsrc_t`/`swsrc_t`.

Files: `gui/gui_common.cpp`, `gui_common.h`, all callers.

### 3b.2 Convert `applyExpos()` parameter from `mixsrc_t` to `SourceRef`

`applyExpos(anas, mode, mixsrc_t srcRaw, value)` is called from input_edit preview code (colorlcd + stdlcd). Change to `SourceRef`.

Files: `mixer.cpp` (declaration), `edgetx.h`, `gui/colorlcd/model/input_edit.cpp`, `gui/128x64/model_input_edit.cpp`, `gui/212x64/model_input_edit.cpp`.

### 3b.3 Convert `s_currSrcRaw` from `mixsrc_t` to `SourceRef`

Global variable used by stdlcd input/mix editors for "current source being edited". Change type and all assignments/reads.

Files: `gui/128x64/model_input_edit.cpp`, `gui/128x64/model_mix_edit.cpp`, `gui/212x64/model_input_edit.cpp`, `gui/212x64/model_mix_edit.cpp`.

### 3b.4 Convert `LogicalSwitchData.v1`/`v2` to typed fields

These are currently `int16_t` (widened from `:10` but not yet structured). They're polymorphic — hold either a source or switch depending on `func`. Options:
- Use a union: `union { SourceRef src; SwitchRef sw; int16_t value; }`
- Or keep as `int16_t` and bridge at access sites

This is the hardest item due to the polymorphic semantics.

Files: `datastructs_private.h`, `switches.cpp`, `yaml_datastructs_funcs.cpp`, `model_logical_switches.cpp` (all 3 UI variants).

### 3b.5 Convert `getSourceTrimValue`/`calcVolumeValue`/`calcBacklightValue`

Internal mixer functions that take `mixsrc_t`. Change to `SourceRef`.

Files: `mixer.cpp`.

### 3b.6 Convert throttle source accessors

`thrTraceSrc`, `getThrottleStickTrimSource()`, `throttleSource2Source()`, `source2ThrottleSource()` work with `mixsrc_t`. Change to `SourceRef`.

Files: `datastructs_private.h`, `model_setup.cpp`, `throttle_params.cpp`.

### 3b.7 Convert `source_numberedit.cpp`

`SourceNumberEdit` bridges ValueOrSource↔SourceNumVal for the weight/offset editors. Needs native ValueOrSource support.

Files: `gui/colorlcd/controls/source_numberedit.cpp`.

### 3b.8 Convert widget data source/switch storage

Widget option data stores source/switch as integers. Bridge at the widget_settings boundary.

Files: `gui/colorlcd/mainview/widget_settings.cpp`.

### 3b.9 Remove backward-compatible overloads

Once all above items are done:
- Remove `getValue(mixsrc_t)` overload
- Remove `getSwitch(swsrc_t)` inline overload
- Remove `getSourceString(mixsrc_t)` overloads
- Remove `getSwitchPositionName(swsrc_t)` overloads
- Remove `drawSource(mixsrc_t)` / `drawSwitch(swsrc_t)` overloads
- Remove `CFN_SWITCH` macro (callers use `cfn->swtch` directly)
- Remove bridge function declarations from `myeeprom.h`
- Keep bridge functions in `mixer.cpp` as `static` (only used by Lua API)

### 3b.10 YAML serialization update

The generated YAML struct definitions still reference old field bit widths. Need custom read/write for SourceRef/SwitchRef/ValueOrSource and regeneration of all 22 yaml_datastructs_*.cpp files.

### 3b.11 Lua API

SourceRef and SwitchRef are 4 bytes = a 32-bit integer, which Lua supports natively. The Lua API can pack/unpack these directly:

```cpp
uint32_t sourceRefToInt(SourceRef ref);  // memcpy or union
SourceRef intToSourceRef(uint32_t v);
```

No bridge functions needed. Lua scripts use the same type+index addressing as the firmware. The old `MIXSRC_*` / `SWSRC_*` Lua constants become pre-computed 32-bit SourceRef/SwitchRef values.

Files: `lua/api_model.cpp`, `lua/api_general.cpp`, `lua/interface.cpp`, `lua/lua_lvgl_widget.cpp`. Also update Lua constant definitions to generate SourceRef-packed values instead of MixSources enum values.

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

1. **STM32F4 arena undersize**: default layout (6,208 B) exceeds `MODEL_ARENA_SIZE` (4096 B). Must be resolved.

2. **Bridge functions**: `sourceRefToMixSrc()`, `switchRefToSwSrc()` etc. are temporary. They reconstruct old enum values from structured refs and are used by `getValue()`/`getSwitch()`/GUI code that hasn't been fully migrated.

3. **modelslist `updateModelCell()`**: reads temp models via `readModelYaml()` with arena save/restore. Could be replaced with header-only read.

4. **`curveEnd[]` parallel array**: sized `MAX_CURVES` (32), should match `MAX_CURVES_HARD` (64).

5. **Duplicate bridge functions**: RESOLVED — `mixSrcToSourceRef()` and `swSrcToSwitchRef()` are now defined in `mixer.cpp` and declared in `myeeprom.h`. All static duplicates removed.
