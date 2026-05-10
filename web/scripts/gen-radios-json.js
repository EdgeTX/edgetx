#!/usr/bin/env node
//
// Generate web/public/radios.json from the authoritative hardware definition
// JSON files in radio/src/boards/hw_defs/.
//
// Usage:  node web/scripts/gen-radios-json.js [flavour1 flavour2 ...]
//
// Flavour names match the hw_defs filenames (e.g. "tx16s", "x9d+2019").
// If no flavours are specified, generates entries for all hw_defs files.
//
// The key left/right side mapping matches Companion's radioKeyDefinitions
// table in companion/src/simulation/simulateduiwidget.cpp.

import { readFileSync, writeFileSync } from 'fs';
import { join, dirname } from 'path';
import { fileURLToPath } from 'url';

const __dirname = dirname(fileURLToPath(import.meta.url));
const ROOT = join(__dirname, '..', '..');
const HW_DEFS = join(ROOT, 'radio', 'src', 'boards', 'hw_defs');
const OUTPUT = join(ROOT, 'web', 'public', 'radios.json');

// Key side and order — matches Companion's radioKeyDefinitions table
// (companion/src/simulation/simulateduiwidget.cpp)
const KEY_LAYOUT = {
  KEY_SYS:    { side: 'L', row: 0 },
  KEY_MODEL:  { side: 'R', row: 0 },
  KEY_PAGEUP: { side: 'L', row: 1 },
  KEY_PAGEDN: { side: 'R', row: 1 },
  KEY_UP:     { side: 'L', row: 2 },
  KEY_DOWN:   { side: 'R', row: 2 },
  KEY_LEFT:   { side: 'L', row: 3 },
  KEY_RIGHT:  { side: 'R', row: 3 },
  KEY_MINUS:  { side: 'L', row: 4 },
  KEY_PLUS:   { side: 'R', row: 4 },
  KEY_TELE:   { side: 'R', row: 5 },
  KEY_MENU:   { side: 'L', row: 6 },
  KEY_SHIFT:  { side: 'R', row: 6 },
  KEY_EXIT:   { side: 'L', row: 7 },
  KEY_ENTER:  { side: 'R', row: 7 },
};

// Pretty labels for keys (unicode symbols instead of plain text)
const KEY_LABELS = {
  KEY_PAGEUP: 'PAGE\u25C0',    // PAGE◀
  KEY_PAGEDN: 'PAGE\u25B6',    // PAGE▶
  KEY_UP: '\u25B2',            // ▲
  KEY_DOWN: '\u25BC',          // ▼
  KEY_LEFT: '\u25C0',          // ◀
  KEY_RIGHT: '\u25B6',         // ▶
  KEY_ENTER: 'Enter \u23CE',   // Enter ⏎
};

/** Build display name lookup from fw.json (the authoritative radio name list). */
function loadDisplayNames() {
  try {
    const fw = JSON.parse(readFileSync(join(ROOT, 'fw.json'), 'utf-8'));
    const map = {};
    for (const [name, prefix] of fw.targets) {
      // prefix is e.g. "tx16s-", "x9dp2019-" — strip trailing dash
      const key = prefix.replace(/-$/, '');
      map[key] = name;
    }
    return map;
  } catch {
    return {};
  }
}

const DISPLAY_NAMES = loadDisplayNames();

function getDisplayName(flavour) {
  // fw.json uses build target names (x9dp2019), hw_defs uses flavour (x9d+2019)
  // Try both the flavour and the p-substituted form
  if (DISPLAY_NAMES[flavour]) return DISPLAY_NAMES[flavour];
  const pForm = flavour.replace('+', 'p');
  if (DISPLAY_NAMES[pForm]) return DISPLAY_NAMES[pForm];
  return flavour.toUpperCase();
}

function processFlavour(flavour) {
  let jsonFile;
  try {
    jsonFile = readFileSync(join(HW_DEFS, `${flavour}.json`), 'utf-8');
  } catch {
    console.warn(`  ⚠ Skipping ${flavour}: no hw_defs JSON found`);
    return null;
  }

  const hw = JSON.parse(jsonFile);

  // Inputs: sticks + flex (skip VBAT, RTC_BAT, etc.)
  const inputs = (hw.adc_inputs?.inputs ?? [])
    .filter(i => i.type === 'STICK' || i.type === 'FLEX')
    .map(i => {
      const entry = { name: i.name, type: i.type };
      if (i.label) entry.label = i.label;
      else entry.label = i.name;
      if (i.default) entry.default = i.default;
      return entry;
    });

  // Switches: hardware type 'ADC' means 3POS (analog-read switch)
  const switches = (hw.switches ?? []).map(s => {
    const swType = (s.type === '3POS' || s.type === 'ADC') ? '3POS' : '2POS';
    return { name: s.name, type: swType, default: s.default || swType };
  });

  // Trims
  const trims = (hw.trims ?? []).map(t => ({ name: t.name }));

  // Keys with side mapping, pretty labels, sorted by Companion grid row
  const keys = (hw.keys ?? []).map(k => ({
    key: k.key,
    label: KEY_LABELS[k.key] || k.label || k.name,
    side: (KEY_LAYOUT[k.key] || {}).side || 'R',
  })).sort((a, b) => {
    const ra = (KEY_LAYOUT[a.key] || {}).row ?? 99;
    const rb = (KEY_LAYOUT[b.key] || {}).row ?? 99;
    return ra - rb;
  });

  // Display / LCD info
  const disp = hw.display ?? {};
  const display = {
    w: disp.w ?? 480,
    h: disp.h ?? 272,
    depth: disp.depth ?? 16,
  };

  // WASM filename uses the flavour name directly
  return {
    name: getDisplayName(flavour),
    wasm: `edgetx-${flavour}-simulator.wasm`,
    display,
    inputs,
    switches,
    trims,
    keys,
  };
}

// Determine which flavours to generate: args, or fw.json targets (supported radios only)
let flavours = process.argv.slice(2);
if (flavours.length === 0) {
  // Use fw.json as the source of supported targets
  // The prefix in fw.json is the build target name (e.g. "x9dp2019-")
  // which may differ from the hw_defs flavour (e.g. "x9d+2019")
  const fw = JSON.parse(readFileSync(join(ROOT, 'fw.json'), 'utf-8'));
  flavours = fw.targets.map(([, prefix]) => {
    const buildTarget = prefix.replace(/-$/, '');
    // Check if hw_defs file exists under this name; if not, try + substitution
    try {
      readFileSync(join(HW_DEFS, `${buildTarget}.json`));
      return buildTarget;
    } catch {
      const plusForm = buildTarget.replace('dp', 'd+');
      try {
        readFileSync(join(HW_DEFS, `${plusForm}.json`));
        return plusForm;
      } catch {
        return buildTarget; // will be skipped later
      }
    }
  });
}

console.log(`Generating radios.json for ${flavours.length} flavours...`);

const radios = [];
for (const flavour of flavours) {
  const entry = processFlavour(flavour);
  if (entry) {
    radios.push(entry);
    console.log(`  ✓ ${flavour} → ${entry.name}`);
  }
}

writeFileSync(OUTPUT, JSON.stringify(radios, null, 2) + '\n');
console.log(`\nWrote ${radios.length} entries to ${OUTPUT}`);
