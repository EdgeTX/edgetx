<script lang="ts">
  import { onDestroy } from 'svelte';
  import { WasmRunner } from './lib/wasm-runner';
  import { renderRgb565, render4bit, render1bit } from './lib/lcd-renderer';

  interface RadioEntry {
    name: string;
    wasm: string;
    available?: boolean;
  }

  let canvas: HTMLCanvasElement;
  let status = $state('Loading radio list...');
  let traceLog = $state('');
  let running = $state(false);
  let loaded = $state(false);
  let loading = $state(false);

  let radios = $state<RadioEntry[]>([]);
  let selectedRadio = $state('');

  let runner: WasmRunner | null = null;
  let pollTimer: number | null = null;
  let lcdWidth = $state(480);
  let lcdHeight = $state(272);
  let lcdDepth = 0;
  let lcdSize = 0;

  // Audio playback via Web Audio API
  let audioCtx: AudioContext | null = null;
  const AUDIO_SAMPLE_RATE = 32000;

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
    const src = audioCtx.createBufferSource();
    src.buffer = buf;
    src.connect(audioCtx.destination);
    src.start();
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
      status = `Loading ${radio?.name ?? selectedRadio}...`;

      runner = new WasmRunner(onTrace, onAudio);
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

    const ex = runner.exports;
    ex.simuInit();
    ex.simuStart(0);
    running = true;
    status = 'Running';

    if (canvas) {
      canvas.width = lcdWidth;
      canvas.height = lcdHeight;
    }

    pollTimer = window.setInterval(pollLcd, 33);
  }

  function teardown() {
    if (pollTimer !== null) {
      clearInterval(pollTimer);
      pollTimer = null;
    }
    if (runner?.exports && running) {
      runner.exports.simuStop();
    }
    runner = null;
    running = false;
    loaded = false;
  }

  function stopSimulator() {
    if (pollTimer !== null) {
      clearInterval(pollTimer);
      pollTimer = null;
    }
    if (runner?.exports) {
      runner.exports.simuStop();
    }
    running = false;
    status = 'Stopped';
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

  // Discover on mount
  discoverRadios();

  onDestroy(() => {
    teardown();
    audioCtx?.close();
  });
</script>

<svelte:document onkeydown={handleKey} onkeyup={handleKey} />

<main>
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
      <button onclick={stopSimulator}>Stop</button>
    {/if}
  </div>

  <p class="status">{status}</p>

  <div class="simulator">
    <canvas
      bind:this={canvas}
      class="lcd"
      style:width="{Math.max(lcdWidth, 480)}px"
      style:aspect-ratio="{lcdWidth} / {lcdHeight}"
      onmousedown={handleCanvasClick}
      onmouseup={handleCanvasClick}
      onmousemove={handleCanvasClick}
      onwheel={handleWheel}
    ></canvas>
  </div>

  <details>
    <summary>Trace output</summary>
    <pre class="trace">{traceLog}</pre>
  </details>
</main>

<style>
  main {
    max-width: 800px;
    margin: 0 auto;
    padding: 1rem;
    font-family: system-ui, sans-serif;
  }

  .toolbar {
    display: flex;
    gap: 0.5rem;
    align-items: center;
    margin: 1rem 0;
  }

  select {
    padding: 0.4rem 0.6rem;
    font-size: 1rem;
  }

  .status {
    color: #666;
    margin: 0.5rem 0;
  }

  button {
    padding: 0.5rem 1.5rem;
    font-size: 1rem;
    cursor: pointer;
  }

  .simulator {
    display: flex;
    justify-content: center;
    margin: 1rem 0;
  }

  .lcd {
    border: 2px solid #333;
    background: #000;
    image-rendering: pixelated;
    cursor: crosshair;
  }

  .trace {
    max-height: 300px;
    overflow-y: auto;
    background: #1a1a1a;
    color: #0f0;
    padding: 0.5rem;
    font-size: 0.8rem;
    white-space: pre-wrap;
    word-break: break-all;
  }
</style>
