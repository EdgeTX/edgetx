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

#include "window.h"

#include "button.h"
#include "form.h"
#include "static.h"
#include "etx_lv_theme.h"

std::list<Window *> Window::trash;
bool Window::_longPressed = false;

const lv_obj_class_t window_base_class = {
    .base_class = &lv_obj_class,
    .constructor_cb = nullptr,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = LV_DPI_DEF,
    .height_def = LV_DPI_DEF,
    .editable = LV_OBJ_CLASS_EDITABLE_FALSE,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_FALSE,
    .instance_size = sizeof(lv_obj_t)};

lv_obj_t *window_create(lv_obj_t *parent)
{
  return etx_create(&window_base_class, parent);
}

void Window::window_event_cb(lv_event_t *e)
{
  Window *window = (Window *)lv_obj_get_user_data(lv_event_get_target(e));
  if (window) 
    window->eventHandler(e);
}

void Window::eventHandler(lv_event_t *e)
{
  lv_obj_t *target = lv_event_get_target(e);
  lv_event_code_t code = lv_event_get_code(e);

  if (code == LV_EVENT_DELETE || deleted()) return;

  switch (code) {
    case LV_EVENT_SCROLL: {
      // exclude pointer based scrolling (only focus scrolling)
      if (!lv_obj_is_scrolling(target)) {
        lv_point_t *p = (lv_point_t *)lv_event_get_param(e);
        lv_coord_t scroll_bottom = lv_obj_get_scroll_bottom(target);

        lv_coord_t scroll_y = lv_obj_get_scroll_y(target);
        TRACE("SCROLL[x=%d;y=%d;top=%d;bottom=%d]", p->x, p->y, scroll_y,
              scroll_bottom);

        if (scroll_y <= 45 && p->y > 0) {
          lv_obj_scroll_by(target, 0, scroll_y, LV_ANIM_OFF);
        } else if (scroll_bottom <= 16 && p->y < 0) {
          lv_obj_scroll_by(target, 0, -scroll_bottom, LV_ANIM_OFF);
        }
      }
    } break;
    case LV_EVENT_CLICKED:
      if (!_longPressed) {
        TRACE("CLICKED[%p]", this);
        onClicked();
      }
      _longPressed = false;
      break;
    case LV_EVENT_CANCEL:
      TRACE("CANCEL[%p]", this);
      onCancel();
      break;
    case LV_EVENT_FOCUSED:
      if (focusHandler) focusHandler(true);
      break;
    case LV_EVENT_DEFOCUSED:
      if (focusHandler) focusHandler(false);
      break;
    case LV_EVENT_LONG_PRESSED:
      TRACE("LONG PRESS[%p]", this);
      _longPressed = onLongPress();
      break;
    default:
      break;
  }
}

// Constructor to allow lvobj to be created separately - used by NumberEdit and
// TextEdit
Window::Window(const rect_t &rect) : rect(rect), parent(nullptr)
{
  lvobj = nullptr;
}

Window::Window(Window *parent, const rect_t &rect, LvglCreate objConstruct) :
    rect(rect), parent(parent)
{
  lv_obj_t *lv_parent = parent ? parent->lvobj : nullptr;

  if (objConstruct == nullptr) objConstruct = window_create;
  lvobj = objConstruct(lv_parent);

  lv_obj_set_user_data(lvobj, this);
  lv_obj_add_event_cb(lvobj, Window::window_event_cb, LV_EVENT_ALL, nullptr);

  if (rect.x || rect.y) lv_obj_set_pos(lvobj, rect.x, rect.y);
  if (rect.w) lv_obj_set_width(lvobj, rect.w);
  if (rect.h) lv_obj_set_height(lvobj, rect.h);

  lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_SCROLL_ELASTIC);

  if (parent) {
    parent->addChild(this);
  }
}

Window::~Window()
{
  TRACE_WINDOWS("Destroy %p %s", this, getWindowDebugString().c_str());

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

std::string Window::getWindowDebugString(const char *name) const
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

void Window::setWindowFlag(WindowFlags flag)
{
  windowFlags |= flag;

  // honor the no focus flag of libopenui
  if (this->windowFlags & NO_FOCUS)
    lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_CLICK_FOCUSABLE);
}

void Window::clearWindowFlag(WindowFlags flag) { windowFlags &= ~flag; }

void Window::setTextFlag(LcdFlags flag) { textFlags |= flag; }
void Window::clearTextFlag(LcdFlags flag) { textFlags &= ~flag; }

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
  }
}

void Window::deleteLater(bool detach, bool trash)
{
  if (_deleted) return;

  TRACE_WINDOWS("Delete %p %s", this, getWindowDebugString().c_str());

  _deleted = true;

  if (closeHandler) {
    closeHandler();
  }

  if (detach)
    this->detach();
  else
    parent = nullptr;

  if (trash) {
    Window::trash.push_back(this);
  }

  deleteChildren();

  if (lvobj != nullptr) {
    auto obj = lvobj;
    lvobj = nullptr;
    lv_obj_del(obj);
  }
}

