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

#ifndef _COLORLCD_THEME_H_
#define _COLORLCD_THEME_H_

#include <list>
#include <vector>
#include "zone.h"

enum IconState {
  STATE_DEFAULT,
  STATE_PRESSED,
};

// TODO: hotfix, through FatFS out of libopenui instead
#if !defined(YAML_GENERATOR)
#include "ffconf.h"
#else
#define FF_MAX_LFN 255
#endif

class BitmapBuffer;
class PageTab;

#define MAX_THEME_OPTIONS              5

class EdgeTxTheme;
void registerTheme(EdgeTxTheme * theme);

// YAML_GENERATOR defs
#if !defined(USE_IDX)
#define USE_IDX
#endif

class EdgeTxTheme;
extern EdgeTxTheme * theme;

class EdgeTxTheme
{
  public:
    struct PersistentData {
      ZoneOptionValueTyped options[MAX_THEME_OPTIONS] USE_IDX;
    };

    explicit EdgeTxTheme(const char * name, const ZoneOption * options = nullptr):
      name(name),
      options(options),
      thumb(nullptr)
    {
      registerTheme(this);
    }

    static EdgeTxTheme * instance()
    {
      return theme;
    }

    inline const char * getName() const
    {
      return name;
    }

    const char * getFilePath(const char * filename) const;

    void drawThumb(BitmapBuffer * dc, coord_t x, coord_t y, uint32_t flags);

    inline const ZoneOption * getOptions() const
    {
      return options;
    }

    void init() const;

    virtual void update(bool reload = true) const
    {
    }

    ZoneOptionValue * getOptionValue(unsigned int index) const;

    virtual void setBackgroundImageFileName(const char *fileName)
    {
      strncpy(backgroundImageFileName, fileName, FF_MAX_LFN);
      backgroundImageFileName[FF_MAX_LFN] = '\0'; // ensure string termination
    }

    virtual void load() const;

    virtual void drawBackground(BitmapBuffer * dc) const;

    virtual void drawPageHeaderBackground(BitmapBuffer *dc, uint8_t icon,
                                          const char *title) const = 0;

    virtual void drawMenuIcon(BitmapBuffer *dc, uint8_t icon, bool checked) const = 0;

    virtual void drawCheckBox(BitmapBuffer * dc, bool checked, coord_t x, coord_t y, bool focus) const;

    virtual void drawHeaderIcon(BitmapBuffer * dc, uint8_t icon) const = 0;

    virtual void drawUsbPluggedScreen(BitmapBuffer * dc) const;

    virtual const BitmapBuffer * getIconMask(uint8_t index) const = 0;

    virtual uint16_t* getDefaultColors() const = 0;

  protected:
    const char * name;
    const ZoneOption * options;
    BitmapBuffer * thumb;
    char backgroundImageFileName[FF_MAX_LFN + 1];

  public:
    static const BitmapBuffer * error;
    static const BitmapBuffer * busy;
    static const BitmapBuffer * shutdown;
};

EdgeTxTheme * getTheme(const char * name);
void loadTheme(EdgeTxTheme * theme);
void loadTheme();

std::list<EdgeTxTheme *> & getRegisteredThemes();

#endif // _COLORLCD_THEME_H_
