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

#include "display.h"
#include "simu.h"

static void _set_pixel(uint8_t* pixel, const SDL_Color& color)
{
  pixel[0] = color.a;
  pixel[1] = color.b;
  pixel[2] = color.g;
  pixel[3] = color.r;
}

static void _blit_simu_screen_color(void* screen_buffer, Uint32 format, int w, int h, int pitch)
{
  pixel_t* src_buffer = simuLcdBuf;
  uint8_t* line_buffer = (uint8_t*)screen_buffer;

  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      pixel_t z = *src_buffer++;
      // Alpha
      line_buffer[0] = SDL_ALPHA_OPAQUE;
      // Blue
      line_buffer[1] = (((z & 0x001F) << 3) & 0x00F8) + ((z & 0x001C) >> 2);
      // Green
      line_buffer[2] = ((z & 0x07E0) >> 3) + ((z & 0x0600) >> 9);
      // Red
      line_buffer[3] = ((z & 0xF800) >> 8) + ((z & 0xE000) >> 13);
      line_buffer += SDL_BYTESPERPIXEL(format);
    }
  }
}

static void _blit_simu_screen_1bit(void* screen_buffer, Uint32 format, int w, int h, int pitch)
{
  const SDL_Color on_color = {.r = 0, .g = 0, .b = 0};

  uint8_t* line_buffer = (uint8_t*)screen_buffer;

  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      SDL_Color c = on_color;
      if (simuLcdBuf[x + (y / 8) * w] & (1 << (y % 8))) {
        c.a = SDL_ALPHA_OPAQUE;
      } else {
        c.a = SDL_ALPHA_TRANSPARENT;
      }
      _set_pixel(line_buffer, c);
      line_buffer += SDL_BYTESPERPIXEL(format);
    }
  }
}

// static inline uint8_t _4bit_blend(uint16_t px, uint16_t bg)
// {
//   uint16_t c = bg * (16 - px);
//   return c / 16;
// }

static void _blit_simu_screen_4bit(void* screen_buffer, Uint32 format, int w, int h, int pitch)
{
  const SDL_Color on_color = {.r = 0, .g = 0, .b = 0};

  uint8_t* line_buffer = (uint8_t*)screen_buffer;

  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {

      pixel_t p = simuLcdBuf[y / 2 * LCD_W + x];
      uint8_t z = (y & 1) ? (p >> 4) : (p & 0x0F);

      SDL_Color c = on_color;
      c.a = (uint16_t)z * SDL_ALPHA_OPAQUE / 16;

      _set_pixel(line_buffer, c);
      line_buffer += SDL_BYTESPERPIXEL(format);
    }
  }
}

void refreshDisplay(SDL_Texture* screen)
{
  if (simuLcdRefresh) {

    // fetch texture format
    Uint32 format = 0;
    int width = 0, height = 0;

    if (SDL_QueryTexture(screen, &format, nullptr, &width, &height) != 0) {
      SDL_Log("SDL_QueryTexture: %s", SDL_GetError());
      return;
    }
    
    // raw pixel buffer
    void* screen_buffer = nullptr;

    // length of one row in bytes
    int pitch = 0;

    if (SDL_LockTexture(screen, nullptr, &screen_buffer, &pitch) != 0) {
      SDL_Log("SDL_LockTexture: %s", SDL_GetError());
      return;
    }

    if (LCD_DEPTH == 1) {
      _blit_simu_screen_1bit(screen_buffer, format, width, height, pitch);
    } else if (LCD_DEPTH == 4) {
      _blit_simu_screen_4bit(screen_buffer, format, width, height, pitch);
    } else if (LCD_DEPTH == 16) {
      _blit_simu_screen_color(screen_buffer, format, width, height, pitch);
    }

    SDL_UnlockTexture(screen);
    simuLcdRefresh = false;
    lcdFlushed();
  }
}


