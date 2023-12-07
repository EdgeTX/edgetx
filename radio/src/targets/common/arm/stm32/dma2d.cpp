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

#include "definitions.h"
#include "libopenui_defines.h"
#include "dma2d.h"

void DMACopyBitmap(uint16_t *dest, uint16_t destw, uint16_t desth, uint16_t x,
                   uint16_t y, const uint16_t *src, uint16_t srcw,
                   uint16_t srch, uint16_t srcx, uint16_t srcy, uint16_t w,
                   uint16_t h)
{
  DMAWait();

#if __CORTEX_M >= 0x07
  SCB_CleanInvalidateDCache();
#endif

  LL_DMA2D_DeInit(DMA2D);

  LL_DMA2D_InitTypeDef DMA2D_InitStruct;
  LL_DMA2D_StructInit(&DMA2D_InitStruct);

  DMA2D_InitStruct.Mode = LL_DMA2D_MODE_M2M;
  DMA2D_InitStruct.ColorMode = LL_DMA2D_OUTPUT_MODE_RGB565;
  DMA2D_InitStruct.OutputMemoryAddress = CONVERT_PTR_UINT(dest + y*destw + x);
  DMA2D_InitStruct.LineOffset = destw - w;
  DMA2D_InitStruct.NbrOfLines = h;
  DMA2D_InitStruct.NbrOfPixelsPerLines = w;
  LL_DMA2D_Init(DMA2D, &DMA2D_InitStruct);

  LL_DMA2D_FGND_SetMemAddr(DMA2D, CONVERT_PTR_UINT(src + srcy*srcw + srcx));
  LL_DMA2D_FGND_SetLineOffset(DMA2D, srcw - w);
  LL_DMA2D_FGND_SetColorMode(DMA2D, LL_DMA2D_INPUT_MODE_RGB565);
  LL_DMA2D_FGND_SetAlphaMode(DMA2D, LL_DMA2D_ALPHA_MODE_NO_MODIF);
  LL_DMA2D_FGND_SetAlpha(DMA2D, 0);

  /* Start Transfer */
  LL_DMA2D_Start(DMA2D);

}

void DMACopyAlphaBitmap(uint16_t *dest, uint16_t destw, uint16_t desth,
                        uint16_t x, uint16_t y, const uint16_t *src,
                        uint16_t srcw, uint16_t srch, uint16_t srcx,
                        uint16_t srcy, uint16_t w, uint16_t h)
{
  DMAWait();

#if __CORTEX_M >= 0x07
  SCB_CleanInvalidateDCache();
#endif

  LL_DMA2D_DeInit(DMA2D);

  LL_DMA2D_InitTypeDef DMA2D_InitStruct;
  LL_DMA2D_StructInit(&DMA2D_InitStruct);

  DMA2D_InitStruct.Mode = LL_DMA2D_MODE_M2M_BLEND;
  DMA2D_InitStruct.ColorMode = LL_DMA2D_OUTPUT_MODE_RGB565;
  DMA2D_InitStruct.OutputMemoryAddress = CONVERT_PTR_UINT(dest + y*destw + x);
  DMA2D_InitStruct.LineOffset = destw - w;
  DMA2D_InitStruct.NbrOfLines = h;
  DMA2D_InitStruct.NbrOfPixelsPerLines = w;
  LL_DMA2D_Init(DMA2D, &DMA2D_InitStruct);

  LL_DMA2D_FGND_SetMemAddr(DMA2D, CONVERT_PTR_UINT(src + srcy*srcw + srcx));
  LL_DMA2D_FGND_SetLineOffset(DMA2D, srcw - w);
  LL_DMA2D_FGND_SetColorMode(DMA2D, LL_DMA2D_INPUT_MODE_ARGB4444);
  LL_DMA2D_FGND_SetAlphaMode(DMA2D, LL_DMA2D_ALPHA_MODE_NO_MODIF);
  LL_DMA2D_FGND_SetAlpha(DMA2D, 0);

  LL_DMA2D_BGND_SetMemAddr(DMA2D, CONVERT_PTR_UINT(dest + y*destw + x));
  LL_DMA2D_BGND_SetLineOffset(DMA2D, destw - w);
  LL_DMA2D_BGND_SetColorMode(DMA2D, LL_DMA2D_OUTPUT_MODE_RGB565);
  LL_DMA2D_BGND_SetAlphaMode(DMA2D, LL_DMA2D_ALPHA_MODE_NO_MODIF);
  LL_DMA2D_BGND_SetAlpha(DMA2D, 0);

  /* Start Transfer */
  LL_DMA2D_Start(DMA2D);

}

