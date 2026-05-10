// ---- Shader sources ----

const VERT_SRC = `
attribute vec2 aPos;
varying vec2 vUv;
void main() {
  vUv = vec2(aPos.x * 0.5 + 0.5, 0.5 - aPos.y * 0.5);
  gl_Position = vec4(aPos, 0.0, 1.0);
}`;

const FRAG_SRC = `
precision mediump float;
varying vec2 vUv;
uniform sampler2D uTex;
uniform vec2 uLcdSize;      // LCD pixel dimensions (e.g. 212, 64)
uniform int uDotMatrix;     // 1 = dot-matrix effect, 0 = plain
uniform vec3 uBacklight;    // backlight color (normalised)
uniform float uGridAlpha;   // grid line blend strength

void main() {
  // Nearest-neighbour sample of the LCD texture
  vec2 texel = (floor(vUv * uLcdSize) + 0.5) / uLcdSize;
  vec4 pixel = texture2D(uTex, texel);

  if (uDotMatrix == 0) {
    gl_FragColor = pixel;
    return;
  }

  // Position within the LCD pixel cell (0..1)
  vec2 cell = fract(vUv * uLcdSize);

  // Distance from cell edge (0 at edge, 0.5 at centre)
  vec2 d = 0.5 - abs(cell - 0.5);

  // Grid line: smoothstep near the edge
  float gridH = 1.0 - smoothstep(0.0, 0.25, d.x);
  float gridV = 1.0 - smoothstep(0.0, 0.25, d.y);
  float grid = max(gridH, gridV);

  // Blend pixel colour toward backlight at grid lines
  vec3 color = mix(pixel.rgb, uBacklight, grid * uGridAlpha);
  gl_FragColor = vec4(color, 1.0);
}`;

// Backlight color (matches Companion: rgb(47, 123, 227))
const BG_R = 47 / 255, BG_G = 123 / 255, BG_B = 227 / 255;

// ---- Framebuffer decode helpers (CPU → RGBA texture data) ----

/** Decode RGB565 framebuffer into RGBA. */
function decodeRgb565(data: Uint8Array, w: number, h: number, out: Uint8Array): void {
  const view = new DataView(data.buffer, data.byteOffset, data.byteLength);
  for (let i = 0; i < w * h; i++) {
    const rgb565 = view.getUint16(i * 2, true);
    const j = i * 4;
    out[j]     = ((rgb565 >> 11) & 0x1f) << 3;
    out[j + 1] = ((rgb565 >> 5) & 0x3f) << 2;
    out[j + 2] = (rgb565 & 0x1f) << 3;
    out[j + 3] = 255;
  }
}

/** Decode 4-bit grayscale framebuffer into RGBA with backlight tint. */
function decode4bit(data: Uint8Array, w: number, h: number, out: Uint8Array): void {
  for (let y = 0; y < h; y++) {
    for (let x = 0; x < w; x++) {
      const byteIdx = (y >> 1) * w + x;
      const nibble = (y & 1) ? (data[byteIdx] >> 4) & 0x0f : data[byteIdx] & 0x0f;
      const t = nibble / 15;
      const j = (y * w + x) * 4;
      out[j]     = (BG_R * 255 * (1 - t)) | 0;
      out[j + 1] = (BG_G * 255 * (1 - t)) | 0;
      out[j + 2] = (BG_B * 255 * (1 - t)) | 0;
      out[j + 3] = 255;
    }
  }
}

/** Decode 1-bit monochrome framebuffer into RGBA with backlight tint. */
function decode1bit(data: Uint8Array, w: number, h: number, out: Uint8Array): void {
  for (let y = 0; y < h; y++) {
    for (let x = 0; x < w; x++) {
      const byteIdx = (y >> 3) * w + x;
      const bit = (data[byteIdx] >> (y & 7)) & 1;
      const j = (y * w + x) * 4;
      if (bit) {
        out[j] = 0; out[j + 1] = 0; out[j + 2] = 0;
      } else {
        out[j] = BG_R * 255; out[j + 1] = BG_G * 255; out[j + 2] = BG_B * 255;
      }
      out[j + 3] = 255;
    }
  }
}

// ---- WebGL LCD Renderer ----

