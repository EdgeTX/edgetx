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

#include "static.h"

#include "bitmaps.h"
#include "lz4/lz4.h"
#include "sdcard.h"
#include "etx_lv_theme.h"

//-----------------------------------------------------------------------------

StaticText::StaticText(Window* parent, const rect_t& rect, std::string txt,
                       LcdColorIndex color, LcdFlags textFlags) :
    Window(parent, rect, lv_label_create), text(std::move(txt))
{
  setTextFlag(textFlags);

  lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_CLICK_FOCUSABLE);

  etx_font(lvobj, FONT_INDEX(textFlags));
  etx_txt_color(lvobj, color);

  if (textFlags & CENTERED)
    etx_obj_add_style(lvobj, styles->text_align_center, LV_PART_MAIN);
  else if (textFlags & RIGHT)
    etx_obj_add_style(lvobj, styles->text_align_right, LV_PART_MAIN);

  lv_obj_set_style_grid_cell_x_align(lvobj, LV_GRID_ALIGN_STRETCH,
                                     LV_PART_MAIN);
  lv_label_set_text(lvobj, text.c_str());
  if (rect.h == 0) lv_obj_set_height(lvobj, LV_SIZE_CONTENT);
}

#if defined(DEBUG_WINDOWS)
std::string StaticText::getName() const
{
  return "StaticText \"" + text.substr(0, 20) + "\"";
}
#endif

void StaticText::setText(std::string value)
{
  if (text != value) {
    text = std::move(value);
    if (lvobj) lv_label_set_text(lvobj, text.c_str());
  }
}

const std::string& StaticText::getText() const { return text; }

//-----------------------------------------------------------------------------

template <>
void DynamicNumber<uint32_t>::updateText()
{
  if (lvobj) {
    const char* p = prefix ? prefix : "";
    const char* s = suffix ? suffix : "";
    if ((textFlags & PREC2) == PREC2) {
      lv_label_set_text_fmt(lvobj, "%s%" PRIu32 ".%02" PRIu32 "%s", p,
                            value / 100, value % 100, s);
    } else if (textFlags & PREC1) {
      lv_label_set_text_fmt(lvobj, "%s%" PRIu32 ".%01" PRIu32 "%s", p,
                            value / 10, value % 10, s);
    } else {
      lv_label_set_text_fmt(lvobj, "%s%" PRIu32 "%s", p, value, s);
    }
  }
}

template <>
void DynamicNumber<int32_t>::updateText()
{
  if (lvobj) {
    const char* p = prefix ? prefix : "";
    const char* s = suffix ? suffix : "";
    if ((textFlags & PREC2) == PREC2) {
      lv_label_set_text_fmt(lvobj, "%s%" PRId32 ".%02" PRIu32 "%s", p,
                            value / 100, (uint32_t)abs(value % 100), s);
    } else if (textFlags & PREC1) {
      lv_label_set_text_fmt(lvobj, "%s%" PRId32 ".%01" PRIu32 "%s", p,
                            value / 10, (uint32_t)abs(value % 10), s);
    } else {
      lv_label_set_text_fmt(lvobj, "%s%" PRId32 "%s", p, value, s);
    }
  }
}

template <>
void DynamicNumber<uint16_t>::updateText()
{
  if (lvobj) {
    const char* p = prefix ? prefix : "";
    const char* s = suffix ? suffix : "";
    if ((textFlags & PREC2) == PREC2) {
      lv_label_set_text_fmt(lvobj, "%s%" PRIu16 ".%02" PRIu16 "%s", p,
                            (uint16_t)(value / 100), (uint16_t)(value % 100),
                            s);
    } else if (textFlags & PREC1) {
      lv_label_set_text_fmt(lvobj, "%s%" PRIu16 ".%01" PRIu16 "%s", p,
                            (uint16_t)(value / 10), (uint16_t)(value % 10), s);
    } else {
      lv_label_set_text_fmt(lvobj, "%s%" PRIu16 "%s", p, value, s);
    }
  }
}

