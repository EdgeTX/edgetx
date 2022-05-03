/*
 * Copyright (C) OpenTX
 *
 * Source:
 *  https://github.com/opentx/libopenui
 *
 * This file is a part of libopenui library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */

#include "window.h"
#include "touch.h"
#include "mainwindow.h"

Window * Window::focusWindow = nullptr;
std::list<Window *> Window::trash;

extern lv_obj_t *virtual_kb;

static bool is_scrolling = false;
static bool inhibit_focus = false;

static void window_event_cb(lv_event_t * e)
{
  lv_obj_t *target = lv_event_get_target(e);
  lv_event_code_t code = lv_event_get_code(e);

  // we do this before looking for user data (libopenui ptr) for this, and in 
  // fact during delete operations the window will be deleted and you need 
  // to reset scrolling if you get a pressing as when you are deleting controls
  // you get scroll begin events and no scroll end
  if (code == LV_EVENT_SCROLL_BEGIN) {
    TRACE("SCROLL_BEGIN");
    is_scrolling = true;
  } else if (code == LV_EVENT_SCROLL_END || code == LV_EVENT_PRESSING) {
    TRACE("SCROLL_END");
    is_scrolling = false;
  } 

  Window* window = (Window *)lv_obj_get_user_data(target);
  if (!window) return;

  if(code == LV_EVENT_DELETE || window->deleted()) return;

  if (code == LV_EVENT_DRAW_MAIN) {

    lv_draw_ctx_t* draw_ctx = lv_event_get_draw_ctx(e);

    lv_area_t a, clipping, obj_coords;
    lv_area_copy(&a, draw_ctx->buf_area);
    lv_area_copy(&clipping, draw_ctx->clip_area);
    lv_obj_get_coords(target, &obj_coords);

    auto w = a.x2 - a.x1 + 1;
    auto h = a.y2 - a.y1 + 1;

    TRACE_WINDOWS("DRAW_MAIN %s", window->getWindowDebugString().c_str());

    BitmapBuffer buf = {BMP_RGB565, (uint16_t)w, (uint16_t)h,
                        (uint16_t *)draw_ctx->buf};

    buf.setDrawCtx(draw_ctx);
    
    buf.setOffset(obj_coords.x1 - a.x1, obj_coords.y1 - a.y1);
    buf.setClippingRect(clipping.x1 - a.x1, clipping.x2 + 1 - a.x1,
                        clipping.y1 - a.y1, clipping.y2 + 1 - a.y1);

    window->paint(&buf);

  }
#if defined(HARDWARE_TOUCH)
  else if (code == LV_EVENT_PRESSED || code == LV_EVENT_RELEASED) {
    TRACE_WINDOWS("PRESSED: %s", window->getWindowDebugString().c_str());

    lv_indev_t *click_source = (lv_indev_t *)lv_event_get_param(e);
    if (click_source == NULL || is_scrolling) {
      TRACE_WINDOWS("CLICK WHILE SCROLLING");
      return;
    }

    // Exclude keyboard ?
    // if(lv_indev_get_type(click_source) == LV_INDEV_TYPE_KEYPAD ||
    //    lv_indev_get_type(click_source) == LV_INDEV_TYPE_ENCODER) {
    //   return;
    // }
    lv_area_t obj_coords;
    lv_obj_get_coords(target, &obj_coords);

    lv_point_t point_act;
    lv_indev_get_point(click_source, &point_act);

    // Ignore event from keypad
    if (point_act.x < 0 || point_act.y < 0) return;

    lv_point_t rel_pos;
    rel_pos.x = point_act.x - obj_coords.x1;
    rel_pos.y = point_act.y - obj_coords.y1;


    if (code == LV_EVENT_PRESSED) {
      TRACE("PRESSED[%d|%d]", rel_pos.x, rel_pos.y);
      window->onTouchStart(rel_pos.x, rel_pos.y);
    } else {
      lv_point_t vect_act;
      lv_indev_get_vect(click_source, &vect_act);

      TRACE("RELEASED[%d|%d] vect[%d|%d]", rel_pos.x, rel_pos.y, vect_act.x,
            vect_act.y, lv_indev_get_scroll_dir(click_source));

      if (vect_act.x != 0 || vect_act.y != 0) return;
      window->onTouchEnd(rel_pos.x, rel_pos.y);
    }
  }
#endif
  else if (code == LV_EVENT_SCROLL) {

#if defined(DEBUG_WINDOWS)
    lv_coord_t scroll_y = lv_obj_get_scroll_y(target);
    lv_coord_t scroll_x = lv_obj_get_scroll_x(target);

    TRACE_WINDOWS("SCROLL[x=%d;y=%d] %s", scroll_x, scroll_y,
                  window->getWindowDebugString().c_str());
#endif


  } else if (code == LV_EVENT_FOCUSED) {
    if (inhibit_focus) return;
    bool focused = ((Window *)target->user_data)->hasFocus();
    TRACE_WINDOWS("FOCUSED[%d] %s", focused,
                 window->getWindowDebugString().c_str());
    if (!focused) { window->setFocus(); }
  }
}

