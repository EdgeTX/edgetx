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

#pragma once

#include <utility>
#include <vector>
#include "form.h"
#include "libopenui_config.h"
#include "font.h"

class Table: public FormField
{
  public:
    class Cell
    {
      public:
        virtual ~Cell() = default;

        virtual void paint(BitmapBuffer * dc, coord_t x, coord_t y, LcdFlags flags) = 0;

        virtual bool needsInvalidate() = 0;
    };

    class StringCell : public Cell
    {
      public:
        StringCell() = default;

        explicit StringCell(std::string value):
          value(std::move(value))
        {
        }

        void paint(BitmapBuffer * dc, coord_t x, coord_t y, LcdFlags flags) override
        {
          dc->drawText(x, y - 2 + (TABLE_LINE_HEIGHT - getFontHeight(TABLE_HEADER_FONT)) / 2 + 3, value.c_str(), flags);
        }

        bool needsInvalidate() override
        {
          return false;
        }

      protected:
        std::string value;
    };

    class DynamicStringCell : public Cell
    {
      public:
        explicit DynamicStringCell(std::function<std::string()> getText):
          getText(std::move(getText))
        {
        }

        void paint(BitmapBuffer * dc, coord_t x, coord_t y, LcdFlags flags) override
        {
          auto text = getText();
          dc->drawText(x, y - 2 + (TABLE_LINE_HEIGHT - getFontHeight(TABLE_BODY_FONT)) / 2 + 3, text.c_str(), SPACING_NUMBERS_CONST | flags);
        }

        bool needsInvalidate() override
        {
          return true; // TODO optimize this
        }

      protected:
        std::function<std::string()> getText;
    };

    class CustomCell : public Cell
    {
      public:
        explicit CustomCell(std::function<void(BitmapBuffer * /*dc*/, coord_t /*x*/, coord_t /*y*/, LcdFlags /*flags*/)> paintFunction):
          paintFunction(std::move(paintFunction))
        {
        }

        void paint(BitmapBuffer * dc, coord_t x, coord_t y, LcdFlags flags) override
        {
          paintFunction(dc, x, y, flags);
        }

        bool needsInvalidate() override
        {
          return true;
        }

      protected:
        std::function<void(BitmapBuffer * dc, coord_t x, coord_t y, LcdFlags flags)> paintFunction;
    };

    class Line
    {
      public:
        explicit Line(uint8_t columnsCount):
          cells(columnsCount, nullptr)
        {
        }

        ~Line()
        {
          for (auto cell: cells) {
            delete cell;
          }
        }
        std::vector<Cell *> cells;
        std::function<void()> onPress;
        std::function<void()> onSelect;
        LcdFlags flags = TABLE_BODY_FONT;
    };

    class Header: public Window, public Line
    {
      public:
        Header(Table * parent, const rect_t & rect, uint8_t columnsCount):
          Window(parent, rect, OPAQUE),
          Line(columnsCount)
        {
        }

#if defined(DEBUG_WINDOWS)
        std::string getName() const override
        {
          return "Table::Header";
        }
#endif

        void paint(BitmapBuffer * dc) override;
    };

    class Body: public Window
    {
      friend class Table;

      public:
        Body(Table * parent, const rect_t & rect, WindowFlags windowFlags):
          Window(parent, rect, windowFlags)
        {
        }

        ~Body() override
        {
          clear();
        }

#if defined(DEBUG_WINDOWS)
        std::string getName() const override
        {
          return "Table::Body";
        }
#endif

        void addLine(Line * line)
        {
          lines.push_back(line);
          if (hasFocus() && selection < 0) {
            select(0, true);
          }
        }

        void setLineFlags(uint8_t index, LcdFlags flags)
        {
          if (lines[index]->flags != flags) {
            lines[index]->flags = flags;
            invalidate({0, index * TABLE_LINE_HEIGHT, width(), TABLE_LINE_HEIGHT});
          }
        }

        void clear()
        {
          for (auto line: lines) {
            delete line;
          }
          lines.clear();
        }

