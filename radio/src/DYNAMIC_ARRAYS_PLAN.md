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

**Bridge functions** (temporary, until all code uses structured types natively):
- `sourceRefToMixSrc()` / `switchRefToSwSrc()` in `mixer.cpp` — convert new→old for `getValue()`/`getSwitch()`/GUI functions that still expect enum integers
- `mixSrcToSourceRef()` / `swSrcToSwitchRef()` — reverse, used in Lua API SET paths and GUI editors
- `valueOrSourceToLegacy()` / `legacyToValueOrSource()` — bridge ValueOrSource↔SourceNumVal packed format

**Files updated:** mixer.cpp, mixes.cpp, expos.cpp, model_init.cpp, curves.cpp, edgetx.cpp, all GUI editors (colorlcd + 212x64 + 128x64), Lua API, all test files.

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

## Phase 4: Dynamic Arena Sizing

Models use only the arena space they need. Simple models leave room for more entries.

1. Populate `ModelDynData` counts after model load
2. Compact arena layout based on actual usage
3. Arena-aware insert/delete (`insertInSection`/`deleteFromSection`)
4. Dynamic loop bounds in mixer hot path
5. GUI memory indicator
6. STM32F4 arena sizing resolution

---

## Phase 5 (Future): GVars + FlightModes

GVars are entangled with `FlightModeData` because per-FM values live at `flightModeData[fm].gvars[MAX_GVARS]`. Defer unless users need more than 15 gvars.

## Phase 6 (Future): Growable Arena

Replace the static arena buffer with a dynamically allocated one (`malloc`/SDRAM).

---

## Capacity Limits (updated after Phase 3b)

### MixData and ExpoData (migrated to SourceRef/SwitchRef)

The 10-bit `srcRaw`/`swtch` overflow is **fixed**. Each source/switch type gets 0-65535 index range. No enum range pressure.

### Remaining bit-field limits

| Field | Bits | Range | References | Limit |
|-------|------|-------|------------|-------|
| `MixData.destCh` | 8 | 0-255 | Output channels | 256 (was 32 with 5 bits) |
| `ExpoData.chn` | 5 | 0-31 | Input channels | 32 |
| `MixData.flightModes` | 9 | bitmask | FM bitmask | 9 flight modes |
| `LimitData.curve` | 8 | -1..127 | Curve index | 128 curves |
| `CurveHeader.points` | 6 | -32..31 | Points minus 5 | 36 points per curve |
| `CustomFunctionData.func` | 6 | 0-63 | Function type | 64 function types |
| `LogicalSwitchData.v1` | 10 | -512..511 | **Still old enum** | **Needs migration** |
| `LogicalSwitchData.andsw` | 10 | -512..511 | **Still old enum** | **Needs migration** |
| `CustomFunctionData.swtch` | 10 | -512..511 | **Still old enum** | **Needs migration** |
| `FlightModeData.swtch` | 10 | -512..511 | **Still old enum** | **Needs migration** |
| `TimerData.swtch` | 10 | -512..511 | **Still old enum** | **Needs migration** |

### Accessor function parameter types

All use `uint8_t idx` (max 255). Sufficient for current hard caps.

---

## Known Issues

1. **STM32F4 arena undersize**: default layout (6,208 B) exceeds `MODEL_ARENA_SIZE` (4096 B). Must be resolved.

2. **Bridge functions**: `sourceRefToMixSrc()`, `switchRefToSwSrc()` etc. are temporary. They reconstruct old enum values from structured refs and are used by `getValue()`/`getSwitch()`/GUI code that hasn't been fully migrated.

3. **modelslist `updateModelCell()`**: reads temp models via `readModelYaml()` with arena save/restore. Could be replaced with header-only read.

4. **`curveEnd[]` parallel array**: sized `MAX_CURVES` (32), should match `MAX_CURVES_HARD` (64).

5. **Duplicate bridge functions**: `mixSrcToSourceRef()` and similar are duplicated as `static` in multiple GUI files. Should be consolidated.