// same as DMACopyAlphaBitmap(), but with an 8 bit mask for each pixel (used by fonts)
void DMACopyAlphaMask(uint16_t *dest, uint16_t destw, uint16_t desth,
                      uint16_t x, uint16_t y, const uint8_t *src, uint16_t srcw,
                      uint16_t srch, uint16_t srcx, uint16_t srcy, uint16_t w,
                      uint16_t h, uint16_t bg_color)
{
  DMAWait();

#if __CORTEX_M >= 0x07
  SCB_CleanInvalidateDCache();
#endif

  LL_DMA2D_DeInit(DMA2D);

  LL_DMA2D_InitTypeDef DMA2D_InitStruct;
  LL_DMA2D_StructInit(&DMA2D_InitStruct);

  DMA2D_InitStruct.Mode = LL_DMA2D_MODE_M2M_BLEND;
  DMA2D_InitStruct.ColorMode = LL_DMA2D_OUTPUT_MODE_RGB565;
  DMA2D_InitStruct.OutputMemoryAddress = CONVERT_PTR_UINT(dest + y*destw + x);
  DMA2D_InitStruct.LineOffset = destw - w;
  DMA2D_InitStruct.NbrOfLines = h;
  DMA2D_InitStruct.NbrOfPixelsPerLines = w;
  LL_DMA2D_Init(DMA2D, &DMA2D_InitStruct);

  LL_DMA2D_FGND_SetMemAddr(DMA2D, CONVERT_PTR_UINT(src + srcy*srcw + srcx));
  LL_DMA2D_FGND_SetLineOffset(DMA2D, srcw - w);
  LL_DMA2D_FGND_SetColorMode(DMA2D, LL_DMA2D_INPUT_MODE_A8);
  LL_DMA2D_FGND_SetAlphaMode(DMA2D, LL_DMA2D_ALPHA_MODE_NO_MODIF);
  LL_DMA2D_FGND_SetAlpha(DMA2D, 0);
  LL_DMA2D_FGND_SetRedColor(DMA2D, GET_RED(bg_color));     // 8 bit red
  LL_DMA2D_FGND_SetGreenColor(DMA2D, GET_GREEN(bg_color)); // 8 bit green
  LL_DMA2D_FGND_SetBlueColor(DMA2D, GET_BLUE(bg_color));   // 8 bit blue

  LL_DMA2D_BGND_SetMemAddr(DMA2D, CONVERT_PTR_UINT(dest + y*destw + x));
  LL_DMA2D_BGND_SetLineOffset(DMA2D, destw - w);
  LL_DMA2D_BGND_SetColorMode(DMA2D, LL_DMA2D_OUTPUT_MODE_RGB565);
  LL_DMA2D_BGND_SetAlphaMode(DMA2D, LL_DMA2D_ALPHA_MODE_NO_MODIF);
  LL_DMA2D_BGND_SetAlpha(DMA2D, 0);

  /* Start Transfer */
  LL_DMA2D_Start(DMA2D);

}

void DMABitmapConvert(uint16_t * dest, const uint8_t * src, uint16_t w, uint16_t h, uint32_t format)
{
  DMAWait();
#if __CORTEX_M >= 0x07
  SCB_CleanInvalidateDCache();
#endif

  LL_DMA2D_DeInit(DMA2D);

  LL_DMA2D_InitTypeDef DMA2D_InitStruct;
  LL_DMA2D_StructInit(&DMA2D_InitStruct);

  DMA2D_InitStruct.Mode = LL_DMA2D_MODE_M2M_PFC;
  DMA2D_InitStruct.ColorMode = format;
  DMA2D_InitStruct.OutputMemoryAddress = CONVERT_PTR_UINT(dest);
  DMA2D_InitStruct.NbrOfLines = h;
  DMA2D_InitStruct.NbrOfPixelsPerLines = w;
  LL_DMA2D_Init(DMA2D, &DMA2D_InitStruct);

  LL_DMA2D_FGND_SetMemAddr(DMA2D, CONVERT_PTR_UINT(src));
  LL_DMA2D_FGND_SetLineOffset(DMA2D, 0);
  LL_DMA2D_FGND_SetColorMode(DMA2D, LL_DMA2D_INPUT_MODE_ARGB8888);
  LL_DMA2D_FGND_SetAlphaMode(DMA2D, LL_DMA2D_ALPHA_MODE_REPLACE);
  LL_DMA2D_FGND_SetAlpha(DMA2D, 0);

  /* Start Transfer */
  LL_DMA2D_Start(DMA2D);

}
