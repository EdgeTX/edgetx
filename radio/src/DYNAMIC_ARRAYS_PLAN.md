# Dynamic Array Allocation for EdgeTX ModelData

## Context

The `ModelData` struct (`radio/src/datastructs_private.h:753`) uses fixed-size arrays for inputs, mixes, curves, and other components. This wastes memory on simple models (a 4-channel trainer allocates the same ~6.5 KB as a 32-channel heli) while simultaneously limiting power users who hit the hard caps (64 mixes, 64 inputs, 32 curves). Moving to dynamic allocation solves both problems: simple models shrink, complex models can grow.

Outputs (`LimitData[MAX_OUTPUT_CHANNELS]`) stay static -- they're physically limited to 32 channels.

---

## Current State

### Memory layout (ModelData ~6.3-7.6 KB depending on radio)

| Array | Element | Max | Bytes | Typical use |
|-------|---------|-----|-------|-------------|
| `mixData[MAX_MIXERS]` | 20 B | 64 | 1280 | 5-30 entries |
| `expoData[MAX_EXPOS]` | 18 B | 64 | 1152 | 4-20 entries |
| `curves[MAX_CURVES]` | 4 B | 32 | 128 | 0-10 entries |
| `points[MAX_CURVE_POINTS]` | 1 B | 512 | 512 | 0-200 points |
| `gvars[MAX_GVARS]` | 7 B | 9-15 | 63-105 | 0-9 entries |
| `logicalSw[64]` | 9 B | 64 | 576 | 5-30 entries |
| `customFn[64]` | 11 B | 64 | 704 | 0-20 entries |

**Dynamic candidates total: ~4,400 bytes** out of ~6,500 in ModelData.

### Access patterns

- **Accessor functions**: `mixAddress(idx)`, `expoAddress(idx)` return `&g_model.array[idx]`
- **Insert/delete**: `memmove()` to shift trailing elements (`mixes.cpp:37-80`)
- **Curves**: shared points pool with `curveEnd[]` offset tracking (`curves.cpp:30-101`)
- **Mixer hot path**: linear scan, breaks on `srcRaw == 0` (mixes) / `!EXPO_VALID(ed)` (expos)
- **Parallel state arrays**: `mixState[MAX_MIXERS]` (globals.h:74), `act[MAX_MIXERS]` (globals.h:75), stack-local `activeMixes[MAX_MIXERS]` (mixer.cpp:839) -- must stay in sync
- **Note**: `mixState[]` serves double duty -- indexed by expo index for `activeExpo`, by mix index for `activeMix`

### Platform constraints

- **STM32F4**: 128 KB RAM + 64 KB CCM. FreeRTOS static-only (`configSUPPORT_DYNAMIC_ALLOCATION = 0`), but libc malloc/free are used elsewhere (modelslist.cpp, USB, FTL driver)
- **STM32H7**: ~288 KB RAM. More headroom.
- **RTC backup**: 4 KB battery-backed SRAM, RLE-compressed ModelData + RadioData
- **Storage**: YAML on SD card (naturally supports variable-length), binary RLC for EEPROM radios
- **CHKSIZE()**: compile-time size checks in `datastructs.h:40-123` enforce exact struct sizes

---

## Design: Arena-Backed Dynamic Arrays

### Why arena allocation

| Approach | Pros | Cons |
|----------|------|------|
| malloc per element | Flexible | Fragmentation, overhead per element, cache-unfriendly |
| Linked lists | Easy insert/delete | 8B overhead/node, cache-unfriendly, breaks memmove pattern |
| realloc | Simple API | Fragmentation, unpredictable, copies on grow |
| **Arena** | **No fragmentation, cache-friendly, deterministic, memcpy-safe** | Cascading shift on insert (acceptable -- only during user edits) |

### Core architecture

Replace the fixed arrays inside `ModelData` with counts + offsets into a separate statically-allocated arena:

```
ModelData (shrinks to ~2 KB)          Arena (MODEL_ARENA_SIZE bytes)
+---------------------------+        +-------+-------+--------+--------+----+----+------+
| header, timers, flags...  |        | mixes | expos | curves | points | LS | SF | free |
| mixCount, expoCount, ...  |        | N*20B | M*18B | K*4B   | P*1B   |    |    |      |
| mixOffset, expoOffset,... |------->+-------+-------+--------+--------+----+----+------+
| limitData[32] (static)    |
| flightModeData[9] (static)|
| modules, failsafe (static)|
+---------------------------+
```

