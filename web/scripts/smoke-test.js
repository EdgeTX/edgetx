#!/usr/bin/env node
//
// Start every radio of a built site in a browser, and check that it gets past its
// splash screen. Catches what cannot be seen by looking at the files, such as a module
// importing a function the page does not provide.
//
// Usage:  node web/scripts/smoke-test.js [module.wasm ...]
//
// Needs `npm run build`, and `npx playwright-core install chromium`.

import { readFileSync } from 'fs';
import { join, dirname } from 'path';
import { fileURLToPath } from 'url';
import { chromium } from 'playwright-core';
import { preview } from 'vite';

const JOBS = 3;
const TIMEOUT_MS = 120000;

const webDir = join(dirname(fileURLToPath(import.meta.url)), '..');
const radios = JSON.parse(readFileSync(join(webDir, 'dist', 'radios.json'), 'utf-8'));
const selected = process.argv.slice(2);
const queue = radios.filter(({ wasm }) => !selected.length || selected.includes(wasm));

if (!queue.length || (selected.length && queue.length !== selected.length)) {
  console.error(`Selected ${queue.length} of the radios in radios.json`);
  process.exit(2);
}

// Reads back a screenshot, as the WebGL canvas cannot be read directly
async function lcdColours(page) {
  const png = await page.locator('canvas.lcd').screenshot();
  return page.evaluate(async (base64) => {
    const bytes = Uint8Array.from(atob(base64), (c) => c.charCodeAt(0));
    const image = await createImageBitmap(new Blob([bytes], { type: 'image/png' }));
    const canvas = new OffscreenCanvas(image.width, image.height);
    const ctx = canvas.getContext('2d');
    ctx.drawImage(image, 0, 0);
    // Leave out the edges, as the rounded corners show the page behind the LCD
    const [x, y] = [image.width >> 3, image.height >> 3];
    const { data } = ctx.getImageData(x, y, image.width - 2 * x, image.height - 2 * y);
    const colours = new Set();
    for (let i = 0; i < data.length; i += 4) {
      colours.add((data[i] << 16) | (data[i + 1] << 8) | data[i + 2]);
    }
    return colours.size;
  }, png.toString('base64'));
}

async function checkRadio(browser, url, { wasm }) {
  const context = await browser.newContext();
  const page = await context.newPage();
  const errors = [];
  page.on('pageerror', (e) => errors.push(e.message));
  page.on('console', (m) => {
    if (m.type() === 'error') errors.push(m.text());
  });

  try {
    const deadline = Date.now() + TIMEOUT_MS;
    const remaining = () => Math.max(deadline - Date.now(), 1);

    await page.goto(url);
    await page.locator(`select option[value="${wasm}"]`).waitFor({ state: 'attached', timeout: remaining() });
    await page.selectOption('select', wasm);
    await page.getByRole('button', { name: 'Run', exact: true }).click({ timeout: remaining() });

    const status = page.getByText(/^(Running|Error: .*|WASM module not found.*)$/);
    await status.first().waitFor({ timeout: remaining() });
    const text = await status.first().innerText();
    if (text !== 'Running') throw new Error(text);

    const fail = (message) => {
      throw new Error(errors[0] ?? message);
    };
    while ((await lcdColours(page)) < 2) {
      if (errors.length || Date.now() > deadline) fail('nothing drawn on the LCD');
      await page.waitForTimeout(250);
    }

    const splash = await page.locator('canvas.lcd').screenshot();
    while (splash.equals(await page.locator('canvas.lcd').screenshot())) {
      if (errors.length || Date.now() > deadline) fail('stuck on the first screen');
      await page.waitForTimeout(250);
    }
    if (errors.length) fail();
  } finally {
    await context.close();
  }
}

const server = await preview({ root: webDir, preview: { port: 0, open: false } });
const url = server.resolvedUrls.local[0];
const browser = await chromium.launch();
const failed = [];
const started = Date.now();

await Promise.all(Array.from({ length: JOBS }, async () => {
  for (let radio = queue.shift(); radio; radio = queue.shift()) {
    const t = Date.now();
    try {
      await checkRadio(browser, url, radio);
      console.log(`${radio.name}: OK (${((Date.now() - t) / 1000).toFixed(1)}s)`);
    } catch (e) {
      failed.push(radio.name);
      console.error(`${radio.name}: ${e.message.split('\n')[0]}`);
    }
  }
}));

await browser.close();
await server.close();

console.log(`${failed.length} failed, ${((Date.now() - started) / 1000).toFixed(0)}s`);
process.exit(failed.length ? 1 : 0);