Window::Window(Window *parent, const rect_t &rect, WindowFlags windowFlags,
               LcdFlags textFlags, LvglCreate objConstruct) :
    rect(rect),
    parent(parent),
    windowFlags(windowFlags),
    textFlags(textFlags)
{
  lv_obj_t *lvParent = parent != nullptr ? parent->lvobj : nullptr;
  lvobj = (objConstruct == nullptr) ? lv_obj_create(lvParent)
                                    : objConstruct(lvParent);

  lv_obj_set_user_data(lvobj, this);

  if (rect.w || rect.h) {
    lv_obj_set_pos(lvobj, rect.x, rect.y);
    lv_obj_set_size(lvobj, rect.w, rect.h);
  }

  if (windowFlags & OPAQUE) {
    lv_obj_set_style_bg_opa(lvobj, LV_OPA_MAX, LV_PART_MAIN);
  }

  lv_obj_set_scrollbar_mode(lvobj, LV_SCROLLBAR_MODE_OFF);
  lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_SCROLL_ELASTIC);
  lv_obj_add_event_cb(lvobj, window_event_cb, LV_EVENT_ALL, this);

  if (parent) {
    parent->addChild(this);
    if (!(windowFlags & TRANSPARENT)) {
      invalidate();
    }
  }

  // honor the no focus flag of libopenui
  if (this->windowFlags & NO_FOCUS) {
    lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_CLICK_FOCUSABLE);
  }
}

Window::Window(Window *parent, lv_obj_t* lvobj)
  : rect(nullRect),
    parent(parent),
    lvobj(lvobj)
{
  lv_obj_set_user_data(lvobj, this);

  if (windowFlags & OPAQUE) {
    lv_obj_set_style_bg_opa(lvobj, LV_OPA_MAX, LV_PART_MAIN);
  }

  lv_obj_set_scrollbar_mode(lvobj, LV_SCROLLBAR_MODE_OFF);
  lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_SCROLL_ELASTIC);
  lv_obj_add_event_cb(lvobj, window_event_cb, LV_EVENT_ALL, this);

  if (parent) {
    parent->addChild(this);
    if (!(windowFlags & TRANSPARENT)) {
      invalidate();
    }
  }

  // honor the no focus flag of libopenui
  if (this->windowFlags & NO_FOCUS) {
    lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_CLICK_FOCUSABLE);
  }
}

Window::~Window()
{
  TRACE_WINDOWS("Destroy %p %s", this, getWindowDebugString().c_str());
  if (focusWindow == this) {
    focusWindow = nullptr;
  }

  if (children.size() > 0) deleteChildren();

  if (lvobj != nullptr) {
    lv_obj_set_user_data(lvobj, nullptr);
    lv_obj_del(lvobj);
    lvobj = nullptr;
  }
}

#if defined(DEBUG_WINDOWS)
std::string Window::getName() const { return "Window"; }

std::string Window::getRectString() const
{
  char result[32];
  sprintf(result, "[%d, %d, %d, %d]", left(), top(), width(), height());
  return result;
}

