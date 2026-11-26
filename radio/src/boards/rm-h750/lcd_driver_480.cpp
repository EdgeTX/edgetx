/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "stm32_hal_ll.h"
#include "stm32_hal.h"
#include "edgetx_types.h"
#include "hal.h"
#include "debug.h"
#include "lcd.h"
#include "lcd_driver_480.h"
#include "board.h"

#include "hal/gpio.h"
#include "stm32_gpio.h"
#include "stm32_qspi.h"

#include "delays_driver.h"
#include "timers_driver.h"

#if !defined(LCD_SW_ROTATE_90)
#error "lcd_driver_480 rotates in software: define LCD_SW_ROTATE_90 for this target"
#endif

static LTDC_HandleTypeDef hltdc;

//
// The ST7365 is a 320x480 portrait panel, mounted in landscape.
//
// The panel could rotate the picture itself (MADCTL MV), but it refreshes the
// glass from its GRAM along its own portrait lines, i.e. along the landscape
// columns, while the LTDC writes landscape rows. Both sweeps cross on every
// frame that changes, showing a diagonal line between the old and new picture.
//
// So the panel is driven in its native portrait order instead, and every area
// LVGL flushes (landscape, LCD_W x LCD_H) is rotated by 90 degrees into a
// portrait frame buffer (LCD_PHYS_W x LCD_PHYS_H):
//
//   landscape (x, y)  ->  portrait (y, LCD_PHYS_H - 1 - x)
//
// DMA2D cannot rotate: the CPU transposes strips of the area into SRAM tiles,
// which the DMA2D then copies into the SDRAM frame buffer. Frame buffers are
// only ever written by the DMA2D, so there is no D-cache maintenance on them,
// and the DMA2D is programmed directly: DMACopyBitmap() would clean and
// invalidate the whole D-cache for each tile.
//
// Two portrait buffers are used: the front one is scanned out by the LTDC, the
// back one is updated, then swapped in on the next vertical blanking. The areas
// of a frame are copied into the other buffer before the next frame is drawn,
// once the swap has happened.
//

static pixel_t _LCD_BUF_1[DISPLAY_BUFFER_SIZE] __SDRAM __ALIGNED(64);
// not used by LVGL, which is not in direct mode here (see lcd.cpp)
extern pixel_t LCD_SECOND_FRAME_BUFFER[DISPLAY_BUFFER_SIZE];
#define _LCD_BUF_2 LCD_SECOND_FRAME_BUFFER

static pixel_t* _front_buffer = _LCD_BUF_1;
static pixel_t* _back_buffer = _LCD_BUF_2;

// Landscape columns transposed per DMA2D transfer. 16 pixels is one 32-byte
// cache line of each source row.
#define ROTATE_STRIP_W 16

// Source rows prefetched ahead of the transpose: each row of a strip is a
// separate cache line, so the transpose is otherwise bound by SDRAM latency.
#define ROTATE_PREFETCH_ROWS 8

// Double buffered, so the CPU fills one tile while the DMA2D reads the other.
// A tile holds up to ROTATE_STRIP_W portrait lines of up to LCD_H pixels.
static pixel_t _rotate_tiles[2][ROTATE_STRIP_W * LCD_H] __DMA;
static uint8_t _rotate_tile_idx = 0;

// Landscape areas written into the back buffer since the last swap
static lv_area_t _dirty_areas[LV_INV_BUF_SIZE];
static uint8_t _dirty_count = 0;
static bool _dirty_full = false;
static bool _sync_pending = false;

// The LTDC scans a frame in ~18ms: wait a couple of frames before giving up
#define LCD_RELOAD_TIMEOUT_MS 50

// Areas left to copy into the back buffer, after removing what the next
// frame redraws anyway (each removal splits an area in up to 4)
#define SYNC_MAX_AREAS 64
static lv_area_t _sync_areas[2][SYNC_MAX_AREAS];

static inline void _dma_wait()
{
  while (DMA2D->CR & DMA2D_CR_START);
}

