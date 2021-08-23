class Window
{
  public:
    Window(coord_t x, coord_t y, coord_t width, coord_t height):
      x(x),
      y(y),
      width(width),
      height(height),
      offsetX(0),
      offsetY(0),
      innerWidth(width),
      innerHeight(height),
      focusX(-1),
      focusY(-1)
    {
    }

    coord_t x, y, width, height, offsetX, offsetY, innerWidth, innerHeight, focusX, focusY;

    void scroll() {
      if (touchState.Event == TE_SLIDE) {
        if (innerHeight > height && touchState.startX >= this->x && touchState.startX <= this->x + this->width) {
          this->offsetY = limit<coord_t>(-innerHeight + height, this->offsetY + touchState.Y - touchState.startY, 0);
          touchState.startY = touchState.Y;
        }
        if (innerWidth > width && touchState.startY >= this->y && touchState.startY <= this->y + this->height) {
          this->offsetX = limit<coord_t>(-innerWidth + width, this->offsetX + touchState.X - touchState.startX, 0);
          touchState.startX = touchState.X;
        }
      }
    }

    bool touchPressed(coord_t x, coord_t y, coord_t w, coord_t h) {
      return ::touchPressed(this->x + this->offsetX + x, this->y + this->offsetY + y, w, h);
    }

    void setHeight(coord_t height) {
      this->height = height;
    }

    void setInnerWidth(coord_t value) {
      innerWidth = value;
    }

    void setInnerHeight(coord_t value) {
      innerHeight = value;
    }

    void setFocus(coord_t x, coord_t y) {
      focusX = x;
      focusY = y;
    }

    void clearFocus() {
      setFocus(-1, -1);
    }

    bool hasFocus(coord_t x, coord_t y) {
      return (x == focusX && y == focusY);
    }

    void drawVerticalScrollbar();

    void drawSolidFilledRect(coord_t x, coord_t y, coord_t w, coord_t h, LcdFlags flags)
    {
      setClippingRect();
      lcd->drawSolidFilledRect(this->x + this->offsetX + x, this->y + this->offsetY + y, w, h, flags);
      clearLcdLimits();
    }

    template<class T>
    void drawBitmap(coord_t x, coord_t y, const T * bmp, coord_t srcx=0, coord_t srcy=0, coord_t w=0, coord_t h=0, float scale=0) {
      if (this->offsetX + x >= this->width || this->offsetY + y >= this->height) {
        return;
      }
      setClippingRect();
      lcd->drawBitmap(this->x + this->offsetX + x, this->y + this->offsetY + y, bmp, srcx, srcy, w, h, scale);
      clearLcdLimits();
    }

    inline void setClippingRect() {
      lcd->setClippingRect(this->x, this->x + this->width, this->y, this->y + this->height);
    }

    inline void clearLcdLimits() {
      lcd->clearClippingRect();
    }

    void drawSizedText(coord_t x, coord_t y, const char * s, uint8_t len, LcdFlags flags) {
      setClippingRect();
      lcd->drawSizedText(this->x + this->offsetX + x, this->y + this->offsetY + y, s, len, flags);
      clearLcdLimits();
    }

};
template <class W>
bool Subtitle(W * window, rect_t & rect, const char * label)
{
  rect.h = 14;
  drawText(window, rect.x, rect.y, label, FONT(BOLD));
}

template <class W>
bool CheckBox(W * window, rect_t & rect, const char * label, uint8_t & value, LcdFlags flags=0)
{
  rect.h = 30;
  bool hasFocus = (flags & HAS_FOCUS) || window->hasFocus(rect.x, rect.y);
  drawText(window, rect.x, rect.y+4, label);
  coord_t x = rect.x + rect.w - 20;
  if (hasFocus) {
    window->drawSolidFilledRect(x-1, rect.y+8, 14, 14, COLOR_THEME_FOCUS);
    window->drawSolidFilledRect(x+1, rect.y+10, 10, 10, COLOR_THEME_SECONDARY3);
    if (value) {
      window->drawSolidFilledRect(x+2, rect.y+11, 8, 8, COLOR_THEME_FOCUS);
    }
  }
  else {
    if (value) {
      window->drawSolidFilledRect(x+2, rect.y+11, 8, 8, COLOR_THEME_FOCUS);
      drawSolidRect(window, x, rect.y+9, 12, 12, 1, COLOR_THEME_PRIMARY3);
    }
    else {
      drawSolidRect(window, x, rect.y+9, 12, 12, 1, COLOR_THEME_PRIMARY3);
    }
  }
  if (window->touchPressed(rect.x, rect.y, rect.w, rect.h)) {
    value ^= 1;
    window->setFocus(rect.x, rect.y);
    return true;
  }
  else {
    return false;
  }
}


template <class W>
void drawWidgetLabel(W * window, rect_t & rect, const char * label) {
  drawText(window, rect.x, rect.y, label, TINSIZE);
}

template <class W>
void drawWidgetLine(W * window, rect_t & rect, LcdFlags color) {
  window->drawSolidFilledRect(rect.x, rect.y + rect.h, rect.w, 1, color);
}

template <class W>
bool Choice(W * window, rect_t & rect, const char * label, const char * values, uint8_t & value, uint8_t vmin, uint8_t vmax, LcdFlags flags=0)
{
  rect.h = 30;
  bool hasFocus = (flags & HAS_FOCUS) || window->hasFocus(rect.x, rect.y);
  LcdFlags textColor = 0;
  LcdFlags lineColor = COLOR_THEME_DISABLED;
  if (hasFocus) {
    textColor = COLOR_THEME_FOCUS;
    lineColor = COLOR_THEME_FOCUS;
  }
  drawTextAtIndex(window, rect.x, rect.y + 9, values, value, textColor);
  drawWidgetLine(window, rect, lineColor);
  drawWidgetLabel(window, rect, label);
  if (window->touchPressed(rect.x, rect.y, rect.w, rect.h)) {
    // value ^= 1;
    window->setFocus(rect.x, rect.y);
    return true;
  }
  else {
    return false;
  }
}


template <class W>
bool TextEdit(W * window, rect_t & rect, const char * label, char * value, uint8_t length, LcdFlags flags=0)
{
  rect.h = 30;
  bool hasFocus = (flags & HAS_FOCUS) || window->hasFocus(rect.x, rect.y);
  LcdFlags textColor = 0;
  LcdFlags lineColor = COLOR_THEME_DISABLED;
  if (hasFocus) {
    textColor = COLOR_THEME_FOCUS;
    lineColor = COLOR_THEME_FOCUS;
  }
  if (!hasFocus && zlen(value, length) == 0)
    menuBodyWindow.drawSizedText(rect.x, rect.y + 9, "---", length, textColor);
  else
    menuBodyWindow.drawSizedText(rect.x, rect.y + 9, value, length, ZCHAR | textColor);
  drawWidgetLine(window, rect, lineColor);
  drawWidgetLabel(window, rect, label);
  if (window->touchPressed(rect.x, rect.y, rect.w, rect.h)) {
    // value ^= 1;
    window->setFocus(rect.x, rect.y);
    return true;
  }
  else {
    return false;
  }
}
#define CHECKBOX(X, label, value) { rect.x = X; rect.w = LCD_W - (X) - 20; uint8_t _value = value; CheckBox(window, rect, label, _value); value = _value; rect.y += rect.h + 10; }
#define CHECKBOX_INVERTED(X, label, value) { rect.x = X; rect.w = LCD_W - (X) - 20; uint8_t _value = !value; CheckBox(window, rect, label, _value); value = !_value; rect.y += rect.h + 10; }
#endif