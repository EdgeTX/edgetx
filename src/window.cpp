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
Window * Window::slidingWindow = nullptr;
Window * Window::capturedWindow = nullptr;
std::list<Window *> Window::trash;

extern lv_obj_t *virtual_kb;

static bool is_scrolling = false;

static void window_event_cb(lv_event_t * e)
{
  lv_obj_t *target = lv_event_get_target(e);
  lv_event_code_t code = lv_event_get_code(e);

  Window* window = (Window *)lv_obj_get_user_data(target);
  if (!window) return;

  if (code == LV_EVENT_GET_SELF_SIZE) {
    lv_point_t *p = (lv_point_t *)lv_event_get_param(e);
    if (p->x >= 0) { p->x = window->getInnerWidth(); }
    if (p->y >= 0) { p->y = window->getInnerHeight(); }
    return;
  } else if (code == LV_EVENT_DRAW_MAIN) {
    TRACE_WINDOWS("DRAW_MAIN %s", window->getWindowDebugString().c_str());
    return;
  } else if (code == LV_EVENT_PRESSED || code == LV_EVENT_RELEASED) {
    TRACE_WINDOWS("PRESSED: %s", window->getWindowDebugString().c_str());

    lv_indev_t *click_source = (lv_indev_t *)lv_event_get_param(e);
    if (click_source == NULL || is_scrolling) return;

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

    rel_pos.x += window->getScrollPositionX();
    rel_pos.y += window->getScrollPositionY();

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
    return;
  } else if (code == LV_EVENT_SCROLL) {
    lv_coord_t scroll_y = lv_obj_get_scroll_y(target);
    lv_coord_t scroll_x = lv_obj_get_scroll_x(target);
    TRACE_WINDOWS("SCROLL[x=%d;y=%d] %s", scroll_x, scroll_y,
                  window->getWindowDebugString().c_str());
    window->setScrollPositionY(scroll_y);
    window->setScrollPositionX(scroll_x);
    return;
  } else if (code == LV_EVENT_SCROLL_BEGIN) {
    TRACE("SCROLL_BEGIN");
    is_scrolling = true;
    return;
  } else if (code == LV_EVENT_SCROLL_END) {
    TRACE("SCROLL_END");
    is_scrolling = false;
    return;
  } else if (code == LV_EVENT_FOCUSED) {
    bool lvgl_focused = lv_obj_has_state(target, LV_STATE_FOCUSED);
    bool loiu_focused = ((Window *)target->user_data)->hasFocus();
    TRACE_WINDOWS("FOCUSED[%d|%d] %s",
                  lvgl_focused, loiu_focused,
                 window->getWindowDebugString().c_str());
    if (!loiu_focused) { window->setFocus(); }
    return;
  }
}

LvglWidgetFactory windowFactory = LvglWidgetFactory(
  [] (lv_obj_t *parent) {
    return lv_obj_create(parent);
  },
  [] (LvglWidgetFactory *factory) {
    lv_style_set_pad_all(&factory->style, 0);
    lv_style_set_bg_opa(&factory->style, LV_OPA_TRANSP);
    lv_style_set_border_width(&factory->style, 0);
    lv_style_set_radius(&factory->style, 0);
  });