std::string Window::getIndentString() const
{
  std::string result;
  auto tmp = parent;
  while (tmp) {
    result += "  ";
    tmp = tmp->getParent();
  }
  return result;
}

std::string Window::getWindowDebugString(const char *name = nullptr) const
{
  return getName() + (name ? std::string(" [") + name + "] " : " ") +
         getRectString();
}
#endif

Window *Window::getFullScreenWindow()
{
  if (width() == LCD_W && height() == LCD_H) return this;
  if (parent) return parent->getFullScreenWindow();
  return nullptr;
}

void Window::setWindowFlags(WindowFlags flags)
{
  // TODO: check if LVGL flags need updating...
  windowFlags = flags;
}

void Window::setTextFlags(LcdFlags flags)
{
  textFlags = flags;
  if (!lvobj) return;

  // lv integration for colors
  auto textColor = COLOR_VAL(flags);
  auto r = GET_RED(textColor), g = GET_GREEN(textColor),
       b = GET_BLUE(textColor);
  lv_obj_set_style_text_color(lvobj, lv_color_make(r, g, b), LV_PART_MAIN);

  // rco: shouldn't this be done via 'setTextFlags()' on the children?
  for (uint32_t i = 0; i < lv_obj_get_child_cnt(lvobj); i++) {
    auto child = lv_obj_get_child(lvobj, i);
    lv_obj_set_style_text_color(child, lv_color_make(r, g, b), LV_PART_MAIN);
  }
}

void Window::attach(Window *newParent)
{
  if (parent) detach();
  parent = newParent;
  if (newParent) {
    newParent->addChild(this);
  }
}

void Window::detach()
{
  if (parent) {
    parent->removeChild(this);
    parent = nullptr;
    // if (lvobj != nullptr) lv_obj_set_parent(lvobj, nullptr);
  }
}

void Window::deleteLater(bool detach, bool trash)
{
  if (_deleted)
    return;

  TRACE_WINDOWS("Delete %p %s", this, getWindowDebugString().c_str());

  _deleted = true;

  if (static_cast<Window *>(focusWindow) == static_cast<Window *>(this)) {
    focusWindow = nullptr;
  }

  if (detach)
    this->detach();
  else
    parent = nullptr;

  if (trash) {
    Window::trash.push_back(this);
  }

  deleteChildren();

  if (closeHandler) {
    closeHandler();
  }
}

void Window::clear()
{
  deleteChildren();
  if(lvobj != nullptr) {
    lv_obj_clean(lvobj);
  }
  invalidate();
}

void Window::clearLvgl()
{
  if(lvobj == nullptr)
    return;

  for (auto window: children)
  {
    if(window->lvobj != nullptr && window->lvobj->parent == lvobj)
      window->clearLvgl();
  }

  lv_obj_remove_event_cb(lvobj, window_event_cb);
  lv_obj_set_user_data(lvobj, nullptr);
  lv_group_remove_obj(lvobj);
  lvobj = nullptr;
}

void Window::deleteChildren()
{
  // prevent LVGL refocus while mass-deleting
  inhibit_focus = true;
  for (auto window: children) {
    window->clearLvgl();
    window->deleteLater(false);
  }
  inhibit_focus = false;
  children.clear();
}

void Window::clearFocus()
{
  if (focusWindow) {
    focusWindow->onFocusLost();
    focusWindow = nullptr;
  }
}

void Window::setFocus(uint8_t flag, Window * from)
{
  if (deleted())
    return;

  TRACE_WINDOWS("%s setFocus()", getWindowDebugString().c_str());

  if (focusWindow != this) {
    // synchronize lvgl focused state with libopenui
    if (lvobj != nullptr &&
        !lv_obj_has_state(lvobj, LV_STATE_FOCUSED | LV_STATE_FOCUS_KEY)) {
      lv_obj_add_state(lvobj, LV_STATE_FOCUSED | LV_STATE_FOCUS_KEY);
      if (focusWindow != nullptr) {
        lv_obj_clear_state(focusWindow->lvobj, LV_STATE_FOCUSED |
                                                   LV_STATE_FOCUS_KEY |
                                                   LV_STATE_EDITED);
      }
    }

    // scroll before calling focusHandler so that the window can adjust the
    // scroll position if needed
#if 1
    // lv_obj_scroll_to_view(lvobj, LV_ANIM_OFF);
    lv_obj_scroll_to_view_recursive(lvobj, LV_ANIM_OFF);
#else
    Window * parent = this->parent;
    while (parent && parent->getWindowFlags() & FORWARD_SCROLL) {
      parent = parent->parent;
    }
    if (parent) {
      parent->scrollTo(this);
      invalidate();
    }
#endif

    clearFocus();
    focusWindow = this;
    if (focusHandler) {
      focusHandler(true);
    }
  }
}

