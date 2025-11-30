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

class TopBar;

class ViewMain : public NavWindow
{
  // singleton
  explicit ViewMain();

 public:
  ~ViewMain() override;

  static ViewMain* instance();

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "ViewMain"; }
#endif

  void addMainView(WidgetsContainer* view, uint32_t viewId);

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
  void showTopBarEdgeTxButton();
  void hideTopBarEdgeTxButton();

  bool hasTopbar();
  bool hasTopbar(unsigned view);
  bool isAppMode();
  bool isAppMode(unsigned view);

  void refreshWidgetSelectTimer();

  static void refreshWidgets();

 protected:
  static ViewMain* _instance;

  bool isVisible = true;
  lv_obj_t* tile_view = nullptr;
  TopBar* topbar = nullptr;
  bool widget_select = false;
  lv_timer_t* widget_select_timer = nullptr;

  void deleteLater() override;

  // Set topbar visibility [0.0 -> 1.0]
  void setTopbarVisible(float visible);
  void setEdgeTxButtonVisible(float visible);

  void _refreshWidgets();

  static void ws_timer(lv_timer_t* t);

#if defined(HARDWARE_KEYS)
  void doKeyShortcut(event_t event);
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
