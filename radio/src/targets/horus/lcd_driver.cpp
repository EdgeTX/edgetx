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

#include "opentx_types.h"
#include "libopenui_config.h"
#include "lcd.h"
#include <lvgl/lvgl.h>

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

#if defined(LCD_VERTICAL_INVERT)
static pixel_t _LCD_BUF_1[DISPLAY_BUFFER_SIZE] __SDRAM;
static pixel_t _LCD_BUF_2[DISPLAY_BUFFER_SIZE] __SDRAM;

static pixel_t _line_buffer[LCD_W];

static uint16_t* _front_buffer = _LCD_BUF_1;
static uint16_t* _back_buffer = _LCD_BUF_2;

// Copy 2 pixels at once to speed up a little
static void _copy_rotate_180(uint16_t* dst, uint16_t* src, const rect_t& copy_area)
{
  coord_t x1 = LCD_W - copy_area.w - copy_area.x;
  coord_t y1 = LCD_H - copy_area.h - copy_area.y;

  auto total = copy_area.w * copy_area.h;
  uint16_t* px_src = src + total - 2;

  dst += y1 * LCD_W + x1;
  
  for (auto line = 0; line < copy_area.h; line++) {

    // invert line into _line_buffer first (SRAM)
    auto px_dst = _line_buffer;

    auto line_end = px_dst + (copy_area.w & ~1);
    while (px_dst != line_end) {
      uint32_t* px2_src = (uint32_t*)px_src;
      uint32_t* px2_dst = (uint32_t*)px_dst;

      uint32_t px = ((*px2_src & 0xFFFF0000) >> 16) | ((*px2_src & 0xFFFF) << 16);
      *px2_dst = px;

      px_src -= 2;
      px_dst += 2;
    }

    if (copy_area.w & 1) {
      *px_dst = *(px_src+1);
      px_src--;
    }

    // ... and DMA back into SDRAM
    DMACopyBitmap(dst, copy_area.w, 1, 0, 0,
                  _line_buffer, copy_area.w, 1, 0, 0,
                  copy_area.w, 1);
    
    dst += LCD_W;
  }
}

static void _rotate_area_180(lv_area_t& area)
{
  lv_coord_t tmp_coord;
  tmp_coord = area.y2;
  area.y2 = LCD_H - area.y1 - 1;
  area.y1 = LCD_H - tmp_coord - 1;
  tmp_coord = area.x2;
  area.x2 = LCD_W - area.x1 - 1;
  area.x1 = LCD_W - tmp_coord - 1;
}
#endif

static volatile uint8_t _frame_addr_reloaded = 0;

static void _update_frame_buffer_addr(uint16_t* addr)
{
  LTDC_Layer1->CFBAR = (uint32_t)addr;

  // reload shadow registers on vertical blank
  _frame_addr_reloaded = 0;
  LTDC->SRCR = LTDC_SRCR_VBR;

  // wait for reload
  // TODO: replace through some smarter mechanism without busy wait
  while(_frame_addr_reloaded == 0);
}

