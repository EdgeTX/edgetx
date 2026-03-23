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
- **RTC backup**: arena elements copied field-by-field via `datacopy.inc` templates, stripping NOBACKUP fields (names). Backup arena buffer sized using `Backup::` struct sizes, capped to `MODEL_ARENA_SIZE`.
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

All bridge functions (`sourceRefToMixSrc`, `mixSrcToSourceRef`, `switchRefToSwSrc`, `swSrcToSwitchRef`, `getValue(mixsrc_t)`) have been removed. All code uses SourceRef/SwitchRef natively.

**Current arena element sizes** (updated):

| Array | Element size | Per-radio max | Hard cap (F4) | Hard cap (H7+) |
|-------|-------------|---------------|---------------|----------------|
| mixData | 35 B | 64 | 64 | 128 |
| expoData | 33 B | 64 | 64 | 128 |
| curves | 4 B | 32 | 64 | 64 |
| points | 1 B | 512 | 1024 | 1024 |
| logicalSw | 18 B | 64 | 64 | 64 |
| customFn | 14 B | 64 | 64 | 64 |

Default arena usage: 64×35 + 64×33 + 32×4 + 512 + 64×18 + 64×14 = **6,976 B** (up from 4,352 due to larger structs).

Platform arena sizes:

| Platform | MODEL_ARENA_SIZE | Default used | Status |
|----------|-----------------|-------------|--------|
| STM32F4 | 4096 B | 6,976 B | **Needs Phase 4 dynamic sizing** |
| STM32H7 | 8192 B | 6,976 B | OK |
| Companion/Sim | 65536 B | 6,976 B | OK |

**F4 RAM impact**: ~+192 B net vs mainline (ModelData shrink offsets arena + backup additions). `MAX_MIXERS_HARD`/`MAX_EXPOS_HARD` capped at 64 on F4. RTC backup arena capped to `MODEL_ARENA_SIZE`.

---

## Remaining Work (Phase 3b cleanup)

### Phase 3b status: ALL CONVERSIONS COMPLETE

All code uses SourceRef/SwitchRef/ValueOrSource natively. **Bridge functions have been fully removed.**

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

- ✅ 3b.29: YAML serialization update (custom read/write for SourceRef/SwitchRef/ValueOrSource)
- ✅ 3b.30: Lua API conversion + full bridge removal

### 3b.30 Lua API conversion (details)

Converted the entire Lua API from `MIXSRC_*`/`SWSRC_*` 16-bit integers to `SourceRef::toUint32()`/`SwitchRef::toUint32()` packed 32-bit values. Also converted all remaining `getValue(MIXSRC_*)` callers and removed bridge functions entirely.

**What changed:**
- `LuaField.id`, `LuaSingleField.id`, `LuaMultipleField.id`: widened from `uint16_t` to `uint32_t`
- All Lua field tables use `SourceRef_().toUint32()` instead of `MIXSRC_*` constants
- `luaGetValueAndPush()` takes `SourceRef` directly (dispatches by `ref.type`)
- `getSourceIndex()` / `getSwitchIndex()` return packed `uint32_t` (0 = not found)
- `valueOrSourceToLuaInt()` returns raw `toUint32()` (no ±1024 offset; packed values ≥ 0x01000000 are self-identifying)
- All `model.getMix/setMix`, `getInput/setInput`, `getTimer/setTimer`, `getFlightMode/setFlightMode`, `getLogicalSwitch/setLogicalSwitch`, `getCustomFunction/setCustomFunction`, `getSwashRing/setSwashRing` use `toUint32()`/`fromUint32()` directly
- `sources()` / `switches()` iterators walk SourceRef/SwitchRef type tables natively
- `checkSourceAvailable(SourceRef, uint32_t)` / `checkSwitchAvailable(SwitchRef, uint32_t)` overloads added
- All ~25 non-Lua `getValue(MIXSRC_*)` callers converted to `getValue(SourceRef_(...))`
- Jinja templates (`lua_inputs.jinja`, `lua_mixsrc.jinja`) emit `SourceRef_().toUint32()` values
- Bridge functions removed: `sourceRefToMixSrc`, `mixSrcToSourceRef`, `switchRefToSwSrc`, `swSrcToSwitchRef`, `getValue(mixsrc_t)` overload
- `CFN_SWITCH` macro updated to `(p)->swtch.toUint32()`

**Backward compatibility:** Lua scripts using string names (`getSourceIndex("Rud")`, `getValue("Thr")`) work unchanged. Scripts using global constants (`getValue(MIXSRC_Thr)`) get correct packed values. Scripts hardcoding raw integers will break.


---

### Phase 4: Dynamic Arena Sizing

Arena is now dynamically sized to actual model usage after load.

- ✅ 4.1: `computeDynCounts()` scans arena contents and populates `ModelDynData` counts
- ✅ 4.2: `ModelArena::compact()` relocates section data in-place to compacted layout
- ✅ 4.3: `modelArenaInit()` starts with zero counts (no F4 overflow)
- ✅ 4.4: `readModelYaml()` uses temp heap buffer for F4 when MAX layout exceeds arena, compacts after parse
- ✅ 4.5: YAML callbacks return `g_model.dyn` counts (MAX during read, actual during write)
- ✅ 4.6: `insertMix/deleteMix`, `insertExpo/deleteExpo`, `insertCustomFn/deleteCustomFn` use `insertInSection`/`deleteFromSection` + update dyn counts
- ✅ 4.7: `ensureSectionCapacity()` for grow-on-demand (LS/CF position-indexed sections)
- ✅ 4.8: `lswAddress()`/`customFnAddress()` return static dummy for out-of-range reads
- ✅ 4.9: Mixer hot path uses `getMixCount()`/`getExpoCount()` instead of MAX constants
- ✅ 4.10: GUI capacity checks use `freeBytes() < sizeof(Element)` + HARD max
- ✅ 4.11: Model memory usage indicator on colorlcd model setup page
- ✅ 4.12: `curveEnd[]` sized to `MAX_CURVES_HARD` (was `MAX_CURVES`)
- ✅ 4.13: Fix YAML round-trip for IDX extern arrays — `toNextElmt()` was growing arena during `generate()` (infinite loop producing duplicate output); `toChild()` after IDX lost element index for extern arrays (data written to element 0)
- ✅ 4.14: `trimTrailingEmpty()` reclaims arena space when UI allocates sparse indexed slots (LS/CF) then backs out without changes or clears trailing slots. Called from close handlers and clear actions.
- ✅ 4.15: Extract `lswAddress`/`lswAllocAt`/`lswTrimTrailing` into `logicalsw.h` (mirrors `customfn.h`)

---

## Phase 5 (Future): GVars + FlightModes

## Phase 6 (Future): Growable Arena

---

## Known Issues

1. **modelslist `updateModelCell()`**: reads temp models via `readModelYaml()` with arena save/restore. Could be replaced with header-only read.

2. ~~**stdlcd LS/CF editing beyond dyn count**~~: Fixed — stdlcd LS editors use `lswAllocAt` when entering edit mode; CF editors call `customFnAllocAt` to grow arena on demand before writing.

3. **Future: eliminate `ramBackupUncompressed` buffer**: replace RLE with LZ4 streaming compression directly from live data. Would save ~4-8 KB on all platforms.
