#!/usr/bin/env node
//
// Check that a deployed web simulator serves what the app needs at runtime.
//
// Usage:  node web/scripts/check-deployment.js <site-url>
//
// A radio only shows up in the app if a HEAD request for its module succeeds
// with a WASM content type, and threads need the COOP/COEP headers.

import { readFileSync } from 'fs';
import { join, dirname } from 'path';
import { fileURLToPath } from 'url';

const ATTEMPTS = 3;
const RETRY_DELAY_MS = 15000;

const __dirname = dirname(fileURLToPath(import.meta.url));
const radios = JSON.parse(
  readFileSync(join(__dirname, '..', 'public', 'radios.json'), 'utf-8'),
);

const REQUIRED_HEADERS = {
  'cross-origin-embedder-policy': 'require-corp',
  'cross-origin-opener-policy': 'same-origin',
};

async function checkSite(site) {
  const errors = [];

  const index = await fetch(site);
  if (!index.ok) errors.push(`${site}: status ${index.status}`);
  for (const [name, expected] of Object.entries(REQUIRED_HEADERS)) {
    const actual = index.headers.get(name);
    if (actual !== expected) {
      errors.push(`${site}: ${name} is '${actual}', expected '${expected}'`);
    }
  }

  await Promise.all(radios.map(async ({ wasm }) => {
    const resp = await fetch(new URL(wasm, site), { method: 'HEAD' });
    const type = resp.headers.get('content-type') || '';
    if (!resp.ok || !type.includes('wasm')) {
      errors.push(`${wasm}: status ${resp.status}, content type '${type}'`);
    }
  }));

  // Must not fall back to index.html
  const missing = await fetch(new URL('does-not-exist.wasm', site), { method: 'HEAD' });
  if (missing.status !== 404) {
    errors.push(`Missing file: status ${missing.status}, expected 404`);
  }

  return errors;
}

const url = process.argv[2];
if (!url) {
  console.error('Usage: check-deployment.js <site-url>');
  process.exit(2);
}
const site = url.endsWith('/') ? url : `${url}/`;

// A new deployment can take a moment to be served everywhere
for (let attempt = 1; ; attempt++) {
  const errors = await checkSite(site);
  if (errors.length === 0) {
    console.log(`${site}: OK, ${radios.length} radios`);
    break;
  }
  errors.forEach((e) => console.error(e));
  if (attempt === ATTEMPTS) process.exit(1);
  console.error(`Attempt ${attempt}/${ATTEMPTS} failed, retrying`);
  await new Promise((resolve) => setTimeout(resolve, RETRY_DELAY_MS));
}