static void startLcdRefresh(lv_disp_drv_t *disp_drv, uint16_t *buffer,
                            const rect_t &copy_area)
{
#if defined(LCD_VERTICAL_INVERT)
  _copy_rotate_180(_back_buffer, buffer, copy_area);

  if (lv_disp_flush_is_last(disp_drv)) {

    // swap back/front
    if (_front_buffer == _LCD_BUF_1) {
      _front_buffer = _LCD_BUF_2;
      _back_buffer = _LCD_BUF_1;
    } else {
      _front_buffer = _LCD_BUF_1;
      _back_buffer = _LCD_BUF_2;
    }

    // Trigger async refresh
    _update_frame_buffer_addr(_front_buffer);

    // Copy refreshed & rotated areas into new back buffer
    uint16_t* src = _front_buffer;
    uint16_t* dst = _back_buffer;

    lv_disp_t* disp = _lv_refr_get_disp_refreshing();
    for(int i = 0; i < disp->inv_p; i++) {
      if(disp->inv_area_joined[i]) continue;

      lv_area_t refr_area;
      lv_area_copy(&refr_area, &disp->inv_areas[i]);

      TRACE("{%d,%d,%d,%d}", refr_area.x1,
            refr_area.y1, refr_area.x2, refr_area.y2);

      _rotate_area_180(refr_area);

      auto area_w = refr_area.x2 - refr_area.x1 + 1;
      auto area_h = refr_area.y2 - refr_area.y1 + 1;
      
      DMACopyBitmap(dst, LCD_W, LCD_H, refr_area.x1, refr_area.y1,
                    src, LCD_W, LCD_H, refr_area.x1, refr_area.y1,
                    area_w, area_h);
    }
  }
  lv_disp_flush_ready(disp_drv);
#else
  // Direct mode
  _update_frame_buffer_addr(buffer);
#endif
}

inline void LCD_NRST_LOW()
{
  LCD_GPIO_NRST->BSRRH = LCD_GPIO_PIN_NRST;
}

inline void LCD_NRST_HIGH()
{
  LCD_GPIO_NRST->BSRRL = LCD_GPIO_PIN_NRST;
}

static void LCD_AF_GPIOConfig()
{
  GPIO_InitTypeDef GPIO_InitStructure;

      /* GPIOs Configuration */
      /*
       +------------------------+-----------------------+----------------------------+
       +                       LCD pins assignment                                   +
       +------------------------+-----------------------+----------------------------
       |                                       |  LCD_TFT G2 <-> PJ.09 |                                            |
       |  LCD_TFT R3 <-> PJ.02  |  LCD_TFT G3 <-> PJ.10 |  LCD_TFT B3 <-> PJ.15      |
       |  LCD_TFT R4 <-> PJ.03  |  LCD_TFT G4 <-> PJ.11 |  LCD_TFT B4 <-> PK.03      |
       |  LCD_TFT R5 <-> PJ.04  |  LCD_TFT G5 <-> PK.00 |  LCD_TFT B5 <-> PK.04      |
       |  LCD_TFT R6 <-> PJ.05  |  LCD_TFT G6 <-> PK.01 |  LCD_TFT B6 <-> PK.05      |
       |  LCD_TFT R7 <-> PJ.06  |  LCD_TFT G7 <-> PK.02 |  LCD_TFT B7 <-> PK.06      |
       -------------------------------------------------------------------------------
                |  LCD_TFT HSYNC <-> PI.12  | LCDTFT VSYNC <->  PI.13 |
                |  LCD_TFT CLK   <-> PI.14  | LCD_TFT DE   <->  PK.07 ///
                 -----------------------------------------------------
                | LCD_CS <-> PI.10    |LCD_SCK<->PI.11
                 -----------------------------------------------------
  */
  // GPIOI configuration
  GPIO_PinAFConfig(GPIOI, GPIO_PinSource12, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOI, GPIO_PinSource13, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOI, GPIO_PinSource14, GPIO_AF_LTDC);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14;
  GPIO_InitStructure.GPIO_Speed =GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode =GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType =GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd =GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOI, &GPIO_InitStructure);

  GPIO_PinAFConfig(GPIOK, GPIO_PinSource7, GPIO_AF_LTDC);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
  GPIO_Init(GPIOK, &GPIO_InitStructure);

  // GPIOJ configuration
  GPIO_PinAFConfig(GPIOJ, GPIO_PinSource2, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOJ, GPIO_PinSource3, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOJ, GPIO_PinSource4, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOJ, GPIO_PinSource5, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOJ, GPIO_PinSource6, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOJ, GPIO_PinSource9, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOJ, GPIO_PinSource10, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOJ, GPIO_PinSource11, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOJ, GPIO_PinSource15, GPIO_AF_LTDC);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | \
                               GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_15;

  GPIO_Init(GPIOJ, &GPIO_InitStructure);

  // GPIOK configuration
  GPIO_PinAFConfig(GPIOK, GPIO_PinSource0, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOK, GPIO_PinSource1, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOK, GPIO_PinSource2, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOK, GPIO_PinSource3, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOK, GPIO_PinSource4, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOK, GPIO_PinSource5, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOK, GPIO_PinSource6, GPIO_AF_LTDC);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 ;

  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOK, &GPIO_InitStructure);
}