Window::Window(Window * parent, const rect_t & rect, WindowFlags windowFlags, LcdFlags textFlags, LvglWidgetFactory *factory) :
  parent(parent),
  rect(rect),
  innerWidth(rect.w),
  innerHeight(rect.h),
  windowFlags(windowFlags),
  textFlags(textFlags)
{
  lv_obj_t *lvParent = parent != nullptr ? parent->lvobj : nullptr;
  lvobj = (factory == nullptr) ?
    windowFactory.construct(lvParent) :
    factory->construct(lvParent);


  lv_obj_add_style(lvobj, &windowFactory.style, LV_PART_MAIN);
  lv_obj_set_pos(lvobj, rect.x, rect.y);
  lv_obj_set_size(lvobj, rect.w, rect.h);
  lv_obj_set_user_data(lvobj, this);

  lv_obj_set_scrollbar_mode(lvobj, LV_SCROLLBAR_MODE_OFF);
  lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_SCROLL_ELASTIC);
  lv_obj_add_event_cb(lvobj, window_event_cb, LV_EVENT_ALL, this);

  if (parent) {
    parent->addChild(this, windowFlags & PUSH_FRONT);
    if (!(windowFlags & TRANSPARENT)) {
      invalidate();
    }
  }
}
extern lv_obj_t * canvas;
Window::~Window()
{
  TRACE_WINDOWS("Destroy %p %s", this, getWindowDebugString().c_str());
  if (focusWindow == this) {
    focusWindow = nullptr;
  }

  if(children.size() > 0)
    deleteChildren();
  if (lvobj != nullptr && lvobj != canvas)
  {
    lv_obj_del(lvobj);
    lvobj = nullptr;
  }

}

void Window::attach(Window * newParent)
{
  if (parent) detach();
  parent = newParent;
  if (newParent)
  {
    newParent->addChild(this);
  }
}

void Window::detach()
{
  if (parent) {
    parent->removeChild(this);
    parent = nullptr;
    if(lvobj != nullptr)
      lv_obj_set_parent(lvobj, nullptr);
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
  scrollPositionX = 0;
  scrollPositionY = 0;
  innerWidth = rect.w;
  innerHeight = rect.h;
  deleteChildren();
  if(lvobj != nullptr)
    lv_obj_clean(lvobj);
  invalidate();
}

void Window::clearLvgl()
{
  for (auto window: children)
  {
    if(lvobj != nullptr && window->lvobj != nullptr && window->lvobj->parent == lvobj)
      window->clearLvgl();
  }
  lvobj = nullptr;
}

void Window::deleteChildren()
{
  for (auto window: children) {
    window->clearLvgl();
    window->deleteLater(false);
  }
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
    if (!lv_obj_has_state(lvobj, LV_STATE_FOCUSED)) {
      lv_obj_add_state(lvobj, LV_STATE_FOCUSED);
      if (focusWindow != nullptr) {
        lv_obj_clear_state(focusWindow->lvobj, LV_STATE_FOCUSED | LV_STATE_EDITED);
      }
    }

    // scroll before calling focusHandler so that the window can adjust the scroll position if needed
    Window * parent = this->parent;
    while (parent && parent->getWindowFlags() & FORWARD_SCROLL) {
      parent = parent->parent;
    }
    if (parent) {
      parent->scrollTo(this);
      invalidate();
    }

    clearFocus();
    focusWindow = this;
    if (focusHandler) {
      focusHandler(true);
    }
  }
}

void Window::setScrollPositionX(coord_t value)
{
  auto newScrollPosition = max<coord_t>(0, min<coord_t>(innerWidth - width(), value));
  if (newScrollPosition != scrollPositionX) {
    scrollPositionX = newScrollPosition;
    invalidate();
  }
}

void Window::setScrollPositionY(coord_t value)
{
  auto newScrollPosition = min<coord_t>(innerHeight - height(), value);

  if (newScrollPosition < 0 && innerHeight != INFINITE_HEIGHT) {
    newScrollPosition = 0;
  }

  if (newScrollPosition != scrollPositionY) {
    scrollPositionY = newScrollPosition;
    invalidate();
  }
}

void Window::scrollTo(Window * child)
{
  coord_t offsetX = 0;
  coord_t offsetY = 0;

  Window * parentWindow = child->getParent();
  while (parentWindow && parentWindow != this) {
    offsetX += parentWindow->left();
    offsetY += parentWindow->top();
    parentWindow = parentWindow->getParent();
  }

  const rect_t scrollRect = {
    offsetX + child->left(),
    offsetY + child->top(),
    min(child->width(), width()),
    min(child->height(), height())
  };

  scrollTo(scrollRect);
}

