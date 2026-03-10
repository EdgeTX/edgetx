<script lang="ts">
  import { onDestroy } from 'svelte';
  import { WasmRunner } from './lib/wasm-runner';
  import { PersistentFS } from './lib/persistent-fs';
  import { renderRgb565, render4bit, render1bit } from './lib/lcd-renderer';

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
  let persistentFs: PersistentFS | null = null;
  let pollTimer: number | null = null;
  let autoSaveTimer: number | null = null;
  let saving = $state(false);
  let lcdWidth = $state(480);
  let lcdHeight = $state(272);
  let lcdDepth = 0;
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

  function getVisibleSwitches(): { sw: SwitchDef; index: number }[] {
    const switches = currentRadio?.switches ?? [];
    return switches
      .map((sw, index) => ({ sw, index }))
      .filter(({ sw }) => sw.default !== 'NONE');
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

  /** Read stickMode from /RADIO/radio.yml in memfs. */
  function readStickMode(): void {
    if (!persistentFs) return;
    try {
      const data = persistentFs.fs.readFileSync('/RADIO/radio.yml', 'utf8') as string;
      const match = data.match(/^stickMode:\s*(\d+)/m);
      if (match) {
        stickMode = parseInt(match[1], 10);
        onTrace(`[persist] stickMode=${stickMode} (Mode ${stickMode + 1})\n`);
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

      // Probe each WASM file with a HEAD request
      const checks = manifest.map(async (r) => {
        try {
          const head = await fetch(`./${r.wasm}`, { method: 'HEAD' });
          return { ...r, available: head.ok };
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
    teardown();

    try {
      loading = true;
      loaded = false;
      traceLog = '';
      const radio = radios.find((r) => r.wasm === selectedRadio);
      currentRadio = radio ?? null;
      status = `Loading ${radio?.name ?? selectedRadio}...`;

      const radioKey = PersistentFS.radioKeyFromWasm(selectedRadio);
      persistentFs = new PersistentFS(radioKey);
      const hadData = await persistentFs.load();
      if (hadData) {
        const files = persistentFs.listFiles('/');
        onTrace(`[persist] restored ${files.length} file(s) from OPFS for "${radioKey}"\n`);
        readStickMode();
      } else {
        onTrace(`[persist] no previous data for "${radioKey}"\n`);
      }
      status = hadData
        ? `Loading ${radio?.name ?? selectedRadio} (restored SD card)...`
        : `Loading ${radio?.name ?? selectedRadio}...`;

      runner = new WasmRunner(onTrace, onAudio, persistentFs.fs);
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

      // Prime the SharedArrayBuffer with initial analog values
      for (let i = 0; i < analogValues.length; i++) {
        runner.setAnalog(i, analogValues[i]);
      }

      loaded = true;
      loading = false;
      status = `${radio?.name}: LCD ${lcdWidth}\u00d7${lcdHeight} ${lcdDepth}bpp`;
    } catch (e) {
      loading = false;
      status = `Error: ${e instanceof Error ? e.message : e}`;
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
      canvas.width = lcdWidth;
      canvas.height = lcdHeight;
    }

    pollTimer = window.setInterval(pollLcd, 33);

    // Auto-save every 30 seconds while running
    autoSaveTimer = window.setInterval(async () => {
      try {
        await persistentFs?.save();
      } catch { /* best effort */ }
    }, 30_000);
  }

  function teardown() {
    if (pollTimer !== null) {
      clearInterval(pollTimer);
      pollTimer = null;
    }
    if (autoSaveTimer !== null) {
      clearInterval(autoSaveTimer);
      autoSaveTimer = null;
    }
    if (runner?.exports && running) {
      try { runner.exports.simuStop(); } catch { /* expected */ }
    }
    runner = null;
    persistentFs = null;
    running = false;
    loaded = false;
    currentRadio = null;
  }

  async function stopSimulator() {
    if (pollTimer !== null) {
      clearInterval(pollTimer);
      pollTimer = null;
    }
    if (autoSaveTimer !== null) {
      clearInterval(autoSaveTimer);
      autoSaveTimer = null;
    }
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
    // settings, closing files, etc.) before saving memfs to OPFS.
    await waitForShutdown();

    status = 'Saving SD card...';
    saving = true;
    try {
      const files = persistentFs?.listFiles('/') ?? [];
      await persistentFs?.save();
      onTrace(`[persist] saved ${files.length} file(s) to OPFS\n`);
    } catch (e) {
      onTrace(`[persist] save error: ${e}\n`);
    }
    saving = false;

    // Tear down the old WASM instance and reload so Start works again.
    // Chrome caches compiled WASM, so subsequent loads are near-instant.
    runner = null;
    loaded = false;
    await loadSelected();
    status = 'Stopped (SD card saved)';
  }

  function waitForShutdown(): Promise<void> {
    return new Promise((resolve) => {
      let elapsed = 0;
      const interval = 200;
      const maxWait = 5000;
      const check = () => {
        elapsed += interval;
        const files = persistentFs?.listFiles('/') ?? [];
        // Shutdown is done once files appear and no new ones for a beat,
        // or we've waited long enough.
        if (files.length > 0 || elapsed >= maxWait) {
          // Give a small extra delay for any trailing writes
          setTimeout(resolve, 500);
        } else {
          setTimeout(check, interval);
        }
      };
      setTimeout(check, interval);
    });
  }

  function pollLcd() {
    const ex = runner?.exports;
    if (!ex || !canvas) return;

    if (!ex.simuLcdChanged()) return;

    const data = runner!.copyLcd(lcdSize);
    if (!data) return;

    const ctx = canvas.getContext('2d');
    if (!ctx) return;

    if (lcdDepth === 16) {
      renderRgb565(ctx, data, lcdWidth, lcdHeight);
    } else if (lcdDepth === 4) {
      render4bit(ctx, data, lcdWidth, lcdHeight);
    } else {
      render1bit(ctx, data, lcdWidth, lcdHeight);
    }

    ex.simuLcdFlushed();
  }

  function handleCanvasClick(e: MouseEvent) {
    const ex = runner?.exports;
    if (!ex || !running || !canvas) return;

    const rect = canvas.getBoundingClientRect();
    const scaleX = lcdWidth / rect.width;
    const scaleY = lcdHeight / rect.height;
    const x = Math.round((e.clientX - rect.left) * scaleX);
    const y = Math.round((e.clientY - rect.top) * scaleY);

    if (e.type === 'mousedown') {
      ex.simuTouchDown(x, y);
    } else if (e.type === 'mouseup') {
      ex.simuTouchUp();
    } else if (e.type === 'mousemove' && e.buttons === 1) {
      ex.simuTouchDown(x, y);
    }
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

  // --- Switch handling ---
  function toggleSwitch(index: number) {
    const sw = currentRadio?.switches?.[index];
    if (!sw) return;
    const cur = switchStates[index];
    if (sw.type === '2POS') {
      updateSwitch(index, cur === -1 ? 1 : -1);
    } else {
      // 3POS: cycle -1 -> 0 -> 1 -> -1
      if (cur === -1) updateSwitch(index, 0);
      else if (cur === 0) updateSwitch(index, 1);
      else updateSwitch(index, -1);
    }
  }

  function switchLabel(state: number): string {
    if (state === -1) return '\u2191'; // up arrow
    if (state === 0) return '\u2014'; // em dash (middle)
    return '\u2193'; // down arrow
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
    if (!persistentFs) return;
    if (!confirm('Wipe all SD card data for this radio? This cannot be undone.')) return;
    await persistentFs.wipe();
    onTrace('[persist] SD card data wiped\n');
    status = 'SD card wiped. Reload to start fresh.';
  }

  function handleFileUpload() {
    if (!persistentFs) return;
    const targetDir = prompt('Upload to directory:', '/') ?? '/';
    const input = document.createElement('input');
    input.type = 'file';
    input.multiple = true;
    input.onchange = async () => {
      if (!input.files || !persistentFs) return;
      for (const file of input.files) {
        const dir = targetDir.endsWith('/') ? targetDir : targetDir + '/';
        const path = dir + file.name;
        const data = await file.arrayBuffer();
        persistentFs.uploadFile(path, data);
        onTrace(`[persist] uploaded ${path} (${data.byteLength} bytes)\n`);
      }
      status = `Uploaded ${input.files.length} file(s) to ${targetDir}`;
    };
    input.click();
  }

  function handleFolderUpload() {
    if (!persistentFs) return;
    const input = document.createElement('input');
    input.type = 'file';
    input.multiple = true;
    (input as any).webkitdirectory = true;
    input.onchange = async () => {
      if (!input.files || !persistentFs) return;
      let count = 0;
      for (const file of input.files) {
        // webkitRelativePath gives "SOUNDS/en/system/hello.wav"
        // Keep as-is so selecting "SOUNDS" creates /SOUNDS/...
        const relPath = (file as any).webkitRelativePath as string;
        if (!relPath) continue;
        const path = '/' + relPath;
        const data = await file.arrayBuffer();
        persistentFs.uploadFile(path, data);
        count++;
      }
      onTrace(`[persist] uploaded ${count} file(s) from folder\n`);
      status = `Uploaded ${count} file(s)`;
    };
    input.click();
  }

  // Discover on mount
  discoverRadios();

  // Best-effort save on page close — async isn't guaranteed to finish,
  // but the periodic auto-save (every 30s) ensures minimal data loss.
  function handleBeforeUnload() {
    persistentFs?.save();  // fire-and-forget
  }
  window.addEventListener('beforeunload', handleBeforeUnload);

  onDestroy(() => {
    window.removeEventListener('beforeunload', handleBeforeUnload);
    teardown();
    audioCtx?.close();
  });
</script>

<svelte:document onkeydown={handleKey} onkeyup={handleKey} />

<main>
  <div class="header">
    <h1>EdgeTX Simulator</h1>
    <div class="toolbar">
      <select
        bind:value={selectedRadio}
        disabled={running || loading}
      >
        {#each radios as radio}
          <option value={radio.wasm} disabled={!radio.available}>
            {radio.name}{radio.available ? '' : ' (not found)'}
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
        <button onclick={stopSimulator} disabled={saving}>Stop</button>
      {/if}
      {#if loaded && !running && !saving}
        <button onclick={handleFileUpload} class="secondary">Upload Files</button>
        <button onclick={handleFolderUpload} class="secondary">Upload Folder</button>
        <button onclick={resetSdCard} class="danger">Reset SD</button>
      {/if}
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
      <canvas
        bind:this={canvas}
        class="lcd"
        style:width="{Math.max(lcdWidth, 150 * lcdWidth / lcdHeight, 320)}px"
        style:aspect-ratio="{lcdWidth} / {lcdHeight}"
        onmousedown={handleCanvasClick}
        onmouseup={handleCanvasClick}
        onmousemove={handleCanvasClick}
        onwheel={handleWheel}
      ></canvas>
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

      <!-- Main controls area: switches | gimbal | sliders | gimbal | switches -->
      <div class="controls-area">
        <!-- Left switches -->
        <div class="switch-column">
          {#each getLeftSwitches() as { sw, index }}
            <div class="switch-control">
              <span class="switch-name">{sw.name}</span>
              <button
                class="switch-btn"
                class:sw-up={switchStates[index] === -1}
                class:sw-mid={switchStates[index] === 0}
                class:sw-down={switchStates[index] === 1}
                onclick={() => toggleSwitch(index)}
              >{switchLabel(switchStates[index])}</button>
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
              <button
                class="switch-btn"
                class:sw-up={switchStates[index] === -1}
                class:sw-mid={switchStates[index] === 0}
                class:sw-down={switchStates[index] === 1}
                onclick={() => toggleSwitch(index)}
              >{switchLabel(switchStates[index])}</button>
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
    <summary>Trace output</summary>
    <pre class="trace">{traceLog}</pre>
  </details>
</main>

<style>
  :global(body) {
    margin: 0;
    background: #1a1a1a;
    color: #e0e0e0;
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

  h1 {
    font-size: 1.2rem;
    margin: 0 0 0.5rem;
    color: #4caf50;
    letter-spacing: 0.05em;
    text-transform: uppercase;
  }

  .toolbar {
    display: flex;
    gap: 0.5rem;
    align-items: center;
    justify-content: center;
    margin: 0.5rem 0;
  }

  select {
    padding: 0.35rem 0.5rem;
    font-size: 0.9rem;
    background: #2a2a2a;
    color: #e0e0e0;
    border: 1px solid #444;
    border-radius: 4px;
  }

  .status {
    color: #888;
    margin: 0.25rem 0;
    font-size: 0.85rem;
  }

  button {
    padding: 0.35rem 1rem;
    font-size: 0.9rem;
    cursor: pointer;
    background: #333;
    color: #e0e0e0;
    border: 1px solid #555;
    border-radius: 4px;
    transition: background 0.15s;
  }

  button:hover {
    background: #444;
  }

  button:active {
    background: #555;
  }

  button.secondary {
    background: #2a3a2a;
    border-color: #4a6a4a;
  }

  button.danger {
    background: #3a2a2a;
    border-color: #6a4a4a;
  }

  button.danger:hover {
    background: #4a2a2a;
  }

  /* Radio body */
  .radio-body {
    background: #222;
    border-radius: 12px;
    padding: 1rem;
    border: 2px solid #333;
    box-shadow: 0 4px 20px rgba(0, 0, 0, 0.5);
  }

  /* LCD */
  .lcd-area {
    display: flex;
    justify-content: center;
    align-items: center;
    gap: 0.5rem;
    margin-bottom: 0.75rem;
  }

  .lcd {
    border: 3px solid #111;
    background: #000;
    image-rendering: pixelated;
    cursor: crosshair;
    border-radius: 4px;
    box-shadow: inset 0 0 10px rgba(0, 0, 0, 0.8);
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
    background: #2a2a2a;
    color: #ccc;
    border: 1px solid #444;
    border-radius: 4px;
    cursor: pointer;
    min-width: 3.5rem;
    user-select: none;
    -webkit-user-select: none;
  }

  .hw-key:active {
    background: #555;
    border-color: #888;
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
    color: #aaa;
    text-transform: uppercase;
    letter-spacing: 0.05em;
  }

  .pot-knob {
    width: 80px;
    accent-color: #4caf50;
  }

  .pot-value {
    font-size: 0.65rem;
    color: #666;
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
    color: #aaa;
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
    background: #333;
    border: 1px solid #555;
  }

  .multipos-btn.active {
    background: #4caf50;
    color: #000;
    border-color: #4caf50;
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
    color: #aaa;
    text-transform: uppercase;
    letter-spacing: 0.05em;
  }

  .switch-btn {
    width: 36px;
    height: 32px;
    padding: 0;
    font-size: 1rem;
    border-radius: 4px;
    display: flex;
    align-items: center;
    justify-content: center;
    font-weight: bold;
    transition: all 0.1s;
  }

  .switch-btn.sw-up {
    background: linear-gradient(180deg, #4caf50 0%, #388e3c 100%);
    border-color: #4caf50;
    color: #fff;
  }

  .switch-btn.sw-mid {
    background: linear-gradient(180deg, #ff9800 0%, #e65100 100%);
    border-color: #ff9800;
    color: #fff;
  }

  .switch-btn.sw-down {
    background: linear-gradient(180deg, #f44336 0%, #c62828 100%);
    border-color: #f44336;
    color: #fff;
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
    color: #aaa;
    text-transform: uppercase;
  }

  .vertical-slider {
    writing-mode: vertical-lr;
    direction: rtl;
    height: 120px;
    width: 20px;
    accent-color: #4caf50;
  }

  /* Gimbal */
  .gimbal-wrapper {
    display: flex;
    align-items: center;
    gap: 0.25rem;
  }

  .gimbal-label {
    font-size: 0.7rem;
    color: #666;
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
    background: radial-gradient(circle at center, #2a2a2a 0%, #1a1a1a 100%);
    border: 2px solid #444;
    border-radius: 12px;
    position: relative;
    cursor: crosshair;
    touch-action: none;
    user-select: none;
    box-shadow: inset 0 2px 8px rgba(0, 0, 0, 0.5);
  }

  .gimbal-crosshair-h {
    position: absolute;
    left: 0;
    right: 0;
    height: 1px;
    background: rgba(76, 175, 80, 0.25);
    pointer-events: none;
  }

  .gimbal-crosshair-v {
    position: absolute;
    top: 0;
    bottom: 0;
    width: 1px;
    background: rgba(76, 175, 80, 0.25);
    pointer-events: none;
  }

  .gimbal-dot {
    position: absolute;
    width: 18px;
    height: 18px;
    border-radius: 50%;
    background: radial-gradient(circle at 40% 40%, #6fbf73 0%, #4caf50 60%, #2e7d32 100%);
    border: 2px solid #81c784;
    transform: translate(-50%, -50%);
    pointer-events: none;
    box-shadow: 0 2px 6px rgba(0, 0, 0, 0.4);
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
    color: #666;
    min-width: 18px;
    text-align: center;
  }

  .trim-btn {
    width: 32px;
    height: 22px;
    padding: 0;
    font-size: 0.6rem;
    border-radius: 3px;
    background: #2a2a2a;
    border: 1px solid #444;
    color: #ccc;
    font-family: monospace;
  }

  .trim-btn:active {
    background: #4caf50;
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
    color: #888;
    font-size: 0.85rem;
    padding: 0.25rem 0;
  }

  .trace {
    max-height: 300px;
    overflow-y: auto;
    background: #111;
    color: #0f0;
    padding: 0.5rem;
    font-size: 0.75rem;
    white-space: pre-wrap;
    word-break: break-all;
    border-radius: 4px;
    border: 1px solid #333;
  }
</style>
