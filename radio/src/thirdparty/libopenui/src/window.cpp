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

Window * Window::focusWindow = nullptr;
std::list<Window *> Window::trash;

Window::Window(Window * parent, const rect_t & rect, WindowFlags windowFlags, LcdFlags textFlags):
  parent(parent),
  rect(rect),
  innerWidth(rect.w),
  innerHeight(rect.h),
  windowFlags(windowFlags),
  textFlags(textFlags)
{
  if (parent) {
    parent->addChild(this);
    if (!(windowFlags & TRANSPARENT)) {
      invalidate();
    }
  }
}

Window::~Window()
{
  TRACE_WINDOWS("Destroy %p %s", this, getWindowDebugString().c_str());

  if (focusWindow == this) {
    focusWindow = nullptr;
  }

  deleteChildren();
}

void Window::attach(Window * window)
{
  if (parent)
    detach();
  parent = window;
  parent->addChild(this);
}

void Window::detach()
{
  if (parent) {
    parent->removeChild(this);
    parent = nullptr;
  }
}

void Window::deleteLater(bool detach)
{
  TRACE_WINDOWS("Delete %p %s", this, getWindowDebugString().c_str());

  if (static_cast<Window *>(focusWindow) == static_cast<Window *>(this)) {
    focusWindow = nullptr;
  }

  if (detach)
    this->detach();
  else
    parent = nullptr;

  if (closeHandler) {
    closeHandler();
  }

  trash.push_back(this);
}

void Window::clear()
{
  scrollPositionX = 0;
  scrollPositionY = 0;
  innerWidth = rect.w;
  innerHeight = rect.h;
  deleteChildren();
  invalidate();
}

