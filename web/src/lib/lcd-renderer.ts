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

/** Render a 4-bit grayscale LCD framebuffer.
 *  Firmware layout: byte at (y>>1)*width+x, even y = low nibble, odd y = high nibble.
 *  Grayscale: 0 = white, 0xF = black. */
export function render4bit(
  ctx: CanvasRenderingContext2D,
  data: Uint8Array,
  width: number,
  height: number
): void {
  const img = ctx.createImageData(width, height);
  const pixels = img.data;

  for (let y = 0; y < height; y++) {
    for (let x = 0; x < width; x++) {
      const byteIdx = (y >> 1) * width + x;
      const nibble = (y & 1) ? (data[byteIdx] >> 4) & 0x0f : data[byteIdx] & 0x0f;
      const gray = 255 - nibble * 17; // 0xF=black(0), 0=white(255)
      const j = (y * width + x) * 4;
      pixels[j] = gray;
      pixels[j + 1] = gray;
      pixels[j + 2] = gray;
      pixels[j + 3] = 255;
    }
  }

  ctx.putImageData(img, 0, 0);
}

/** Render a 1-bit monochrome LCD framebuffer.
 *  Firmware layout: row-major, byte at (y>>3)*width+x, bit (y&7). Set bit = black. */
export function render1bit(
  ctx: CanvasRenderingContext2D,
  data: Uint8Array,
  width: number,
  height: number
): void {
  const img = ctx.createImageData(width, height);
  const pixels = img.data;

  for (let y = 0; y < height; y++) {
    for (let x = 0; x < width; x++) {
      const byteIdx = (y >> 3) * width + x;
      const bit = (data[byteIdx] >> (y & 7)) & 1;
      const color = bit ? 0 : 255; // set bit = black pixel on white background
      const j = (y * width + x) * 4;
      pixels[j] = color;
      pixels[j + 1] = color;
      pixels[j + 2] = color;
      pixels[j + 3] = 255;
    }
  }

  ctx.putImageData(img, 0, 0);
}
