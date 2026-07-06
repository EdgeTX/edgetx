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

#include "hal/gpio.h"
#include "stm32_gpio.h"
#include "stm32_hal_ll.h"
#include "stm32_hal.h"

#include "edgetx_types.h"
#include "dma2d.h"
#include "board.h"
#include "hal.h"
#include "delays_driver.h"

#include "lcd.h"
#include <lvgl/lvgl.h>
#include <vector>

#if defined(RADIO_T18)
  #define HBP  43
  #define VBP  12

  #define HSW  2
  #define VSW  4

  #define HFP  8
  #define VFP  8
#else
  #define HBP  42
  #define VBP  12

  #define HSW  2
  #define VSW  10

  #define HFP  3
  #define VFP  2
#endif

#define GPIO_AF_LTDC GPIO_AF14

static LTDC_HandleTypeDef hltdc;
static void* initialFrameBuffer = nullptr;

#if LCD_VERTICAL_INVERT
typedef uint16_t pixel_t;
static pixel_t _LCD_BUF_1[DISPLAY_BUFFER_SIZE] __SDRAM __ALIGNED(64);
#if defined(RADIO_F16)
// F16 can be either normal or inverted
static pixel_t _LCD_BUF_2[DISPLAY_BUFFER_SIZE] __SDRAM __ALIGNED(64);
#else
// LVGL will only use one buffer when display is inverted so reuse 2nd
// buffer here
extern pixel_t LCD_SECOND_FRAME_BUFFER[DISPLAY_BUFFER_SIZE];
#define _LCD_BUF_2 LCD_SECOND_FRAME_BUFFER
#endif

// Frame buffer pointers
static uint16_t* _front_buffer = _LCD_BUF_1;
static uint16_t* _back_buffer = _LCD_BUF_2;

// Vector to save areas that need to be copied from front buffer to back buffer
static std::vector<lv_area_t> dma_areas;

// Copy 2 pixels at once to speed up a little
static void _copy_rotate_180(uint16_t* dst, uint16_t* src, const rect_t& copy_area)
{
  static pixel_t _line_buffer[LCD_W];

  coord_t x1 = LCD_W - copy_area.w - copy_area.x;
  coord_t y1 = LCD_H - copy_area.h - copy_area.y;

  src += copy_area.w - 2;
  dst += (y1 + copy_area.h - 1) * LCD_W + x1;

  for (auto line = 0; line < copy_area.h; line++) {

    // invert line into _line_buffer first (SRAM)
    auto px_dst = _line_buffer;

    auto line_end = px_dst + (copy_area.w & ~1);
    while (px_dst != line_end) {
      uint32_t* px2_src = (uint32_t*)src;

      *((uint32_t*)px_dst) = ((*px2_src & 0xFFFF0000) >> 16) | ((*px2_src & 0xFFFF) << 16);

      src -= 2;
      px_dst += 2;
    }

    if (copy_area.w & 1) {
      *px_dst = *(src+1);
      src--;
    }

    // ... and DMA back into SDRAM
    DMACopyBitmap(dst, copy_area.w, 1, 0, 0,
                  _line_buffer, copy_area.w, 1, 0, 0,
                  copy_area.w, 1);

    src += copy_area.w * 2;
    dst -= LCD_W;
  }
}
#endif

static void _update_frame_buffer_addr(uint16_t* addr)
{
  LTDC_Layer1->CFBAR = (uint32_t)addr;
  // reload shadow registers on vertical blank
  LTDC->SRCR = LTDC_SRCR_VBR;

  __HAL_LTDC_ENABLE_IT(&hltdc, LTDC_IT_LI);
}