```cpp
// In ModelData (replaces the array fields)
struct ModelDynData {
    uint8_t   mixCount;
    uint8_t   expoCount;
    uint8_t   curveCount;
    uint16_t  pointsCount;
    uint8_t   logicalSwCount;
    uint8_t   customFnCount;
    // Byte offsets into arena (order: mix, expo, curve, points, LS, SF)
    uint16_t  mixOffset;      // always 0 (mixes first)
    uint16_t  expoOffset;
    uint16_t  curveOffset;
    uint16_t  pointsOffset;
    uint16_t  logicalSwOffset;
    uint16_t  customFnOffset;
    uint16_t  usedBytes;      // total arena bytes consumed
};

// Arena manager -- abstracts backing storage so it can be static, heap, or SDRAM
class ModelArena {
    uint8_t* _base;     // pointer to backing storage (NOT owned)
    uint16_t _capacity; // current capacity in bytes
public:
    void attach(uint8_t* buf, uint16_t capacity);  // bind to a buffer
    bool grow(uint16_t newCapacity);                // reallocate (future)
    uint8_t* base() const { return _base; }
    uint16_t capacity() const { return _capacity; }
    uint16_t freeBytes() const;
    // Section management
    bool insertSlot(uint16_t offset, uint16_t slotSize, uint16_t usedBytes);
    void deleteSlot(uint16_t offset, uint16_t slotSize, uint16_t usedBytes);
};

// Phase 2: static backing buffer (can be replaced with malloc/SDRAM later)
static uint8_t g_modelArenaBuf[MODEL_ARENA_SIZE] __attribute__((aligned(4)));
extern ModelArena g_modelArena;
```

**Key design choice**: The `ModelArena` class holds a pointer + capacity, not a fixed array. This means:
- **Phase 2**: `attach(g_modelArenaBuf, MODEL_ARENA_SIZE)` -- static buffer, zero risk
- **Future**: `attach(malloc(size), size)` or `attach(sdramAddr, largerSize)` -- the arena can grow at runtime, e.g., when a model is loaded that needs more space, or when SDRAM is available
- The arena can even be reallocated (allocate new buffer, memcpy, update pointer) as long as this is done outside the mixer hot path (during model load or user edit)

### Arena sizing per platform

Defined per radio type (MCU family, SDRAM presence, etc). Starting points:

| Platform | MODEL_ARENA_SIZE | vs. current static | Effect |
|----------|-----------------|-------------------|--------|
| STM32F4 (baseline) | 4096 B | ~4,400 B static today | Similar budget, flexible allocation |
| STM32H7 | 8192+ B | ~4,400 B static today | ~2x capacity |
| Radios with SDRAM | TBD (larger) | -- | Much higher limits |
| Companion/Sim | 65536 B | N/A | Effectively unlimited |

The arena size is a per-radio compile-time constant, allowing fine-tuning based on available RAM.

### New limits model

Replace per-type hard caps with a **shared memory budget**:
- Each model has `MODEL_ARENA_SIZE` bytes available
- User can trade off: more mixes means fewer curves, and vice versa
- Hard safety caps remain (e.g., `MAX_MIXERS_HARD = 128`) to bound parallel state arrays
- GUI shows "Model memory: X% used" or "X bytes free"
- Insert operations check `arena.freeBytes() >= sizeof(ElementType)`

### Accessor function changes

Today:
```cpp
MixData* mixAddress(uint8_t idx) { return &g_model.mixData[idx]; }
```

After:
```cpp
MixData* mixAddress(uint8_t idx) {
    return reinterpret_cast<MixData*>(g_modelArena.base() + g_model.dyn.mixOffset) + idx;
}
```

The mixer hot path (`mixer.cpp`) already uses `mixAddress(i)` and `expoAddress(i)` exclusively. Loop bounds change from `MAX_MIXERS` to `g_model.dyn.mixCount`. **No allocation in the hot path.**

### Insert/delete mechanics

