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

#ifndef _VIEW_MAIN_H_
#define _VIEW_MAIN_H_

#include <memory>
#include "form.h"
#include "topbar.h"


class SetupWidgetsPage;
class SetupTopBarWidgetsPage;

class ViewMain: public Window
{
    // singleton
    explicit ViewMain();

  public:
    ~ViewMain() override;

    static ViewMain * instance()
    {
      if (!_instance)
        _instance = new ViewMain();

      return _instance;
    }

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "ViewMain";
    }
#endif

    void addMainView(Window* view, uint32_t viewId);

    void enableTopbar();
    void disableTopbar();
    void updateTopbarVisibility();

    // Get the available space in the middle of the screen
    // (without topbar)
    rect_t getMainZone(rect_t zone, bool hasTopbar) const;

    unsigned getMainViewsCount() const;
    //coord_t getMainViewLeftPos(unsigned view) const;
  
    unsigned getCurrentMainView() const;
    void setCurrentMainView(unsigned view);

    void nextMainView();
    void previousMainView();

    Topbar* getTopbar();
  
    void onEvent(event_t event) override;
    void onClicked() override;
    void onCancel() override;
  
  protected:
    static ViewMain * _instance;

    unsigned    views = 0;
    lv_obj_t*   tile_view = nullptr;
    TopbarImpl* topbar = nullptr;
    bool        widget_select = false;
    lv_timer_t* widget_select_timer = nullptr;

    void paint(BitmapBuffer * dc) override;
    void deleteLater(bool detach = true, bool trash = true) override;

    // Widget setup requires special permissions ;-)
    friend class SetupWidgetsPage;
    friend class SetupTopBarWidgetsPage;

    // Set topbar visibility [0.0 -> 1.0]
    void setTopbarVisible(float visible);

    void openMenu();
    bool enableWidgetSelect(bool enable);
    void refreshWidgetSelectTimer();

    static void long_pressed(lv_event_t* e);
    static void ws_timer(lv_timer_t* t);
};

#endif // _VIEW_MAIN_H_