static void startLcdRefresh(lv_disp_drv_t *disp_drv, uint16_t *buffer,
                            const rect_t &copy_area)
{
#if LCD_VERTICAL_INVERT
  if (disp_drv->direct_mode) {
    // Direct mode / not inverted
    _update_frame_buffer_addr(buffer);
    return;
  }

  // Copy areas that were updated last frame to the back buffer
  if (dma_areas.size() > 0) {
    for (auto a: dma_areas) {
      DMACopyBitmap(_back_buffer, LCD_W, LCD_H, a.x1, a.y1,
                    _front_buffer, LCD_W, LCD_H, a.x1, a.y1,
                    a.x2 - a.x1 + 1, a.y2 - a.y1 + 1);
    }
    dma_areas.clear();
    DMAWait();
  }

  // Copy changes from LVGL to LCD back buffer with 180 degree rotation
  _copy_rotate_180(_back_buffer, buffer, copy_area);

  // Check for last LVGL update before we need to swap buffers
  if (lv_disp_flush_is_last(disp_drv)) {
    // swap back/front buffers
    if (_front_buffer == _LCD_BUF_1) {
      _front_buffer = _LCD_BUF_2;
      _back_buffer = _LCD_BUF_1;
    } else {
      _front_buffer = _LCD_BUF_1;
      _back_buffer = _LCD_BUF_2;
    }

    // Trigger async refresh
    _update_frame_buffer_addr(_front_buffer);

    // Save areas that need to be updated next time around
    lv_disp_t* disp = _lv_refr_get_disp_refreshing();
    for(int i = 0; i < disp->inv_p; i++) {
      if(disp->inv_area_joined[i]) continue;

      // Rotate area 180 degrees
      lv_area_t refr_area;
      refr_area.x1 = LCD_W - disp->inv_areas[i].x2 - 1;
      refr_area.x2 = LCD_W - disp->inv_areas[i].x1 - 1;
      refr_area.y1 = LCD_H - disp->inv_areas[i].y2 - 1;
      refr_area.y2 = LCD_H - disp->inv_areas[i].y1 - 1;

      dma_areas.push_back(refr_area);
    }
  } else {
    lv_disp_flush_ready(disp_drv);
  }
#else
  // Direct mode
  _update_frame_buffer_addr(buffer);
#endif
}

inline void LCD_NRST_LOW()
{
  gpio_clear(LCD_GPIO_NRST);
}

inline void LCD_NRST_HIGH()
{
  gpio_set(LCD_GPIO_NRST);
}

static void LCD_AF_GPIOConfig()
{
  /* GPIOs Configuration */
  /*
    +------------------------+-----------------------+----------------------------+
    +                       LCD pins assignment                                   +
    +------------------------+-----------------------+----------------------------+
    |                        |  LCD_TFT G2 <-> PJ.09 |                            |
    |  LCD_TFT R3 <-> PJ.02  |  LCD_TFT G3 <-> PJ.10 |  LCD_TFT B3 <-> PJ.15      |
    |  LCD_TFT R4 <-> PJ.03  |  LCD_TFT G4 <-> PJ.11 |  LCD_TFT B4 <-> PK.03      |
    |  LCD_TFT R5 <-> PJ.04  |  LCD_TFT G5 <-> PK.00 |  LCD_TFT B5 <-> PK.04      |
    |  LCD_TFT R6 <-> PJ.05  |  LCD_TFT G6 <-> PK.01 |  LCD_TFT B6 <-> PK.05      |
    |  LCD_TFT R7 <-> PJ.06  |  LCD_TFT G7 <-> PK.02 |  LCD_TFT B7 <-> PK.06      |
    -------------------------------------------------------------------------------
    |  LCD_TFT HSYNC <-> PI.12  | LCD_TFT VSYNC <->  PI.13 |
    |  LCD_TFT CLK   <-> PI.14  | LCD_TFT DE    <->  PK.07 |
    --------------------------------------------------------
    | LCD_CS <-> PI.10          | LCD_SCK<->PI.11          |
    --------------------------------------------------------
  */

  // GPIOI configuration
  gpio_init_af(GPIO_PIN(GPIOI, 12), GPIO_AF_LTDC, GPIO_PIN_SPEED_LOW);
  gpio_init_af(GPIO_PIN(GPIOI, 13), GPIO_AF_LTDC, GPIO_PIN_SPEED_LOW);
  gpio_init_af(GPIO_PIN(GPIOI, 14), GPIO_AF_LTDC, GPIO_PIN_SPEED_LOW);

  // GPIOJ configuration
  gpio_init_af(GPIO_PIN(GPIOJ, 2), GPIO_AF_LTDC, GPIO_PIN_SPEED_LOW);
  gpio_init_af(GPIO_PIN(GPIOJ, 3), GPIO_AF_LTDC, GPIO_PIN_SPEED_LOW);
  gpio_init_af(GPIO_PIN(GPIOJ, 4), GPIO_AF_LTDC, GPIO_PIN_SPEED_LOW);
  gpio_init_af(GPIO_PIN(GPIOJ, 5), GPIO_AF_LTDC, GPIO_PIN_SPEED_LOW);
  gpio_init_af(GPIO_PIN(GPIOJ, 6), GPIO_AF_LTDC, GPIO_PIN_SPEED_LOW);
  gpio_init_af(GPIO_PIN(GPIOJ, 9), GPIO_AF_LTDC, GPIO_PIN_SPEED_LOW);
  gpio_init_af(GPIO_PIN(GPIOJ, 10), GPIO_AF_LTDC, GPIO_PIN_SPEED_LOW);
  gpio_init_af(GPIO_PIN(GPIOJ, 11), GPIO_AF_LTDC, GPIO_PIN_SPEED_LOW);
  gpio_init_af(GPIO_PIN(GPIOJ, 15), GPIO_AF_LTDC, GPIO_PIN_SPEED_LOW);

  // GPIOK configuration
  gpio_init_af(GPIO_PIN(GPIOK, 0), GPIO_AF_LTDC, GPIO_PIN_SPEED_LOW);
  gpio_init_af(GPIO_PIN(GPIOK, 1), GPIO_AF_LTDC, GPIO_PIN_SPEED_LOW);
  gpio_init_af(GPIO_PIN(GPIOK, 2), GPIO_AF_LTDC, GPIO_PIN_SPEED_LOW);
  gpio_init_af(GPIO_PIN(GPIOK, 3), GPIO_AF_LTDC, GPIO_PIN_SPEED_LOW);
  gpio_init_af(GPIO_PIN(GPIOK, 4), GPIO_AF_LTDC, GPIO_PIN_SPEED_LOW);
  gpio_init_af(GPIO_PIN(GPIOK, 5), GPIO_AF_LTDC, GPIO_PIN_SPEED_LOW);
  gpio_init_af(GPIO_PIN(GPIOK, 6), GPIO_AF_LTDC, GPIO_PIN_SPEED_LOW);
  gpio_init_af(GPIO_PIN(GPIOK, 7), GPIO_AF_LTDC, GPIO_PIN_SPEED_LOW);
}

