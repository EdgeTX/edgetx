/** Render a 16-bit RGB565 LCD framebuffer onto a canvas 2D context. */
export function renderRgb565(
  ctx: CanvasRenderingContext2D,
  data: Uint8Array,
  width: number,
  height: number
): void {
  const img = ctx.createImageData(width, height);
  const pixels = img.data;
  const view = new DataView(data.buffer, data.byteOffset, data.byteLength);

  for (let i = 0; i < width * height; i++) {
    const rgb565 = view.getUint16(i * 2, true);
    const r = ((rgb565 >> 11) & 0x1f) << 3;
    const g = ((rgb565 >> 5) & 0x3f) << 2;
    const b = (rgb565 & 0x1f) << 3;
    const j = i * 4;
    pixels[j] = r;
    pixels[j + 1] = g;
    pixels[j + 2] = b;
    pixels[j + 3] = 255;
  }

  ctx.putImageData(img, 0, 0);
}

// Dot-matrix LCD scale factor. Each LCD pixel becomes SCALE × SCALE canvas pixels.
// Grid lines are drawn on top via 2D API, 1 canvas pixel wide = 1/SCALE of an LCD pixel.
const SCALE = 3;

// Backlight-on background color (matches Companion: rgb(47, 123, 227))
const BG_R = 47, BG_G = 123, BG_B = 227;

/** Return the canvas dimensions for a dot-matrix LCD. */
export function dotMatrixSize(width: number, height: number): { w: number; h: number } {
  return { w: width * SCALE, h: height * SCALE };
}

/** Render a 4-bit grayscale LCD framebuffer with backlight tint and dot-matrix grid.
 *  Firmware layout: byte at (y>>1)*width+x, even y = low nibble, odd y = high nibble.
 *  Grayscale: 0 = white, 0xF = black. */
export function render4bit(
  ctx: CanvasRenderingContext2D,
  data: Uint8Array,
  width: number,
  height: number
): void {
  const cw = width * SCALE;
  const ch = height * SCALE;
  const img = ctx.createImageData(cw, ch);
  const pixels = img.data;

  for (let y = 0; y < height; y++) {
    for (let x = 0; x < width; x++) {
      const byteIdx = (y >> 1) * width + x;
      const nibble = (y & 1) ? (data[byteIdx] >> 4) & 0x0f : data[byteIdx] & 0x0f;
      const t = nibble / 15;
      const r = BG_R + (0 - BG_R) * t | 0;
      const g = BG_G + (0 - BG_G) * t | 0;
      const b = BG_B + (0 - BG_B) * t | 0;
      fillBlock(pixels, cw, x * SCALE, y * SCALE, r, g, b);
    }
  }

  ctx.putImageData(img, 0, 0);
  drawGrid(ctx, width, height, cw, ch);
}

/** Render a 1-bit monochrome LCD framebuffer with backlight tint and dot-matrix grid.
 *  Firmware layout: row-major, byte at (y>>3)*width+x, bit (y&7). Set bit = black. */
export function render1bit(
  ctx: CanvasRenderingContext2D,
  data: Uint8Array,
  width: number,
  height: number
): void {
  const cw = width * SCALE;
  const ch = height * SCALE;
  const img = ctx.createImageData(cw, ch);
  const pixels = img.data;

  // Fill entire image with backlight background
  for (let i = 0; i < pixels.length; i += 4) {
    pixels[i] = BG_R;
    pixels[i + 1] = BG_G;
    pixels[i + 2] = BG_B;
    pixels[i + 3] = 255;
  }

  // Paint dark ink pixels
  for (let y = 0; y < height; y++) {
    for (let x = 0; x < width; x++) {
      const byteIdx = (y >> 3) * width + x;
      const bit = (data[byteIdx] >> (y & 7)) & 1;
      if (bit) {
        fillBlock(pixels, cw, x * SCALE, y * SCALE, 0, 0, 0);
      }
    }
  }

  ctx.putImageData(img, 0, 0);
  drawGrid(ctx, width, height, cw, ch);
}

/** Fill a SCALE × SCALE block in the pixel buffer. */
function fillBlock(
  pixels: Uint8ClampedArray, canvasWidth: number,
  ox: number, oy: number,
  r: number, g: number, b: number,
) {
  for (let dy = 0; dy < SCALE; dy++) {
    const row = (oy + dy) * canvasWidth + ox;
    for (let dx = 0; dx < SCALE; dx++) {
      const j = (row + dx) * 4;
      pixels[j] = r;
      pixels[j + 1] = g;
      pixels[j + 2] = b;
      pixels[j + 3] = 255;
    }
  }
}

/** Draw dot-matrix grid lines on top of the rendered pixels. */
function drawGrid(
  ctx: CanvasRenderingContext2D,
  lcdW: number, lcdH: number,
  cw: number, ch: number,
) {
  ctx.fillStyle = `rgba(${BG_R}, ${BG_G}, ${BG_B}, 0.3)`;
  for (let x = 1; x < lcdW; x++) {
    ctx.fillRect(x * SCALE, 0, 1, ch);
  }
  for (let y = 1; y < lcdH; y++) {
    ctx.fillRect(0, y * SCALE, cw, 1);
  }
}