void Window::clear()
{
  deleteChildren();
  if (lvobj != nullptr) {
    lv_obj_clean(lvobj);
  }
  invalidate();
}

void Window::deleteChildren()
{
  // prevent LVGL refocus while mass-deleting
  // inhibit_focus = true;
  for (auto window : children) {
    window->deleteLater(false);
  }
  // inhibit_focus = false;
  children.clear();
}

bool Window::hasFocus() const
{
  return lvobj && lv_obj_has_state(lvobj, LV_STATE_FOCUSED);
}

void Window::padLeft(coord_t pad) { lv_obj_set_style_pad_left(lvobj, pad, 0); }

void Window::padRight(coord_t pad)
{
  lv_obj_set_style_pad_right(lvobj, pad, 0);
}

void Window::padTop(coord_t pad) { lv_obj_set_style_pad_top(lvobj, pad, 0); }

void Window::padBottom(coord_t pad)
{
  lv_obj_set_style_pad_bottom(lvobj, pad, 0);
}

void Window::padAll(PaddingSize pad) { etx_padding(lvobj, pad, LV_PART_MAIN); }

void Window::padRow(coord_t pad) { lv_obj_set_style_pad_row(lvobj, pad, 0); }

void Window::padColumn(coord_t pad)
{
  lv_obj_set_style_pad_column(lvobj, pad, 0);
}

void Window::bringToTop()
{
  attach(parent);  // does a detach + attach
  if (lvobj && lv_obj_get_parent(lvobj)) lv_obj_move_foreground(lvobj);
}

void Window::checkEvents()
{
  auto copy = children;
  for (auto child : copy) {
    if (!child->deleted()) {
      child->checkEvents();
    }
  }
}

void Window::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X",
                Window::getWindowDebugString("Window").c_str(), event);
  if (parent)
    parent->onEvent(event);
}

void Window::onClicked()
{
  if (parent && !(windowFlags & OPAQUE))
    parent->onClicked();
}

void Window::onCancel()
{
  if (parent)
    parent->onCancel();
}

bool Window::onLongPress()
{
  return true;
}

void Window::addChild(Window *window)
{
  auto lv_parent = lv_obj_get_parent(window->lvobj);
  if (lv_parent && (lv_parent != lvobj)) {
    lv_obj_set_parent(window->lvobj, lvobj);
  }

  children.push_back(window);
}

void Window::removeChild(Window *window)
{
  children.remove(window);
  invalidate();
}

void Window::invalidate()
{
  if (lvobj) lv_obj_invalidate(lvobj);
}

void Window::setFlexLayout(lv_flex_flow_t flow, lv_coord_t padding,
                           coord_t width, coord_t height)
{
  lv_obj_set_flex_flow(lvobj, flow);
  if (_LV_FLEX_COLUMN & flow) {
    lv_obj_set_style_pad_row(lvobj, padding, LV_PART_MAIN);
  } else {
    lv_obj_set_style_pad_column(lvobj, padding, LV_PART_MAIN);
  }
  lv_obj_set_width(lvobj, width);
  lv_obj_set_height(lvobj, height);
}

FormLine *Window::newLine(FlexGridLayout &layout)
{
  return new FormLine(this, layout);
}

void Window::show(bool visible)
{
  if (!_deleted && lvobj) {
    if (lv_obj_has_flag(lvobj, LV_OBJ_FLAG_HIDDEN) == visible) {
      if (visible)
        lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_HIDDEN);
      else
        lv_obj_add_flag(lvobj, LV_OBJ_FLAG_HIDDEN);
    }
  }
}

void Window::enable(bool enabled)
{
  if (!_deleted && lvobj) {
    if (lv_obj_has_state(lvobj, LV_STATE_DISABLED) == enabled) {
      if (enabled)
        lv_obj_clear_state(lvobj, LV_STATE_DISABLED);
      else
        lv_obj_add_state(lvobj, LV_STATE_DISABLED);
    }
  }
}

#if defined(HARDWARE_TOUCH)
void Window::addBackButton()
{
  new ButtonBase(
      this, {0, 0, EdgeTxStyles::MENU_HEADER_HEIGHT, EdgeTxStyles::MENU_HEADER_HEIGHT},
      [=]() -> uint8_t {
        onCancel();
        return 0;
      },
      window_create);
}
#endif