static void LCD_NRSTConfig(void)
{
  gpio_init(LCD_GPIO_NRST, GPIO_OUT, GPIO_PIN_SPEED_LOW);
}

static void lcdReset(void)
{
#if defined(RADIO_T18)     // T18 screen has issues if NRST is ever brought low
  LCD_NRST_HIGH();
#else
  LCD_NRST_HIGH();
  delay_ms(1);

  LCD_NRST_LOW(); //  RESET();
  delay_ms(20);

  LCD_NRST_HIGH();
  delay_ms(30);
#endif
}

void LCD_Init_LTDC()
{
  __HAL_RCC_LTDC_CLK_ENABLE();
  hltdc.Instance = LTDC;

  /* Configure PLLSAI prescalers for LCD */
  /* PLLSAI_VCO Input = HSE_VALUE/PLL_M = 1 MHz */
  /* PLLSAI_VCO Output = PLLSAI_VCO Input * PLLSAI_N = 192 MHz */
  /* PLLLCDCLK = PLLSAI_VCO Output/PLL_LTDC = 192/3 = 64 MHz */
  /* LTDC clock frequency = PLLLCDCLK / RCC_PLLSAIDivR = 64/4 = 16 MHz */
  /* alternatively LTDC clock frequency = PLLLCDCLK / RCC_PLLSAIDivR = 64/8 = 8 MHz */
  //second pam is for audio
  //third pam is for LCD
  RCC_PeriphCLKInitTypeDef clkConfig;
  clkConfig.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
  clkConfig.PLLSAI.PLLSAIN = 192;
  clkConfig.PLLSAI.PLLSAIR = 3;
  clkConfig.PLLSAIDivQ = 6;
  #if defined(RADIO_TX16S) || defined(RADIO_F16)
    clkConfig.PLLSAIDivR = RCC_PLLSAIDIVR_8;
  #else
    clkConfig.PLLSAIDivR = RCC_PLLSAIDIVR_4;
  #endif
  HAL_RCCEx_PeriphCLKConfig(&clkConfig);

  /* LTDC Configuration *********************************************************/
  /* Polarity configuration */
  /* Initialize the horizontal synchronization polarity as active low */
  hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
  /* Initialize the vertical synchronization polarity as active low */
  hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
  /* Initialize the data enable polarity as active low */
  hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
  /* Initialize the pixel clock polarity as input pixel clock */
  hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;

  /* Configure R,G,B component values for LCD background color */
  hltdc.Init.Backcolor.Red = 0;
  hltdc.Init.Backcolor.Green = 0;
  hltdc.Init.Backcolor.Blue = 0;

  /* Configure horizontal synchronization width */
  hltdc.Init.HorizontalSync = HSW;
  /* Configure vertical synchronization height */
  hltdc.Init.VerticalSync = VSW;
  /* Configure accumulated horizontal back porch */
  hltdc.Init.AccumulatedHBP = HBP;
  /* Configure accumulated vertical back porch */
  hltdc.Init.AccumulatedVBP = VBP;
  /* Configure accumulated active width */
  hltdc.Init.AccumulatedActiveW = LCD_PHYS_W + HBP;
  /* Configure accumulated active height */
  hltdc.Init.AccumulatedActiveH = LCD_PHYS_H + VBP;
  /* Configure total width */
  hltdc.Init.TotalWidth = LCD_PHYS_W + HBP + HFP;
  /* Configure total height */
  hltdc.Init.TotalHeigh = LCD_PHYS_H + VBP + VFP;

  HAL_LTDC_Init(&hltdc);

  // Configure IRQ (line)
  NVIC_SetPriority(LTDC_IRQn, LTDC_IRQ_PRIO);
  NVIC_EnableIRQ(LTDC_IRQn);

  // Trigger on last line
  HAL_LTDC_ProgramLineEvent(&hltdc, LCD_PHYS_H);

#if 0
  DMA2D_ITConfig(DMA2D_CR_TCIE, ENABLE);
  NVIC_InitStructure.NVIC_IRQChannel = DMA2D_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = DMA_SCREEN_IRQ_PRIO;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; /* Not used as 4 bits are used for the pr     e-emption priority. */;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init( &NVIC_InitStructure );

  DMA2D->IFCR = (unsigned long)DMA2D_IFSR_CTCIF;
#endif
}