void Window::deleteChildren()
{
  for (auto window: children) {
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

void Window::setFocus(uint8_t flag)
{
  TRACE_WINDOWS("%s setFocus()", getWindowDebugString().c_str());

  if (focusWindow != this) {
    clearFocus();
    focusWindow = this;
    if (focusHandler) {
      focusHandler();
    }
  }

  Window * parent = this->parent;
  while (parent && parent->getWindowFlags() & FORWARD_SCROLL) {
    parent = parent->parent;
  }

  if (parent) {
    parent->scrollTo(this);
    invalidate();
  }
}

void Window::setScrollPositionX(coord_t value)
{
  coord_t newScrollPosition = max<coord_t>(0, min<coord_t>(innerWidth - width(), value));
  if (newScrollPosition != scrollPositionX) {
    scrollPositionX = newScrollPosition;
    invalidate();
  }
}

void Window::setScrollPositionY(coord_t value)
{
  coord_t newScrollPosition = max<coord_t>(0, min<coord_t>(innerHeight - height(), value));
  if (newScrollPosition != scrollPositionY) {
    scrollPositionY = newScrollPosition;
    invalidate();
  }
}

void Window::scrollTo(Window * child)
{
  coord_t offsetX = 0;
  coord_t offsetY = 0;

  Window * parent = child->getParent();
  while (parent && parent != this) {
    offsetX += parent->left();
    offsetY += parent->top();
    parent = parent->getParent();
  }

  coord_t left = offsetX + child->left();
  coord_t right = offsetX + child->right();
  coord_t top = offsetY + child->top();
  coord_t bottom = offsetY + child->bottom();

  if (top < scrollPositionY) {
    setScrollPositionY(pageHeight ? top - (top % pageHeight) : top - 5);
  }
  else if (bottom > scrollPositionY + height() - 5) {
    setScrollPositionY(pageHeight ? top - (top % pageHeight) : bottom - height() + 5);
  }

  if (left < scrollPositionX) {
    setScrollPositionX(pageWidth ? left - (left % pageWidth) : left - 5);
  }
  else if (right > scrollPositionX + width() - 5) {
    setScrollPositionX(pageWidth ? left - (left % pageWidth) : right - width() + 5);
  }
}

void Window::fullPaint(BitmapBuffer * dc)
{
  TRACE_WINDOWS("%s", getWindowDebugString().c_str());

  if (windowFlags & PAINT_CHILDREN_FIRST) {
    coord_t xmin, xmax, ymin, ymax;
    coord_t x = dc->getOffsetX();
    coord_t y = dc->getOffsetY();
    dc->getClippingRect(xmin, xmax, ymin, ymax);
    paintChildren(dc);
    dc->setOffset(x, y);
    dc->setClippingRect(xmin, xmax, ymin, ymax);
  }

  paint(dc);

  if (!(windowFlags & NO_SCROLLBAR)) {
    drawVerticalScrollbar(dc);
  }

  if (!(windowFlags & PAINT_CHILDREN_FIRST)) {
    paintChildren(dc);
  }
}

bool Window::isChildFullSize(Window * child)
{
  return child->top() == 0 && child->height() == height() && child->left() == 0 && child->width() == width();
}

bool Window::isChildVisible(Window * window)
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

void Window::paintChildren(BitmapBuffer * dc)
{
  coord_t x = dc->getOffsetX();
  coord_t y = dc->getOffsetY();
  coord_t xmin, xmax, ymin, ymax;
  dc->getClippingRect(xmin, xmax, ymin, ymax);

  auto it = children.end();

  while(it != children.begin()) {
    auto child = *(--it);
    if ((child->windowFlags & OPAQUE) && isChildFullSize(child)) {
      break;
    }
  }

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

void Window::checkEvents()
{
  auto copy = children;
  for (auto child: copy) {
    child->checkEvents();
  }

  if (this == focusWindow) {
    event_t event = getWindowEvent();
    if (event) {
      TRACE_WINDOWS("Event 0x%x received ...", event);
      this->onEvent(event);
    }
  }

  if (windowFlags & REFRESH_ALWAYS) {
    invalidate();
  }

#if defined(HARDWARE_TOUCH)
  if (touchState.event != TE_SLIDE) {
    if (pageWidth) {
      coord_t relativeScrollPosition = scrollPositionX % pageWidth;
      if (relativeScrollPosition) {
        setScrollPositionX(getScrollPositionX() - relativeScrollPosition + (relativeScrollPosition > pageWidth / 2 ? pageWidth : 0));
      }
    }
    if (pageHeight) {
      coord_t relativeScrollPosition = scrollPositionY % pageHeight;
      if (relativeScrollPosition) {
        setScrollPositionY(getScrollPositionY() - relativeScrollPosition + (relativeScrollPosition > pageHeight / 2 ? pageHeight : 0));
      }
    }
  }
#endif
}

void Window::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", Window::getWindowDebugString().c_str(), event);
  if (parent) {
    parent->onEvent(event);
  }
}

#if defined(HARDWARE_TOUCH)
bool Window::onTouchStart(coord_t x, coord_t y)
{
  for (auto it = children.rbegin(); it != children.rend(); ++it) {
    auto child = *it;
    if (pointInRect(x, y, child->rect)) {
      if (child->onTouchStart(x - child->rect.x + child->scrollPositionX, y - child->rect.y + child->scrollPositionY)) {
        return true;
      }
    }
  }

  return false;
}

bool Window::onTouchEnd(coord_t x, coord_t y)
{
  for (auto it = children.rbegin(); it != children.rend(); ++it) {
    auto child = *it;
    if (pointInRect(x, y, child->rect)) {
      if (child->onTouchEnd(x - child->rect.x + child->scrollPositionX, y - child->rect.y + child->scrollPositionY)) {
        return true;
      }
    }
  }

  return windowFlags & OPAQUE;
}

bool Window::onTouchSlide(coord_t x, coord_t y, coord_t startX, coord_t startY, coord_t slideX, coord_t slideY)
{
  for (auto it = children.rbegin(); it != children.rend(); ++it) {
    auto child = *it;
    if (pointInRect(startX, startY, child->rect)) {
      if (child->onTouchSlide(x - child->rect.x, y - child->rect.y, startX - child->rect.x, startY - child->rect.y, slideX, slideY)) {
        return true;
      }
    }
  }

  if (slideY && innerHeight > rect.h) {
    setScrollPositionY(scrollPositionY - slideY);
    return true;
  }

  if (slideX && innerWidth > rect.w) {
    setScrollPositionX(scrollPositionX - slideX);
    return true;
  }

  return false;
}
#endif

void Window::adjustInnerHeight()
{
  innerHeight = 0;
  for (auto child: children) {
    innerHeight = max(innerHeight, child->rect.y + child->rect.h);
  }
}

coord_t Window::adjustHeight()
{
  adjustInnerHeight();
  coord_t old = rect.h;
  rect.h = innerHeight;
  return rect.h - old;
}

void Window::moveWindowsTop(coord_t y, coord_t delta)
{
  for (auto child: children) {
    if (child->rect.y > y) {
      child->rect.y += delta;
    }
  }
  innerHeight += delta;
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
    dc->drawSolidFilledRect(rect.w - SCROLLBAR_WIDTH, scrollPositionY + yofs, SCROLLBAR_WIDTH, yhgt, SCROLLBAR_COLOR);
  }
}
