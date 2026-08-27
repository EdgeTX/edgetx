# Companion MVC Improvement Plan

## Context

Companion dialogs currently use direct widget manipulation with manual `setVisible()` / `setEnabled()` calls scattered through monolithic `update()` methods. The existing Auto* widget system handles bidirectional data binding well, but there is no abstraction for conditional UI state (visibility, enabled, dynamic labels). The hardest panels — TelemetrySensorPanel (~25 rules) and ModulePanel (~40+ rules, 26 mask constants) — are where this hurts most.

This plan extends the existing Auto* / GenericPanel infrastructure incrementally. Each phase is independently shippable.

---

## Phase 1: Visibility Binding Infrastructure

**Goal:** Extend `AutoWidget` / `GenericPanel` to handle visibility and enabled state declaratively.

**What to build** (~200-300 lines):

Add a `WidgetBinding` utility that connects widget visibility/enabled state to a predicate:

```cpp
// In GenericPanel or a new small header
void GenericPanel::bindVisible(QWidget *widget, std::function<bool()> pred);
void GenericPanel::bindEnabled(QWidget *widget, std::function<bool()> pred);
void GenericPanel::applyBindings();  // evaluate all predicates, apply results
```

Usage in a panel constructor:

```cpp
bindVisible(ui->ratio,       [&]{ return sensor.getMask() & SENSOR_HAS_RATIO; });
bindVisible(ui->ratioLabel,  [&]{ return sensor.getMask() & SENSOR_HAS_RATIO
                                         && sensor.unit != UNIT_RPMS; });
bindVisible(ui->bladesLabel, [&]{ return sensor.getMask() & SENSOR_HAS_RATIO
                                         && sensor.unit == UNIT_RPMS; });
bindEnabled(ui->autoOffset,  [&]{ return (sensor.getMask() & SENSOR_HAS_RATIO)
                                         && sensor.unit != UNIT_RPMS; });
```

Then `update()` shrinks to:

```cpp
void TelemetrySensorPanel::update() {
    lock = true;
    updateAutoWidgets();  // existing - syncs data to widgets
    applyBindings();      // new - evaluates all visibility/enabled predicates
    lock = false;
}
```

**Why this first:** Small, self-contained, non-breaking. Existing panels don't need to change — new code opts in. The predicate lambdas capture the same business rules that already exist in `getMask()`, but declared once at setup time instead of scattered through `update()`.

---

## Phase 2: Pilot on Telemetry Panel

**Goal:** Prove the pattern on a real complex panel.

Telemetry is the right first target because:
- `getMask()` already encapsulates the business rules cleanly
- ~25 visibility rules, complex enough to be meaningful, small enough to finish
- Self-contained — doesn't cascade into other panels

**Steps:**
1. Convert `TelemetrySensorPanel::update()` to use `bindVisible` / `bindEnabled`
2. Move data population to Auto* widgets where not already done
3. Delete the manual `setVisible()` / `setEnabled()` calls from `update()`
4. Result: `update()` body becomes essentially `updateAutoWidgets(); applyBindings();`

**Validation:** Behavior should be identical. Test every sensor type x formula x unit combination.

---

## Phase 3: Tackle ModulePanel

**Goal:** Apply the pattern to the hardest case.

ModulePanel has 26 mask constants and a 400-line `update()`. This is where the real payoff is — but also where edge cases in the binding system will surface:

- **Cascading dependencies** — protocol change triggers subtype options which trigger failsafe visibility
- **Dynamic widget creation** — receiver rows created/destroyed based on protocol
- **Cross-panel effects** — module settings affect other panels

Expect to extend the binding infrastructure. Likely additions:
- `bindText(QLabel*, std::function<QString()>)` for dynamic labels
- `bindModel(QComboBox*, std::function<QAbstractItemModel*()>)` for swapping combo contents
- Possibly group bindings that share an invalidation trigger

---

## Phase 4: Virtualized Grid Panels (model-arena Support)

**Goal:** Replace fixed-count widget grids with virtualized views so panels scale with per-radio object limits.

### The Problem

The `model-arena` branch introduces dynamic array limits that vary by radio type (e.g., H7 radios may support more logical switches, custom functions, etc.). Several panels currently allocate all widgets upfront in static grids:

| Panel | Widgets/Row | Current Max | Total Widgets |
|-------|------------|-------------|---------------|
| LogicalSwitches | 12 | 64 | 768 |
| CustomFunctions | ~10 | 64 | ~640 |
| Telemetry sensors | ~15 | 60 | ~900 |

If limits double on higher-end radios, widget counts double with them. Mixes and Inputs already use `QListWidget` and won't have this problem.

### Current Architecture (LogicalSwitches)

