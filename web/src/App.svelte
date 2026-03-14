<script lang="ts">
  import { onDestroy } from 'svelte';
  import { WasmRunner, radioKeyFromWasm } from './lib/wasm-runner';
  import { LcdRenderer } from './lib/lcd-renderer';
  import edgetxLogo from './assets/edgetx-logo.svg';

  interface InputDef {
    name: string;
    type: 'STICK' | 'FLEX';
    label: string;
    default?: string;
  }

  interface SwitchDef {
    name: string;
    type: '2POS' | '3POS';
    default: string;
  }

  interface TrimDef {
    name: string;
  }

  interface KeyDef {
    key: string;   // e.g. "KEY_EXIT"
    label: string; // display label
    side: 'L' | 'R';
  }

  // EnumKeys indices from hal/key_driver.h
  const KEY_INDEX: Record<string, number> = {
    KEY_MENU: 0, KEY_EXIT: 1, KEY_ENTER: 2,
    KEY_PAGEUP: 3, KEY_PAGEDN: 4,
    KEY_UP: 5, KEY_DOWN: 6, KEY_LEFT: 7, KEY_RIGHT: 8,
    KEY_PLUS: 9, KEY_MINUS: 10,
    KEY_MODEL: 11, KEY_TELE: 12, KEY_SYS: 13,
    KEY_SHIFT: 14, KEY_BIND: 15,
  };

  interface DisplayDef {
    w: number;
    h: number;
    depth: number;
  }

  interface RadioEntry {
    name: string;
    wasm: string;
    available?: boolean;
    display?: DisplayDef;
    inputs?: InputDef[];
    switches?: SwitchDef[];
    trims?: TrimDef[];
    keys?: KeyDef[];
  }

  let canvas = $state<HTMLCanvasElement>(undefined!);
  let canvasKey = $state(0);
  let status = $state('Loading radio list...');
  let traceLog = $state('');
  let running = $state(false);
  let loading = $state(false);

  let radios = $state<RadioEntry[]>([]);
  let selectedRadio = $state('');
  let currentRadio = $state<RadioEntry | null>(null);

  /** True when the FS Worker is alive and the sim is not running — uploads are possible. */
  let fsReady = $derived(!running && !loading && runner !== null);
  let fsHasContent = $state(false);

  let runner = $state<WasmRunner | null>(null);
  let lcdRenderer: LcdRenderer | null = null;
  let lcdLoopActive = false;
  let fps = $state(0);
  let frameCount = 0;
  let lastFpsTime = 0;
  const themeCycle: Record<string, string> = { auto: 'dark', dark: 'light', light: 'auto' };
  const themeIcon: Record<string, string> = { auto: '\u25D0', dark: '\u263E', light: '\u2600' };
  let themeMode = $state(localStorage.getItem('theme') ?? 'auto');

  function toggleTheme() {
    themeMode = themeCycle[themeMode];
    if (themeMode === 'auto') {
      localStorage.removeItem('theme');
      document.documentElement.removeAttribute('data-theme');
    } else {
      localStorage.setItem('theme', themeMode);
      document.documentElement.setAttribute('data-theme', themeMode);
    }
  }

  // Apply stored theme on mount
  $effect(() => {
    if (themeMode !== 'auto') {
      document.documentElement.setAttribute('data-theme', themeMode);
    }
  });

  // When the dropdown changes, tear down any old instance and init FS for the new radio
  $effect(() => {
    if (!selectedRadio || running || loading) return;
    localStorage.setItem('selectedRadio', selectedRadio);
    const key = radioKeyFromWasm(selectedRadio);
    const url = new URL(location.href);
    url.searchParams.set('radio', key);
    history.replaceState(null, '', url);
    // If there's already a runner for a different radio, tear it down
    if (runner && currentRadio?.wasm !== selectedRadio) {
      teardown().then(() => initFsForRadio(selectedRadio));
    } else if (!runner) {
      initFsForRadio(selectedRadio);
    }
  });

  /** Create a WasmRunner and init its FS Worker for the given radio. */
  async function initFsForRadio(wasmFile: string) {
    const radioKey = radioKeyFromWasm(wasmFile);
    const radio = radios.find((r) => r.wasm === wasmFile);
    const r = new WasmRunner(onTrace, onAudio);
    try {
      const { hasContent } = await r.initFs(radioKey);
      // Guard: if selectedRadio changed while we were awaiting, discard
      if (selectedRadio !== wasmFile) {
        r.stopFs().catch(() => {});
        return;
      }
      runner = r;
      currentRadio = radio ?? null;
      fsHasContent = hasContent;
      // Apply LCD dimensions from manifest
      if (radio?.display) {
        lcdWidth = radio.display.w;
        lcdHeight = radio.display.h;
        lcdDepth = radio.display.depth;
      }
      if (hasContent) {
        const files = await r.fsListFiles('/');
        onTrace(`[fs] ${files.length} file(s) in OPFS for "${radioKey}"\n`);
        await readStickMode();
      } else {
        onTrace(`[fs] no previous data for "${radioKey}"\n`);
      }
      initControls();
      status = `Ready — ${radio?.name ?? wasmFile}`;
    } catch (e) {
      const msg = e instanceof Error ? e.message : String(e);
      onTrace(`[fs] initFs error: ${msg}\n`);
    }
  }

  let lcdWidth = $state(480);
  let lcdHeight = $state(272);
  let lcdDepth = $state(0);
  let lcdSize = 0;

  let isPortrait = $derived(lcdHeight > lcdWidth);
  let cssWidth = $derived(Math.max(lcdWidth, 150 * lcdWidth / lcdHeight, 320));
  let cssHeight = $derived(cssWidth * lcdHeight / lcdWidth);

  // Stick mode: 0=Mode1(thr=RV), 1=Mode2(thr=LV), 2=Mode3(thr=RV), 3=Mode4(thr=LV)
  let stickMode = 0;

  // Audio playback via Web Audio API
  let audioCtx: AudioContext | null = null;
  const AUDIO_SAMPLE_RATE = 32000;
  let audioNextTime = 0; // scheduled playback cursor
  const AUDIO_LATENCY = 0.05; // 50ms latency buffer

  // --- Control state ---
  // Analog inputs: indexed by position in the inputs array
  let analogValues = $state<number[]>([]);
  // Switch states: indexed by position in switches array. -1=up, 0=mid, 1=down
  let switchStates = $state<number[]>([]);
  // 3POS direction tracker: remembers which way to go next from mid position
  let switchDir: number[] = [];
  // Custom switch LED colors: indexed by custom switch index (0-based), CSS color strings
  let customSwitchColors = $state<string[]>([]);

  // Gimbal drag state
  let leftGimbalEl = $state<HTMLDivElement>(undefined!);
  let rightGimbalEl = $state<HTMLDivElement>(undefined!);
  let draggingGimbal = $state<'left' | 'right' | null>(null);

  // Derived: stick indices (always first 4 inputs: LH, LV, RV, RH)
  function getStickIndices(): { lh: number; lv: number; rv: number; rh: number } {
    const inputs = currentRadio?.inputs ?? [];
    let lh = -1, lv = -1, rv = -1, rh = -1;
    for (let i = 0; i < inputs.length; i++) {
      if (inputs[i].type === 'STICK') {
        if (inputs[i].name === 'LH') lh = i;
        else if (inputs[i].name === 'LV') lv = i;
        else if (inputs[i].name === 'RV') rv = i;
        else if (inputs[i].name === 'RH') rh = i;
      }
    }
    return { lh, lv, rv, rh };
  }

  function isCustomSwitch(sw: SwitchDef): boolean {
    return sw.name.startsWith('SW');
  }

  function getVisibleSwitches(): { sw: SwitchDef; index: number }[] {
    const switches = currentRadio?.switches ?? [];
    return switches
      .map((sw, index) => ({ sw, index }))
      .filter(({ sw }) => sw.default !== 'NONE' && !isCustomSwitch(sw));
  }

  function getCustomSwitches(): { sw: SwitchDef; index: number }[] {
    const switches = currentRadio?.switches ?? [];
    return switches
      .map((sw, index) => ({ sw, index }))
      .filter(({ sw }) => sw.default !== 'NONE' && isCustomSwitch(sw));
  }

  function getLeftSwitches(): { sw: SwitchDef; index: number }[] {
    const all = getVisibleSwitches();
    const half = Math.ceil(all.length / 2);
    return all.slice(0, half);
  }

  function getRightSwitches(): { sw: SwitchDef; index: number }[] {
    const all = getVisibleSwitches();
    const half = Math.ceil(all.length / 2);
    return all.slice(half);
  }

  function getFlexInputs(): { input: InputDef; index: number }[] {
    const inputs = currentRadio?.inputs ?? [];
    return inputs
      .map((input, index) => ({ input, index }))
      .filter(({ input }) => input.type === 'FLEX' && input.default && input.default !== 'NONE');
  }

  /** Pots + multipos in original JSON order (e.g. S1, 6POS, S2 on TX16S). */
  function getPotsAndMultipos(): { input: InputDef; index: number }[] {
    return getFlexInputs().filter(({ input }) =>
      input.default === 'POT' || input.default === 'POT_CENTER' || input.default === 'MULTIPOS'
    );
  }

  function getLeftPotsAndMultipos(): { input: InputDef; index: number }[] {
    const all = getPotsAndMultipos();
    return all.slice(0, Math.ceil(all.length / 2));
  }

  function getRightPotsAndMultipos(): { input: InputDef; index: number }[] {
    const all = getPotsAndMultipos();
    return all.slice(Math.ceil(all.length / 2));
  }

  function getSliders(): { input: InputDef; index: number }[] {
    return getFlexInputs().filter(({ input }) => input.default === 'SLIDER');
  }

  /** Read stickMode from /RADIO/radio.yml via FS Worker. */
  async function readStickMode(): Promise<void> {
    if (!runner) return;
    try {
      const data = await runner.fsReadTextFile('/RADIO/radio.yml');
      if (data) {
        const match = data.match(/^stickMode:\s*(\d+)/m);
        if (match) {
          stickMode = parseInt(match[1], 10);
          onTrace(`[fs] stickMode=${stickMode} (Mode ${stickMode + 1})\n`);
        }
      }
    } catch {
      // No radio.yml yet — use default mode
    }
  }

  /** Get the input index of the throttle axis based on stick mode. */
  function getThrottleIndex(): number {
    const si = getStickIndices();
    // Modes 1,3 (stickMode 0,2): throttle = RV; Modes 2,4 (stickMode 1,3): throttle = LV
    return (stickMode & 1) ? si.lv : si.rv;
  }

  // Initialize control state from current radio
  function initControls() {
    if (!currentRadio) return;
    const thrIdx = getThrottleIndex();
    const inputs = currentRadio.inputs ?? [];
    analogValues = inputs.map((input, i) => {
      if (input.default === 'MULTIPOS') return 0;
      if (i === thrIdx) return 0; // throttle at lowest
      return 2048; // others centered
    });

    const switches = currentRadio.switches ?? [];
    switchStates = switches.map(() => -1); // all up by default
    switchDir = switches.map(() => 1);    // first mid click goes down
  }

  // Send analog value to WASM in ADC range (0..4096, center=2048)
  function updateAnalog(index: number, value: number) {
    analogValues[index] = Math.round(Math.max(0, Math.min(4096, value)));
    runner?.setAnalog(index, analogValues[index]);
  }

  // Send switch state to WASM
  function updateSwitch(index: number, state: number) {
    switchStates[index] = state;
    runner?.exports?.simuSetSwitch(index, state);
  }

  // Discover which radios have WASM files available
  async function discoverRadios() {
    try {
      const resp = await fetch('./radios.json');
      if (!resp.ok) {
        status = 'No radios.json manifest found';
        return;
      }
      const manifest: RadioEntry[] = await resp.json();

      // Probe each WASM file with a HEAD request, verify content-type
      // (Vite's dev server returns 200 with text/html for missing files)
      const checks = manifest.map(async (r) => {
        try {
          const head = await fetch(`./${r.wasm}`, { method: 'HEAD' });
          const ct = head.headers.get('content-type') || '';
          return { ...r, available: head.ok && ct.includes('wasm') };
        } catch {
          return { ...r, available: false };
        }
      });

      radios = await Promise.all(checks);

      const avail = radios.filter((r) => r.available);
      if (avail.length > 0) {
        const urlParam = new URLSearchParams(location.search).get('radio');
        const saved = localStorage.getItem('selectedRadio');
        const match = urlParam && avail.find((r) => {
          const key = radioKeyFromWasm(r.wasm);
          const q = urlParam.toLowerCase();
          return key.toLowerCase() === q || r.name.toLowerCase() === q;
        });
        selectedRadio = match ? match.wasm
          : (saved && avail.some((r) => r.wasm === saved)) ? saved
          : avail[0].wasm;
        status = `${avail.length} radio(s) available`;
      } else {
        status = 'No .wasm files found in public/';
      }
    } catch (e) {
      status = `Error: ${e instanceof Error ? e.message : e}`;
    }
  }

  function onTrace(text: string) {
    traceLog += text;
    const lines = traceLog.split('\n');
    if (lines.length > 200) {
      traceLog = lines.slice(-200).join('\n');
    }
  }

  function onAudio(samples: Int16Array) {
    if (!audioCtx) return;
    const buf = audioCtx.createBuffer(1, samples.length, AUDIO_SAMPLE_RATE);
    const chan = buf.getChannelData(0);
    for (let i = 0; i < samples.length; i++) {
      chan[i] = samples[i] / 32768;
    }

    const now = audioCtx.currentTime;
    // If we've fallen behind or haven't started, reset the cursor
    if (audioNextTime < now) {
      audioNextTime = now + AUDIO_LATENCY;
    }

    const src = audioCtx.createBufferSource();
    src.buffer = buf;
    src.connect(audioCtx.destination);
    src.start(audioNextTime);
    audioNextTime += samples.length / AUDIO_SAMPLE_RATE;
  }

  /** Load WASM + start the simulator in one step. */
  async function run() {
    if (!selectedRadio || loading) return;

    try {
      loading = true;
      traceLog = '';
      const radio = radios.find((r) => r.wasm === selectedRadio);
      currentRadio = radio ?? null;
      status = `Loading ${radio?.name ?? selectedRadio}...`;

      const radioKey = radioKeyFromWasm(selectedRadio);

      // Reuse existing runner/FS Worker from radio selection, or create fresh
      if (!runner || !runner.hasFsWorker) {
        await teardown();
        runner = new WasmRunner(onTrace, onAudio);
        const { hasContent } = await runner.initFs(radioKey);
        if (hasContent) {
          const files = await runner.fsListFiles('/');
          onTrace(`[fs] ${files.length} file(s) in OPFS for "${radioKey}"\n`);
        } else {
          onTrace(`[fs] no previous data for "${radioKey}"\n`);
        }
      }

      // Always re-read stick mode before starting (user may have changed it in a previous run)
      await readStickMode();

      await runner.load(`./${selectedRadio}`);

      const ex = runner.exports!;
      lcdWidth = ex.simuLcdGetWidth();
      lcdHeight = ex.simuLcdGetHeight();
      lcdDepth = ex.simuLcdGetDepth();

      if (lcdDepth === 1) {
        lcdSize = lcdWidth * ((lcdHeight + 7) >> 3);
      } else if (lcdDepth === 4) {
        lcdSize = (lcdWidth * lcdHeight * 4) >> 3;
      } else {
        lcdSize = lcdWidth * lcdHeight * (lcdDepth >> 3);
      }

      initControls();

      // Push all analog/switch values to WASM before starting
      for (let i = 0; i < analogValues.length; i++) {
        runner.setAnalog(i, analogValues[i]);
      }
      for (let i = 0; i < switchStates.length; i++) {
        ex.simuSetSwitch(i, switchStates[i]);
      }

      // --- Init audio ---
      if (!audioCtx) {
        audioCtx = new AudioContext({ sampleRate: AUDIO_SAMPLE_RATE });
      }
      if (audioCtx.state === 'suspended') {
        await audioCtx.resume();
      }
      audioNextTime = 0;

      // --- Start sim ---
      ex.simuInit();
      runner.setFatfsPaths('/', '/');
      ex.simuCreateDefaults();
      ex.simuStart(1);

      if (canvas) {
        lcdRenderer = new LcdRenderer(canvas);
        const dpr = window.devicePixelRatio || 1;
        // Color LCDs: integer-scale for crisp pixels.
        // Dot-matrix LCDs: match CSS size so shader has enough pixels for grid effect.
        let canvasW: number, canvasH: number;
        if (lcdDepth >= 16) {
          const scale = Math.max(1, Math.round(dpr));
          canvasW = lcdWidth * scale;
          canvasH = lcdHeight * scale;
        } else {
          canvasW = Math.round(cssWidth * dpr);
          canvasH = Math.round(cssHeight * dpr);
        }
        lcdRenderer.resize(canvasW, canvasH);
        onTrace(`[lcd] ${lcdWidth}×${lcdHeight}@${lcdDepth}bpp css=${Math.round(cssWidth)}×${Math.round(cssHeight)} buf=${canvasW}×${canvasH} dpr=${dpr}\n`);
      }

      loading = false;
      running = true;
      status = 'Running';

      lcdLoop().catch((e) => console.error('lcdLoop error:', e));
    } catch (e) {
      loading = false;
      const msg = e instanceof Error ? e.message : String(e);
      if (msg.includes('MIME') || msg.includes('Failed to fetch')) {
        status = `WASM module not found: ${selectedRadio}`;
      } else {
        status = `Error: ${msg}`;
      }
    }
  }

  /** Stop the simulator.  Keeps the FS Worker alive for uploads. */
  async function stop() {
    lcdLoopActive = false;
    running = false;
    loading = true; // suppress fsReady until fully stopped
    lcdRenderer = null;
    status = 'Stopping...';

    if (runner?.exports) {
      try {
        runner.exports.simuStop();
      } catch {
        // simuStop() sets the shutdown flag and signals threads to stop,
        // but traps when trying to join (Atomics.wait on main thread).
        // Worker threads continue their shutdown asynchronously.
      }
    }

    // Wait for worker threads to finish writing settings/files, then terminate them.
    await new Promise<void>((resolve) => setTimeout(resolve, 2000));
    runner?.stopSim();
    fsHasContent = true; // sim writes settings/models on shutdown
    loading = false;

    status = `Stopped — ${currentRadio?.name ?? selectedRadio}`;
  }

  /** Full teardown: stop sim + terminate FS Worker. */
  async function teardown() {
    lcdLoopActive = false;
    const r = runner;
    // Set state synchronously first to avoid re-entrant effects
    runner = null;
    lcdRenderer = null;
    running = false;
    currentRadio = null;
    fsHasContent = false;
    lcdDepth = 0;
    // Force Svelte to recreate the canvas element (avoids stale WebGL context)
    canvasKey++;

    if (r) {
      if (r.exports) {
        try { r.exports.simuStop(); } catch { /* expected */ }
      }
      r.stopSim();
      try { await r.stopFs(); } catch { /* best effort */ }
    }
  }

  function pollCustomSwitches() {
    const ex = runner?.exports;
    if (!ex) return;
    const numCs = ex.simuGetNumCustomSwitches();
    if (numCs > 0) {
      for (let i = 0; i < numCs; i++) {
        const on = ex.simuGetCustomSwitchState(i);
        if (on) {
          const rgb = ex.simuGetCustomSwitchColor(i);
          const r = (rgb >> 16) & 0xff, g = (rgb >> 8) & 0xff, b = rgb & 0xff;
          customSwitchColors[i] = (r || g || b) ? `rgb(${r},${g},${b})` : '#38bff9';
        } else {
          customSwitchColors[i] = '';
        }
      }
    }
  }

  /** Async LCD loop: waits for firmware frame notifications via Atomics.waitAsync. */
  async function lcdLoop() {
    lcdLoopActive = true;
    frameCount = 0;
    lastFpsTime = performance.now();
    fps = 0;

    // Update FPS display every 250ms, smoothed with exponential moving average
    let smoothFps = 0;
    const fpsInterval = setInterval(() => {
      const now = performance.now();
      const elapsed = now - lastFpsTime;
      if (elapsed > 0) {
        const instantFps = frameCount * 1000 / elapsed;
        smoothFps += (instantFps - smoothFps) * 0.4;
        fps = Math.round(smoothFps);
      }
      frameCount = 0;
      lastFpsTime = now;
    }, 250);

    try {
      while (lcdLoopActive && runner) {
        const hasFrame = await runner.waitForLcdFrame(100);
        if (!lcdLoopActive || !runner) break;

        const ex = runner.exports;
        if (!ex || !lcdRenderer) continue;

        pollCustomSwitches();

        const lcdChanged = ex.simuLcdChanged();
        if (!hasFrame && !lcdChanged) continue;

        const data = runner.copyLcd(lcdSize);
        if (!data) continue;

        lcdRenderer.render(data, lcdWidth, lcdHeight, lcdDepth);
        ex.simuLcdFlushed();
        frameCount++;
      }
    } finally {
      clearInterval(fpsInterval);
      fps = 0;
    }
  }

  function canvasToLcd(clientX: number, clientY: number): { x: number; y: number } {
    const rect = canvas.getBoundingClientRect();
    const scaleX = lcdWidth / rect.width;
    const scaleY = lcdHeight / rect.height;
    return {
      x: Math.round((clientX - rect.left) * scaleX),
      y: Math.round((clientY - rect.top) * scaleY),
    };
  }

  function handleCanvasMouseDown(e: MouseEvent) {
    const ex = runner?.exports;
    if (!ex || !running || !canvas) return;

    const { x, y } = canvasToLcd(e.clientX, e.clientY);
    ex.simuTouchDown(x, y);

    const onMove = (ev: MouseEvent) => {
      const pos = canvasToLcd(ev.clientX, ev.clientY);
      ex.simuTouchDown(pos.x, pos.y);
    };
    const onUp = () => {
      ex.simuTouchUp();
      window.removeEventListener('mousemove', onMove);
      window.removeEventListener('mouseup', onUp);
    };
    window.addEventListener('mousemove', onMove);
    window.addEventListener('mouseup', onUp);
  }

  function handleCanvasTouchStart(e: TouchEvent) {
    e.preventDefault();
    const ex = runner?.exports;
    if (!ex || !running || !canvas) return;

    const t = e.touches[0];
    const { x, y } = canvasToLcd(t.clientX, t.clientY);
    ex.simuTouchDown(x, y);

    const onMove = (ev: TouchEvent) => {
      ev.preventDefault();
      const t = ev.touches[0];
      const pos = canvasToLcd(t.clientX, t.clientY);
      ex.simuTouchDown(pos.x, pos.y);
    };
    const onEnd = () => {
      ex.simuTouchUp();
      window.removeEventListener('touchmove', onMove);
      window.removeEventListener('touchend', onEnd);
      window.removeEventListener('touchcancel', onEnd);
    };
    window.addEventListener('touchmove', onMove, { passive: false });
    window.addEventListener('touchend', onEnd);
    window.addEventListener('touchcancel', onEnd);
  }

  let lastWheelTime = 0;
  function handleWheel(e: WheelEvent) {
    const ex = runner?.exports;
    if (!ex || !running) return;
    e.preventDefault();
    const now = performance.now();
    if (now - lastWheelTime < 200) return;
    lastWheelTime = now;
    const steps = e.deltaY > 0 ? 1 : -1;
    ex.simuRotaryEncoderEvent(steps);
  }

  // Key constants matching radio/src/hal/key_driver.h
  const KEY_MENU = 0, KEY_EXIT = 1, KEY_ENTER = 2;
  const KEY_PAGEUP = 3, KEY_PAGEDN = 4;
  const KEY_UP = 5, KEY_DOWN = 6, KEY_LEFT = 7, KEY_RIGHT = 8;
  const KEY_PLUS = 9, KEY_MINUS = 10;
  const KEY_MODEL = 11, KEY_TELE = 12, KEY_SYS = 13;

  const ROTARY_ENCODER_GRANULARITY = 1;

  // Map keyboard keys to radio key names
  const KEYBOARD_MAP: Record<string, string> = {
    'Escape': 'KEY_EXIT',
    'Enter': 'KEY_ENTER',
    'ArrowLeft': 'KEY_LEFT',
    'ArrowRight': 'KEY_RIGHT',
    'PageUp': 'KEY_PAGEUP',
    'PageDown': 'KEY_PAGEDN',
    '+': 'KEY_PLUS',
    '-': 'KEY_MINUS',
    'm': 'KEY_MENU',
    's': 'KEY_SYS',
    't': 'KEY_TELE',
  };

  function handleKey(e: KeyboardEvent) {
    const ex = runner?.exports;
    if (!ex || !running) return;

    if (e.repeat) return;
    if (e.metaKey || e.ctrlKey || e.altKey) return; // don't hijack browser shortcuts
    const down = e.type === 'keydown';

    // Rotary encoder: arrow up/down
    if (e.key === 'ArrowUp' || e.key === 'ArrowDown') {
      if (down) ex.simuRotaryEncoderEvent(e.key === 'ArrowUp' ? -ROTARY_ENCODER_GRANULARITY : ROTARY_ENCODER_GRANULARITY);
      e.preventDefault();
      return;
    }

    const keyName = KEYBOARD_MAP[e.key];
    if (!keyName) return; // don't prevent default for unmapped keys

    if (down) keyDown(keyName);
    else keyUp(keyName);
    e.preventDefault();
  }

  // --- Gimbal handling ---
  function gimbalPos(el: HTMLDivElement, e: MouseEvent | Touch): { x: number; y: number } {
    const rect = el.getBoundingClientRect();
    const rawX = (e.clientX - rect.left) / rect.width;
    const rawY = (e.clientY - rect.top) / rect.height;
    const x = Math.max(-1, Math.min(1, (rawX - 0.5) * 2));
    const y = Math.max(-1, Math.min(1, (0.5 - rawY) * 2)); // invert Y: up is positive
    return { x, y };
  }

  function handleGimbalDown(side: 'left' | 'right', e: MouseEvent) {
    e.preventDefault();
    draggingGimbal = side;
    const el = side === 'left' ? leftGimbalEl : rightGimbalEl;
    const pos = gimbalPos(el, e);
    applyGimbal(side, pos.x, pos.y);

    const onMove = (ev: MouseEvent) => {
      ev.preventDefault();
      const p = gimbalPos(el, ev);
      applyGimbal(side, p.x, p.y);
    };
    const onUp = () => {
      draggingGimbal = null;
      releaseGimbal(side);
      window.removeEventListener('mousemove', onMove);
      window.removeEventListener('mouseup', onUp);
    };
    window.addEventListener('mousemove', onMove);
    window.addEventListener('mouseup', onUp);
  }

  function handleGimbalTouchStart(side: 'left' | 'right', e: TouchEvent) {
    e.preventDefault();
    draggingGimbal = side;
    const el = side === 'left' ? leftGimbalEl : rightGimbalEl;
    const touch = e.touches[0];
    const pos = gimbalPos(el, touch);
    applyGimbal(side, pos.x, pos.y);

    const onMove = (ev: TouchEvent) => {
      ev.preventDefault();
      const t = ev.touches[0];
      const p = gimbalPos(el, t);
      applyGimbal(side, p.x, p.y);
    };
    const onEnd = () => {
      draggingGimbal = null;
      releaseGimbal(side);
      window.removeEventListener('touchmove', onMove);
      window.removeEventListener('touchend', onEnd);
      window.removeEventListener('touchcancel', onEnd);
    };
    window.addEventListener('touchmove', onMove, { passive: false });
    window.addEventListener('touchend', onEnd);
    window.addEventListener('touchcancel', onEnd);
  }

  // Convert -1..+1 UI position to 0..4096 ADC range (center=2048)
  function toAdc(v: number): number {
    return Math.round((v + 1) * 2048);
  }

  // Convert 0..4096 ADC value back to -1..+1 for UI display
  function fromAdc(v: number): number {
    return v / 2048 - 1;
  }

  /** Animate an axis back to a target with damped spring physics. */
  let springAnimations = new Map<number, number>(); // index → rafId

  function springTo(index: number, target: number) {
    // Cancel any existing animation on this axis
    const existing = springAnimations.get(index);
    if (existing) cancelAnimationFrame(existing);

    let pos = analogValues[index];
    let vel = 0;
    let prev = performance.now();
    const stiffness = 600;  // spring force
    const damping = 25;     // friction

    const tick = (now: number) => {
      const dt = Math.min((now - prev) / 1000, 0.033);
      prev = now;

      const force = (target - pos) * stiffness;
      vel = (vel + force * dt) * Math.exp(-damping * dt);
      pos += vel * dt;

      updateAnalog(index, Math.round(pos));

      if (Math.abs(pos - target) < 1 && Math.abs(vel) < 10) {
        updateAnalog(index, target);
        springAnimations.delete(index);
        return;
      }
      springAnimations.set(index, requestAnimationFrame(tick));
    };

    springAnimations.set(index, requestAnimationFrame(tick));
  }

  /** On release, spring back to center except throttle axis keeps its position. */
  function releaseGimbal(side: 'left' | 'right') {
    const si = getStickIndices();
    const thrIdx = getThrottleIndex();
    if (side === 'left') {
      if (si.lh >= 0) springTo(si.lh, 2048);
      if (si.lv >= 0 && si.lv !== thrIdx) springTo(si.lv, 2048);
    } else {
      if (si.rh >= 0) springTo(si.rh, 2048);
      if (si.rv >= 0 && si.rv !== thrIdx) springTo(si.rv, 2048);
    }
  }

  function applyGimbal(side: 'left' | 'right', x: number, y: number) {
    const si = getStickIndices();
    if (side === 'left') {
      if (si.lh >= 0) updateAnalog(si.lh, toAdc(x));
      if (si.lv >= 0) updateAnalog(si.lv, toAdc(y));
    } else {
      if (si.rh >= 0) updateAnalog(si.rh, toAdc(x));
      if (si.rv >= 0) updateAnalog(si.rv, toAdc(y));
    }
  }

  // Gimbal crosshair position as CSS percentage
  function gimbalCrossX(side: 'left' | 'right'): number {
    const si = getStickIndices();
    const idx = side === 'left' ? si.lh : si.rh;
    if (idx < 0) return 50;
    return 50 + fromAdc(analogValues[idx]) * 50;
  }

  function gimbalCrossY(side: 'left' | 'right'): number {
    const si = getStickIndices();
    const idx = side === 'left' ? si.lv : si.rv;
    if (idx < 0) return 50;
    return 50 - fromAdc(analogValues[idx]) * 50;
  }

  // --- Key handling ---
  function keyDown(keyName: string) {
    const idx = KEY_INDEX[keyName];
    if (idx !== undefined) runner?.exports?.simuSetKey(idx, 1);
  }

  function keyUp(keyName: string) {
    const idx = KEY_INDEX[keyName];
    if (idx !== undefined) runner?.exports?.simuSetKey(idx, 0);
  }

  function getLeftKeys(): KeyDef[] {
    return (currentRadio?.keys ?? []).filter(k => k.side === 'L');
  }

  function getRightKeys(): KeyDef[] {
    return (currentRadio?.keys ?? []).filter(k => k.side === 'R');
  }

  // --- Switch toggle handling ---
  function handleSwitchKey(index: number, sw: SwitchDef, e: KeyboardEvent) {
    if (e.key === 'ArrowUp' || e.key === 'ArrowLeft') {
      e.preventDefault();
      const min = sw.type === '2POS' ? -1 : -1;
      updateSwitch(index, Math.max(min, switchStates[index] - 1));
    } else if (e.key === 'ArrowDown' || e.key === 'ArrowRight') {
      e.preventDefault();
      updateSwitch(index, Math.min(1, switchStates[index] + 1));
    }
  }

  function handleSwitchTrackClick(index: number, sw: SwitchDef, e: MouseEvent) {
    const track = e.currentTarget as HTMLElement;
    const rect = track.getBoundingClientRect();
    const y = (e.clientY - rect.top) / rect.height; // 0=top, 1=bottom
    if (sw.type === '2POS') {
      updateSwitch(index, y < 0.5 ? -1 : 1);
    } else {
      if (y < 0.33) updateSwitch(index, -1);
      else if (y < 0.67) updateSwitch(index, 0);
      else updateSwitch(index, 1);
    }
  }

  function snapSwitch(index: number, sw: SwitchDef, track: HTMLElement, clientY: number) {
    const rect = track.getBoundingClientRect();
    const y = Math.max(0, Math.min(1, (clientY - rect.top) / rect.height));
    if (sw.type === '2POS') {
      updateSwitch(index, y < 0.5 ? -1 : 1);
    } else {
      if (y < 0.33) updateSwitch(index, -1);
      else if (y < 0.67) updateSwitch(index, 0);
      else updateSwitch(index, 1);
    }
  }

  function handleSwitchDrag(index: number, sw: SwitchDef, e: MouseEvent) {
    e.preventDefault();
    const track = (e.currentTarget as HTMLElement).parentElement!;
    const onMove = (ev: MouseEvent) => snapSwitch(index, sw, track, ev.clientY);
    const onUp = () => {
      window.removeEventListener('mousemove', onMove);
      window.removeEventListener('mouseup', onUp);
    };
    window.addEventListener('mousemove', onMove);
    window.addEventListener('mouseup', onUp);
  }

  function handleSwitchTouchDrag(index: number, sw: SwitchDef, e: TouchEvent) {
    e.preventDefault();
    const track = (e.currentTarget as HTMLElement).parentElement!;
    const onMove = (ev: TouchEvent) => {
      ev.preventDefault();
      snapSwitch(index, sw, track, ev.touches[0].clientY);
    };
    const onEnd = () => {
      window.removeEventListener('touchmove', onMove);
      window.removeEventListener('touchend', onEnd);
      window.removeEventListener('touchcancel', onEnd);
    };
    window.addEventListener('touchmove', onMove, { passive: false });
    window.addEventListener('touchend', onEnd);
    window.addEventListener('touchcancel', onEnd);
  }

  function switchKnobTop(state: number, is3pos: boolean): string {
    // Knob is 14px tall, use calc() to keep it inside the track
    if (is3pos) {
      if (state === -1) return '0px';
      if (state === 0) return 'calc(50% - 7px)';
      return 'calc(100% - 14px)';
    }
    return state === -1 ? '0px' : 'calc(100% - 14px)';
  }

  // --- Pot/Slider handling ---
  function handlePotChange(index: number, e: Event) {
    const val = parseInt((e.target as HTMLInputElement).value);
    updateAnalog(index, val);
  }

  // --- Multipos handling ---
  function handleMultiposChange(index: number, position: number) {
    // 6 positions mapped to 0..4096 ADC range (matches SDL: (pos * 4096) / 5)
    updateAnalog(index, Math.round((position * 4096) / 5));
  }

  // --- Trim handling ---
  // Board::TrimSwitches enum (physical axis order):
  //   LH_DEC=0 LH_INC=1, LV_DEC=2 LV_INC=3, RV_DEC=4 RV_INC=5, RH_DEC=6 RH_INC=7, T5..T6..
  // JSON trims T1..T4 map to physical axes in reverse: T1=RH(6/7), T2=RV(4/5), T3=LV(2/3), T4=LH(0/1)
  // T5+ map sequentially starting at index 8.
  function trimSwitchIndex(trimIndex: number, direction: 'dec' | 'inc'): number {
    const maxMain = Math.min(currentRadio?.trims?.length ?? 4, 4);
    const base = trimIndex < maxMain ? (maxMain - 1 - trimIndex) * 2 : trimIndex * 2;
    return base + (direction === 'inc' ? 1 : 0);
  }

  function handleTrimDown(trimIndex: number, direction: 'dec' | 'inc') {
    const ex = runner?.exports;
    if (!ex || !running) return;
    ex.simuSetTrim(trimSwitchIndex(trimIndex, direction), 1);
  }

  function handleTrimUp(trimIndex: number, direction: 'dec' | 'inc') {
    const ex = runner?.exports;
    if (!ex || !running) return;
    ex.simuSetTrim(trimSwitchIndex(trimIndex, direction), 0);
  }

  async function resetSdCard() {
    if (!runner) return;
    if (!confirm('Wipe all SD card data for this radio? This cannot be undone.')) return;
    await runner.fsWipe();
    fsHasContent = false;
    onTrace('[fs] SD card data wiped\n');
    status = 'SD card wiped. Reload to start fresh.';
  }

  function handleFileUpload() {
    if (!runner) return;
    const targetDir = prompt('Upload to directory:', '/') ?? '/';
    const input = document.createElement('input');
    input.type = 'file';
    input.multiple = true;
    input.onchange = async () => {
      if (!input.files || !runner) return;
      const total = input.files.length;
      let count = 0;
      if (total > 1) status = `Uploading 0/${total} files...`;
      for (const file of input.files) {
        const dir = targetDir.endsWith('/') ? targetDir : targetDir + '/';
        const path = dir + file.name;
        const data = await file.arrayBuffer();
        await runner.fsWriteFile(path, data);
        count++;
        if (total > 1 && (count % 10 === 0 || count === total)) {
          status = `Uploading ${count}/${total} files to ${targetDir}...`;
        }
      }
      onTrace(`[fs] uploaded ${count} file(s) to ${targetDir}\n`);
      if (count > 0) fsHasContent = true;
      status = `Uploaded ${count} file(s) to ${targetDir}`;
    };
    input.click();
  }

  function handleFolderUpload() {
    if (!runner) return;
    const input = document.createElement('input');
    input.type = 'file';
    input.multiple = true;
    (input as any).webkitdirectory = true;
    input.onchange = async () => {
      if (!input.files || !runner) return;
      const total = input.files.length;
      let count = 0;
      status = `Uploading 0/${total} files...`;
      for (const file of input.files) {
        // webkitRelativePath gives "SOUNDS/en/system/hello.wav"
        // Keep as-is so selecting "SOUNDS" creates /SOUNDS/...
        const relPath = (file as any).webkitRelativePath as string;
        if (!relPath) continue;
        const path = '/' + relPath;
        const data = await file.arrayBuffer();
        await runner.fsWriteFile(path, data);
        count++;
        if (count % 10 === 0 || count === total) {
          status = `Uploading ${count}/${total} files...`;
        }
      }
      onTrace(`[fs] uploaded ${count} file(s) from folder\n`);
      if (count > 0) fsHasContent = true;
      status = `Uploaded ${count} file(s)`;
    };
    input.click();
  }

  // Discover on mount
  discoverRadios();

  // OPFS writes are durable immediately — no save-on-unload needed.

  onDestroy(() => {
    teardown();
    audioCtx?.close();
  });