void Window::scrollTo(Window * child)
{



}

void Window::scrollTo(const rect_t & rect)
{
}

bool Window::hasOpaqueRect(const rect_t & testRect) const
{
  if (!rect.contains(testRect))
    return false;

  if (windowFlags & OPAQUE) {
    return true;
  }

  rect_t relativeRect = {testRect.x - rect.x, testRect.y - rect.y, testRect.w, testRect.h};
  for (auto child: children) {
    if (child->hasOpaqueRect(relativeRect))
      return true;
  }

  return false;
}

void Window::fullPaint(BitmapBuffer * dc)
{
  if(lvobj != nullptr)
    lv_obj_invalidate(lvobj);
  bool paintNeeded = true;
  std::list<Window *>::iterator firstChild;

  coord_t xmin, xmax, ymin, ymax;
  dc->getClippingRect(xmin, xmax, ymin, ymax);
  coord_t x = dc->getOffsetX();
  coord_t y = dc->getOffsetY();

  if (windowFlags & PAINT_CHILDREN_FIRST) {
    paintChildren(dc, children.begin());
    dc->setOffset(x, y);
    dc->setClippingRect(xmin, xmax, ymin, ymax);
  }
  else {
    firstChild = children.end();
    rect_t relativeRect = {xmin - x, ymin - y, xmax - xmin, ymax - ymin};
    while (firstChild != children.begin()) {
      auto child = *(--firstChild);
      if (child->hasOpaqueRect(relativeRect)) {
        paintNeeded = false;
        break;
      }
    }
  }

  if (paintNeeded) {
    TRACE_WINDOWS_INDENT("%s%s", getWindowDebugString().c_str(), hasFocus() ? " (*)" : "");
    paint(dc);
#if defined(WINDOWS_INSPECT_BORDER_COLOR)
    dc->drawSolidRect(0, 0, width(), height(), 1, WINDOWS_INSPECT_BORDER_COLOR);
#endif
  }
  else {
    TRACE_WINDOWS_INDENT("%s (skipped)", getWindowDebugString().c_str());
  }


  if (!(windowFlags & PAINT_CHILDREN_FIRST)) {
    paintChildren(dc, firstChild);
  }
}

bool Window::isChildFullSize(const Window * child) const
{
  return child->top() == 0 && child->height() == height() && child->left() == 0 && child->width() == width();
}

bool Window::isChildVisible(const Window * window) const
{
  for (auto rit = children.rbegin(); rit != children.rend(); rit++) {
    auto child = *rit;
    if (child == window) {
      return true;
    }
    if ((child->getWindowFlags() & OPAQUE) & isChildFullSize(child)) {
      return false;
    }
  }
  return false;
}


void Window::paintChildren(BitmapBuffer * dc, std::list<Window *>::iterator it)
{
  coord_t x = dc->getOffsetX();
  coord_t y = dc->getOffsetY();
  coord_t xmin, xmax, ymin, ymax;
  dc->getClippingRect(xmin, xmax, ymin, ymax);

  for (; it != children.end(); it++) {
    auto child = *it;

    coord_t child_xmin = x + child->rect.x;
    if (child_xmin >= xmax)
      continue;
    coord_t child_ymin = y + child->rect.y;
    if (child_ymin >= ymax)
      continue;
    coord_t child_xmax = child_xmin + child->rect.w;
    if (child_xmax <= xmin)
      continue;
    coord_t child_ymax = child_ymin + child->rect.h;
    if (child_ymax <= ymin)
      continue;

    // dc->setOffset(x + child->rect.x - child->scrollPositionX,
    //               y + child->rect.y - child->scrollPositionY);
    dc->setClippingRect(
        max(xmin, x + child->rect.left()), min(xmax, x + child->rect.right()),
        max(ymin, y + child->rect.top()), min(ymax, y + child->rect.bottom()));
    child->fullPaint(dc);
  }
}

