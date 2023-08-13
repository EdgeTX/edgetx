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

#include "window.h"
#include "flexlayout.h"

class FormField : public Window
{
  public:
    FormField(Window *parent, const rect_t &rect, WindowFlags windowFlags = 0,
              LcdFlags textFlags = 0, LvglCreate objConstruct = nullptr);

    virtual void changeEnd(bool forceChanged = false)
    {
      if (changeHandler) { changeHandler(); }
    }

    void setChangeHandler(std::function<void()> handler)
    {
      changeHandler = std::move(handler);
    }

    inline bool isEditMode() const { return editMode; }
    virtual void setEditMode(bool newEditMode);

    void enable(bool value = true);
    void disable() { enable(false); }

    void onClicked() override;
    void onCancel() override;
  
  protected:
    bool editMode = false;
    bool enabled = true;
    std::function<void()> changeHandler = nullptr;
};

class FormWindow : public Window
{
  public:
    class Line : public Window
    {
      public:
        Line(Window *parent, lv_obj_t *obj, FlexGridLayout *layout = nullptr);
        Line(Window *parent, FlexGridLayout *layout = nullptr);

        void setLayout(FlexGridLayout *l);

      protected:
        FlexGridLayout *layout;

        void addChild(Window *window) override;
        void construct();
    };

    FormWindow(Window *parent, const rect_t &rect, WindowFlags windowflags = 0);

#if defined(DEBUG_WINDOWS)
    std::string getName() const override { return "FormWindow"; }
#endif

    void setFlexLayout(lv_flex_flow_t flow = LV_FLEX_FLOW_COLUMN, lv_coord_t padding = 0);
    Line* newLine(FlexGridLayout* layout = nullptr, lv_coord_t left_padding = 0);
};
