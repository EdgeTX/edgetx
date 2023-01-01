/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
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

#ifndef _PAGE_H_
#define _PAGE_H_

#include "window.h"
#include "button.h"
#include "static.h"

class Page;

class PageHeader : public FormWindow
{
 public:
  PageHeader(Page* parent, uint8_t icon);

  uint8_t getIcon() const { return icon; }
  void setTitle(std::string txt) { title->setText(std::move(txt)); }
  StaticText* setTitle2(std::string txt);

  void paint(BitmapBuffer* dc) override;

 protected:
  uint8_t icon;
  StaticText* title;
  StaticText* title2 = nullptr;
};

class Page : public Window
{
 public:
  explicit Page(unsigned icon);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "Page"; }
#endif

  void onCancel() override;
  void onClicked() override;

  void deleteLater(bool detach = true, bool trash = true) override;

 protected:
  PageHeader header;
  FormWindow body;

  void onEvent(event_t event) override;
};

// Helper for building layout
class FormBuilder
{
public:
  static const lv_coord_t default_col_dsc[];
  static const lv_coord_t default_row_dsc[];

  FormBuilder(Window* box, lv_coord_t padding = 4, const lv_coord_t col_dsc[] = default_col_dsc, const lv_coord_t row_dsc[] = default_row_dsc) : m_grid(col_dsc, row_dsc, padding)
  {
    m_form = new FormWindow(box, rect_t{});
    m_form->padAll(0);
    m_form->setFlexLayout();
  }

  FormGroup::Line* newLine(uint8_t pad = 0)
  {
    m_line = m_form->newLine(&m_grid);
    m_line->padAll(lv_dpx(pad));
    lv_obj_set_grid_align(m_line->getLvObj(), LV_GRID_ALIGN_SPACE_BETWEEN, LV_GRID_ALIGN_SPACE_BETWEEN);
    return m_line;
  }

  Window* newBox(coord_t w = 0, coord_t h = 0, coord_t padding = 4)
  {
    auto box = new Window(m_line, rect_t{});
    box->padLeft(lv_dpx(padding));
    box->padRight(lv_dpx(padding));
    if (w)
      box->setWidth(w);
    if (h)
      box->setHeight(h);
    return box;
  }

  void cell(Window* part, lv_grid_align_t x_align, uint8_t col_pos, uint8_t col_span,
                          lv_grid_align_t y_align = LV_GRID_ALIGN_CENTER, uint8_t row_pos = 0, uint8_t row_span = 1)
  {
    lv_obj_set_grid_cell(part->getLvObj(), x_align, col_pos, col_span, y_align, row_pos, row_span);
  }

  FormWindow* form() const { return m_form; }

private:
  FlexGridLayout m_grid;
  FormWindow* m_form = nullptr;
  FormGroup::Line* m_line = nullptr;
};

#endif // _PAGE_H_