#if defined(HARDWARE_TOUCH)
coord_t Window::getSnapStep(coord_t relativeScrollPosition, coord_t pageSize)
{
  coord_t result = 0;
  if (relativeScrollPosition > pageSize / 2) {
    // closer to next page
    result = (pageSize - relativeScrollPosition);
  }
  else {
    // closer to previous page
    result = (0 - relativeScrollPosition);
  }

  // do not get too slow
  if (abs(result) > 32)
    result /= 2;

  return result;
}
#endif

void lvglPushEncoderEvent(event_t& evt);
void Window::checkEvents()
{
  auto copy = children;
  for (auto child: copy) {
    if (!child->deleted()) {
      child->checkEvents();
    }
  }

  if (this == Window::focusWindow) {
    event_t event = getWindowEvent();
//    lvglPushEncoderEvent(event);
    if (event) {
      this->onEvent(event);
    }
  }

  if (windowFlags & REFRESH_ALWAYS) {
    invalidate();
  }
}

void Window::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", Window::getWindowDebugString("Window").c_str(), event);
  if (parent) {
    parent->onEvent(event);
  }
}

#if defined(HARDWARE_TOUCH)
bool Window::onTouchStart(coord_t x, coord_t y)
{
  if (parent)
    return parent->onTouchStart(x, y);
  return true;
}

bool Window::onTouchEnd(coord_t x, coord_t y)
{
  if (parent && !(windowFlags & OPAQUE))
    return parent->onTouchEnd(x, y);
  return true;
}

bool Window::onTouchSlide(coord_t x, coord_t y, coord_t startX, coord_t startY, coord_t slideX, coord_t slideY)
{

  for (auto it = children.rbegin(); it != children.rend(); ++it) {
    auto child = *it;
    if (child->rect.contains(startX, startY)) {
      if (child->onTouchSlide(x - child->rect.x, y - child->rect.y, startX - child->rect.x, startY - child->rect.y, slideX, slideY)) {
        return true;
      }
    }
  }

  return false;
}
#endif

void Window::adjustInnerHeight()
{
}

coord_t Window::adjustHeight()
{
  coord_t old = rect.h;
  adjustInnerHeight();
  if (lvobj) lv_obj_set_style_height(lvobj, rect.h, LV_PART_MAIN);
  return rect.h - old;
}

void Window::moveWindowsTop(coord_t y, coord_t delta)
{
  if (getWindowFlags() & FORWARD_SCROLL) {
    parent->moveWindowsTop(bottom(), delta);
  }

  for (auto child: children) {
    if (child->rect.y >= y) {
      child->rect.y += delta;
      invalidate();
    }
  }
}

void Window::updateSize()
{
  lv_obj_set_height(lvobj, LV_SIZE_CONTENT);
  lv_obj_update_layout(lvobj);

  rect.x = lv_obj_get_x(lvobj);
  rect.y = lv_obj_get_y(lvobj);
  rect.w = lv_obj_get_width(lvobj);
  rect.h = lv_obj_get_height(lvobj);
  invalidate();
}

void Window::addChild(Window* window)
{
  auto lv_parent = lv_obj_get_parent(window->lvobj);
  if (lv_parent && (lv_parent != lvobj)) {
    lv_obj_set_parent(window->lvobj, lvobj);
  }

  children.push_back(window);
}

void Window::removeChild(Window* window)
{
  children.remove(window);
  if (window->lvobj != nullptr) lv_obj_set_parent(window->lvobj, nullptr);
  invalidate();
}

void Window::invalidate(const rect_t & rect)
{
  if (lvobj) lv_obj_invalidate(lvobj);
}