static void LCD_NRSTConfig(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = LCD_GPIO_PIN_NRST;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(LCD_GPIO_NRST, &GPIO_InitStructure);
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
  LTDC_InitTypeDef LTDC_InitStruct;

  /* Configure PLLSAI prescalers for LCD */
  /* PLLSAI_VCO Input = HSE_VALUE/PLL_M = 1 MHz */
  /* PLLSAI_VCO Output = PLLSAI_VCO Input * PLLSAI_N = 192 MHz */
  /* PLLLCDCLK = PLLSAI_VCO Output/PLL_LTDC = 192/3 = 64 MHz */
  /* LTDC clock frequency = PLLLCDCLK / RCC_PLLSAIDivR = 64/4 = 16 MHz */
  /* alternatively LTDC clock frequency = PLLLCDCLK / RCC_PLLSAIDivR = 64/8 = 8 MHz */
  //second pam is for audio
  //third pam is for LCD
  RCC_PLLSAIConfig(192, 6, 3);
  #if defined(RADIO_TX16S)
    RCC_LTDCCLKDivConfig(RCC_PLLSAIDivR_Div8);
  #else
    RCC_LTDCCLKDivConfig(RCC_PLLSAIDivR_Div4);
  #endif
  /* Enable PLLSAI Clock */
  RCC_PLLSAICmd(ENABLE);

  /* Wait for PLLSAI activation */
  while(RCC_GetFlagStatus(RCC_FLAG_PLLSAIRDY) == RESET)
  {
  }

  /* LTDC Configuration *********************************************************/
  /* Polarity configuration */
  /* Initialize the horizontal synchronization polarity as active low */
  LTDC_InitStruct.LTDC_HSPolarity = LTDC_HSPolarity_AL;
  /* Initialize the vertical synchronization polarity as active low */
  LTDC_InitStruct.LTDC_VSPolarity = LTDC_VSPolarity_AL;
  /* Initialize the data enable polarity as active low */
  LTDC_InitStruct.LTDC_DEPolarity = LTDC_DEPolarity_AL;
  /* Initialize the pixel clock polarity as input pixel clock */
  LTDC_InitStruct.LTDC_PCPolarity = LTDC_PCPolarity_IPC;

  /* Configure R,G,B component values for LCD background color */
  LTDC_InitStruct.LTDC_BackgroundRedValue = 0;
  LTDC_InitStruct.LTDC_BackgroundGreenValue = 0;
  LTDC_InitStruct.LTDC_BackgroundBlueValue = 0;

  /* Configure horizontal synchronization width */
  LTDC_InitStruct.LTDC_HorizontalSync = HSW;
  /* Configure vertical synchronization height */
  LTDC_InitStruct.LTDC_VerticalSync = VSW;
  /* Configure accumulated horizontal back porch */
  LTDC_InitStruct.LTDC_AccumulatedHBP = HBP;
  /* Configure accumulated vertical back porch */
  LTDC_InitStruct.LTDC_AccumulatedVBP = VBP;
  /* Configure accumulated active width */
  LTDC_InitStruct.LTDC_AccumulatedActiveW = LCD_PHYS_W + HBP;
  /* Configure accumulated active height */
  LTDC_InitStruct.LTDC_AccumulatedActiveH = LCD_PHYS_H + VBP;
  /* Configure total width */
  LTDC_InitStruct.LTDC_TotalWidth = LCD_PHYS_W + HBP + HFP;
  /* Configure total height */
  LTDC_InitStruct.LTDC_TotalHeigh = LCD_PHYS_H + VBP + VFP;

  LTDC_Init(&LTDC_InitStruct);

  // Configure IRQ (line)
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = LTDC_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = LTDC_IRQ_PRIO;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init( &NVIC_InitStructure );

  // Trigger on last line
  LTDC_LIPConfig(LCD_PHYS_H);
  LTDC_ITConfig(LTDC_IER_LIE, ENABLE);

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
  LTDC_Layer_InitTypeDef LTDC_Layer_InitStruct;

  /* Windowing configuration */
  /* In this case all the active display area is used to display a picture then :
  Horizontal start = horizontal synchronization + Horizontal back porch = 30
  Horizontal stop = Horizontal start + window width -1 = 30 + 240 -1
  Vertical start   = vertical synchronization + vertical back porch     = 4
  Vertical stop   = Vertical start + window height -1  = 4 + 320 -1      */
  LTDC_Layer_InitStruct.LTDC_HorizontalStart = HBP + 1;
  LTDC_Layer_InitStruct.LTDC_HorizontalStop = (LCD_PHYS_W + HBP);
  LTDC_Layer_InitStruct.LTDC_VerticalStart = VBP + 1;;
  LTDC_Layer_InitStruct.LTDC_VerticalStop = (LCD_PHYS_H + VBP);

  /* Pixel Format configuration*/
  LTDC_Layer_InitStruct.LTDC_PixelFormat = LTDC_Pixelformat_RGB565;

  /* Alpha constant (255 totally opaque) */
  LTDC_Layer_InitStruct.LTDC_ConstantAlpha = 255;

  /* Default Color configuration (configure A,R,G,B component values) */
  LTDC_Layer_InitStruct.LTDC_DefaultColorBlue = 0;
  LTDC_Layer_InitStruct.LTDC_DefaultColorGreen = 0;
  LTDC_Layer_InitStruct.LTDC_DefaultColorRed = 0;
  LTDC_Layer_InitStruct.LTDC_DefaultColorAlpha = 0;

  /* Configure blending factors */
  LTDC_Layer_InitStruct.LTDC_BlendingFactor_1 = LTDC_BlendingFactor1_CA;
  LTDC_Layer_InitStruct.LTDC_BlendingFactor_2 = LTDC_BlendingFactor2_CA;

  /* the length of one line of pixels in bytes + 3 then :
  Line Lenth = Active high width x number of bytes per pixel + 3
  Active high width         = LCD_W
  number of bytes per pixel = 2    (pixel_format : RGB565)
  */
  LTDC_Layer_InitStruct.LTDC_CFBLineLength = ((LCD_PHYS_W * 2) + 3);
  /* the pitch is the increment from the start of one line of pixels to the
  start of the next line in bytes, then :
  Pitch = Active high width x number of bytes per pixel */
  LTDC_Layer_InitStruct.LTDC_CFBPitch = (LCD_PHYS_W * 2);

  /* Configure the number of lines */
  LTDC_Layer_InitStruct.LTDC_CFBLineNumber = LCD_PHYS_H;

  /* Start Address configuration : the LCD Frame buffer is defined on SDRAM w/ Offset */
#if defined(LCD_VERTICAL_INVERT)
  uint32_t layer_address = (uint32_t)_LCD_BUF_1;
#else
  uint32_t layer_address = (uint32_t)lcdFront->getData();
#endif
  LTDC_Layer_InitStruct.LTDC_CFBStartAdress = layer_address;

  /* Initialize LTDC layer 1 */
  LTDC_LayerInit(LTDC_Layer1, &LTDC_Layer_InitStruct);

  /* LTDC configuration reload */
  LTDC_ReloadConfig(LTDC_IMReload);

  // Enable layer and reload
  LTDC_LayerCmd(LTDC_Layer1, ENABLE);
  LTDC_ReloadConfig(LTDC_IMReload);
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

void lcdInit()
{
#if defined(LCD_VERTICAL_INVERT)
  // Clear buffer first
  memset(_LCD_BUF_1, 0, sizeof(_LCD_BUF_1));
  memset(_LCD_BUF_2, 0, sizeof(_LCD_BUF_2));
#endif

  // Initialize the LCD
  LCD_Init();
  LCD_LayerInit();

  // Enable LCD display
  LTDC_Cmd(ENABLE);

  lcdSetFlushCb(startLcdRefresh);
}

void DMAWait()
{
  while(DMA2D->CR & DMA2D_CR_START);
}

void DMACopyBitmap(uint16_t *dest, uint16_t destw, uint16_t desth, uint16_t x,
                   uint16_t y, const uint16_t *src, uint16_t srcw,
                   uint16_t srch, uint16_t srcx, uint16_t srcy, uint16_t w,
                   uint16_t h)
{
  DMAWait();
  DMA2D_DeInit();

  DMA2D_InitTypeDef DMA2D_InitStruct;
  DMA2D_InitStruct.DMA2D_Mode = DMA2D_M2M;
  DMA2D_InitStruct.DMA2D_CMode = DMA2D_RGB565;
  DMA2D_InitStruct.DMA2D_OutputMemoryAdd = CONVERT_PTR_UINT(dest + y*destw + x);
  DMA2D_InitStruct.DMA2D_OutputGreen = 0;
  DMA2D_InitStruct.DMA2D_OutputBlue = 0;
  DMA2D_InitStruct.DMA2D_OutputRed = 0;
  DMA2D_InitStruct.DMA2D_OutputAlpha = 0;
  DMA2D_InitStruct.DMA2D_OutputOffset = destw - w;
  DMA2D_InitStruct.DMA2D_NumberOfLine = h;
  DMA2D_InitStruct.DMA2D_PixelPerLine = w;
  DMA2D_Init(&DMA2D_InitStruct);

  DMA2D_FG_InitTypeDef DMA2D_FG_InitStruct;
  DMA2D_FG_StructInit(&DMA2D_FG_InitStruct);
  DMA2D_FG_InitStruct.DMA2D_FGMA = CONVERT_PTR_UINT(src + srcy*srcw + srcx);
  DMA2D_FG_InitStruct.DMA2D_FGO = srcw - w;
  DMA2D_FG_InitStruct.DMA2D_FGCM = CM_RGB565;
  DMA2D_FG_InitStruct.DMA2D_FGPFC_ALPHA_MODE = NO_MODIF_ALPHA_VALUE;
  DMA2D_FG_InitStruct.DMA2D_FGPFC_ALPHA_VALUE = 0;
  DMA2D_FGConfig(&DMA2D_FG_InitStruct);

  /* Start Transfer */
  DMA2D_StartTransfer();
}

void DMACopyAlphaBitmap(uint16_t *dest, uint16_t destw, uint16_t desth,
                        uint16_t x, uint16_t y, const uint16_t *src,
                        uint16_t srcw, uint16_t srch, uint16_t srcx,
                        uint16_t srcy, uint16_t w, uint16_t h)
{
  DMAWait();
  DMA2D_DeInit();

  DMA2D_InitTypeDef DMA2D_InitStruct;
  DMA2D_InitStruct.DMA2D_Mode = DMA2D_M2M_BLEND;
  DMA2D_InitStruct.DMA2D_CMode = DMA2D_RGB565;
  DMA2D_InitStruct.DMA2D_OutputMemoryAdd = CONVERT_PTR_UINT(dest + y*destw + x);
  DMA2D_InitStruct.DMA2D_OutputGreen = 0;
  DMA2D_InitStruct.DMA2D_OutputBlue = 0;
  DMA2D_InitStruct.DMA2D_OutputRed = 0;
  DMA2D_InitStruct.DMA2D_OutputAlpha = 0;
  DMA2D_InitStruct.DMA2D_OutputOffset = destw - w;
  DMA2D_InitStruct.DMA2D_NumberOfLine = h;
  DMA2D_InitStruct.DMA2D_PixelPerLine = w;
  DMA2D_Init(&DMA2D_InitStruct);

  DMA2D_FG_InitTypeDef DMA2D_FG_InitStruct;
  DMA2D_FG_StructInit(&DMA2D_FG_InitStruct);
  DMA2D_FG_InitStruct.DMA2D_FGMA = CONVERT_PTR_UINT(src + srcy*srcw + srcx);
  DMA2D_FG_InitStruct.DMA2D_FGO = srcw - w;
  DMA2D_FG_InitStruct.DMA2D_FGCM = CM_ARGB4444;
  DMA2D_FG_InitStruct.DMA2D_FGPFC_ALPHA_MODE = NO_MODIF_ALPHA_VALUE;
  DMA2D_FG_InitStruct.DMA2D_FGPFC_ALPHA_VALUE = 0;
  DMA2D_FGConfig(&DMA2D_FG_InitStruct);

  DMA2D_BG_InitTypeDef DMA2D_BG_InitStruct;
  DMA2D_BG_StructInit(&DMA2D_BG_InitStruct);
  DMA2D_BG_InitStruct.DMA2D_BGMA = CONVERT_PTR_UINT(dest + y*destw + x);
  DMA2D_BG_InitStruct.DMA2D_BGO = destw - w;
  DMA2D_BG_InitStruct.DMA2D_BGCM = CM_RGB565;
  DMA2D_BG_InitStruct.DMA2D_BGPFC_ALPHA_MODE = NO_MODIF_ALPHA_VALUE;
  DMA2D_BG_InitStruct.DMA2D_BGPFC_ALPHA_VALUE = 0;
  DMA2D_BGConfig(&DMA2D_BG_InitStruct);

  /* Start Transfer */
  DMA2D_StartTransfer();
}

// same as DMACopyAlphaBitmap(), but with an 8 bit mask for each pixel (used by fonts)
void DMACopyAlphaMask(uint16_t *dest, uint16_t destw, uint16_t desth,
                      uint16_t x, uint16_t y, const uint8_t *src, uint16_t srcw,
                      uint16_t srch, uint16_t srcx, uint16_t srcy, uint16_t w,
                      uint16_t h, uint16_t bg_color)
{
  DMAWait();
  DMA2D_DeInit();

  DMA2D_InitTypeDef DMA2D_InitStruct;
  DMA2D_InitStruct.DMA2D_Mode = DMA2D_M2M_BLEND;
  DMA2D_InitStruct.DMA2D_CMode = CM_RGB565;
  DMA2D_InitStruct.DMA2D_OutputMemoryAdd = CONVERT_PTR_UINT(dest + y*destw + x);
  DMA2D_InitStruct.DMA2D_OutputBlue = 0;
  DMA2D_InitStruct.DMA2D_OutputGreen = 0;
  DMA2D_InitStruct.DMA2D_OutputRed = 0;
  DMA2D_InitStruct.DMA2D_OutputAlpha = 0;
  DMA2D_InitStruct.DMA2D_OutputOffset = destw - w;
  DMA2D_InitStruct.DMA2D_NumberOfLine = h;
  DMA2D_InitStruct.DMA2D_PixelPerLine = w;
  DMA2D_Init(&DMA2D_InitStruct);

  DMA2D_FG_InitTypeDef DMA2D_FG_InitStruct;
  DMA2D_FG_StructInit(&DMA2D_FG_InitStruct);
  DMA2D_FG_InitStruct.DMA2D_FGMA = CONVERT_PTR_UINT(src + srcy*srcw + srcx);
  DMA2D_FG_InitStruct.DMA2D_FGO = srcw - w;
  DMA2D_FG_InitStruct.DMA2D_FGCM = CM_A8; // 8 bit inputs every time
  DMA2D_FG_InitStruct.DMA2D_FGPFC_ALPHA_MODE = NO_MODIF_ALPHA_VALUE;
  DMA2D_FG_InitStruct.DMA2D_FGPFC_ALPHA_VALUE = 0;
  DMA2D_FG_InitStruct.DMA2D_FGC_RED   = GET_RED(bg_color);   // 8 bit red
  DMA2D_FG_InitStruct.DMA2D_FGC_GREEN = GET_GREEN(bg_color); // 8 bit green
  DMA2D_FG_InitStruct.DMA2D_FGC_BLUE  = GET_BLUE(bg_color);  // 8 bit blue
  
  DMA2D_FGConfig(&DMA2D_FG_InitStruct);

  DMA2D_BG_InitTypeDef DMA2D_BG_InitStruct;
  DMA2D_BG_StructInit(&DMA2D_BG_InitStruct);
  DMA2D_BG_InitStruct.DMA2D_BGMA = CONVERT_PTR_UINT(dest + y*destw + x);
  DMA2D_BG_InitStruct.DMA2D_BGO = destw - w;
  DMA2D_BG_InitStruct.DMA2D_BGCM = CM_RGB565;
  DMA2D_BG_InitStruct.DMA2D_BGPFC_ALPHA_MODE = NO_MODIF_ALPHA_VALUE;
  DMA2D_BG_InitStruct.DMA2D_BGPFC_ALPHA_VALUE = 0;
  DMA2D_BGConfig(&DMA2D_BG_InitStruct);

  /* Start Transfer */
  DMA2D_StartTransfer();
}

void DMABitmapConvert(uint16_t * dest, const uint8_t * src, uint16_t w, uint16_t h, uint32_t format)
{
  DMA2D_DeInit();

  DMA2D_InitTypeDef DMA2D_InitStruct;
  DMA2D_InitStruct.DMA2D_Mode = DMA2D_M2M_PFC;
  DMA2D_InitStruct.DMA2D_CMode = format;
  DMA2D_InitStruct.DMA2D_OutputMemoryAdd = CONVERT_PTR_UINT(dest);
  DMA2D_InitStruct.DMA2D_OutputGreen = 0;
  DMA2D_InitStruct.DMA2D_OutputBlue = 0;
  DMA2D_InitStruct.DMA2D_OutputRed = 0;
  DMA2D_InitStruct.DMA2D_OutputAlpha = 0;
  DMA2D_InitStruct.DMA2D_OutputOffset = 0;
  DMA2D_InitStruct.DMA2D_NumberOfLine = h;
  DMA2D_InitStruct.DMA2D_PixelPerLine = w;
  DMA2D_Init(&DMA2D_InitStruct);

  DMA2D_FG_InitTypeDef DMA2D_FG_InitStruct;
  DMA2D_FG_StructInit(&DMA2D_FG_InitStruct);
  DMA2D_FG_InitStruct.DMA2D_FGMA = CONVERT_PTR_UINT(src);
  DMA2D_FG_InitStruct.DMA2D_FGO = 0;
  DMA2D_FG_InitStruct.DMA2D_FGCM = CM_ARGB8888;
  DMA2D_FG_InitStruct.DMA2D_FGPFC_ALPHA_MODE = REPLACE_ALPHA_VALUE;
  DMA2D_FG_InitStruct.DMA2D_FGPFC_ALPHA_VALUE = 0;
  DMA2D_FGConfig(&DMA2D_FG_InitStruct);

  /* Start Transfer */
  DMA2D_StartTransfer();
}

extern "C" void LTDC_IRQHandler(void)
{
  // clear interrupt flag
  LTDC->ICR = LTDC_ICR_CLIF;
  _frame_addr_reloaded = 1;
}

