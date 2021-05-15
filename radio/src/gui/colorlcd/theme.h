/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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
#include "thirdparty/libopenui/src/theme.h"

class BitmapBuffer;
class PageTab;

#define MAX_THEME_OPTIONS              5

class OpenTxTheme;
void registerTheme(OpenTxTheme * theme);

// YAML_GENERATOR defs
#if !defined(USE_IDX)
#define USE_IDX
#endif

class OpenTxTheme: public Theme
{
  public:
    struct PersistentData {
      ZoneOptionValueTyped options[MAX_THEME_OPTIONS] USE_IDX;
    };

    explicit OpenTxTheme(const char * name, const ZoneOption * options = nullptr):
      name(name),
      options(options),
      thumb(nullptr)
    {
      registerTheme(this);
    }

    static OpenTxTheme * instance()
    {
      return static_cast<OpenTxTheme *>(theme);
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

    virtual void update() const
    {
    }

    ZoneOptionValue * getOptionValue(unsigned int index) const;

    virtual void load() const;

    virtual void drawBackground(BitmapBuffer * dc) const;

    virtual void drawMenuBackground(BitmapBuffer * dc, uint8_t icon, const char *title) const = 0;

    virtual void drawMenuHeader(BitmapBuffer * dc, std::vector<PageTab *> & tabs, uint8_t currentIndex) const = 0;

    // virtual void drawMessageBox(const char * title, const char * text, const
    //                             char * action, uint32_t flags) const override;
    // virtual void drawProgressBar(BitmapBuffer * dc, coord_t x, coord_t y,
    //                              coord_t w, coord_t h, int value) const = 0;

    void drawCheckBox(BitmapBuffer * dc, bool checked, coord_t x, coord_t y, bool focus) const override;

    void drawChoice(BitmapBuffer * dc, ChoiceBase * choice, const char * str) const override;

    void drawSlider(BitmapBuffer * dc, int vmin, int vmax, int value, const rect_t & rect, bool edit, bool focus) const override;

    virtual void drawTopLeftBitmap(BitmapBuffer * dc) const = 0;

    virtual void drawUsbPluggedScreen(BitmapBuffer * dc) const;

  
  protected:
    const char * name;
    const ZoneOption * options;
    BitmapBuffer * thumb;

  public:
    static const BitmapBuffer * asterisk;
    static const BitmapBuffer * question;
    static const BitmapBuffer * busy;
    static const BitmapBuffer * shutdown;
};

OpenTxTheme * getTheme(const char * name);
void loadTheme(OpenTxTheme * theme);
void loadTheme();

std::list<OpenTxTheme *> & getRegisteredThemes();

#endif // _COLORLCD_THEME_H_