</script>

<svelte:document onkeydown={handleKey} onkeyup={handleKey} />

<main>
  <div class="header">
    <div class="logo-title">
      <img src={edgetxLogo} alt="EdgeTX" class="logo" />
      <span class="subtitle">Simulator</span>
    </div>
    <div class="toolbar">
      <select
        bind:value={selectedRadio}
        disabled={running || loading}
      >
        {#each radios.filter(r => r.available) as radio}
          <option value={radio.wasm}>
            {radio.name}
          </option>
        {/each}
      </select>

      {#if !running && !loading}
        <button onclick={run} disabled={!selectedRadio}>
          Run
        </button>
      {/if}
      {#if running}
        <button onclick={stop}>Stop</button>
      {/if}
      {#if fsReady}
        <button onclick={handleFileUpload} class="secondary">Upload Files</button>
        <button onclick={handleFolderUpload} class="secondary">Upload Folder</button>
        {#if fsHasContent}
          <button onclick={resetSdCard} class="danger">Reset SD</button>
        {/if}
      {/if}
      <button class="theme-toggle" onclick={toggleTheme}
              aria-label="Toggle theme">{themeIcon[themeMode]}</button>
    </div>
    <p class="status">{status}</p>
  </div>

  <!-- Reusable snippets for switch controls and gimbals -->

  {#snippet switchCtrl(sw: SwitchDef, index: number)}
    <div class="switch-control">
      <span class="switch-name">{sw.name}</span>
      <div class="switch-track" class:is-3pos={sw.type === '3POS'}
           role="slider" tabindex="0"
           aria-label={sw.name}
           aria-valuemin={sw.type === '3POS' ? -1 : 0}
           aria-valuemax={1}
           aria-valuenow={switchStates[index]}
           onclick={(e) => handleSwitchTrackClick(index, sw, e)}
           onkeydown={(e) => handleSwitchKey(index, sw, e)}>
        {#if sw.type === '3POS'}
          <div class="switch-notch" style:top="7px"></div>
          <div class="switch-notch" style:top="50%"></div>
          <div class="switch-notch" style:top="calc(100% - 7px)"></div>
        {:else}
          <div class="switch-notch" style:top="7px"></div>
          <div class="switch-notch" style:top="calc(100% - 7px)"></div>
        {/if}
        <!-- svelte-ignore a11y_no_static_element_interactions -->
        <div class="switch-knob" role="presentation"
             class:sw-up={switchStates[index] === -1}
             class:sw-mid={switchStates[index] === 0}
             class:sw-down={switchStates[index] === 1}
             style:top={switchKnobTop(switchStates[index], sw.type === '3POS')}
             onmousedown={(e) => handleSwitchDrag(index, sw, e)}
             ontouchstart={(e) => handleSwitchTouchDrag(index, sw, e)}
        ></div>
      </div>
    </div>
  {/snippet}

  {#snippet lcdScreen()}
    <div class="lcd-bezel">
      {#key canvasKey}
      <canvas
        bind:this={canvas}
        class="lcd"
        style:width="{cssWidth}px"
        style:height="{cssHeight}px"
        style:background="{lcdDepth > 0 && lcdDepth < 16 ? 'rgb(47, 123, 227)' : '#000'}"
        onmousedown={handleCanvasMouseDown}
        ontouchstart={handleCanvasTouchStart}
        onwheel={handleWheel}
      ></canvas>
      {/key}
    </div>
  {/snippet}

  {#snippet leftGimbalBlock()}
    <div class="gimbal-wrapper gimbal-left">
      {#if (currentRadio?.trims?.length ?? 0) >= 4}
        <div class="trim-col">
          <button class="trim-btn"
            onmousedown={() => handleTrimDown(2, 'inc')}
            onmouseup={() => handleTrimUp(2, 'inc')}
            ontouchstart={() => handleTrimDown(2, 'inc')}
            ontouchend={() => handleTrimUp(2, 'inc')}
          >+</button>
          <span class="trim-label">T3</span>
          <button class="trim-btn"
            onmousedown={() => handleTrimDown(2, 'dec')}
            onmouseup={() => handleTrimUp(2, 'dec')}
            ontouchstart={() => handleTrimDown(2, 'dec')}
            ontouchend={() => handleTrimUp(2, 'dec')}
          >-</button>
        </div>
      {/if}
      <div class="gimbal-center">
        <div class="gimbal-label">Left Stick</div>
        <div
          class="gimbal"
          bind:this={leftGimbalEl}
          onmousedown={(e) => handleGimbalDown('left', e)}
          ontouchstart={(e) => handleGimbalTouchStart('left', e)}
          role="slider"
          aria-label="Left gimbal"
          tabindex="0"
          aria-valuemin={-1024}
          aria-valuemax={1024}
          aria-valuenow={analogValues[getStickIndices().lh] ?? 0}
        >
          <div class="gimbal-crosshair-h" style:top="{gimbalCrossY('left')}%"></div>
          <div class="gimbal-crosshair-v" style:left="{gimbalCrossX('left')}%"></div>
          <div
            class="gimbal-dot"
            style:left="{gimbalCrossX('left')}%"
            style:top="{gimbalCrossY('left')}%"
          ></div>
        </div>
        {#if (currentRadio?.trims?.length ?? 0) >= 4}
          <div class="trim-row">
            <button class="trim-btn"
              onmousedown={() => handleTrimDown(3, 'dec')}
              onmouseup={() => handleTrimUp(3, 'dec')}
              ontouchstart={() => handleTrimDown(3, 'dec')}
              ontouchend={() => handleTrimUp(3, 'dec')}
            >-</button>
            <span class="trim-label">T4</span>
            <button class="trim-btn"
              onmousedown={() => handleTrimDown(3, 'inc')}
              onmouseup={() => handleTrimUp(3, 'inc')}
              ontouchstart={() => handleTrimDown(3, 'inc')}
              ontouchend={() => handleTrimUp(3, 'inc')}
            >+</button>
          </div>
        {/if}
      </div>
    </div>
  {/snippet}

  {#snippet rightGimbalBlock()}
    <div class="gimbal-wrapper gimbal-right">
      <div class="gimbal-center">
        <div class="gimbal-label">Right Stick</div>
        <div
          class="gimbal"
          bind:this={rightGimbalEl}
          onmousedown={(e) => handleGimbalDown('right', e)}
          ontouchstart={(e) => handleGimbalTouchStart('right', e)}
          role="slider"
          aria-label="Right gimbal"
          tabindex="0"
          aria-valuemin={-1024}
          aria-valuemax={1024}
          aria-valuenow={analogValues[getStickIndices().rv] ?? 0}
        >
          <div class="gimbal-crosshair-h" style:top="{gimbalCrossY('right')}%"></div>
          <div class="gimbal-crosshair-v" style:left="{gimbalCrossX('right')}%"></div>
          <div
            class="gimbal-dot"
            style:left="{gimbalCrossX('right')}%"
            style:top="{gimbalCrossY('right')}%"
          ></div>
        </div>
        {#if (currentRadio?.trims?.length ?? 0) >= 2}
          <div class="trim-row">
            <button class="trim-btn"
              onmousedown={() => handleTrimDown(0, 'dec')}
              onmouseup={() => handleTrimUp(0, 'dec')}
              ontouchstart={() => handleTrimDown(0, 'dec')}
              ontouchend={() => handleTrimUp(0, 'dec')}
            >-</button>
            <span class="trim-label">T1</span>
            <button class="trim-btn"
              onmousedown={() => handleTrimDown(0, 'inc')}
              onmouseup={() => handleTrimUp(0, 'inc')}
              ontouchstart={() => handleTrimDown(0, 'inc')}
              ontouchend={() => handleTrimUp(0, 'inc')}
            >+</button>
          </div>
        {/if}
      </div>
      {#if (currentRadio?.trims?.length ?? 0) >= 2}
        <div class="trim-col">
          <button class="trim-btn"
            onmousedown={() => handleTrimDown(1, 'inc')}
            onmouseup={() => handleTrimUp(1, 'inc')}
            ontouchstart={() => handleTrimDown(1, 'inc')}
            ontouchend={() => handleTrimUp(1, 'inc')}
          >+</button>
          <span class="trim-label">T2</span>
          <button class="trim-btn"
            onmousedown={() => handleTrimDown(1, 'dec')}
            onmouseup={() => handleTrimUp(1, 'dec')}
            ontouchstart={() => handleTrimDown(1, 'dec')}
            ontouchend={() => handleTrimUp(1, 'dec')}
          >-</button>
        </div>
      {/if}
    </div>
  {/snippet}

  {#snippet flexCtrl(input: InputDef, index: number)}
    {#if input.default === 'MULTIPOS'}
      <div class="multipos-control">
        <span class="control-label">{input.label}</span>
        <div class="multipos-buttons">
          {#each [0, 1, 2, 3, 4, 5] as pos}
            <button
              class="multipos-btn"
              class:active={Math.round(analogValues[index] * 5 / 4096) === pos}
              onclick={() => handleMultiposChange(index, pos)}
            >{pos + 1}</button>
          {/each}
        </div>
        <span class="pot-value">&nbsp;</span>
      </div>
    {:else}
      {@render potCtrl(input, index)}
    {/if}
  {/snippet}

  {#snippet potsRow()}
    {#if getPotsAndMultipos().length > 0}
      <div class="pots-row">
        {#if getCustomSwitches().length > 0}
          <!-- Radios with custom switches: left pots | custom switches | right pots -->
          {#each getLeftPotsAndMultipos() as { input, index }}
            {@render flexCtrl(input, index)}
          {/each}
          {#each getCustomSwitches() as { sw, index: swIdx }, csIdx}
            <div class="custom-switch">
              <button
                class="custom-switch-btn"
                class:active={switchStates[swIdx] === 1}
                aria-label={sw.name}
                style:background={customSwitchColors[csIdx] || ''}
                style:border-color={customSwitchColors[csIdx] || ''}
                style:box-shadow={customSwitchColors[csIdx] ? `0 0 8px ${customSwitchColors[csIdx]}` : ''}
                onmousedown={() => updateSwitch(swIdx, 1)}
                onmouseup={() => updateSwitch(swIdx, -1)}
                onmouseleave={() => { if (switchStates[swIdx] === 1) updateSwitch(swIdx, -1); }}
                ontouchstart={(e) => { e.preventDefault(); updateSwitch(swIdx, 1); }}
                ontouchend={(e) => { e.preventDefault(); updateSwitch(swIdx, -1); }}
              ></button>
              <span class="custom-switch-label">{sw.name}</span>
            </div>
          {/each}
          {#each getRightPotsAndMultipos() as { input, index }}
            {@render flexCtrl(input, index)}
          {/each}
        {:else}
          <!-- No custom switches: render pots+multipos in JSON order -->
          {#each getPotsAndMultipos() as { input, index }}
            {@render flexCtrl(input, index)}
          {/each}
        {/if}
      </div>
    {/if}
  {/snippet}

  {#snippet customSwitchesRow()}
    {#if getCustomSwitches().length > 0}
      <div class="custom-switches-row">
        {#each getCustomSwitches() as { sw, index }, csIdx}
          <div class="custom-switch">
            <button
              class="custom-switch-btn"
              class:active={switchStates[index] === 1}
              aria-label={sw.name}
              style:background={customSwitchColors[csIdx] || ''}
              style:border-color={customSwitchColors[csIdx] || ''}
              style:box-shadow={customSwitchColors[csIdx] ? `0 0 8px ${customSwitchColors[csIdx]}` : ''}
              onmousedown={() => updateSwitch(index, 1)}
              onmouseup={() => updateSwitch(index, -1)}
              onmouseleave={() => { if (switchStates[index] === 1) updateSwitch(index, -1); }}
              ontouchstart={(e) => { e.preventDefault(); updateSwitch(index, 1); }}
              ontouchend={(e) => { e.preventDefault(); updateSwitch(index, -1); }}
            ></button>
            <span class="custom-switch-label">{sw.name}</span>
          </div>
        {/each}
      </div>
    {/if}
  {/snippet}

  {#snippet potCtrl(input: InputDef, index: number)}
    <div class="pot-control">
      <span class="control-label">{input.label}</span>
      <input type="range" min="0" max="4096"
        value={analogValues[index]}
        oninput={(e) => handlePotChange(index, e)}
        class="pot-knob" />
      <span class="pot-value">{analogValues[index]}</span>
    </div>
  {/snippet}

  {#snippet keyRow()}
    {#if currentRadio?.keys?.length}
      <div class="key-row">
        {#each getLeftKeys() as keyDef}
          <button class="hw-key"
            onmousedown={() => keyDown(keyDef.key)}
            onmouseup={() => keyUp(keyDef.key)}
            onmouseleave={() => keyUp(keyDef.key)}
            ontouchstart={(e) => { e.preventDefault(); keyDown(keyDef.key); }}
            ontouchend={(e) => { e.preventDefault(); keyUp(keyDef.key); }}
          >{keyDef.label}</button>
        {/each}
        {#each getRightKeys() as keyDef}
          <button class="hw-key"
            onmousedown={() => keyDown(keyDef.key)}
            onmouseup={() => keyUp(keyDef.key)}
            onmouseleave={() => keyUp(keyDef.key)}
            ontouchstart={(e) => { e.preventDefault(); keyDown(keyDef.key); }}
            ontouchend={(e) => { e.preventDefault(); keyUp(keyDef.key); }}
          >{keyDef.label}</button>
        {/each}
      </div>
    {/if}
  {/snippet}

  {#snippet extraTrims()}
    {#if (currentRadio?.trims?.length ?? 0) > 4}
      <div class="extra-trims">
        {#each (currentRadio?.trims ?? []).slice(4) as trim, i}
          <div class="trim-row">
            <button class="trim-btn"
              onmousedown={() => handleTrimDown(4 + i, 'dec')}
              onmouseup={() => handleTrimUp(4 + i, 'dec')}
              ontouchstart={() => handleTrimDown(4 + i, 'dec')}
              ontouchend={() => handleTrimUp(4 + i, 'dec')}
            >-</button>
            <span class="trim-label">{trim.name}</span>
            <button class="trim-btn"
              onmousedown={() => handleTrimDown(4 + i, 'inc')}
              onmouseup={() => handleTrimUp(4 + i, 'inc')}
              ontouchstart={() => handleTrimDown(4 + i, 'inc')}
              ontouchend={() => handleTrimUp(4 + i, 'inc')}
            >+</button>
          </div>
        {/each}
      </div>
    {/if}
  {/snippet}

  <div class="radio-body" class:portrait={isPortrait}>
    {#if isPortrait && currentRadio}
      <!-- Portrait layout: left/right columns flanking LCD, keys below -->
      <div class="portrait-main">
        <!-- Left column: switches, pots, slider, gimbal -->
        <div class="portrait-side">
          <div class="switch-column">
            {#each getLeftSwitches() as { sw, index }}
              {@render switchCtrl(sw, index)}
            {/each}
          </div>
          {#if getLeftPotsAndMultipos().length > 0}
            <div class="portrait-pots">
              {#each getLeftPotsAndMultipos() as { input, index }}
                {@render flexCtrl(input, index)}
              {/each}
            </div>
          {/if}
          {#if getSliders().length > 0}
            <div class="slider-column">
              {#each getSliders().slice(0, 1) as { input, index }}
                <div class="slider-control">
                  <span class="control-label">{input.label}</span>
                  <input type="range" min="0" max="4096"
                    value={analogValues[index]}
                    oninput={(e) => handlePotChange(index, e)}
                    orient="vertical" class="vertical-slider" />
                </div>
              {/each}
            </div>
          {/if}
          {@render leftGimbalBlock()}
        </div>

        <!-- Centre: LCD screen -->
        <div class="portrait-center">
          {@render lcdScreen()}
          {@render keyRow()}
        </div>

        <!-- Right column: switches, pots, slider, gimbal -->
        <div class="portrait-side">
          <div class="switch-column">
            {#each getRightSwitches() as { sw, index }}
              {@render switchCtrl(sw, index)}
            {/each}
          </div>
          {#if getRightPotsAndMultipos().length > 0}
            <div class="portrait-pots">
              {#each getRightPotsAndMultipos() as { input, index }}
                {@render flexCtrl(input, index)}
              {/each}
            </div>
          {/if}
          {#if getSliders().length > 1}
            <div class="slider-column">
              {#each getSliders().slice(1, 2) as { input, index }}
                <div class="slider-control">
                  <span class="control-label">{input.label}</span>
                  <input type="range" min="0" max="4096"
                    value={analogValues[index]}
                    oninput={(e) => handlePotChange(index, e)}
                    orient="vertical" class="vertical-slider" />
                </div>
              {/each}
            </div>
          {/if}
          {@render rightGimbalBlock()}
        </div>
      </div>

      {#if getPotsAndMultipos().length === 0}
        {@render customSwitchesRow()}
      {/if}
      {@render extraTrims()}

    {:else}
      <!-- Landscape layout: LCD on top, controls below -->
      <div class="lcd-area">
        {#if currentRadio?.keys?.length}
          <div class="key-column">
            {#each getLeftKeys() as keyDef}
              <button class="hw-key"
                onmousedown={() => keyDown(keyDef.key)}
                onmouseup={() => keyUp(keyDef.key)}
                onmouseleave={() => keyUp(keyDef.key)}
                ontouchstart={(e) => { e.preventDefault(); keyDown(keyDef.key); }}
                ontouchend={(e) => { e.preventDefault(); keyUp(keyDef.key); }}
              >{keyDef.label}</button>
            {/each}
          </div>
        {/if}
        {@render lcdScreen()}
        {#if currentRadio?.keys?.length}
          <div class="key-column">
            {#each getRightKeys() as keyDef}
              <button class="hw-key"
                onmousedown={() => keyDown(keyDef.key)}
                onmouseup={() => keyUp(keyDef.key)}
                onmouseleave={() => keyUp(keyDef.key)}
                ontouchstart={(e) => { e.preventDefault(); keyDown(keyDef.key); }}
                ontouchend={(e) => { e.preventDefault(); keyUp(keyDef.key); }}
              >{keyDef.label}</button>
            {/each}
          </div>
        {/if}
      </div>

      {#if currentRadio}
        {@render potsRow()}
        {#if getPotsAndMultipos().length === 0}
          {@render customSwitchesRow()}
        {/if}

        <div class="controls-area">
          <div class="switch-column">
            {#each getLeftSwitches() as { sw, index }}
              {@render switchCtrl(sw, index)}
            {/each}
          </div>

          {#if getSliders().length > 0}
            <div class="slider-column">
              {#each getSliders().slice(0, 1) as { input, index }}
                <div class="slider-control">
                  <span class="control-label">{input.label}</span>
                  <input type="range" min="0" max="4096"
                    value={analogValues[index]}
                    oninput={(e) => handlePotChange(index, e)}
                    orient="vertical" class="vertical-slider" />
                </div>
              {/each}
            </div>
          {/if}

          {@render leftGimbalBlock()}
          {@render rightGimbalBlock()}

          {#if getSliders().length > 1}
            <div class="slider-column">
              {#each getSliders().slice(1, 2) as { input, index }}
                <div class="slider-control">
                  <span class="control-label">{input.label}</span>
                  <input type="range" min="0" max="4096"
                    value={analogValues[index]}
                    oninput={(e) => handlePotChange(index, e)}
                    orient="vertical" class="vertical-slider" />
                </div>
              {/each}
            </div>
          {/if}

          <div class="switch-column">
            {#each getRightSwitches() as { sw, index }}
              {@render switchCtrl(sw, index)}
            {/each}
          </div>
        </div>

        {@render extraTrims()}
      {/if}
    {/if}
  </div>

  <details>
    <summary class="trace-summary">Trace output{#if running}<span class="fps">{fps} fps</span>{/if}</summary>
    <pre class="trace">{traceLog}</pre>
  </details>
</main>

<style>
  /* ---- Theme tokens ---- */

  /* Light mode: default */
  :global(:root) {
    --bg-page: #f0f0f0;
    --bg-body: #e0e0e0;
    --bg-surface: #d5d5d5;
    --bg-element: #ccc;
    --bg-hover: #bbb;
    --bg-active: #aaa;
    --border: #bbb;
    --border-light: #aaa;
    --border-body: #ccc;
    --text: #1a1a1a;
    --text-muted: #555;
    --text-label: #444;
    --text-dim: #666;
    --text-bright: #333;
    --accent: #0e96df;
    --accent-dark: #0a6fb0;
    --accent-glow: rgba(14, 150, 223, 0.3);
    --bezel-top: #c0c0c0;
    --bezel-mid: #d0d0d0;
    --bezel-bot: #c8c8c8;
    --bezel-shadow: rgba(0, 0, 0, 0.3);
    --bezel-highlight: rgba(255, 255, 255, 0.4);
    --gimbal-inner: #d5d5d5;
    --gimbal-outer: #c0c0c0;
    --shadow: rgba(0, 0, 0, 0.15);
    --shadow-strong: rgba(0, 0, 0, 0.2);
    --btn-secondary-bg: #c5d8c5;
    --btn-secondary-border: #8ab08a;
    --btn-danger-bg: #e0c5c5;
    --btn-danger-border: #b08a8a;
    --btn-danger-hover: #d5b0b0;
    --trace-bg: #fff;
    --trace-text: #060;
    --logo-invert: 1;
  }

  /* Dark mode: forced via attribute or auto via OS preference */
  :global([data-theme="dark"]),
  :global(:root:not([data-theme])) {
    --bg-page: #1a1a1a;
    --bg-body: #222;
    --bg-surface: #2a2a2a;
    --bg-element: #333;
    --bg-hover: #444;
    --bg-active: #555;
    --border: #444;
    --border-light: #555;
    --border-body: #333;
    --text: #e0e0e0;
    --text-muted: #888;
    --text-label: #aaa;
    --text-dim: #666;
    --text-bright: #ccc;
    --accent: #38bff9;
    --accent-dark: #0e96df;
    --accent-glow: rgba(56, 191, 249, 0.25);
    --bezel-top: #1a1a1a;
    --bezel-mid: #2a2a2a;
    --bezel-bot: #222;
    --bezel-shadow: rgba(0, 0, 0, 0.8);
    --bezel-highlight: rgba(255, 255, 255, 0.05);
    --gimbal-inner: #2a2a2a;
    --gimbal-outer: #1a1a1a;
    --shadow: rgba(0, 0, 0, 0.5);
    --shadow-strong: rgba(0, 0, 0, 0.4);
    --btn-secondary-bg: #2a3a2a;
    --btn-secondary-border: #4a6a4a;
    --btn-danger-bg: #3a2a2a;
    --btn-danger-border: #6a4a4a;
    --btn-danger-hover: #4a2a2a;
    --trace-bg: #111;
    --trace-text: #0f0;
    --logo-invert: 0;
  }

  /* Auto mode: respect OS light preference */
  @media (prefers-color-scheme: light) {
    :global(:root:not([data-theme])) {
      --bg-page: #f0f0f0;
      --bg-body: #e0e0e0;
      --bg-surface: #d5d5d5;
      --bg-element: #ccc;
      --bg-hover: #bbb;
      --bg-active: #aaa;
      --border: #bbb;
      --border-light: #aaa;
      --border-body: #ccc;
      --text: #1a1a1a;
      --text-muted: #555;
      --text-label: #444;
      --text-dim: #666;
      --text-bright: #333;
      --accent: #0e96df;
      --accent-dark: #0a6fb0;
      --accent-glow: rgba(14, 150, 223, 0.3);
      --bezel-top: #c0c0c0;
      --bezel-mid: #d0d0d0;
      --bezel-bot: #c8c8c8;
      --bezel-shadow: rgba(0, 0, 0, 0.3);
      --bezel-highlight: rgba(255, 255, 255, 0.4);
      --gimbal-inner: #d5d5d5;
      --gimbal-outer: #c0c0c0;
      --shadow: rgba(0, 0, 0, 0.15);
      --shadow-strong: rgba(0, 0, 0, 0.2);
      --btn-secondary-bg: #c5d8c5;
      --btn-secondary-border: #8ab08a;
      --btn-danger-bg: #e0c5c5;
      --btn-danger-border: #b08a8a;
      --btn-danger-hover: #d5b0b0;
      --trace-bg: #fff;
      --trace-text: #060;
      --logo-invert: 1;
    }
  }

  :global(body) {
    margin: 0;
    background: var(--bg-page);
    color: var(--text);
    transition: background 0.2s, color 0.2s;
  }

  main {
    max-width: 820px;
    margin: 0 auto;
    padding: 0.75rem;
    font-family: 'Segoe UI', system-ui, -apple-system, sans-serif;
  }

  .header {
    text-align: center;
    margin-bottom: 0.5rem;
  }

  .logo-title {
    display: flex;
    align-items: center;
    justify-content: center;
    gap: 0.5rem;
    margin-bottom: 0.5rem;
  }

  .logo {
    height: 1.6rem;
    filter: invert(var(--logo-invert));
    transition: filter 0.2s;
  }

  .subtitle {
    font-size: 1.2rem;
    color: var(--text-muted);
    font-weight: 300;
    letter-spacing: 0.05em;
    text-transform: uppercase;
  }

  .toolbar {
    display: flex;
    gap: 0.5rem;
    align-items: center;
    justify-content: center;
    margin: 0.5rem 0;
    flex-wrap: wrap;
  }

  .theme-toggle {
    font-size: 0.75rem;
    padding: 0.25rem 0.5rem;
  }

  select {
    padding: 0.35rem 0.5rem;
    font-size: 0.9rem;
    background: var(--bg-surface);
    color: var(--text);
    border: 1px solid var(--border);
    border-radius: 4px;
  }

  .status {
    color: var(--text-muted);
    margin: 0.25rem 0;
    font-size: 0.85rem;
  }

  button {
    padding: 0.35rem 1rem;
    font-size: 0.9rem;
    cursor: pointer;
    background: var(--bg-element);
    color: var(--text);
    border: 1px solid var(--border-light);
    border-radius: 4px;
    transition: background 0.15s;
  }

  button:hover {
    background: var(--bg-hover);
  }

  button:active {
    background: var(--bg-active);
  }

  button.secondary {
    background: var(--btn-secondary-bg);
    border-color: var(--btn-secondary-border);
  }

  button.danger {
    background: var(--btn-danger-bg);
    border-color: var(--btn-danger-border);
  }

  button.danger:hover {
    background: var(--btn-danger-hover);
  }

  /* Radio body */
  .radio-body {
    background: var(--bg-body);
    border-radius: 12px;
    padding: 1rem;
    border: 2px solid var(--border-body);
    box-shadow: 0 4px 20px var(--shadow);
    transition: background 0.2s, border-color 0.2s;
  }

  /* LCD */
  .lcd-area {
    display: flex;
    justify-content: center;
    align-items: center;
    gap: 0.5rem;
    margin-bottom: 0.75rem;
  }

  .lcd-bezel {
    display: inline-block;
    padding: 6px;
    border-radius: 10px;
    background: linear-gradient(180deg, var(--bezel-top) 0%, var(--bezel-mid) 40%, var(--bezel-bot) 100%);
    box-shadow:
      inset 0 2px 4px var(--bezel-shadow),
      inset 0 -1px 2px var(--bezel-highlight),
      0 1px 3px var(--shadow);
  }

  .lcd {
    display: block;
    background: #000;
    image-rendering: pixelated;
    cursor: crosshair;
    border-radius: 4px;
    border: none;
    max-width: 100%;
  }

  .key-column {
    display: flex;
    flex-direction: column;
    gap: 0.4rem;
  }

  .hw-key {
    padding: 0.3rem 0.6rem;
    font-size: 0.7rem;
    font-weight: bold;
    background: var(--bg-surface);
    color: var(--text-bright);
    border: 1px solid var(--border);
    border-radius: 4px;
    cursor: pointer;
    min-width: 3.5rem;
    user-select: none;
    -webkit-user-select: none;
  }

  .hw-key:active {
    background: var(--bg-active);
    border-color: var(--text-muted);
  }

  /* Pots row */
  .pots-row {
    display: flex;
    justify-content: center;
    align-items: flex-end;
    gap: 1.5rem;
    margin-bottom: 0.75rem;
    padding: 0.5rem;
    flex-wrap: wrap;
  }

  .pot-control {
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 0.25rem;
  }

  .pot-control .control-label {
    font-size: 0.75rem;
    color: var(--text-label);
    text-transform: uppercase;
    letter-spacing: 0.05em;
  }

  .pot-knob {
    width: 80px;
    accent-color: var(--accent);
  }

  .pot-value {
    font-size: 0.65rem;
    color: var(--text-dim);
    font-family: monospace;
  }

  .multipos-control {
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 0.25rem;
  }

  .multipos-control .control-label {
    font-size: 0.75rem;
    color: var(--text-label);
    text-transform: uppercase;
  }

  .multipos-buttons {
    display: flex;
    gap: 2px;
  }

  .multipos-btn {
    width: 22px;
    height: 22px;
    padding: 0;
    font-size: 0.65rem;
    border-radius: 3px;
    background: var(--bg-element);
    border: 1px solid var(--border-light);
  }

  .multipos-btn.active {
    background: var(--accent);
    color: #fff;
    border-color: var(--accent);
  }

  /* Custom switches (push buttons with LED) */
  .custom-switches-row {
    display: flex;
    justify-content: center;
    gap: 0.75rem;
    margin-bottom: 0.75rem;
  }

  .custom-switch {
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 0.25rem;
  }

  .custom-switch-btn {
    width: 36px;
    height: 22px;
    border-radius: 6px;
    border: 3px solid var(--border);
    background: var(--bg-element);
    cursor: pointer;
    box-shadow: 0 2px 4px var(--shadow), inset 0 1px 0 rgba(255, 255, 255, 0.08);
    transition: all 0.1s;
    user-select: none;
    -webkit-user-select: none;
  }

  .custom-switch-btn:active,
  .custom-switch-btn.active {
    background: var(--accent);
    border-color: var(--accent);
    box-shadow: 0 0 8px rgba(56, 191, 249, 0.5), inset 0 1px 3px rgba(0, 0, 0, 0.2);
    transform: scale(0.95);
  }

  .custom-switch-label {
    font-size: 0.65rem;
    color: var(--text-dim);
    font-family: monospace;
  }

  /* Controls area */
  .controls-area {
    display: flex;
    align-items: flex-start;
    justify-content: center;
    gap: 0.5rem;
    margin-bottom: 0.5rem;
  }

  /* Portrait layout (tall-screen radios like EL18, NB4+, NV14) */
  .portrait .portrait-main {
    display: flex;
    justify-content: center;
    align-items: flex-start;
    gap: 0.75rem;
    margin-bottom: 0.75rem;
  }

  .portrait .portrait-side {
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 0.75rem;
    min-width: 52px;
  }

  .portrait .portrait-center {
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 0.5rem;
  }

  .portrait .portrait-pots {
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 0.5rem;
  }

  .portrait .key-row {
    display: flex;
    justify-content: center;
    gap: 0.4rem;
    flex-wrap: wrap;
  }

  /* Switch column */
  .switch-column {
    display: grid;
    grid-template-columns: repeat(auto-fill, minmax(44px, 1fr));
    max-width: calc(3 * 52px);
    gap: 0.4rem;
    min-width: 52px;
    padding-top: 0.5rem;
  }

  .switch-control {
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 0.15rem;
  }

  .switch-name {
    font-size: 0.65rem;
    color: var(--text-label);
    text-transform: uppercase;
    letter-spacing: 0.05em;
  }

  .switch-track {
    width: 16px;
    height: 36px;
    background: var(--bg-surface);
    border: 1px solid var(--border);
    border-radius: 8px;
    position: relative;
    cursor: pointer;
    box-shadow: inset 0 1px 3px rgba(0, 0, 0, 0.3);
  }

  .switch-track.is-3pos {
    height: 44px;
  }

  .switch-notch {
    position: absolute;
    left: -3px;
    right: -3px;
    height: 1px;
    background: var(--border);
    pointer-events: none;
  }

  .switch-knob {
    position: absolute;
    left: -2px;
    right: -2px;
    height: 14px;
    border-radius: 4px;
    transition: top 0.1s, background 0.1s;
    cursor: grab;
    border: 1px solid var(--border-light);
  }

  .switch-knob:active {
    cursor: grabbing;
  }

  .switch-knob.sw-up {
    background: linear-gradient(180deg, #38bff9 0%, #0e96df 100%);
    border-color: #38bff9;
  }

  .switch-knob.sw-mid {
    background: linear-gradient(180deg, #ff9800 0%, #e65100 100%);
    border-color: #ff9800;
  }

  .switch-knob.sw-down {
    background: linear-gradient(180deg, #f44336 0%, #c62828 100%);
    border-color: #f44336;
  }

  /* Slider column */
  .slider-column {
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 0.25rem;
    padding-top: 0.5rem;
  }

  .slider-control {
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 0.25rem;
  }

  .slider-control .control-label {
    font-size: 0.7rem;
    color: var(--text-label);
    text-transform: uppercase;
  }

  .vertical-slider {
    writing-mode: vertical-lr;
    direction: rtl;
    height: 120px;
    width: 20px;
    accent-color: var(--accent);
  }

  /* Gimbal */
  .gimbal-wrapper {
    display: flex;
    align-items: center;
    gap: 0.25rem;
  }

  .gimbal-label {
    font-size: 0.7rem;
    color: var(--text-dim);
    margin-bottom: 0.25rem;
    text-transform: uppercase;
    letter-spacing: 0.05em;
    text-align: center;
  }

  .gimbal-center {
    display: flex;
    flex-direction: column;
    align-items: center;
  }

  .gimbal {
    width: 150px;
    height: 150px;
    background: radial-gradient(circle at center, var(--gimbal-inner) 0%, var(--gimbal-outer) 100%);
    border: 2px solid var(--border);
    border-radius: 12px;
    position: relative;
    cursor: crosshair;
    touch-action: none;
    user-select: none;
    box-shadow: inset 0 2px 8px var(--shadow);
  }

  .gimbal-crosshair-h {
    position: absolute;
    left: 0;
    right: 0;
    height: 1px;
    background: var(--accent-glow);
    pointer-events: none;
  }

  .gimbal-crosshair-v {
    position: absolute;
    top: 0;
    bottom: 0;
    width: 1px;
    background: var(--accent-glow);
    pointer-events: none;
  }

  .gimbal-dot {
    position: absolute;
    width: 18px;
    height: 18px;
    border-radius: 50%;
    background: radial-gradient(circle at 40% 40%, #5bcfff 0%, #38bff9 60%, #3b56c8 100%);
    border: 2px solid #81c784;
    transform: translate(-50%, -50%);
    pointer-events: none;
    box-shadow: 0 2px 6px var(--shadow-strong);
  }

  /* Trims */
  .trim-row {
    display: flex;
    align-items: center;
    gap: 0.25rem;
    margin-top: 0.3rem;
  }

  .trim-label {
    font-size: 0.6rem;
    color: var(--text-dim);
    min-width: 18px;
    text-align: center;
  }

  .trim-btn {
    width: 22px;
    height: 22px;
    padding: 0;
    font-size: 0.6rem;
    border-radius: 3px;
    background: var(--bg-surface);
    border: 1px solid var(--border);
    color: var(--text-bright);
    font-family: monospace;
  }

  .trim-btn:active {
    background: var(--accent);
    color: #000;
  }

  .trim-col {
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 0.2rem;
  }

  /* Extra trims */
  .extra-trims {
    display: flex;
    justify-content: center;
    gap: 1rem;
    margin-top: 0.5rem;
    flex-wrap: wrap;
  }

  /* Trace */
  details {
    margin-top: 0.75rem;
  }

  summary {
    cursor: pointer;
    color: var(--text-muted);
    font-size: 0.85rem;
    padding: 0.25rem 0;
  }

  .trace-summary {
    display: flex;
    justify-content: space-between;
    align-items: center;
  }

  .fps {
    font-size: 0.8rem;
    color: var(--text-muted);
    font-weight: normal;
  }

  .trace {
    max-height: 300px;
    overflow-y: auto;
    background: var(--trace-bg);
    color: var(--trace-text);
    padding: 0.5rem;
    font-size: 0.75rem;
    white-space: pre-wrap;
    word-break: break-all;
    border-radius: 4px;
    border: 1px solid var(--border-body);
  }
</style>