// RGB565 copy, w x h pixels. Strides in pixels. No D-cache maintenance:
// the callers take care of the source, and the destination is only ever
// written by the DMA2D.
static void _dma_copy(pixel_t* dst, uint32_t dst_stride, const pixel_t* src,
                      uint32_t src_stride, uint32_t w, uint32_t h)
{
  _dma_wait();
  DMA2D->CR = LL_DMA2D_MODE_M2M;
  DMA2D->FGMAR = (uint32_t)src;
  DMA2D->FGOR = src_stride - w;
  DMA2D->FGPFCCR = LL_DMA2D_INPUT_MODE_RGB565;
  DMA2D->OPFCCR = LL_DMA2D_OUTPUT_MODE_RGB565;
  DMA2D->OMAR = (uint32_t)dst;
  DMA2D->OOR = dst_stride - w;
  DMA2D->NLR = (w << DMA2D_NLR_PL_Pos) | h;
  DMA2D->CR |= DMA2D_CR_START;
}

// Copy a landscape area from the front to the back buffer
static void _sync_copy(const lv_area_t& area)
{
  // landscape (x, y) -> portrait (y, LCD_PHYS_H - 1 - x)
  uint32_t offset = (LCD_PHYS_H - 1 - area.x2) * LCD_PHYS_W + area.y1;
  _dma_copy(_back_buffer + offset, LCD_PHYS_W, _front_buffer + offset,
            LCD_PHYS_W, area.y2 - area.y1 + 1, area.x2 - area.x1 + 1);
}

static void _add_dirty_area(const rect_t& area)
{
  if (_dirty_full) return;
  if (_dirty_count < LV_INV_BUF_SIZE) {
    _dirty_areas[_dirty_count++] = {area.x, area.y,
                                    (lv_coord_t)(area.x + area.w - 1),
                                    (lv_coord_t)(area.y + area.h - 1)};
  } else {
    _dirty_full = true;
  }
}

// Transpose 'n' landscape columns of 'h' rows (stride 'w') into 'n' tile
// lines of stride 'ts' (even): tile line (n - 1 - i) is column i, top to
// bottom. Works on 2x2 pixel blocks: two 32-bit loads, two 32-bit stores.
static void _transpose_strip(pixel_t* tile, coord_t ts, const pixel_t* src,
                             coord_t w, coord_t h, coord_t n)
{
  pixel_t* last_line = tile + (n - 1) * ts;

  coord_t y = 0;
  for (; y + 1 < h; y += 2) {
    const pixel_t* s0 = src + y * w;
    const pixel_t* s1 = s0 + w;
    __builtin_prefetch(s0 + ROTATE_PREFETCH_ROWS * w);
    __builtin_prefetch(s1 + ROTATE_PREFETCH_ROWS * w);

    // 'y' and 'ts' are even: 32-bit aligned
    pixel_t* d = last_line + y;
    coord_t i = 0;
    for (; i + 1 < n; i += 2) {
      uint32_t a = __UNALIGNED_UINT32_READ(s0 + i);  // (i, y)   (i+1, y)
      uint32_t b = __UNALIGNED_UINT32_READ(s1 + i);  // (i, y+1) (i+1, y+1)
      *(uint32_t*)d = __PKHBT(a, b, 16);             // column i
      d -= ts;
      *(uint32_t*)d = __PKHTB(b, a, 16);             // column i + 1
      d -= ts;
    }
    if (i < n) {
      *(uint32_t*)d = s0[i] | ((uint32_t)s1[i] << 16);
    }
  }

  if (y < h) {
    // odd height: last row on its own
    const pixel_t* s0 = src + y * w;
    pixel_t* d = last_line + y;
    for (coord_t i = 0; i < n; i++) {
      *d = s0[i];
      d -= ts;
    }
  }
}

