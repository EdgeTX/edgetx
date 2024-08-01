/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   libopenui - https://github.com/opentx/libopenui
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

#include "bitmaps.h"
#include "window.h"

struct LZ4Bitmap;
class BitmapBuffer;

//-----------------------------------------------------------------------------

class StaticText : public Window
{
 public:
  StaticText(Window *parent, const rect_t &rect, std::string text = "",
             LcdColorIndex color = COLOR_THEME_PRIMARY1_INDEX, LcdFlags textFlags = 0);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override;
#endif

  void setText(std::string value);
  const std::string &getText() const;

 protected:
  std::string text;
};

#define Subtitle(parent, text) \
  StaticText(parent, rect_t{}, text, COLOR_THEME_PRIMARY1_INDEX, FONT(BOLD))

//-----------------------------------------------------------------------------

class DynamicText : public StaticText
{
 public:
  DynamicText(Window *parent, const rect_t &rect,
              std::function<std::string()> textHandler,
              LcdColorIndex color = COLOR_THEME_PRIMARY1_INDEX, LcdFlags textFlags = 0) :
      StaticText(parent, rect, "", color, textFlags),
      textHandler(std::move(textHandler))
  {
  }

  void checkEvents() override
  {
    StaticText::checkEvents();
    setText(textHandler());
  }

 protected:
  std::function<std::string()> textHandler;
};

template <class T>
class DynamicNumber : public StaticText
{
 public:
  DynamicNumber(Window *parent, const rect_t &rect,
                std::function<T()> numberHandler,
                LcdColorIndex color = COLOR_THEME_PRIMARY1_INDEX, LcdFlags textFlags = 0,
                const char *prefix = nullptr, const char *suffix = nullptr) :
      StaticText(parent, rect, "", color, textFlags),
      numberHandler(std::move(numberHandler)),
      prefix(prefix),
      suffix(suffix)
  {
    value = this->numberHandler();
    updateText();
  }

  void checkEvents() override
  {
    T newValue = numberHandler();
    if (value != newValue) {
      value = newValue;
      updateText();
    }
  }

  void setPrefix(const char *value)
  {
    prefix = value;
    updateText();
  }

  void setSuffix(const char *value)
  {
    suffix = value;
    updateText();
  }

 protected:
  T value = 0;
  std::function<T()> numberHandler;
  const char *prefix;
  const char *suffix;

  void updateText();
};

//-----------------------------------------------------------------------------

class StaticIcon : public Window
{
 public:
  StaticIcon(Window *parent, coord_t x, coord_t y, EdgeTxIcon icon,
             LcdColorIndex color);
  StaticIcon(Window *parent, coord_t x, coord_t y, const char* filename,
             LcdColorIndex color);

  void deleteLater(bool detach = true, bool trash = true) override;

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "StaticIcon"; }
#endif

  void setColor(LcdColorIndex color);
  void setIcon(EdgeTxIcon icon);
  void center(coord_t w, coord_t h);

 protected:
  LcdColorIndex currentColor;
  MaskBitmap* mask = nullptr;
};

//-----------------------------------------------------------------------------

class StaticImage : public Window
{
 public:
  StaticImage(Window *parent, const rect_t &rect,
              const char *filename = nullptr, bool fillFrame = false, bool dontEnlarge = false);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "StaticImage"; }
#endif

  void setZoom();
  void setSource(std::string filename);
  void clearSource();
  bool hasImage() const;

 protected:
  bool fillFrame;
  bool dontEnlarge = false;
  lv_obj_t *image = nullptr;
};

//-----------------------------------------------------------------------------

class StaticBitmap : public Window
{
 public:
  StaticBitmap(Window *parent, const rect_t &rect,
               const char *filename = nullptr);
  ~StaticBitmap();

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "StaticBitmap"; }
#endif

  void setSource(const char *filename);
  bool hasImage() const;

 protected:
  lv_obj_t *canvas = nullptr;
  BitmapBuffer *img = nullptr;
};

//-----------------------------------------------------------------------------

class StaticLZ4Image : public Window
{
 public:
  StaticLZ4Image(Window *parent, coord_t x, coord_t y,
                 const LZ4Bitmap *lz4Bitmap);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "StaticILZ4mage"; }
#endif

 protected:
  uint8_t *imgData = nullptr;

  void deleteLater(bool detach, bool trash) override;
};

//-----------------------------------------------------------------------------

class QRCode : public Window
{
 public:
  QRCode(Window *parent, coord_t x, coord_t y, coord_t sz, std::string data,
         LcdFlags color = COLOR_THEME_SECONDARY1, LcdFlags bgColor = COLOR_THEME_SECONDARY3);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "QRCode"; }
#endif

 protected:
};