void Window::scrollTo(const rect_t & rect)
{
  if (rect.top() < scrollPositionY) {
    auto y =
      pageHeight ? rect.top() - (rect.top() % pageHeight) : rect.top();
    lv_obj_scroll_to_y(lvobj, y, LV_ANIM_OFF);
  } else if (rect.bottom() > scrollPositionY + height()) {
    auto y = pageHeight ? rect.top() - (rect.top() % pageHeight)
                        : rect.bottom() - height();
    lv_obj_scroll_to_y(lvobj, y, LV_ANIM_OFF);
  }

  if (rect.left() < scrollPositionX) {
    auto x = pageWidth ? rect.left() - (rect.left() % pageWidth) : rect.left();
    lv_obj_scroll_to_x(lvobj, x, LV_ANIM_OFF);
  }
  else if (rect.right() > scrollPositionX + width()) {
    auto x = pageWidth ? rect.left() - (rect.left() % pageWidth) : rect.right() - width();
    lv_obj_scroll_to_x(lvobj, x, LV_ANIM_OFF);
  }
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

  if (!(windowFlags & NO_SCROLLBAR)) {
    drawVerticalScrollbar(dc);
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

void Window::setInsideParentScrollingArea()
{
  Window * parent = getParent();
  while (parent && parent->getWindowFlags() & FORWARD_SCROLL) {
    parent = parent->parent;
  }
  if (parent) {
    parent->scrollTo(this);
    invalidate();
  }
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

    dc->setOffset(x + child->rect.x - child->scrollPositionX, y + child->rect.y - child->scrollPositionY);
    dc->setClippingRect(max(xmin, x + child->rect.left()),
                        min(xmax, x + child->rect.right()),
                        max(ymin, y + child->rect.top()),
                        min(ymax, y + child->rect.bottom()));
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
  startX += getScrollPositionX();
  startY += getScrollPositionY();

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
  coord_t bottomMax = 0;
  for (auto child: children) {
    bottomMax = max(bottomMax, child->rect.y + child->rect.h);
  }
  setInnerHeight(bottomMax);
}

coord_t Window::adjustHeight()
{
  coord_t old = rect.h;
  adjustInnerHeight();
  rect.h = innerHeight;
  lv_obj_set_style_height(getLvObj(), rect.h, LV_PART_MAIN);
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

void Window::invalidate(const rect_t & rect)
{
  if (isVisible()) {
    parent->invalidate({this->rect.x + rect.x - parent->scrollPositionX, this->rect.y + rect.y - parent->scrollPositionY, rect.w, rect.h});
  }
}

void Window::drawVerticalScrollbar(BitmapBuffer * dc)
{
  if (innerHeight > rect.h) {
    coord_t yofs = divRoundClosest(rect.h * scrollPositionY, innerHeight);
    coord_t yhgt = divRoundClosest(rect.h * rect.h, innerHeight);
    if (yhgt < 15)
      yhgt = 15;
    if (yhgt + yofs > rect.h)
      yhgt = rect.h - yofs;
    dc->drawSolidFilledRect(rect.w - SCROLLBAR_WIDTH, scrollPositionY + yofs, SCROLLBAR_WIDTH, yhgt, COLOR_THEME_PRIMARY3);
  }
}

void Window::drawHorizontalScrollbar(BitmapBuffer * dc)
{
  if (innerWidth > rect.w) {
    coord_t xofs = divRoundClosest(rect.w * scrollPositionX, innerWidth);
    coord_t xwdth = divRoundClosest(rect.w * rect.w, innerWidth);
    if (xwdth < 15)
      xwdth = 15;
    if (xwdth + xofs > rect.w)
      xwdth = rect.w - xofs;
    dc->drawSolidFilledRect(scrollPositionX + xofs, rect.h - SCROLLBAR_WIDTH, xwdth, SCROLLBAR_WIDTH, COLOR_THEME_PRIMARY3);
  }
}