export class LcdRenderer {
  private gl: WebGLRenderingContext;
  private texture: WebGLTexture;
  private texBuf: Uint8Array | null = null;
  private texW = 0;
  private texH = 0;

  // Uniform locations
  private uTex: WebGLUniformLocation;
  private uLcdSize: WebGLUniformLocation;
  private uDotMatrix: WebGLUniformLocation;
  private uBacklight: WebGLUniformLocation;
  private uGridAlpha: WebGLUniformLocation;

  constructor(canvas: HTMLCanvasElement) {
    const gl = canvas.getContext('webgl', { antialias: false, alpha: false })!;
    if (!gl) throw new Error('WebGL not available');
    this.gl = gl;

    // Compile shaders
    const vs = this.compileShader(gl.VERTEX_SHADER, VERT_SRC);
    const fs = this.compileShader(gl.FRAGMENT_SHADER, FRAG_SRC);
    const prog = gl.createProgram()!;
    gl.attachShader(prog, vs);
    gl.attachShader(prog, fs);
    gl.linkProgram(prog);
    if (!gl.getProgramParameter(prog, gl.LINK_STATUS)) {
      throw new Error('Shader link: ' + gl.getProgramInfoLog(prog));
    }
    gl.useProgram(prog);

    // Get uniform locations
    this.uTex = gl.getUniformLocation(prog, 'uTex')!;
    this.uLcdSize = gl.getUniformLocation(prog, 'uLcdSize')!;
    this.uDotMatrix = gl.getUniformLocation(prog, 'uDotMatrix')!;
    this.uBacklight = gl.getUniformLocation(prog, 'uBacklight')!;
    this.uGridAlpha = gl.getUniformLocation(prog, 'uGridAlpha')!;

    // Full-screen quad
    const buf = gl.createBuffer()!;
    gl.bindBuffer(gl.ARRAY_BUFFER, buf);
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array([
      -1, -1,  1, -1,  -1, 1,
      -1,  1,  1, -1,   1, 1,
    ]), gl.STATIC_DRAW);
    const aPos = gl.getAttribLocation(prog, 'aPos');
    gl.enableVertexAttribArray(aPos);
    gl.vertexAttribPointer(aPos, 2, gl.FLOAT, false, 0, 0);

    // Create texture
    this.texture = gl.createTexture()!;
    gl.bindTexture(gl.TEXTURE_2D, this.texture);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
  }

  /** Resize the canvas and viewport. Call when LCD dimensions are known. */
  resize(canvasWidth: number, canvasHeight: number): void {
    const canvas = this.gl.canvas as HTMLCanvasElement;
    canvas.width = canvasWidth;
    canvas.height = canvasHeight;
    this.gl.viewport(0, 0, canvasWidth, canvasHeight);
  }

  /** Render an LCD frame. */
  render(data: Uint8Array, width: number, height: number, depth: number): void {
    const gl = this.gl;

    // (Re)allocate texture buffer if LCD size changed
    if (width !== this.texW || height !== this.texH) {
      this.texW = width;
      this.texH = height;
      this.texBuf = new Uint8Array(width * height * 4);
    }

    // Decode framebuffer into RGBA
    const buf = this.texBuf!;
    if (depth === 16) {
      decodeRgb565(data, width, height, buf);
    } else if (depth === 4) {
      decode4bit(data, width, height, buf);
    } else {
      decode1bit(data, width, height, buf);
    }

    // Upload texture
    gl.bindTexture(gl.TEXTURE_2D, this.texture);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, width, height, 0, gl.RGBA, gl.UNSIGNED_BYTE, buf);

    // Set uniforms
    gl.uniform1i(this.uTex, 0);
    gl.uniform2f(this.uLcdSize, width, height);
    gl.uniform1i(this.uDotMatrix, depth < 16 ? 1 : 0);
    gl.uniform3f(this.uBacklight, BG_R, BG_G, BG_B);
    gl.uniform1f(this.uGridAlpha, 0.5);

    // Draw
    gl.drawArrays(gl.TRIANGLES, 0, 6);
  }

  private compileShader(type: number, src: string): WebGLShader {
    const gl = this.gl;
    const shader = gl.createShader(type)!;
    gl.shaderSource(shader, src);
    gl.compileShader(shader);
    if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
      throw new Error('Shader compile: ' + gl.getShaderInfoLog(shader));
    }
    return shader;
  }
}
