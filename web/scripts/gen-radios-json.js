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

import { readFileSync, readdirSync, writeFileSync } from 'fs';
import { join, dirname, basename } from 'path';
import { fileURLToPath } from 'url';

const __dirname = dirname(fileURLToPath(import.meta.url));
const ROOT = join(__dirname, '..', '..');
const HW_DEFS = join(ROOT, 'radio', 'src', 'boards', 'hw_defs');
const OUTPUT = join(ROOT, 'web', 'public', 'radios.json');

// Key side mapping — matches Companion's radioKeyDefinitions table
const KEY_SIDE = {
  KEY_SYS: 'L',
  KEY_MODEL: 'R',
  KEY_PAGEUP: 'L',
  KEY_PAGEDN: 'R',
  KEY_UP: 'L',
  KEY_DOWN: 'R',
  KEY_LEFT: 'L',
  KEY_RIGHT: 'R',
  KEY_MINUS: 'L',
  KEY_PLUS: 'R',
  KEY_TELE: 'R',
  KEY_MENU: 'L',
  KEY_SHIFT: 'R',
  KEY_EXIT: 'L',
  KEY_ENTER: 'R',
};

// Pretty labels for keys (unicode symbols instead of plain text)
const KEY_LABELS = {
  KEY_PAGEUP: 'PAGE\u25C0',   // PAGE◀
  KEY_PAGEDN: 'PAGE\u25B6',   // PAGE▶
  KEY_UP: '\u25B2',            // ▲
  KEY_DOWN: '\u25BC',          // ▼
  KEY_LEFT: '\u25C0',          // ◀
  KEY_RIGHT: '\u25B6',         // ▶
  KEY_ENTER: '\u21B5',         // ↵
};

// Display names for flavours (where they differ from uppercase filename)
const DISPLAY_NAMES = {
  'tx16smk3': 'TX16S MK3',
  'tx12mk2': 'TX12 MK2',
  'x10express': 'X10 Express',
  'x9d+2019': 'X9D+ 2019',
  'x9d+': 'X9D+',
  'x7access': 'X7 Access',
  'x9lites': 'X9 Lite S',
  'x9lite': 'X9 Lite',
  'xlite': 'X-Lite',
  'xlites': 'X-Lite S',
  'tlite': 'T-Lite',
  'lr3pro': 'LR3 Pro',
  'tpros': 'T-Pro S',
  'tprov2': 'T-Pro V2',
  'pl18ev': 'PL18 EV',
  't12max': 'T12 MAX',
  't15pro': 'T15 Pro',
  't20v2': 'T20 V2',
  'nb4p': 'NB4+',
};

function getDisplayName(flavour) {
  if (DISPLAY_NAMES[flavour]) return DISPLAY_NAMES[flavour];
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

  // Keys with side mapping and pretty labels
  const keys = (hw.keys ?? []).map(k => ({
    key: k.key,
    label: KEY_LABELS[k.key] || k.label || k.name,
    side: KEY_SIDE[k.key] || 'R',
  }));

  // WASM filename uses the flavour name directly
  return {
    name: getDisplayName(flavour),
    wasm: `edgetx-${flavour}-simulator.wasm`,
    inputs,
    switches,
    trims,
    keys,
  };
}

// Determine which flavours to generate: args or all hw_defs files
let flavours = process.argv.slice(2);
if (flavours.length === 0) {
  flavours = readdirSync(HW_DEFS)
    .filter(f => f.endsWith('.json'))
    .map(f => basename(f, '.json'))
    .sort();
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
