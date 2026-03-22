# Dynamic Array Allocation for EdgeTX ModelData

## Context

The `ModelData` struct used fixed-size arrays for inputs, mixes, curves, and other components. This wasted memory on simple models (a 4-channel trainer allocated the same ~6.5 KB as a 32-channel heli) while simultaneously limiting power users who hit the hard caps (64 mixes, 64 inputs, 32 curves). Moving to dynamic allocation solves both problems: simple models shrink, complex models can grow.

Outputs (`LimitData[MAX_OUTPUT_CHANNELS]`) stay static — physically limited to 32 channels.

---

## Completed Work

### Phase 1: Accessor Abstraction (commit `a019cd6`)

All ModelData array access routed through accessor functions. No direct `g_model.mixData[i]` etc. remains in non-test code.

- `mixAddress(idx)`, `expoAddress(idx)`, `curveHeaderAddress(idx)`, `curvePointsBase()`, `lswAddress(idx)`, `customFnAddress(idx)`
- Centralized `expos.cpp` (insert/delete/copy/move) mirroring `mixes.cpp`
- Centralized `customfn.cpp` (insert/delete/clear)
- `getExpoCount()` / `updateExpoCount()` paralleling `getMixCount()`

### Phase 2: Arena Allocator (commits `85680d7` through `2977393`)

Six arrays removed from ModelData and stored in a shared arena:

| Array | Element | Per-radio max | Hard cap |
|-------|---------|---------------|----------|
| mixData | 20 B | 64 | 128 |
| expoData | 18 B | 64 | 128 |
| curves | 4 B | 32 | 64 |
| points | 1 B | 512 | 1024 |
| logicalSw | 9 B | 64 | 64 |
| customFn | 11 B | 64 | 64 |

**ModelData shrunk from ~6.5 KB to ~2.5 KB** (~4.3 KB saved).

Key components:
- **`ModelArena`** class (`model_arena.h/cpp`): manages a contiguous buffer with section-based layout. Supports `insertSlot`/`deleteSlot` with cascading offset updates. Uses `uint32_t` for capacity/offsets (supports SDRAM).
- **`ModelDynData`** in `ModelData`: counts for each arena section (mixCount, expoCount, etc.)
- **YAML `YDT_EXTERN_ARRAY`**: new tree walker node type that redirects data access to arena memory via `get_ptr` callbacks and per-state `data_override` pointers
- **`CUST_EXTERN_ARRAY`** annotation macro + generator support: `generate_yaml.py` handles `extern_array:` annotations, computing per-element bit sizes from struct definitions
- **RTC backup**: `RamBackupUncompressed` includes `uint8_t arena[MODEL_ARENA_SIZE]`; write saves arena, restore repopulates it
- **Parallel state arrays**: `mixState[MAX_MIXERS_HARD]`, `act[MAX_MIXERS_HARD]`, `activeMixes[MAX_MIXERS_HARD]` sized to hard caps
- **Label operations**: `patchModelYamlLabels()` does direct YAML text patching (no model round-trip); `readModelYaml()` saves/restores arena for temp model reads
- **Unit tests**: 23 tests covering ModelArena class, accessor functions, insert/delete operations, section non-overlap, model reset

### Current arena layout

The arena is initialized with **max-size layout** (`modelArenaInit`) — all sections pre-allocated at their per-radio `MAX_*` sizes. This makes the arena behavior identical to the old static arrays: same capacity, same memmove semantics. The `ModelDynData` counts in `g_model.dyn` are not yet populated after YAML load (not needed with max-size layout).

```
Arena (MODEL_ARENA_SIZE bytes, static buffer)
+----------+----------+---------+--------+------+------+------+
| mixes    | expos    | curves  | points | LS   | SF   | free |
| 64×20=   | 64×18=   | 32×4=   | 512×1= | 64×9=| 64×11|      |
| 1280 B   | 1152 B   | 128 B   | 512 B  | 576 B| 704 B|      |
+----------+----------+---------+--------+------+------+------+
Total used: 4352 B
```

Platform arena sizes:
| Platform | MODEL_ARENA_SIZE | Default used |
|----------|-----------------|-------------|
| STM32F4 | 4096 B | 4352 B (!) |
| STM32H7 | 8192 B | 4352 B |
| Companion/Sim | 65536 B | 4352 B |

**Note**: STM32F4 arena (4096) is smaller than the default layout (4352). This works today because `modelArenaInit` doesn't bounds-check. This must be resolved before enabling dynamic sizing — either increase STM32F4 arena or reduce default counts.

---

## Phase 3: Dynamic Arena Sizing

**Goal**: Models use only the arena space they need. Simple models leave room for more entries in other sections. Complex models can exceed per-radio defaults up to the hard caps.

### 3.1 Populate `ModelDynData` counts after model load

After `readModelYaml()` + `postModelLoad()`:
1. `updateMixCount()` and `updateExpoCount()` already compute packed counts
2. Add `updateCurveCount()`: count non-empty `CurveHeader` entries
3. LS and SF are position-indexed (sparse) — always `MAX_LOGICAL_SWITCHES` / `MAX_SPECIAL_FUNCTIONS` slots
4. Points count: sum of `getCurvePoints(i)` for all curves
5. Store all counts in `g_model.dyn`
6. Call `g_modelArena.layout(g_model.dyn)` to compact the layout