Inserting a mix at position `idx`:
1. Check `arena.freeBytes() >= sizeof(MixData)` -- if not, warn "Model memory full"
2. `mixerTaskStop()` (already done today)
3. Shift arena content after the mix region forward by `sizeof(MixData)` -- this moves expos, curves, points
4. Update expoOffset, curveOffset, pointsOffset (add 20)
5. `memmove()` within the mix region to open a slot at `idx` (same as today)
6. Increment `mixCount`, update `usedBytes`
7. `mixerTaskStart()`

This cascading shift is the main cost of the arena approach. But it only happens during user edits (not in the mixer loop), the total data is <8 KB, and `memmove` on that size is microseconds on ARM Cortex-M.

### Logical switches and special functions

`logicalSw[64]` (576 B) and `customFn[64]` (704 B) move into the arena alongside mixes/expos/curves. Same pattern: count + offset in `ModelDynData`, accessor functions, arena insert/delete. This brings the total dynamic pool to ~4,400 bytes freed from ModelData, all sharing the arena budget.

Arena section order: `mixes | expos | curves | points | logicalSw | customFn | free`

LS and SF have simpler access patterns than mixes/expos (no sorted-by-channel discipline, no parallel state arrays like `act[]`), making them easier to migrate.

---

## GVars: Defer

GVars are entangled with `FlightModeData` because per-FM values live at `flightModeData[fm].gvars[MAX_GVARS]`. Making gvars dynamic means `FlightModeData` becomes variable-size, which cascades into the `flightModeData[9]` array. This is a deeper refactor with less payoff (~105-270 bytes). Defer to a separate effort.

---

## Impact Analysis