```
CPN_MAX_LOGICAL_SWITCHES = 64 (compile-time constant)
    -> fixed-size C arrays in header: cbFunction[64], dsbValue[64], ...
    -> constructor loop creates all 768 widgets
    -> updateLine(i) shows/hides widgets per row based on function type
    -> no scrolling, no virtualization
```

### Target Architecture

Replace the static grid with `QTableView` + a `QAbstractTableModel` adapter:

```
Model:  LogicalSwitchTableModel (wraps LogicalSwitchData array)
          -> rowCount() returns actual count from firmware capability
          -> data()/setData() for each column (function, v1, v2, AND, duration, delay, persist)
          -> dataChanged() signals drive view updates

View:   QTableView with custom delegates
          -> only instantiates widgets for visible rows (virtualized)
          -> delegates handle the conditional widget type per cell:
             column 2 shows combobox OR spinbox depending on function family

Binding: visibility masks from updateLine() become delegate logic
          -> delegate's createEditor() checks function family
          -> no 768 pre-allocated widgets, just ~12 × visible rows
```

### Why This Phase Matters Now

1. **model-arena makes limits radio-dependent** — `getCapability(LogicalSwitches)` will return different values per board. Fixed C arrays sized to the global max waste memory for smaller radios and may not be large enough for larger ones.
2. **Widget count becomes the load-time bottleneck** — 768 widgets is already slow; 1500+ is unacceptable. Virtualization is the only real fix.
3. **The binding infrastructure from Phase 1 applies** — delegate visibility logic can use the same predicate pattern, just inside `createEditor()` / `paint()` instead of panel-level `applyBindings()`.

### Migration Steps

1. **LogicalSwitches first** — highest widget density, cleanest row structure, most likely to hit raised limits
2. **Write `LogicalSwitchTableModel`** — one column per field, rowCount from firmware capability
3. **Write delegates** — `LogicalSwitchDelegate::createEditor()` returns the appropriate widget type based on function family (replaces updateLine's visibility masks)
4. **Replace TableLayout with QTableView** — drop the 12 fixed arrays
5. **Repeat for CustomFunctions** — similar structure, similar migration
6. **Telemetry sensors** — already panel-per-sensor, may need a different approach (lazy panel creation rather than table virtualization)

### Interaction with model-arena

The `model-arena` branch changes how Companion receives object counts:
- Today: `CPN_MAX_LOGICAL_SWITCHES` is a compile-time constant shared across all radios
- After: limits come from the radio profile / firmware capability, potentially varying at runtime

The `QAbstractTableModel` adapter naturally handles this — `rowCount()` queries the capability, and the view adjusts. No fixed-size arrays needed in the panel header.

If arena-backed arrays can grow (user adds a logical switch beyond the previous radio's limit on a new radio), the model emits `rowsInserted()` and the view updates. This is standard Qt Model/View — no custom code needed for the dynamic count case.

---

## Phase 5: Convert Remaining Medium-Complexity Panels

**Goal:** Normalize the binding pattern across remaining panels.

In rough priority order (complexity vs. effort):
1. **FlightModes** — GVar linked/unlinked toggling
2. **ExpoDialog / MixerDialog** — firmware capability checks
3. **FunctionSwitches** — config/group-driven enable states
4. **Hardware** — board-dependent visibility

Simple panels (Trainer, Checklist) may not need conversion — the manual approach is fine when there are only 2-3 rules.

---

## Phase 6: Extract ViewModels Where Justified

**Goal:** Pull business rules out of panels entirely, but only where it earns its keep.

After Phases 2-3, the panels with rules complex enough to warrant a separate ViewModel class will be clear. The candidates are likely:
- `ModulePanel` -> `ModuleViewModel`
- `TelemetrySensorPanel` -> `SensorViewModel`

For these, the ViewModel owns the predicates and the panel just binds to them. For everything else, lambdas in the panel constructor are fine.

---

## What This Plan Avoids

- **No `QDataWidgetMapper`** — it doesn't help with the hard problem (conditional visibility) and the Auto* system already solves data binding well
- **No .ui file changes** (Phases 1-3, 5-6) — improvements are in the .cpp layer. Phase 4 replaces .ui grid layouts with QTableView but keeps dialog structure intact
- **No big-bang migration** — each phase is independently shippable and testable
- **No framework adoption** — the binding utility is ~200 lines of code, not a dependency

## Expected Outcome

**Maintainability** (Phases 1-3, 5-6): Adding a new sensor formula or protocol means adding a few `bindVisible` lines and the predicate logic, not hunting through a monolithic `update()` to find every `setVisible` call. The `lock` flag anti-pattern fades away naturally since bindings are evaluated in batch, not triggered by signal cascades.

**Scalability** (Phase 4): Grid-style panels handle variable per-radio object limits without linear widget cost. Dialog load time becomes independent of max object count. This is the prerequisite for `model-arena` support in Companion — without it, raising limits on H7/H5 radios would make panels unacceptably slow to open.
