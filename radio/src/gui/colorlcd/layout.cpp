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

#include "opentx.h"
#include "view_main.h"

WidgetsContainer * customScreens[MAX_CUSTOM_SCREENS] = {};

std::list<const LayoutFactory *> & getRegisteredLayouts()
{
  static std::list<const LayoutFactory *> layouts;
  return layouts;
}

void registerLayout(const LayoutFactory * factory)
{
  TRACE("register layout %s", factory->getId());
  getRegisteredLayouts().push_back(factory);
}

const LayoutFactory * getLayoutFactory(const char * name)
{
  auto it = getRegisteredLayouts().cbegin();
  for (; it != getRegisteredLayouts().cend(); ++it) {
    if (!strcmp(name, (*it)->getId())) {
      return (*it);
    }
  }
  return nullptr;
}

//
// Loads a layout, but does not attach it to any window
//
WidgetsContainer *
loadLayout(const char * name, LayoutPersistentData * persistentData)
{
  const LayoutFactory * factory = getLayoutFactory(name);
  if (factory) {
    return factory->load(persistentData);
  }
  return nullptr;
}

//
// Detaches and deletes all custom screens
//
void deleteCustomScreens()
{
  for (auto& screen : customScreens) {
    if (screen) {
      screen->deleteLater();
      screen = nullptr;
    }
  }
}

extern const LayoutFactory * defaultLayout;

void loadDefaultLayout()
{
  auto& screen = customScreens[0];
  auto& screenData = g_model.screenData[0];

  if (screen == nullptr && defaultLayout != nullptr) {

    strcpy(screenData.LayoutId, defaultLayout->getId());
    screen = defaultLayout->create(&screenData.layoutData);
    //
    // TODO:
    // -> attach a few default widgets
    //    - ModelBmp
    //    - Timer
    //    - ???
    //
    if (screen) {
      screen->attach(ViewMain::instance());
    }
  }
}

//
// Loads and attaches all configured custom screens
//
void loadCustomScreens()
{
  unsigned i = 0;
  auto viewMain = ViewMain::instance();

  while (i < MAX_CUSTOM_SCREENS) {

    auto& screen = customScreens[i];
    screen = loadLayout(g_model.screenData[i].LayoutId,
                        &g_model.screenData[i].layoutData);

    if (!screen) {
      // no more layouts
      break;
    }

    // layout is ok, let's add it
    screen->attach(viewMain);
    viewMain->setMainViewsCount(i + 1);
    screen->setLeft(viewMain->getMainViewLeftPos(i));
    i++;
  }

  auto topbar = viewMain->getTopbar();

  // TODO: how to reload this guy?
  topbar->load();
  
  viewMain->setCurrentMainView(0);
  viewMain->updateTopbarVisibility();
  viewMain->setFocus();
}

//
// Creates a new customer screen from factory:
//  - the old screen is detached & deleted (including children)
//  - new screen is configured into g_model
//  - the new screen is returned (not attached)
//
WidgetsContainer *
createCustomScreen(const LayoutFactory* factory, unsigned customScreenIndex)
{
  if (!factory || (customScreenIndex >= MAX_CUSTOM_SCREENS))
    return nullptr;

  if (customScreens[customScreenIndex]) {
    customScreens[customScreenIndex]->deleteLater(true, false);
    delete customScreens[customScreenIndex];
  }

  auto screen = factory->create(&g_model.screenData[customScreenIndex].layoutData);
  customScreens[customScreenIndex] = screen;

  if (screen) {
    auto dst = g_model.screenData[customScreenIndex].LayoutId;
    auto src = factory->getId();
    strncpy(dst, src, sizeof(CustomScreenData::LayoutId));

    return screen;
  }

  return nullptr;
}

void disposeCustomScreen(unsigned idx)
{
  // move custom screen data
  if (idx >= MAX_CUSTOM_SCREENS) {
    return;
  }

  auto dst = &g_model.screenData[idx];
  auto src = dst + 1;
  auto len = sizeof(CustomScreenData) * (MAX_CUSTOM_SCREENS - idx - 1);
  memmove(dst, src, len);

  dst = &g_model.screenData[MAX_CUSTOM_SCREENS - 1];
  len = sizeof(CustomScreenData);
  memset(dst, 0, len);
}

LayoutFactory::LayoutFactory(const char * id, const char * name):
  id(id),
  name(name)
{
  registerLayout(this);
}

