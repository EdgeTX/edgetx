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

#pragma once

#include "topbar.h"
#include "window.h"

class SetupWidgetsPage;
class SetupTopBarWidgetsPage;
class ViewMainMenu;

class ViewMain : public NavWindow
{
  // singleton
  explicit ViewMain();

 public:
  ~ViewMain() override;

  static ViewMain* instance()
  {
    if (!_instance) _instance = new ViewMain();

    return _instance;
  }

  static ViewMain* getInstance() { return _instance; }

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "ViewMain"; }
#endif

  void addMainView(WidgetsContainer* view, uint32_t viewId);

  void enableTopbar();
  void disableTopbar();
  void updateTopbarVisibility();
  bool enableWidgetSelect(bool enable);

  // Get the available space in the middle of the screen
  // (without topbar)
  rect_t getMainZone(rect_t zone, bool hasTopbar) const;

  unsigned getMainViewsCount() const;
  unsigned getCurrentMainView() const;
  void setCurrentMainView(unsigned view);

  void nextMainView();
  void previousMainView();

  TopBar* getTopbar();

  void onClicked() override;
  void onCancel() override;
  void openMenu();
  bool onLongPress() override;

  void show(bool visible = true) override;
  bool viewIsVisible() const { return isVisible; }
  void showTopBarEdgeTxButton();
  void hideTopBarEdgeTxButton();

  bool hasTopbar();
  bool hasTopbar(unsigned view);
  bool isAppMode();
  bool isAppMode(unsigned view);

  void runBackground();
  void refreshWidgetSelectTimer();

 protected:
  static ViewMain* _instance;

  bool isVisible = true;
  lv_obj_t* tile_view = nullptr;
  TopBar* topbar = nullptr;
  bool widget_select = false;
  lv_timer_t* widget_select_timer = nullptr;
  ViewMainMenu* viewMainMenu = nullptr;

  void deleteLater(bool detach = true, bool trash = true) override;

  // Widget setup requires special permissions ;-)
  friend class SetupWidgetsPage;
  friend class SetupTopBarWidgetsPage;

  // Set topbar visibility [0.0 -> 1.0]
  void setTopbarVisible(float visible);
  void setEdgeTxButtonVisible(float visible);

  static void ws_timer(lv_timer_t* t);

#if defined(HARDWARE_KEYS)
  void onPressSYS() override;
  void onLongPressSYS() override;
  void onPressMDL() override;
  void onLongPressMDL() override;
  void onPressTELE() override;
  void onLongPressTELE() override;
  void onPressPGUP() override;
  void onPressPGDN() override;
#endif
};
