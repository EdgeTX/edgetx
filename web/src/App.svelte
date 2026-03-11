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

  interface RadioEntry {
    name: string;
    wasm: string;
    available?: boolean;
    inputs?: InputDef[];
    switches?: SwitchDef[];
    trims?: TrimDef[];
    keys?: KeyDef[];
  }

  let canvas: HTMLCanvasElement;
  let status = $state('Loading radio list...');
  let traceLog = $state('');
  let running = $state(false);
  let loaded = $state(false);
  let loading = $state(false);

  let radios = $state<RadioEntry[]>([]);
  let selectedRadio = $state('');
  let currentRadio = $state<RadioEntry | null>(null);

  let runner: WasmRunner | null = null;
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
  let lcdWidth = $state(480);
  let lcdHeight = $state(272);
  let lcdDepth = $state(0);
  let lcdSize = 0;

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

  function getPots(): { input: InputDef; index: number }[] {
    return getFlexInputs().filter(({ input }) =>
      input.default === 'POT' || input.default === 'POT_CENTER'
    );
  }

  function getSliders(): { input: InputDef; index: number }[] {
    return getFlexInputs().filter(({ input }) => input.default === 'SLIDER');
  }

  function getMultipos(): { input: InputDef; index: number }[] {
    return getFlexInputs().filter(({ input }) => input.default === 'MULTIPOS');
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
        selectedRadio = avail[0].wasm;
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

  async function loadSelected() {
    if (!selectedRadio || loading) return;

    // Tear down previous instance
    await teardown();

    try {
      loading = true;
      loaded = false;
      traceLog = '';
      const radio = radios.find((r) => r.wasm === selectedRadio);
      currentRadio = radio ?? null;
      status = `Loading ${radio?.name ?? selectedRadio}...`;

      const radioKey = radioKeyFromWasm(selectedRadio);
      runner = new WasmRunner(onTrace, onAudio);
      const { hasContent } = await runner.load(`./${selectedRadio}`, radioKey);

      if (hasContent) {
        const files = await runner.fsListFiles('/');
        onTrace(`[fs] ${files.length} file(s) in OPFS for "${radioKey}"\n`);
        await readStickMode();
        status = `Loading ${radio?.name ?? selectedRadio} (restored SD card)...`;
      } else {
        onTrace(`[fs] no previous data for "${radioKey}"\n`);
      }

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

      // Prime the SharedArrayBuffer with initial analog values
      for (let i = 0; i < analogValues.length; i++) {
        runner.setAnalog(i, analogValues[i]);
      }

      loaded = true;
      loading = false;
      status = `${radio?.name}: LCD ${lcdWidth}\u00d7${lcdHeight} ${lcdDepth}bpp`;
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

  async function startSimulator() {
    if (!runner?.exports) return;

    if (!audioCtx) {
      audioCtx = new AudioContext({ sampleRate: AUDIO_SAMPLE_RATE });
    }
    if (audioCtx.state === 'suspended') {
      await audioCtx.resume();
    }
    audioNextTime = 0; // reset scheduling cursor

    const ex = runner.exports;

    // Push all analog values to WASM before starting
    for (let i = 0; i < analogValues.length; i++) {
      runner.setAnalog(i, analogValues[i]);
    }

    // Push all switch states to WASM before starting
    for (let i = 0; i < switchStates.length; i++) {
      ex.simuSetSwitch(i, switchStates[i]);
    }

    ex.simuInit();
    runner.setFatfsPaths('/', '/');
    ex.simuStart(1);
    running = true;
    status = 'Running';

    if (canvas) {
      lcdRenderer = new LcdRenderer(canvas);
      // Set canvas resolution to match CSS display size (× devicePixelRatio for retina)
      const cssWidth = Math.max(lcdWidth, 150 * lcdWidth / lcdHeight, 320);
      const dpr = window.devicePixelRatio || 1;
      const canvasW = Math.round(cssWidth * dpr);
      const canvasH = Math.round(canvasW * lcdHeight / lcdWidth);
      lcdRenderer.resize(canvasW, canvasH);
    }

    lcdLoop().catch((e) => console.error('lcdLoop error:', e));
  }

  async function teardown() {
    lcdLoopActive = false;
    if (runner?.exports && running) {
      try { runner.exports.simuStop(); } catch { /* expected */ }
    }
    if (runner) {
      try { await runner.stopFs(); } catch { /* best effort */ }
    }
    runner = null;
    lcdRenderer = null;
    running = false;
    loaded = false;
    currentRadio = null;
  }

  async function stopSimulator() {
    lcdLoopActive = false;
    if (runner?.exports) {
      try {
        runner.exports.simuStop();
      } catch {
        // simuStop() sets the shutdown flag and signals threads to stop,
        // but traps when trying to join (Atomics.wait on main thread).
        // Worker threads continue their shutdown asynchronously.
      }
    }
    running = false;
    status = 'Waiting for shutdown...';

    // Wait for worker threads to finish their shutdown tasks (writing
    // settings, closing files, etc.).  OPFS writes are durable immediately.
    await waitForShutdown();

    // Stop the FS Worker (closes all handles, flushes)
    if (runner) {
      await runner.stopFs();
    }
    onTrace('[fs] shutdown complete\n');

    // Tear down the old WASM instance and reload so Start works again.
    // Chrome caches compiled WASM, so subsequent loads are near-instant.
    runner = null;
    lcdRenderer = null;
    loaded = false;
    await loadSelected();
    status = 'Stopped';
  }

  function waitForShutdown(): Promise<void> {
    // Give worker threads time to finish writing settings/files
    return new Promise((resolve) => setTimeout(resolve, 2000));
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

  function canvasToLcd(e: MouseEvent): { x: number; y: number } {
    const rect = canvas.getBoundingClientRect();
    const scaleX = lcdWidth / rect.width;
    const scaleY = lcdHeight / rect.height;
    return {
      x: Math.round((e.clientX - rect.left) * scaleX),
      y: Math.round((e.clientY - rect.top) * scaleY),
    };
  }

  function handleCanvasMouseDown(e: MouseEvent) {
    const ex = runner?.exports;
    if (!ex || !running || !canvas) return;

    const { x, y } = canvasToLcd(e);
    ex.simuTouchDown(x, y);

    const onMove = (ev: MouseEvent) => {
      const pos = canvasToLcd(ev);
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

  function handleKey(e: KeyboardEvent) {
    const ex = runner?.exports;
    if (!ex || !running) return;

    if (e.repeat) return;
    const down = e.type === 'keydown';

    switch (e.key) {
      case 'Escape':
        ex.simuSetKey(KEY_EXIT, down ? 1 : 0);
        break;
      case 'Enter':
        ex.simuSetKey(KEY_ENTER, down ? 1 : 0);
        break;
      case 'ArrowUp':
        if (down) ex.simuRotaryEncoderEvent(-ROTARY_ENCODER_GRANULARITY);
        break;
      case 'ArrowDown':
        if (down) ex.simuRotaryEncoderEvent(ROTARY_ENCODER_GRANULARITY);
        break;
      case 'ArrowLeft':
        ex.simuSetKey(KEY_LEFT, down ? 1 : 0);
        break;
      case 'ArrowRight':
        ex.simuSetKey(KEY_RIGHT, down ? 1 : 0);
        break;
      case 'PageUp':
        ex.simuSetKey(KEY_PAGEUP, down ? 1 : 0);
        break;
      case 'PageDown':
        ex.simuSetKey(KEY_PAGEDN, down ? 1 : 0);
        break;
      case '+':
        ex.simuSetKey(KEY_PLUS, down ? 1 : 0);
        break;
      case '-':
        ex.simuSetKey(KEY_MINUS, down ? 1 : 0);
        break;
      case 'm':
        ex.simuSetKey(KEY_MENU, down ? 1 : 0);
        break;
      case 's':
        ex.simuSetKey(KEY_SYS, down ? 1 : 0);
        break;
      case 't':
        ex.simuSetKey(KEY_TELE, down ? 1 : 0);
        break;
      default:
        return; // don't prevent default for unmapped keys
    }
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

  function handleSwitchDrag(index: number, sw: SwitchDef, e: MouseEvent) {
    e.preventDefault();
    const track = (e.currentTarget as HTMLElement).parentElement!;
    const snap = (ev: MouseEvent) => {
      const rect = track.getBoundingClientRect();
      const y = Math.max(0, Math.min(1, (ev.clientY - rect.top) / rect.height));
      if (sw.type === '2POS') {
        updateSwitch(index, y < 0.5 ? -1 : 1);
      } else {
        if (y < 0.33) updateSwitch(index, -1);
        else if (y < 0.67) updateSwitch(index, 0);
        else updateSwitch(index, 1);
      }
    };
    const up = () => {
      window.removeEventListener('mousemove', snap);
      window.removeEventListener('mouseup', up);
    };
    window.addEventListener('mousemove', snap);
    window.addEventListener('mouseup', up);
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

      {#if !loaded && !loading}
        <button onclick={loadSelected} disabled={!selectedRadio || loading}>
          Load
        </button>
      {/if}
      {#if loaded && !running}
        <button onclick={startSimulator}>Start</button>
      {/if}
      {#if running}
        <button onclick={stopSimulator}>Stop</button>
      {/if}
      {#if loaded && !running}
        <button onclick={handleFileUpload} class="secondary">Upload Files</button>
        <button onclick={handleFolderUpload} class="secondary">Upload Folder</button>
        <button onclick={resetSdCard} class="danger">Reset SD</button>
      {/if}
      <button class="theme-toggle" onclick={toggleTheme}
              aria-label="Toggle theme">{themeIcon[themeMode]}</button>
    </div>
    <p class="status">{status}</p>
  </div>

  <div class="radio-body">
    <!-- LCD Screen with keys on either side -->
    <div class="lcd-area">
      {#if currentRadio?.keys?.length}
        <div class="key-column">
          {#each getLeftKeys() as keyDef}
            <button
              class="hw-key"
              onmousedown={() => keyDown(keyDef.key)}
              onmouseup={() => keyUp(keyDef.key)}
              onmouseleave={() => keyUp(keyDef.key)}
              ontouchstart={(e) => { e.preventDefault(); keyDown(keyDef.key); }}
              ontouchend={(e) => { e.preventDefault(); keyUp(keyDef.key); }}
            >{keyDef.label}</button>
          {/each}
        </div>
      {/if}
      <div class="lcd-bezel">
        <canvas
          bind:this={canvas}
          class="lcd"
          style:width="{Math.max(lcdWidth, 150 * lcdWidth / lcdHeight, 320)}px"
          style:aspect-ratio="{lcdWidth} / {lcdHeight}"
          style:background="{lcdDepth > 0 && lcdDepth < 16 ? 'rgb(47, 123, 227)' : '#000'}"
          onmousedown={handleCanvasMouseDown}
          onwheel={handleWheel}
        ></canvas>
      </div>
      {#if currentRadio?.keys?.length}
        <div class="key-column">
          {#each getRightKeys() as keyDef}
            <button
              class="hw-key"
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

    {#if loaded && currentRadio}
      <!-- Pots row -->
      {#if getPots().length > 0 || getMultipos().length > 0}
        <div class="pots-row">
          {#each getPots() as { input, index }}
            <div class="pot-control">
              <span class="control-label">{input.label}</span>
              <input
                type="range"
                min="0"
                max="4096"
                value={analogValues[index]}
                oninput={(e) => handlePotChange(index, e)}
                class="pot-knob"
              />
              <span class="pot-value">{analogValues[index]}</span>
            </div>
          {/each}
          {#each getMultipos() as { input, index }}
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
            </div>
          {/each}
        </div>
      {/if}

      <!-- Custom switches (SW1-SW6 etc.) - momentary push buttons -->
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

      <!-- Main controls area: switches | gimbal | sliders | gimbal | switches -->
      <div class="controls-area">
        <!-- Left switches -->
        <div class="switch-column">
          {#each getLeftSwitches() as { sw, index }}
            <div class="switch-control">
              <span class="switch-name">{sw.name}</span>
              <div class="switch-track" class:is-3pos={sw.type === '3POS'}
                   onclick={(e) => handleSwitchTrackClick(index, sw, e)}>
                {#if sw.type === '3POS'}
                  <div class="switch-notch" style:top="7px"></div>
                  <div class="switch-notch" style:top="50%"></div>
                  <div class="switch-notch" style:top="calc(100% - 7px)"></div>
                {:else}
                  <div class="switch-notch" style:top="7px"></div>
                  <div class="switch-notch" style:top="calc(100% - 7px)"></div>
                {/if}
                <div class="switch-knob"
                     class:sw-up={switchStates[index] === -1}
                     class:sw-mid={switchStates[index] === 0}
                     class:sw-down={switchStates[index] === 1}
                     style:top={switchKnobTop(switchStates[index], sw.type === '3POS')}
                     onmousedown={(e) => handleSwitchDrag(index, sw, e)}
                ></div>
              </div>
            </div>
          {/each}
        </div>

        <!-- Left slider (if any) -->
        {#if getSliders().length > 0}
          <div class="slider-column">
            {#each getSliders().slice(0, 1) as { input, index }}
              <div class="slider-control">
                <span class="control-label">{input.label}</span>
                <input
                  type="range"
                  min="0"
                  max="4096"
                  value={analogValues[index]}
                  oninput={(e) => handlePotChange(index, e)}
                  orient="vertical"
                  class="vertical-slider"
                />
              </div>
            {/each}
          </div>
        {/if}

        <!-- Left gimbal: T3 vertical (left), T4 horizontal (below) -->
        <div class="gimbal-wrapper gimbal-left">
          {#if (currentRadio.trims?.length ?? 0) >= 4}
            <div class="trim-col">
              <button
                class="trim-btn"
                onmousedown={() => handleTrimDown(2, 'inc')}
                onmouseup={() => handleTrimUp(2, 'inc')}
                ontouchstart={() => handleTrimDown(2, 'inc')}
                ontouchend={() => handleTrimUp(2, 'inc')}
              >T3+</button>
              <span class="trim-label">T3</span>
              <button
                class="trim-btn"
                onmousedown={() => handleTrimDown(2, 'dec')}
                onmouseup={() => handleTrimUp(2, 'dec')}
                ontouchstart={() => handleTrimDown(2, 'dec')}
                ontouchend={() => handleTrimUp(2, 'dec')}
              >T3-</button>
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
            {#if (currentRadio.trims?.length ?? 0) >= 4}
              <div class="trim-row">
                <button
                  class="trim-btn"
                  onmousedown={() => handleTrimDown(3, 'dec')}
                  onmouseup={() => handleTrimUp(3, 'dec')}
                  ontouchstart={() => handleTrimDown(3, 'dec')}
                  ontouchend={() => handleTrimUp(3, 'dec')}
                >T4-</button>
                <span class="trim-label">T4</span>
                <button
                  class="trim-btn"
                  onmousedown={() => handleTrimDown(3, 'inc')}
                  onmouseup={() => handleTrimUp(3, 'inc')}
                  ontouchstart={() => handleTrimDown(3, 'inc')}
                  ontouchend={() => handleTrimUp(3, 'inc')}
                >T4+</button>
              </div>
            {/if}
          </div>
        </div>

        <!-- Right gimbal: T2 vertical (right), T1 horizontal (below) -->
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
            {#if (currentRadio.trims?.length ?? 0) >= 2}
              <div class="trim-row">
                <button
                  class="trim-btn"
                  onmousedown={() => handleTrimDown(0, 'dec')}
                  onmouseup={() => handleTrimUp(0, 'dec')}
                  ontouchstart={() => handleTrimDown(0, 'dec')}
                  ontouchend={() => handleTrimUp(0, 'dec')}
                >T1-</button>
                <span class="trim-label">T1</span>
                <button
                  class="trim-btn"
                  onmousedown={() => handleTrimDown(0, 'inc')}
                  onmouseup={() => handleTrimUp(0, 'inc')}
                  ontouchstart={() => handleTrimDown(0, 'inc')}
                  ontouchend={() => handleTrimUp(0, 'inc')}
                >T1+</button>
              </div>
            {/if}
          </div>
          {#if (currentRadio.trims?.length ?? 0) >= 2}
            <div class="trim-col">
              <button
                class="trim-btn"
                onmousedown={() => handleTrimDown(1, 'inc')}
                onmouseup={() => handleTrimUp(1, 'inc')}
                ontouchstart={() => handleTrimDown(1, 'inc')}
                ontouchend={() => handleTrimUp(1, 'inc')}
              >T2+</button>
              <span class="trim-label">T2</span>
              <button
                class="trim-btn"
                onmousedown={() => handleTrimDown(1, 'dec')}
                onmouseup={() => handleTrimUp(1, 'dec')}
                ontouchstart={() => handleTrimDown(1, 'dec')}
                ontouchend={() => handleTrimUp(1, 'dec')}
              >T2-</button>
            </div>
          {/if}
        </div>

        <!-- Right slider (if any) -->
        {#if getSliders().length > 1}
          <div class="slider-column">
            {#each getSliders().slice(1, 2) as { input, index }}
              <div class="slider-control">
                <span class="control-label">{input.label}</span>
                <input
                  type="range"
                  min="0"
                  max="4096"
                  value={analogValues[index]}
                  oninput={(e) => handlePotChange(index, e)}
                  orient="vertical"
                  class="vertical-slider"
                />
              </div>
            {/each}
          </div>
        {/if}

        <!-- Right switches -->
        <div class="switch-column">
          {#each getRightSwitches() as { sw, index }}
            <div class="switch-control">
              <span class="switch-name">{sw.name}</span>
              <div class="switch-track" class:is-3pos={sw.type === '3POS'}
                   onclick={(e) => handleSwitchTrackClick(index, sw, e)}>
                {#if sw.type === '3POS'}
                  <div class="switch-notch" style:top="7px"></div>
                  <div class="switch-notch" style:top="50%"></div>
                  <div class="switch-notch" style:top="calc(100% - 7px)"></div>
                {:else}
                  <div class="switch-notch" style:top="7px"></div>
                  <div class="switch-notch" style:top="calc(100% - 7px)"></div>
                {/if}
                <div class="switch-knob"
                     class:sw-up={switchStates[index] === -1}
                     class:sw-mid={switchStates[index] === 0}
                     class:sw-down={switchStates[index] === 1}
                     style:top={switchKnobTop(switchStates[index], sw.type === '3POS')}
                     onmousedown={(e) => handleSwitchDrag(index, sw, e)}
                ></div>
              </div>
            </div>
          {/each}
        </div>
      </div>

      <!-- Extra trims (T5, T6, etc.) if present -->
      {#if (currentRadio.trims?.length ?? 0) > 4}
        <div class="extra-trims">
          {#each (currentRadio.trims ?? []).slice(4) as trim, i}
            <div class="trim-row">
              <button
                class="trim-btn"
                onmousedown={() => handleTrimDown(4 + i, 'dec')}
                onmouseup={() => handleTrimUp(4 + i, 'dec')}
                ontouchstart={() => handleTrimDown(4 + i, 'dec')}
                ontouchend={() => handleTrimUp(4 + i, 'dec')}
              >{trim.name}-</button>
              <span class="trim-label">{trim.name}</span>
              <button
                class="trim-btn"
                onmousedown={() => handleTrimDown(4 + i, 'inc')}
                onmouseup={() => handleTrimUp(4 + i, 'inc')}
                ontouchstart={() => handleTrimDown(4 + i, 'inc')}
                ontouchend={() => handleTrimUp(4 + i, 'inc')}
              >{trim.name}+</button>
            </div>
          {/each}
        </div>
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
    text-transform: uppercase;
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
    height: 28px;
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
    font-size: 0.6rem;
    color: var(--text-muted);
    text-transform: uppercase;
    letter-spacing: 0.05em;
  }

  /* Controls area */
  .controls-area {
    display: flex;
    align-items: flex-start;
    justify-content: center;
    gap: 0.5rem;
    margin-bottom: 0.5rem;
  }

  /* Switch column */
  .switch-column {
    display: flex;
    flex-direction: column;
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
    width: 32px;
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