### YAML storage (radio/src/storage/yaml/)
YAML already handles variable-length arrays conceptually. Changes:
- Writer iterates `mixCount` entries instead of `MAX_MIXERS`
- Reader counts entries during parse, grows arena as needed
- `NO_IDX` attribute on mix/expo arrays already suppresses index prefixes
- **Backward compatible**: old YAML files load fine (parser fills what's present). New files work in old firmware up to old limits.

### Binary RTC backup (radio/src/storage/rtc_backup.cpp)
Backup needs to serialize the arena content alongside ModelData. Two options depending on arena growth:
- **Static arena (Phase 2)**: Backup struct includes `uint8_t arena[MODEL_ARENA_SIZE]` -- simple, predictable. Unused space is zeros which RLE-compress to nearly nothing.
- **Growable arena (future)**: Backup serializes only `usedBytes` of arena content, prefixed by a length field. This keeps backup size proportional to actual model complexity, not arena capacity.

```cpp
// Phase 2 (static)
PACK(struct RamBackupUncompressed {
    ModelData model;
    uint8_t arena[MODEL_ARENA_SIZE];
    RadioData radio;
});
```

### Companion (companion/src/firmwares/)
Companion has its own `ModelData` class with `CPN_MAX_MIXERS` etc. **No urgent change needed** -- YAML is the interface. Long-term, Companion could switch to `QVector<MixData>` for truly unlimited storage.

### Simulator
Uses firmware `ModelData` directly. Gets a large `MODEL_ARENA_SIZE` -- works transparently.

### Lua API (radio/src/lua/api_model.cpp)
Uses `mixAddress()`, `expoAddress()`, `getFirstMix()`, `getMixesCount()` exclusively. Works unchanged. `model.getInfo()` could expose available arena space.

### Parallel state arrays
`mixState[MAX_MIXERS_HARD]`, `act[MAX_MIXERS_HARD]`, stack-local `activeMixes[MAX_MIXERS_HARD]` -- sized to the hard cap (e.g., 128). Only iterated up to `mixCount` / `expoCount` in the hot path.

---

## Migration Strategy

### Phase 1: Abstract the Accessor Layer

**Goal**: All code goes through accessor functions. No direct `g_model.mixData[i]` access remains. Zero data layout change, zero risk.

1. **Centralize expo operations**: Currently `insertExpo()`, `deleteExpo()`, `copyExpo()` are duplicated in GUI code (`gui/colorlcd/model/model_inputs.cpp` and `gui/common/stdlcd/model_inputs.cpp`). Move to a shared `expos.cpp` like `mixes.cpp` already exists.

2. **Add missing accessors**: `curveHeaderAddress(idx)`, `curvePointsBase()`, `curveCount()`, `expoCount()`.

3. **Replace all direct array access**: `g_model.mixData[i]` -> `mixAddress(i)`, `g_model.expoData[i]` -> `expoAddress(i)`, `g_model.curves[i]` -> `curveHeaderAddress(i)`.

4. **Replace loop bounds**: `i < MAX_MIXERS` -> `i < getMixCount()` (already exists for mixes), add equivalent for expos.

5. **Replace capacity checks**: `getMixCount() >= MAX_MIXERS` -> `canInsertMix()`.

Key files:
- `radio/src/mixes.cpp` -- already centralized, template for expos
- `radio/src/curves.cpp` -- needs accessor additions
- `radio/src/mixer.cpp` -- loop bounds (lines 190, 839, 1128)
- `radio/src/edgetx.cpp` -- expo accessor (line 253), add curveHeader accessor
- `radio/src/gui/colorlcd/model/model_inputs.cpp` -- move insert/delete/copy to shared code
- `radio/src/gui/common/stdlcd/model_inputs.cpp` -- same
- `radio/src/gui/colorlcd/model/input_edit.cpp` -- loop bound (line 272)
- `radio/src/storage/storage_common.cpp` -- loop bound (line 136)

**Validation**: All CHKSIZE() checks pass unchanged. YAML output is identical. All tests pass.

### Phase 2: Introduce the Arena

**Goal**: Move mixes, expos, curves, points, logical switches, and special functions into the arena. ModelData shrinks. Limits become flexible.

1. Add `ModelDynData dyn` to `ModelData`. Remove `mixData[]`, `expoData[]`, `curves[]`, `points[]`, `logicalSw[]`, `customFn[]` fields.
2. Add `g_modelArena[]` global, sized per radio via `MODEL_ARENA_SIZE`.
3. Implement `ModelArena` class: `layout()`, `insertSlot()`, `deleteSlot()`, `freeBytes()`, `canInsert()`.
4. Update all accessor functions to use arena offsets.
5. Add accessors for LS and SF (same pattern as mixes/expos).
6. Update YAML serialization for dynamic arrays.
7. Update RTC backup to include arena.
8. Update CHKSIZE() checks (ModelData gets smaller).
9. Size `mixState[]`, `act[]` to `MAX_MIXERS_HARD`.
10. Add "Model memory" indicator to GUI.
11. Update `datacopy` code generator if applicable.

Key files (in addition to Phase 1 files):
- `radio/src/datastructs_private.h` -- core struct change
- `radio/src/datastructs.h` -- CHKSIZE updates
- `radio/src/globals.h` -- parallel array sizing
- `radio/src/storage/rtc_backup.cpp` -- backup struct
- `radio/src/storage/yaml/yaml_datastructs_funcs.cpp` -- serialization
- Per-radio board headers -- `MODEL_ARENA_SIZE` definition

### Phase 3 (Future): GVars + FlightModes

Only if users need more than 15 gvars. Requires making `FlightModeData` variable-size.

---

## Risks and Mitigations

| Risk | Mitigation |
|------|-----------|
| Mixer real-time disruption | Phase 1 proves accessor abstraction before any layout change. No allocation in hot path. |
| Cascading memmove on insert | Only during user edits (mixerTaskStop already called). <8 KB total, microseconds on ARM. |
| RTC backup overflow | Unused arena compresses to ~zero with RLE. Worst case = today. |
| YAML backward compat | YAML inherently variable-length. Old files load fine. New files in old firmware: truncated at old limits. |
| Stack overflow from `activeMixes[MAX_MIXERS_HARD]` | 128 bytes on stack vs current 64. Verify stack headroom. |
| Companion divergence | YAML is the interface. Companion unchanged initially. |

---

## Verification

- **Phase 1**: Run full test suite. Compare YAML output before/after (must be byte-identical). Verify CHKSIZE unchanged.
- **Phase 2**: Unit tests for `ModelArena` (insert/delete at boundaries, arena full, empty model, max model). Integration test: load complex model YAML, verify round-trip. Verify RTC backup/restore. Test on real hardware (STM32F4 and H7). Stress test: fill arena to capacity, verify graceful "memory full" behavior.
