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

#ifndef _TABLE_H_
#define _TABLE_H_

#include "window.h"
#include <vector>
#include "libopenui_config.h"

class Table: public Window {
    class Line {
      public:
        std::list<std::string> values;
        std::function<void()> onPress;
    };

    class Header: public Window {
      public:
        Header(Window * parent, const rect_t & rect):
          Window(parent, rect, OPAQUE)
        {
        }

        void setLine(const Line & line)
        {
          header = line;
        }

        void paint(BitmapBuffer * dc) override;

      protected:
        Line header;
    };

    class Body: public Window {
      friend class Table;

      public:
        Body(Window * parent, const rect_t & rect):
          Window(parent, rect, OPAQUE)
        {
        }

        void addLine(const Line & line)
        {
          lines.push_back(line);
          setInnerHeight(lines.size() * TABLE_LINE_HEIGHT - 2);
        }

        void clear()
        {
          lines.clear();
        }

        void paint(BitmapBuffer * dc) override;

#if defined(HARDWARE_TOUCH)
        bool onTouchEnd(coord_t x, coord_t y)
        {
          unsigned index = y / TABLE_LINE_HEIGHT;
          auto onPress = lines[index].onPress;
          if (onPress)
            onPress();
          return true;
        }
#endif

      protected:
        std::vector<Line> lines;
        int selection = -1;
    };

  public:
    Table(Window * parent, const rect_t & rect, uint8_t columnsCount):
      Window(parent, rect),
      header(this, {0, 0, width(), TABLE_HEADER_HEIGHT}),
      body(this, {0, TABLE_HEADER_HEIGHT, width(), height() - TABLE_HEADER_HEIGHT})
    {
      setColumnsCount(columnsCount);
    }

    ~Table() override
    {
      header.detach();
      body.detach();
    }

    void setColumnsCount(uint8_t value)
    {
      count = value;
    }

    int getSelection() const
    {
      return body.selection;
    }

    void setSelection(int index, bool scroll = false)
    {
      body.selection = index;
      if (scroll) {
        body.setScrollPositionY(index * TABLE_LINE_HEIGHT);
      }
      body.invalidate();
    }

    void setHeader(const char * values[])
    {
      Line line;
      for (uint8_t i = 0; i < count; i++) {
        line.values.push_back(values[i]);
      }
      header.setLine(line);
    }

    void addLine(const char * values[], std::function<void()> onPress = nullptr)
    {
      Line line;
      for (uint8_t i = 0; i < count; i++) {
        line.values.push_back(values[i]);
      }
      line.onPress = onPress;
      body.addLine(line);
    }

    void clear()
    {
      body.clear();
    }

  protected:
    uint8_t count = 1;
    Header header;
    Body body;
};

#endif // _TABLE_H_