// 'src' holds the area only (LVGL is not in direct mode): stride is area.w
static void _copy_rotate_90(pixel_t* dst, const pixel_t* src,
                            const rect_t& area)
{
  const coord_t w = area.w;
  const coord_t h = area.h;
  const coord_t ts = (h + 1) & ~1;

  for (coord_t col = 0; col < w; col += ROTATE_STRIP_W) {
    const coord_t n = (w - col < ROTATE_STRIP_W) ? w - col : ROTATE_STRIP_W;

    // The DMA2D may still read the other tile, but not this one: it was
    // waited for before the last transfer was started.
    pixel_t* tile = _rotate_tiles[_rotate_tile_idx];
    _rotate_tile_idx ^= 1;

    _transpose_strip(tile, ts, src + col, w, h, n);
    SCB_CleanDCache_by_Addr(tile, n * ts * sizeof(pixel_t));

    // Landscape column (area.x + col + n - 1) is portrait line
    // LCD_PHYS_H - 1 - (area.x + col + n - 1)
    coord_t line = LCD_PHYS_H - (area.x + col + n);
    _dma_copy(dst + line * LCD_PHYS_W + area.y, LCD_PHYS_W, tile, ts, h, n);
  }
}

static void _wait_frame_reloaded()
{
  // Not delay_us(): it stops when a debugger disables the DWT cycle counter
  uint32_t start = timersGetMsTick();
  while (LTDC->SRCR & LTDC_SRCR_VBR) {
    if (timersGetMsTick() - start > LCD_RELOAD_TIMEOUT_MS) {
      // LTDC stopped? Better a torn frame than a hung UI.
      LTDC->SRCR = LTDC_SRCR_IMR;
      break;
    }
  }
}

// Bring the back buffer (previous front) up to date with the areas drawn
// into the current front buffer during the last frame, except for what the
// frame being drawn now redraws anyway (LVGL already knows all its areas
// when it flushes the first one).
static void _sync_back_buffer()
{
  _wait_frame_reloaded();

  lv_area_t* todo = _sync_areas[0];
  lv_area_t* next = _sync_areas[1];
  uint32_t todo_n = 0;

  if (_dirty_full) {
    todo[todo_n++] = {0, 0, LCD_W - 1, LCD_H - 1};
  } else {
    for (uint8_t i = 0; i < _dirty_count; i++) todo[todo_n++] = _dirty_areas[i];
  }

  lv_disp_t* disp = _lv_refr_get_disp_refreshing();
  for (uint32_t j = 0; disp && j < disp->inv_p && todo_n > 0; j++) {
    if (disp->inv_area_joined[j]) continue;
    const lv_area_t* redrawn = &disp->inv_areas[j];

    uint32_t next_n = 0;
    for (uint32_t i = 0; i < todo_n; i++) {
      lv_area_t res[4];
      int8_t res_n = _lv_area_diff(res, &todo[i], redrawn);
      if (res_n < 0 || next_n + res_n > SYNC_MAX_AREAS) {
        // untouched, or no room to split it: copy it whole
        if (next_n < SYNC_MAX_AREAS) next[next_n++] = todo[i];
        else _sync_copy(todo[i]);
      } else {
        for (int8_t k = 0; k < res_n; k++) next[next_n++] = res[k];
      }
    }

    lv_area_t* tmp = todo;
    todo = next;
    next = tmp;
    todo_n = next_n;
  }

  for (uint32_t i = 0; i < todo_n; i++) _sync_copy(todo[i]);

  _dirty_count = 0;
  _dirty_full = false;
  _sync_pending = false;
}