template <>
void DynamicNumber<int16_t>::updateText()
{
  if (lvobj) {
    const char* p = prefix ? prefix : "";
    const char* s = suffix ? suffix : "";
    if ((textFlags & PREC2) == PREC2) {
      lv_label_set_text_fmt(lvobj, "%s%" PRId16 ".%02" PRIu16 "%s", p,
                            (int16_t)(value / 100), (uint16_t)abs(value % 100),
                            s);
    } else if (textFlags & PREC1) {
      lv_label_set_text_fmt(lvobj, "%s%" PRId16 ".%01" PRIu16 "%s", p,
                            (int16_t)(value / 10), (uint16_t)abs(value % 10),
                            s);
    } else {
      lv_label_set_text_fmt(lvobj, "%s%" PRId16 "%s", p, value, s);
    }
  }
}

//-----------------------------------------------------------------------------

StaticIcon::StaticIcon(Window* parent, coord_t x, coord_t y, EdgeTxIcon icon,
                       LcdColorIndex color) :
    Window(parent, rect_t{x, y, 0, 0}, lv_canvas_create),
    currentColor(color)
{
  setWindowFlag(NO_FOCUS);

  lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_CLICKABLE);

  setIcon(icon);

  etx_img_color(lvobj, currentColor, LV_PART_MAIN);
}

StaticIcon::StaticIcon(Window* parent, coord_t x, coord_t y, const char* filename,
                       LcdColorIndex color) :
    Window(parent, rect_t{x, y, 0, 0}, lv_canvas_create),
    currentColor(color)
{
  setWindowFlag(NO_FOCUS);

  lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_CLICKABLE);

  auto bm = BitmapBuffer::loadBitmap(filename, BMP_RGB565);
  if (bm) {
    size_t size;
    mask = bm->to8bitMask(&size);
    if (mask) {
      setSize(mask->width, mask->height);
      lv_canvas_set_buffer(lvobj, (void*)mask->data, mask->width, mask->height,
                           LV_IMG_CF_ALPHA_8BIT);
    }
    delete bm;
  }

  etx_img_color(lvobj, currentColor, LV_PART_MAIN);
}

void StaticIcon::deleteLater(bool detach, bool trash)
{
  if (_deleted) return;
  if (mask) free(mask);
  mask = nullptr;
}

void StaticIcon::setColor(LcdColorIndex color)
{
  if (currentColor != color) {
    etx_img_color(lvobj, color, LV_PART_MAIN);
    currentColor = color;
  }
}

void StaticIcon::setIcon(EdgeTxIcon icon)
{
  auto newMask = getBuiltinIcon(icon);
  setSize(newMask->width, newMask->height);
  lv_canvas_set_buffer(lvobj, (void*)newMask->data, newMask->width, newMask->height,
                       LV_IMG_CF_ALPHA_8BIT);
}

void StaticIcon::center(coord_t w, coord_t h)
{
  setPos((w - width()) / 2, (h - height()) / 2);
}

//-----------------------------------------------------------------------------

// Display image from file system using LVGL with LVGL scaling
//  - LVGL scaling is slow so don't use this if there are many images
StaticImage::StaticImage(Window* parent, const rect_t& rect,
                         const char* filename, bool fillFrame, bool dontEnlarge) :
    Window(parent, rect), fillFrame(fillFrame), dontEnlarge(dontEnlarge)
{
  setWindowFlag(NO_FOCUS);

  lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_CLICKABLE);

  if (!filename) filename = "";
  setSource(filename);
}

