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

#include "edgetx.h"
#include "topbar_impl.h"
#include "view_main.h"

WidgetsContainer* customScreens[MAX_CUSTOM_SCREENS] = {};

std::list<const LayoutFactory*>& LayoutFactory::getRegisteredLayouts()
{
  static std::list<const LayoutFactory*> layouts;
  return layouts;
}

const LayoutFactory* LayoutFactory::getLayoutFactory(const char* name)
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
WidgetsContainer* LayoutFactory::loadLayout(
    Window* parent, const char* name, LayoutPersistentData* persistentData)
{
  const LayoutFactory* factory = getLayoutFactory(name);
  if (factory) {
    return factory->load(parent, persistentData);
  }
  return nullptr;
}

//
// Detaches and deletes all custom screens
//
void LayoutFactory::deleteCustomScreens()
{
  for (auto& screen : customScreens) {
    if (screen) {
      screen->deleteLater();
      screen = nullptr;
    }
  }
}

void LayoutFactory::loadDefaultLayout()
{
  auto& screen = customScreens[0];
  auto& screenData = g_model.screenData[0];

  if (screen == nullptr && defaultLayout != nullptr) {
    strcpy(screenData.LayoutId, defaultLayout->getId());

    auto viewMain = ViewMain::instance();
    screen = defaultLayout->create(viewMain, &screenData.layoutData);
    //
    // TODO:
    // -> attach a few default widgets
    //    - ModelBmp
    //    - Timer
    //    - ???
    //
    if (!screen) return;
    viewMain->addMainView(screen, 0);
  }
}

//
// Loads and attaches all configured custom screens
//
void LayoutFactory::loadCustomScreens()
{
  unsigned i = 0;
  auto viewMain = ViewMain::instance();

  while (i < MAX_CUSTOM_SCREENS) {
    auto& screen = customScreens[i];
    screen = loadLayout(viewMain, g_model.screenData[i].LayoutId,
                        &g_model.screenData[i].layoutData);

    if (!screen) {
      // no more layouts
      break;
    }

    // layout is ok, let's add it
    viewMain->addMainView(screen, i);
    i++;
  }

  auto topbar = viewMain->getTopbar();
  topbar->load();

  if (g_model.view < viewMain->getMainViewsCount()) {
    viewMain->setCurrentMainView(g_model.view);
  } else if (viewMain->getMainViewsCount() > 0) {
    g_model.view = viewMain->getMainViewsCount() - 1;
    storageDirty(EE_MODEL);
    viewMain->setCurrentMainView(g_model.view);
  }
  // else {
  //   TODO: load some default view?
  // }

  viewMain->updateTopbarVisibility();
}

//
// Creates a new customer screen from factory:
//  - the old screen is detached & deleted (including children)
//  - new screen is configured into g_model
//  - the new screen is returned (not attached)
//
WidgetsContainer* LayoutFactory::createCustomScreen(
    unsigned customScreenIndex) const
{
  if (customScreenIndex >= MAX_CUSTOM_SCREENS) return nullptr;

  auto& screen = customScreens[customScreenIndex];
  auto& screenData = g_model.screenData[customScreenIndex];

  if (screen != nullptr) {
    screen->deleteLater(true, false);
    delete screen;
  }

  auto viewMain = ViewMain::instance();
  screen = create(viewMain, &screenData.layoutData);

  if (!screen) return nullptr;
  viewMain->addMainView(screen, customScreenIndex);

  auto dst = g_model.screenData[customScreenIndex].LayoutId;
  auto src = getId();
  strncpy(dst, src, sizeof(CustomScreenData::LayoutId));

  return screen;
}

void LayoutFactory::disposeCustomScreen(unsigned idx)
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

LayoutFactory::LayoutFactory(const char* id, const char* name) :
    id(id), name(name)
{
  TRACE("register layout %s", getId());
  getRegisteredLayouts().push_back(this);
}
