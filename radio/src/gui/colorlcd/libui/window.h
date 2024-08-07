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

#include <functional>
#include <list>
#include <string>

#include "LvglWrapper.h"
#include "bitmapbuffer.h"
#include "libopenui_defines.h"
#include "edgetx_helpers.h"
#include "etx_lv_theme.h"

typedef uint32_t WindowFlags;

#if !defined(_GNUC_)
#undef OPAQUE
#endif

constexpr WindowFlags OPAQUE = 1u << 0u;
constexpr WindowFlags NO_FOCUS = 1u << 1u;

typedef lv_obj_t *(*LvglCreate)(lv_obj_t *);

class FlexGridLayout;
class FormLine;

class Window
{
 public:
  Window(const rect_t &rect);
  Window(Window *parent, const rect_t &rect, LvglCreate objConstruct = nullptr);

  virtual ~Window();

#if defined(DEBUG_WINDOWS)
  virtual std::string getName() const;
  std::string getRectString() const;
  std::string getIndentString() const;
  std::string getWindowDebugString(const char *name = nullptr) const;
#endif

  Window *getParent() const { return parent; }

  Window *getFullScreenWindow();

  void setWindowFlag(WindowFlags flag);
  void clearWindowFlag(WindowFlags flag);
  bool hasWindowFlag(WindowFlags flag) const { return windowFlags & flag; }

  void setTextFlag(LcdFlags flag);
  void clearTextFlag(LcdFlags flag);
  LcdFlags getTextFlags() const { return textFlags; }

  typedef std::function<void()> CloseHandler;
  void setCloseHandler(CloseHandler h) { closeHandler = std::move(h); }

  typedef std::function<void(bool)> FocusHandler;
  void setFocusHandler(FocusHandler h) { focusHandler = std::move(h); }

  virtual void clear();
  virtual void deleteLater(bool detach = true, bool trash = true);

  bool hasFocus() const;

  void setRect(rect_t value)
  {
    rect = value;
    lv_obj_set_pos(lvobj, rect.x, rect.y);
    lv_obj_set_size(lvobj, rect.w, rect.h);
  }

  void setWidth(coord_t value)
  {
    rect.w = value;
    lv_obj_set_width(lvobj, rect.w);
  }

  void setHeight(coord_t value)
  {
    rect.h = value;
    lv_obj_set_height(lvobj, rect.h);
  }

  void setTop(coord_t y)
  {
    rect.y = y;
    lv_obj_set_pos(lvobj, rect.x, rect.y);
  }

  void setSize(coord_t w, coord_t h)
  {
    rect.w = w;
    rect.h = h;
    lv_obj_set_size(lvobj, w, h);
  }

  void setPos(coord_t x, coord_t y)
  {
    rect.x = x;
    rect.y = y;
    lv_obj_set_pos(lvobj, x, y);
  }

  coord_t left() const { return rect.x; }

  coord_t right() const { return rect.x + rect.w; }

  coord_t top() const { return rect.y; }

  coord_t bottom() const { return rect.y + rect.h; }

  coord_t width() const { return rect.w; }

  coord_t height() const { return rect.h; }

  rect_t getRect() const { return rect; }

  void padLeft(coord_t pad);
  void padRight(coord_t pad);
  void padTop(coord_t pad);
  void padBottom(coord_t pad);
  void padAll(PaddingSize pad);

  void padRow(coord_t pad);
  void padColumn(coord_t pad);

  virtual void onEvent(event_t event);
  virtual void onClicked();
  virtual void onCancel();
  virtual bool onLongPress();

  void invalidate();

  void bringToTop();

  virtual void checkEvents();

  void attach(Window *window);

  void detach();

  bool deleted() const { return _deleted; }

#if defined(HARDWARE_TOUCH)
  void addBackButton();
#endif

  inline lv_obj_t *getLvObj() { return lvobj; }

  virtual bool isTopBar() { return false; }
  virtual bool isWidgetsContainer() { return false; }

  virtual bool isBubblePopup() { return false; }

  void setFlexLayout(lv_flex_flow_t flow = LV_FLEX_FLOW_COLUMN,
                     lv_coord_t padding = PAD_TINY, coord_t width = LV_PCT(100),
                     coord_t height = LV_SIZE_CONTENT);
  FormLine *newLine(FlexGridLayout &layout);

  virtual void show(bool visible = true);
  void hide() { show(false); }
  void enable(bool enabled = true);
  void disable() { enable(false); }

 protected:
  static std::list<Window *> trash;

  rect_t rect;

  Window *parent = nullptr;
  lv_obj_t *lvobj = nullptr;

  std::list<Window *> children;

  WindowFlags windowFlags = 0;
  LcdFlags textFlags = 0;

  bool _deleted = false;
  static bool _longPressed;

  std::function<void()> closeHandler;
  std::function<void(bool)> focusHandler;

  void deleteChildren();

  virtual void addChild(Window *window);
  void removeChild(Window *window);

  void eventHandler(lv_event_t *e);
  static void window_event_cb(lv_event_t *e);
};

class NavWindow : public Window
{
 public:
  NavWindow(Window *parent, const rect_t &rect,
            LvglCreate objConstruct = nullptr);

 protected:
#if defined(HARDWARE_KEYS)
  virtual void onPressSYS() {}
  virtual void onLongPressSYS() {}
  virtual void onPressMDL() {}
  virtual void onLongPressMDL() {}
  virtual void onPressTELE() {}
  virtual void onLongPressTELE() {}
  virtual void onPressPGUP() {}
  virtual void onPressPGDN() {}
  virtual void onLongPressPGUP() {}
  virtual void onLongPressPGDN() {}
#endif
  virtual bool bubbleEvents() { return true; }
  void onEvent(event_t event) override;
};

struct PageButtonDef {
  const char* title;
  std::function<void()> createPage;
  std::function<bool()> isActive;

  PageButtonDef(const char* title, std::function<void()> createPage, std::function<bool()> isActive = nullptr) :
    title(title), createPage(std::move(createPage)), isActive(std::move(isActive))
  {}
};

class SetupButtonGroup : public Window
{
 public:
  typedef std::list<PageButtonDef> PageDefs;

  SetupButtonGroup(Window* parent, const rect_t& rect, const char* title, int cols,
                   PaddingSize padding, PageDefs pages, coord_t btnHeight = EdgeTxStyles::UI_ELEMENT_HEIGHT);

 protected:
};

struct SetupLineDef {
  const char* title;
  std::function<void(Window*, coord_t, coord_t)> createEdit;
};

class SetupLine : public Window
{
 public:
  SetupLine(Window* parent, coord_t y, coord_t col2, PaddingSize padding, const char* title,
    std::function<void(Window*, coord_t, coord_t)> createEdit, coord_t lblYOffset = 0);

  static coord_t showLines(Window* parent, coord_t y, coord_t col2, PaddingSize padding, SetupLineDef* setupLines, int lineCount);

 protected:
};
