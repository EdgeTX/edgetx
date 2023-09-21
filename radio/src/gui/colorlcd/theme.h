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

#pragma once

// TODO: hotfix, through FatFS out of libopenui instead
#if !defined(YAML_GENERATOR)
#include "ffconf.h"
#else
#define FF_MAX_LFN 255
#endif

class BitmapBuffer;

class EdgeTxTheme
{
  public:
    EdgeTxTheme();

    static EdgeTxTheme * instance();

    const char * getFilePath(const char * filename) const;

    void createIcons();
    void loadColors() const;
    void loadIcons() const;
    virtual void load();
    void update();

    void setBackgroundImageFileName(const char *fileName);

    void drawBackground(BitmapBuffer * dc) const;

    void drawPageHeaderBackground(BitmapBuffer *dc, uint8_t icon, const char *title) const;

    void drawMenuIcon(BitmapBuffer *dc, uint8_t icon, bool checked) const;

    void drawMenuDatetime(BitmapBuffer * dc, coord_t x, coord_t y, LcdFlags color) const;

    void drawHeaderIcon(BitmapBuffer * dc, uint8_t icon) const;

    void drawUsbPluggedScreen(BitmapBuffer * dc) const;

    const BitmapBuffer * getIconMask(uint8_t index) const;

    uint16_t* getDefaultColors() const { return defaultColors; }

  protected:
    bool iconsLoaded = false;
    const char * name;
    char backgroundImageFileName[FF_MAX_LFN + 1];

    const BitmapBuffer * backgroundBitmap = nullptr;
    const BitmapBuffer * topleftBitmap = nullptr;
    BitmapBuffer * currentMenuBackground = nullptr;
    BitmapBuffer ** iconMask = nullptr;

    static uint16_t defaultColors[LCD_COLOR_COUNT];

  public:
    const BitmapBuffer * error = nullptr;
    const BitmapBuffer * busy = nullptr;
    const BitmapBuffer * shutdown = nullptr;
};