static void startLcdRefresh(lv_disp_drv_t *disp_drv, uint16_t *buffer,
                            const rect_t &copy_area)
{
  if (_sync_pending) _sync_back_buffer();

  // LVGL may have drawn parts of the area with the DMA2D. Cleaning the whole
  // D-cache is cheaper than by address for a full screen area.
  _dma_wait();
  SCB_CleanInvalidateDCache();

  _copy_rotate_90(_back_buffer, buffer, copy_area);
  _add_dirty_area(copy_area);

  if (lv_disp_flush_is_last(disp_drv)) {
    pixel_t* tmp = _front_buffer;
    _front_buffer = _back_buffer;
    _back_buffer = tmp;

    _dma_wait();
    LTDC_Layer1->CFBAR = (uint32_t)_front_buffer;
    // reload shadow registers on vertical blank
    LTDC->SRCR = LTDC_SRCR_VBR;

    _sync_pending = true;
  }

  // LVGL's buffer has been consumed: it can draw the next area
  lcdFlushed();
}

static void LCD_Delay(void) {
  volatile unsigned int i;

  for (i = 0; i < 20; i++) {
    ;
  }
}

gpio_t _lcd_af_gpios[] = {
    GPIO_PIN(GPIOI, 12), GPIO_PIN(GPIOI, 13), GPIO_PIN(GPIOI, 14),

    GPIO_PIN(GPIOJ, 1),  GPIO_PIN(GPIOJ, 2),  GPIO_PIN(GPIOJ, 3),
    GPIO_PIN(GPIOJ, 4),  GPIO_PIN(GPIOJ, 5),  GPIO_PIN(GPIOJ, 6),
    GPIO_PIN(GPIOJ, 9),  GPIO_PIN(GPIOJ, 10), GPIO_PIN(GPIOJ, 11),
    GPIO_PIN(GPIOJ, 14), GPIO_PIN(GPIOJ, 15),

    GPIO_PIN(GPIOK, 0),  GPIO_PIN(GPIOK, 1),  GPIO_PIN(GPIOK, 2),
    GPIO_PIN(GPIOK, 3),  GPIO_PIN(GPIOK, 4),  GPIO_PIN(GPIOK, 5),
    GPIO_PIN(GPIOK, 6),  GPIO_PIN(GPIOK, 7),
};