        void select(int index, bool scroll)
        {
          selection = index;
          if (scroll) {
            coord_t y = index * TABLE_LINE_HEIGHT;
            Window * window = this;
            while (window->getWindowFlags() & FORWARD_SCROLL) {
              y += window->top();
              window = window->getParent();
            }
            const rect_t rect = {
              0,
              y,
              width(),
              TABLE_LINE_HEIGHT
            };
            window->scrollTo(rect);
          }
          invalidate();
          if (index >= 0) {
            auto onSelect = lines[index]->onSelect;
            if (onSelect) {
              onSelect();
            }
          }
        }

        void paint(BitmapBuffer * dc) override;

#if defined(HARDWARE_KEYS)
        void onEvent(event_t event) override;
#endif

#if defined(HARDWARE_TOUCH)
        bool onTouchEnd(coord_t x, coord_t y) override;
#endif

        void checkEvents() override;

      protected:
        std::vector<Line *> lines;
        int selection = -1;
    };

  public:
    Table(Window * parent, const rect_t & rect, uint8_t columnsCount, WindowFlags windowFlags = OPAQUE | FORM_NO_BORDER):
      FormField(parent, rect, windowFlags),
      columnsCount(columnsCount),
      columnsWidth(columnsCount, width() / columnsCount),
      header(this, {0, 0, width(), 0}, columnsCount),
      body(this, {0, 0, width(), height()}, windowFlags)
    {
    }

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "Table";
    }
#endif

    void deleteLater(bool detach = true, bool trash = true) override // NOLINT(google-default-arguments)
    {
      if (_deleted)
        return;

      header.deleteLater(true, false);
      body.deleteLater(true, false);

      FormField::deleteLater(detach, trash);
    }

    void setColumnsWidth(const coord_t values[])
    {
      int restColumn = -1;
      coord_t restWidth = width();
      for (uint8_t i = 0; i < columnsCount; i++) {
        auto columnWidth = values[i];
        columnsWidth[i] = columnWidth;
        if (columnWidth == 0) {
          restColumn = i;
        }
        else {
          restWidth -= columnWidth;
        }
      }
      if (restColumn >= 0) {
        columnsWidth[restColumn] = restWidth;
      }
    }

    int getSelection() const
    {
      return body.selection;
    }

    void clearSelection()
    {
      body.selection = -1;
      body.invalidate();
    }

    void setFocus(uint8_t flag = SET_FOCUS_DEFAULT, Window * from = nullptr) override // NOLINT(google-default-arguments)
    {
      if (body.lines.empty()) {
        if (flag == SET_FOCUS_BACKWARD) {
          if (previous) {
            previous->setFocus(flag, this);
          }
        }
        else {
          if (next) {
            next->setFocus(flag, this);
          }
        }
      }
      else {
        body.setFocus(flag, from);
        if (body.selection < 0) {
          select(flag == SET_FOCUS_BACKWARD ? (int)body.lines.size() - 1 : 0);
        }
      }
    }

    void select(int index, bool scroll = true)
    {
      body.select(index, scroll);
    }

    void setLineFlags(uint8_t index, LcdFlags flags)
    {
      body.setLineFlags(index, flags);
    }

    void setHeader(const char * const values[])
    {
      header.setHeight(TABLE_HEADER_HEIGHT);
      body.setTop(TABLE_HEADER_HEIGHT);
      body.setHeight(height() - TABLE_HEADER_HEIGHT);
      for (uint8_t i = 0; i < columnsCount; i++) {
        header.cells[i] = new StringCell(values[i]);
      }
    }

    void addLine(Line * line, std::function<void()> onPress = nullptr, std::function<void()> onSelect = nullptr)
    {
      line->onPress = std::move(onPress);
      line->onSelect = std::move(onSelect);
      body.addLine(line);
    }

    void addLine(const char * const values[], std::function<void()> onPress = nullptr, std::function<void()> onSelect = nullptr)
    {
      Line * line = new Line(columnsCount);
      for (uint8_t i = 0; i < columnsCount; i++) {
        line->cells[i] = new StringCell(values[i]);
      }
      addLine(line, std::move(onPress), std::move(onSelect));
    }

    void clear()
    {
      clearSelection();
      body.clear();
    }

    uint8_t size() const
    {
      return body.lines.size();
    }

  protected:
    uint8_t columnsCount;
    std::vector<coord_t> columnsWidth;
    Header header;
    Body body;
};