void StaticImage::setSource(std::string filename)
{
  if (!filename.empty()) {
    std::string fullpath = std::string("A" PATH_SEPARATOR) + filename;

    if (!image) image = lv_img_create(lvobj);
    lv_obj_set_pos(image, 0, 0);
    lv_obj_set_size(image, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_center(image);
    lv_img_set_src(image, fullpath.c_str());
    if (!hasImage()) {
      // Failed to load
      TRACE("could not load image '%s'", filename.c_str());
      lv_obj_del(image);
      image = nullptr;
    }
    setZoom();
  } else {
    clearSource();
  }
}

void StaticImage::clearSource()
{
  if (image) lv_obj_del(image);
  image = nullptr;
}

bool StaticImage::hasImage() const
{
  lv_img_t* img = (lv_img_t*)image;
  return img && img->w && img->h;
}

void StaticImage::setZoom()
{
  lv_img_t* img = (lv_img_t*)image;
  if (img && img->w && img->h) {
    uint16_t zw = (width() * 256) / img->w;
    uint16_t zh = (height() * 256) / img->h;
    uint16_t z = fillFrame ? max(zw, zh) : min(zw, zh);
    if (dontEnlarge) z = min(z, (uint16_t)256);
    lv_img_set_zoom(image, z);
  }
}

//-----------------------------------------------------------------------------

// Display image from file system with software scaling
//  - uglier but much faster than LVGL scaling
StaticBitmap::StaticBitmap(Window* parent, const rect_t& rect,
                           const char* filename) :
    Window(parent, rect)
{
  setWindowFlag(NO_FOCUS);
  lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_CLICKABLE);

  setSource(filename);
}

void StaticBitmap::setSource(const char *filename)
{
  if (filename) {
    if (img) delete img;
    img = BitmapBuffer::loadBitmap(filename, BMP_ARGB4444);
    if (img) {
      img->resizeToLVGL(width(), height());
      if (canvas) lv_obj_del(canvas);
      canvas = lv_canvas_create(lvobj);
      lv_obj_center(canvas);
      lv_canvas_set_buffer(canvas, img->getData(), img->width(), img->height(),
                          LV_IMG_CF_TRUE_COLOR_ALPHA);
    }
  }
}

StaticBitmap::~StaticBitmap()
{
  if (img) delete img;
}

bool StaticBitmap::hasImage() const
{
  return img && canvas;
}

//-----------------------------------------------------------------------------

StaticLZ4Image::StaticLZ4Image(Window* parent, coord_t x, coord_t y,
                               const LZ4Bitmap* lz4Bitmap) :
    Window(parent, {x, y, lz4Bitmap->width, lz4Bitmap->height},
           lv_canvas_create)
{
  setWindowFlag(NO_FOCUS);

  lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_CLICKABLE);

  // Convert ARGB4444 to LV_IMG_CF_TRUE_COLOR_ALPHA
  uint16_t w = lz4Bitmap->width;
  uint16_t h = lz4Bitmap->height;

  uint32_t pixels = w * h;
  uint32_t size = (pixels + 1) & 0xFFFFFFFE;
  imgData = (uint8_t*)lv_mem_alloc(size * 3);
  uint16_t* decompData = (uint16_t*)(imgData + size);

  LZ4_decompress_safe((const char*)lz4Bitmap->data, (char*)decompData,
                      lz4Bitmap->compressedSize, pixels * sizeof(uint16_t));

  uint8_t* dest = imgData;
  for (uint32_t i = 0; i < pixels; i += 1) {
    uint16_t c = *decompData;
    ARGB_SPLIT(c, a, r, g, b);
    c = RGB_JOIN(r * 2, g * 4, b * 2);
    *dest++ = c & 0xFF;
    *dest++ = c >> 8;
    *dest++ = (a * 255) / 15;
    decompData += 1;
  }

  lv_canvas_set_buffer(lvobj, imgData, w, h, LV_IMG_CF_TRUE_COLOR_ALPHA);
}

void StaticLZ4Image::deleteLater(bool detach, bool trash)
{
  if (!deleted()) {
    if (imgData) lv_mem_free(imgData);
    imgData = nullptr;
    Window::deleteLater(detach, trash);
  }
}

//-----------------------------------------------------------------------------

QRCode::QRCode(Window *parent, coord_t x, coord_t y, coord_t sz, std::string data,
               LcdFlags color, LcdFlags bgColor) :
    Window(parent, {x, y, sz, sz})
{
  auto qr = lv_qrcode_create(lvobj, sz, makeLvColor(color), makeLvColor(bgColor));
  lv_qrcode_update(qr, data.c_str(), data.length());
}