static void LCD_AF_GPIOConfig(void)
{
  for (unsigned i = 0; i < sizeof(_lcd_af_gpios) / sizeof(_lcd_af_gpios[0]); i++) {
    gpio_init_af(_lcd_af_gpios[i], GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  }
}

static void lcdSpiConfig(void)
{
  LL_GPIO_InitTypeDef GPIO_InitStructure;
  LL_GPIO_StructInit(&GPIO_InitStructure);

  GPIO_InitStructure.Pin = LCD_SPI_MOSI_GPIO_PIN;
  GPIO_InitStructure.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStructure.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStructure.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStructure.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(LCD_SPI_MOSI_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.Pin = LCD_SPI_SCK_GPIO_PIN;
  LL_GPIO_Init(LCD_SPI_SCK_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.Pin = LCD_SPI_CS_GPIO_PIN;
  LL_GPIO_Init(LCD_SPI_CS_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.Pin = LCD_RESET_GPIO_PIN;
  LL_GPIO_Init(LCD_RESET_GPIO, &GPIO_InitStructure);

  LCD_CS_HIGH();
}

void lcdDelay() {
  delay_01us(1);
}

static void lcdReset() {
  LCD_CS_HIGH();

  LCD_NRST_HIGH();
  delay_ms(1);

  LCD_NRST_LOW(); // RESET();
  delay_ms(10);

  LCD_NRST_HIGH();
  delay_ms(100);
}

unsigned char LCD_ReadByteOnFallingEdge(void) {
  unsigned int i;
  unsigned char ReceiveData = 0;

  LCD_MOSI_HIGH();
  LCD_MOSI_AS_INPUT();

  for (i = 0; i < 8; i++) {
    LCD_DELAY();
    LCD_SCK_HIGH();
    LCD_DELAY();
    LCD_DELAY();
    ReceiveData <<= 1;

    LCD_SCK_LOW();
    LCD_DELAY();
    LCD_DELAY();
    if (LCD_READ_DATA_PIN()) {
      ReceiveData |= 0x01;
    }
  }

  LCD_MOSI_AS_OUTPUT();

  return (ReceiveData);
}

static void lcdWriteByte(uint8_t data_enable, uint8_t byte) {

  LCD_SCK_LOW();
  lcdDelay();

  if (data_enable) {
    LCD_MOSI_HIGH();
  } else {
    LCD_MOSI_LOW();
  }

  LCD_SCK_HIGH();
  lcdDelay();

  for (int i = 0; i < 8; i++) {
    LCD_SCK_LOW();
    lcdDelay();

    if (byte & 0x80) {
      LCD_MOSI_HIGH();
    } else {
      LCD_MOSI_LOW();
    }

    LCD_SCK_HIGH();
    byte <<= 1;

    lcdDelay();
  }

  LCD_SCK_LOW();
}

unsigned char LCD_ReadByte(void) {
  unsigned int i;
  unsigned char ReceiveData = 0;

  LCD_MOSI_HIGH();
  LCD_MOSI_AS_INPUT();
  for (i = 0; i < 8; i++) {
    LCD_SCK_LOW();
    lcdDelay();
    ReceiveData <<= 1;
    LCD_SCK_HIGH();
    lcdDelay();
    if (LCD_READ_DATA_PIN()) {
      ReceiveData |= 0x01;
    }
  }
  LCD_SCK_LOW();
  LCD_MOSI_AS_OUTPUT();
  return (ReceiveData);
}

unsigned char LCD_ReadRegister(unsigned char Register) {
  unsigned char ReadData = 0;
  LCD_CS_LOW();
  lcdDelay();

  lcdWriteByte(0, Register);
  lcdDelay();
  lcdDelay();
  ReadData = LCD_ReadByte();
  LCD_CS_HIGH();
  return (ReadData);
}

void lcdWriteCommand(uint8_t command) {
  lcdWriteByte(0, command);
}

void lcdWriteData(uint8_t data) {
  lcdWriteByte(1, data);
}

void LCD_ST7365_On(void) {
  LCD_CS_LOW();
  delay_ms(1);
  lcdWriteCommand(0x29);
  LCD_CS_HIGH();
}

void LCD_ST7365_Init(void) {

  LCD_CS_LOW();
  delay_ms(1);
  lcdWriteCommand( 0x11 );
  delay_ms(120);

  lcdWriteCommand( 0xF0 );
  lcdWriteData( 0xC3 );

  lcdWriteCommand( 0xF0 );
  lcdWriteData( 0x96 );


  lcdWriteCommand(0x11);

  delay_ms(120);

  lcdWriteCommand(0xF0);     // Command Set Control
  lcdWriteData(0xC3);

  lcdWriteCommand(0xF0);
  lcdWriteData(0x96);

  // Memory Data Access Control: native portrait order (MV=0), so that the
  // RGB data is written along the lines the panel refreshes (see above).
  // MX=1, MY=0 shows the frame buffer rotated 90 degrees from the former
  // landscape setting (0xE8: MY, MX, MV), with lines written top to bottom
  // like the panel refresh. BGR=1.
  lcdWriteCommand(0x36);
  lcdWriteData(0x48);

  lcdWriteCommand(0x2A); // 0 .. 320-1
  lcdWriteData(0x00);
  lcdWriteData(0x00);
  lcdWriteData(0x01);
  lcdWriteData(0x3F);

  lcdWriteCommand(0x2B); // 0 .. 480-1
  lcdWriteData(0x00);
  lcdWriteData(0x00);
  lcdWriteData(0x01);
  lcdWriteData(0xDF);


  lcdWriteCommand(0x3A);
  lcdWriteData(0x66);   //55:RGB565 66:RGB666 77:RGB888

  lcdWriteCommand(0xB4);
  lcdWriteData(0x01);

  lcdWriteCommand(0xB6);    //RGB
  lcdWriteData(0x20);
  lcdWriteData(0x02);     //02
  lcdWriteData(0x3B);

  lcdWriteCommand(0xB7);
  lcdWriteData(0xC6);

  lcdWriteCommand(0xC0);
  lcdWriteData(0x80);
  lcdWriteData(0x45);

  lcdWriteCommand(0xC1);
  lcdWriteData(0x0F );   //18  //00

  lcdWriteCommand(0xC2);
  lcdWriteData(0xA7);   //A7

  lcdWriteCommand(0xC5);
  lcdWriteData(0x0A);

  lcdWriteCommand(0xE8);
  lcdWriteData(0x40);
  lcdWriteData(0x8A);
  lcdWriteData(0x00);
  lcdWriteData(0x00);
  lcdWriteData(0x29);
  lcdWriteData(0x19);
  lcdWriteData(0xA5);
  lcdWriteData(0x33);

  lcdWriteCommand(0xE0);
  lcdWriteData(0xD0);
  lcdWriteData(0x08);
  lcdWriteData(0x0F);
  lcdWriteData(0x06);
  lcdWriteData(0x06);
  lcdWriteData(0x33);
  lcdWriteData(0x30);
  lcdWriteData(0x33);
  lcdWriteData(0x47);
  lcdWriteData(0x17);
  lcdWriteData(0x13);
  lcdWriteData(0x13);
  lcdWriteData(0x2B);
  lcdWriteData(0x31);

  lcdWriteCommand(0xE1);
  lcdWriteData(0xD0);
  lcdWriteData(0x0A);
  lcdWriteData(0x11);
  lcdWriteData(0x0B);
  lcdWriteData(0x09);
  lcdWriteData(0x07);
  lcdWriteData(0x2F);
  lcdWriteData(0x33);
  lcdWriteData(0x47);
  lcdWriteData(0x38);
  lcdWriteData(0x15);
  lcdWriteData(0x16);
  lcdWriteData(0x2C);
  lcdWriteData(0x32);


  lcdWriteCommand(0xF0);
  lcdWriteData(0x3C);

  lcdWriteCommand(0xF0);
  lcdWriteData(0x69);

  delay_ms(120);

  lcdWriteCommand(0x21);
  LCD_CS_HIGH();
  delay_ms(1);
  LCD_ST7365_On();
}

void LCD_ST7365_Off(void) {
  LCD_CS_LOW();
  delay_ms(1);
  lcdWriteCommand(0x28);
  LCD_CS_HIGH();
}

void LCD_Init_LTDC() {
  hltdc.Instance = LTDC;

  /* Configure PLLSAI prescalers for LCD */
  /* PLLSAI_VCO Input = HSE_VALUE/PLL_M = 1 Mhz */
  /* PLLSAI_VCO Output = PLLSAI_VCO Input * lcdPixelclock * 16 = XX Mhz */
  /* PLLLCDCLK = PLLSAI_VCO Output/PLL_LTDC = PLLSAI_VCO/4 = YY Mhz */
  /* LTDC clock frequency = PLLLCDCLK / RCC_PLLSAIDivR = YY/4 = lcdPixelClock Mhz */
//  uint32_t clock = (lcdPixelClock*16) / 1000000; // clock*16 in MHz
//  RCC_PeriphCLKInitTypeDef clkConfig;
//  clkConfig.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
//  clkConfig.PLLI2S
//  clkConfig.PLLSAI.PLLSAIN = clock;
//  clkConfig.PLLSAI.PLLSAIR = 4;
//  clkConfig.PLLSAIDivQ = 6;
//  clkConfig.PLLSAIDivR = RCC_PLLSAIDIVR_4;
//  HAL_RCCEx_PeriphCLKConfig(&clkConfig);

  /* LTDC Configuration *********************************************************/
  /* Polarity configuration */
  /* Initialize the horizontal synchronization polarity as active low */
  hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
  /* Initialize the vertical synchronization polarity as active low */
  hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
  /* Initialize the data enable polarity as active low */
  hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
  /* Initialize the pixel clock polarity as input pixel clock */
  hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IIPC;

  /* Configure R,G,B component values for LCD background color */
  hltdc.Init.Backcolor.Red = 0;
  hltdc.Init.Backcolor.Green = 0;
  hltdc.Init.Backcolor.Blue = 0;

  /* Configure horizontal synchronization width */
  hltdc.Init.HorizontalSync = HSW-1;
  /* Configure vertical synchronization height */
  hltdc.Init.VerticalSync = VSH-1;
  /* Configure accumulated horizontal back porch */
  hltdc.Init.AccumulatedHBP = HSW+HBP-1;
  /* Configure accumulated vertical back porch */
  hltdc.Init.AccumulatedVBP = VSH+VBP-1;
  /* Configure accumulated active width */
  hltdc.Init.AccumulatedActiveW = LCD_PHYS_W + HBP+HSW-1;
  /* Configure accumulated active height */
  hltdc.Init.AccumulatedActiveH = LCD_PHYS_H + VBP+VSH-1;
  /* Configure total width */
  hltdc.Init.TotalWidth = LCD_PHYS_W + HBP + HFP + HSW -1;
  /* Configure total height */
  hltdc.Init.TotalHeigh = LCD_PHYS_H + VBP + VFP+VSH-1;

  HAL_LTDC_Init(&hltdc);
}

void LCD_LayerInit() {
  auto& layer = hltdc.LayerCfg[0];

  /* Windowing configuration */
  layer.WindowX0 = 0;
  layer.WindowX1 = LCD_PHYS_W;
  layer.WindowY0 = 0;
  layer.WindowY1 = LCD_PHYS_H;

  /* Pixel Format configuration*/
  layer.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
  
  /* Alpha constant (255 totally opaque) */
  layer.Alpha = 255;

  /* Default Color configuration (configure A,R,G,B component values) */
  layer.Backcolor.Blue = 0;
  layer.Backcolor.Green = 0;
  layer.Backcolor.Red = 0;
  layer.Alpha0 = 0;

  /* Configure blending factors */
  layer.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
  layer.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;

  layer.ImageWidth = LCD_PHYS_W;
  layer.ImageHeight = LCD_PHYS_H;

  /* Start Address configuration : the LCD Frame buffer is defined on SDRAM w/ Offset */
  layer.FBStartAdress = (intptr_t)_front_buffer;

  /* Initialize LTDC layer 1 */
  HAL_LTDC_ConfigLayer(&hltdc, &hltdc.LayerCfg[0], 0);

  /* dithering activation */
  HAL_LTDC_EnableDither(&hltdc);
}

// LVGL's buffer is never scanned out: the LTDC shows the driver's own buffers
extern "C"
void lcdSetInitalFrameBuffer(void* fbAddress)
{
  (void)fbAddress;
}

const char* boardLcdType = "";

extern "C"
void lcdInit(void)
{
#if defined(LCD_SPI_CONFLICTS_WITH_QSPI)
  stm32_qspi_nor_deinit();
#endif

  memset(_LCD_BUF_1, 0, sizeof(_LCD_BUF_1));
  memset(_LCD_BUF_2, 0, sizeof(_LCD_BUF_2));
  SCB_CleanDCache();

  /* Configure the LCD SPI+RESET pins */
  lcdSpiConfig();

  /* Reset the LCD --------------------------------------------------------*/
  lcdReset();

  /* Configure the LCD Control pins */
  LCD_AF_GPIOConfig();

  /* Send LCD initialization commands */
  TRACE("LCD INIT (default): ST7365");
  boardLcdType = "ST7365 (Default)";

  __HAL_RCC_LTDC_CLK_ENABLE();
  LCD_ST7365_Init();

  LCD_Init_LTDC();
  LCD_LayerInit();

  // Enable LCD display
  __HAL_LTDC_ENABLE(&hltdc);

  lcdSetFlushCb(startLcdRefresh);

#if defined(LCD_SPI_CONFLICTS_WITH_QSPI)
  stm32_qspi_nor_init();
  stm32_qspi_nor_memory_mapped();
#endif
}