### 3.2 Compact arena layout on model load

After counts are computed, `layout()` recomputes section offsets to pack sections tightly. The arena would look like:

```
Simple 4-channel model:
+-------+-------+---+---+------+------+------ free ------+
| 4 mix | 4 expo| 0 | 0 | 64LS | 64SF |    ~3 KB free    |
| 80 B  | 72 B  |   |   |576 B |704 B |                  |
+-------+-------+---+---+------+------+------------------+

Complex heli model:
+----------+---------+--------+------+------+------+free+
| 40 mixes | 30 expo | 15 crv | 200pt| 64LS | 64SF |    |
| 800 B    | 540 B   | 60 B   |200 B |576 B |704 B |    |
+----------+---------+--------+------+------+------+----+
```

### 3.3 Arena-aware insert/delete

Replace raw `memmove` with `g_modelArena.insertInSection()` / `deleteFromSection()`:

**`insertMix(idx, channel)`**:
```cpp
if (!g_modelArena.insertInSection(ARENA_MIXES, idx, sizeof(MixData))) {
    AUDIO_WARNING2();  // "Model memory full"
    return;
}
g_model.dyn.mixCount++;
// ... initialize the new mix
```

**`deleteMix(idx)`**:
```cpp
g_modelArena.deleteFromSection(ARENA_MIXES, idx, sizeof(MixData));
g_model.dyn.mixCount--;
```

These shift all subsequent sections automatically. The `act[]` parallel array still needs its own memmove (it's not in the arena).

**Capacity check**: `g_modelArena.freeBytes() >= sizeof(MixData)` replaces `getMixCount() >= MAX_MIXERS`.

### 3.4 Update loop bounds

Replace `MAX_*` loop bounds with dynamic counts in the mixer hot path:

```cpp
// mixer.cpp applyExpos():
for (uint8_t i = 0; i < getExpoCount(); i++) {
    ...
}

// mixer.cpp evalFlightModeMixes():
for (uint8_t i = 0; i < getMixCount(); i++) {
    ...
}
```

For LS/SF (position-indexed), loop bounds stay at `MAX_LOGICAL_SWITCHES` / `MAX_SPECIAL_FUNCTIONS`.

### 3.5 GUI memory indicator

Add a "Model memory" display in the model setup page:

```
Model memory: 2880 / 4096 bytes (70%)
[████████████████████░░░░░░░░░]
```

Insert operations show a warning when arena is >90% full and refuse when 100%.

### 3.6 STM32F4 arena sizing

The current STM32F4 arena (4096 B) is smaller than the max-size layout (4352 B). Options:
1. Increase to 4608 B or 5120 B (uses ~1 KB more RAM but matches old capacity)
2. Keep 4096 B — simple models fit, complex models get a "memory full" warning earlier than with old static arrays
3. Define per-radio (some F4 radios have more headroom than others)

### 3.7 YAML `dyn` field serialization

To persist the counts across save/load, `ModelDynData` should be serialized in the YAML file. Options:
1. Serialize `dyn` as a regular YAML struct (simple, but changes file format)
2. Don't serialize — recompute counts on every load (current approach, no format change)
3. Serialize only as a comment/metadata field for debugging

Option 2 (recompute) is preferred — it's robust and doesn't add format dependencies.

### 3.8 Files to modify

| File | Change |
|------|--------|
| `mixes.cpp` | Use `insertInSection`/`deleteFromSection`, update `dyn.mixCount` |
| `expos.cpp` | Same pattern |
| `customfn.cpp` | Same pattern |
| `curves.cpp` | `moveCurve()` → arena point section manipulation |
| `mixer.cpp` | Loop bounds → `getMixCount()`/`getExpoCount()` |
| `storage/storage_common.cpp` | `postModelLoad()` → compute dyn counts, compact layout |
| `model_init.cpp` | `setModelDefaults()` → set dyn counts for default template |
| `model_arena.cpp` | `modelArenaInit()` → consider not pre-allocating max sizes |
| GUI model setup pages | Add memory indicator |
| `dataconstants.h` | Adjust `MODEL_ARENA_SIZE` for STM32F4 if needed |

---

## Phase 4 (Future): GVars + FlightModes

GVars are entangled with `FlightModeData` because per-FM values live at `flightModeData[fm].gvars[MAX_GVARS]`. Making gvars dynamic means `FlightModeData` becomes variable-size. Defer unless users need more than 15 gvars.

## Phase 5 (Future): Growable Arena

Replace the static arena buffer with a dynamically allocated one:
- `attach(malloc(size), size)` or `attach(sdramAddr, largerSize)`
- Arena can grow at runtime when a model is loaded that needs more space
- Requires allocation outside the mixer hot path (during model load)

---

## Known Issues

1. **modelslist `updateModelCell()`**: reads a temp model via `readModelYaml()` which writes to the global arena. Arena is saved/restored around the call, but the round-trip is wasteful. Could be replaced with a header-only read.

2. **STM32F4 arena undersize**: `MODEL_ARENA_SIZE` (4096) < default layout (4352). Works because `modelArenaInit` doesn't bounds-check, but prevents using the arena for models that need all 64 of everything.

3. **`curveEnd[]` parallel array**: sized `MAX_CURVES` (32), should be `MAX_CURVES_HARD` (64) if more curves are supported. Both are currently 64 so no issue yet, but should be updated for consistency.