void LCD_LayerInit()
{
  auto& layer = hltdc.LayerCfg[0];

  /* Windowing configuration */
  /* In this case all the active display area is used to display a picture */
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
#if LCD_VERTICAL_INVERT
  intptr_t layer_address;
#if defined(RADIO_F16)
  if (hardwareOptions.pcbrev > 0)
    layer_address = (intptr_t)initialFrameBuffer;
  else
#endif
    layer_address = (intptr_t)_LCD_BUF_1;
#else
  intptr_t layer_address = (intptr_t)initialFrameBuffer;
#endif
  layer.FBStartAdress = layer_address;

  HAL_LTDC_ConfigLayer(&hltdc, &hltdc.LayerCfg[0], 0);
}

extern "C"
void lcdSetInitalFrameBuffer(void* fbAddress)
{
  initialFrameBuffer = fbAddress;
}

void LCD_Init(void)
{
  /* Reset the LCD --------------------------------------------------------*/
  LCD_NRSTConfig();
  lcdReset();

  /* Configure the LCD Control pins */
  LCD_AF_GPIOConfig();

  // Enable LTDC IRQ in NVIC
  NVIC_EnableIRQ(LTDC_IRQn);
  NVIC_SetPriority(LTDC_IRQn, 7);
  
  LCD_Init_LTDC();
}

extern "C"
void lcdInit()
{
#if LCD_VERTICAL_INVERT
  // Clear buffer first
  memset(_LCD_BUF_1, 0, DISPLAY_BUFFER_SIZE * sizeof(pixel_t));
  memset(_LCD_BUF_2, 0, DISPLAY_BUFFER_SIZE * sizeof(pixel_t));
#endif

  // Initialize the LCD
  LCD_Init();
  LCD_LayerInit();

  // Enable LCD display
  __HAL_LTDC_ENABLE(&hltdc);

  lcdSetFlushCb(startLcdRefresh);
}

extern "C" void LTDC_IRQHandler(void)
{
  // clear interrupt flag
  __HAL_LTDC_CLEAR_FLAG(&hltdc, LTDC_FLAG_LI);
  __HAL_LTDC_DISABLE_IT(&hltdc, LTDC_IT_LI);

  lcdFlushed();
}