void NavWindow::onEvent(event_t event)
{
  switch (event) {
#if defined(HARDWARE_KEYS)
    case EVT_KEY_BREAK(KEY_MODEL):
      onPressMDL();
      break;

    case EVT_KEY_LONG(KEY_MODEL):
      onLongPressMDL();
      break;

    case EVT_KEY_BREAK(KEY_SYS):
      onPressSYS();
      break;

    case EVT_KEY_LONG(KEY_SYS):
      onLongPressSYS();
      break;

    case EVT_KEY_BREAK(KEY_TELE):
      onPressTELE();
      break;

    case EVT_KEY_LONG(KEY_TELE):
      onLongPressTELE();
      break;

    case EVT_KEY_BREAK(KEY_PAGEDN):
      onPressPGDN();
      break;

    case EVT_KEY_BREAK(KEY_PAGEUP):
      onPressPGUP();
      break;
#endif

    default:
      if (bubbleEvents()) Window::onEvent(event);
  }
}

NavWindow::NavWindow(Window *parent, const rect_t &rect,
                     LvglCreate objConstruct) :
    Window(parent, rect, objConstruct)
{
  setWindowFlag(OPAQUE);
}

SetupButtonGroup::SetupButtonGroup(Window* parent, const rect_t& rect, const char* title, int cols,
                                   PaddingSize padding, PageDefs pages, coord_t btnHeight) :
    Window(parent, rect)
{
  padAll(padding);

  coord_t buttonWidth = (width() - PAD_SMALL * (cols + 1) - PAD_TINY * 2) / cols;

  int rows = (pages.size() + cols - 1) / cols;
  int height = rows * btnHeight + (rows - 1) * PAD_MEDIUM + PAD_TINY * 2;
  if (title) {
    height += EdgeTxStyles::PAGE_LINE_HEIGHT + PAD_TINY;
  }
  setHeight(height);

  if (title)
    new Subtitle(this, title);

  int n = 0;
  int remaining = pages.size();
  coord_t yo = title ? EdgeTxStyles::PAGE_LINE_HEIGHT + PAD_TINY : 0;
  coord_t xw = buttonWidth + PAD_SMALL;
  coord_t xo = (width() - (cols * xw - PAD_SMALL)) / 2;
  coord_t x, y;
  for (auto& entry : pages) {
    if (remaining < cols && (n % cols == 0)) {
      coord_t space = ((cols - remaining) * xw) / (remaining + 1);
      xw += space;
      xo += space;
    }
    x = xo + (n % cols) * xw;
    y = yo + (n / cols) * (btnHeight + PAD_MEDIUM);

    // TODO: sort out all caps title strings VS quick menu strings
    std::string title(entry.title);
    for (std::string::iterator it = title.begin(); it != title.end(); ++it) {
      if (*it == '\n')
        *it = ' ';
    }

    auto btn = new TextButton(this, rect_t{x, y, buttonWidth, btnHeight}, title, [&, entry]() {
      entry.createPage();
      return 0;
    });
    btn->setWrap();
    if (entry.isActive) btn->setCheckHandler([=]() { btn->check(entry.isActive()); });
    n += 1;
    remaining -= 1;
  }
}

SetupLine::SetupLine(Window* parent, coord_t y, coord_t col2, PaddingSize padding, const char* title,
                    std::function<void(Window*, coord_t, coord_t)> createEdit, coord_t lblYOffset) :
    Window(parent, {0, y, LCD_W - padding * 2, 0})
{
  padAll(PAD_ZERO);
  coord_t titleY = PAD_LARGE + lblYOffset;
  coord_t titleH = EdgeTxStyles::PAGE_LINE_HEIGHT;
  coord_t h = EdgeTxStyles::UI_ELEMENT_HEIGHT + PAD_TINY * 2 + lblYOffset * 2;
  if (createEdit) {
    coord_t editY = PAD_TINY;
    coord_t lblWidth = col2 - PAD_SMALL - PAD_TINY;
    if (title) {
      if (getTextWidth(title) >= lblWidth) {
        h += PAD_MEDIUM;
        titleY = 0;
        titleH = EdgeTxStyles::UI_ELEMENT_HEIGHT + PAD_TINY + PAD_LARGE;
        editY = PAD_SMALL + 1;
      }
      new StaticText(this, {PAD_TINY, titleY, lblWidth, titleH}, title);
    }
    setHeight(h);
    createEdit(this, col2, editY);
  } else {
    setHeight(h);
    new StaticText(this, {0, titleY, 0, titleH}, title, COLOR_THEME_PRIMARY1_INDEX, FONT(BOLD));
  }
}

coord_t SetupLine::showLines(Window* parent, coord_t y, coord_t col2, PaddingSize padding, SetupLineDef* setupLines, int lineCount)
{
  Window* w;

  for (int i = 0; i < lineCount; i += 1) {
    w = new SetupLine(parent, y, col2, padding, setupLines[i].title, setupLines[i].createEdit);
    y += w->height() + padding;
  }

  return y;
}
