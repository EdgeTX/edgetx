/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

    void updateTopbarVisibility();

    // Get the available space in the middle of the screen
    // (without topbar)
    rect_t getMainZone(rect_t zone, bool hasTopbar) const;

    unsigned getMainViewsCount() const;
    void setMainViewsCount(unsigned views);

    coord_t getMainViewLeftPos(unsigned view) const;
  
    unsigned getCurrentMainView() const;
    void setCurrentMainView(unsigned view);

    void nextMainView();
    void previousMainView();

    Topbar* getTopbar();
  
#if defined(HARDWARE_KEYS)
    void onEvent(event_t event) override;
#endif
  
  protected:
    static ViewMain * _instance;

    unsigned    views = 0;
    TopbarImpl* topbar = nullptr;

    // Widget setup requires special permissions ;-)
    friend class SetupWidgetsPage;
    friend class SetupTopBarWidgetsPage;

    // Set topbar visibility [0.0 -> 1.0]
    void setTopbarVisible(float visible);

    void setScrollPositionX(coord_t value) override;
    void setScrollPositionY(coord_t value) override;

#if defined(HARDWARE_TOUCH)
    unsigned char prevSlideState = 0;
    unsigned int  startSlidePage = 0;

    bool onTouchSlide(coord_t x, coord_t y, coord_t startX, coord_t startY, coord_t slideX, coord_t slideY) override;
    bool onTouchEnd(coord_t x, coord_t y) override;
#endif

    void paint(BitmapBuffer * dc) override;

    void openMenu();
};

#endif // _VIEW_MAIN_H_
