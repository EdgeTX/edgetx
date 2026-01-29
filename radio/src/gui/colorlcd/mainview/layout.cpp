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
#include "topbar.h"
#include "view_main.h"
#include "widget.h"

WidgetsContainer* customScreens[MAX_CUSTOM_SCREENS] = {};

const LayoutOption defaultLayoutOptions[] = {LAYOUT_COMMON_OPTIONS,
                                           LAYOUT_OPTIONS_END};

//-----------------------------------------------------------------------------

void ZonePersistentData::clear()
{
  widgetName.clear();
  widgetData.clear();
}

void LayoutPersistentData::clearZone(int idx)
{
  zones[idx].clear();
}

void LayoutPersistentData::clear()
{
  for (int i = 0; i < MAX_LAYOUT_ZONES; i += 1)
    clearZone(i);
  for (int i = 0; i < MAX_LAYOUT_OPTIONS; i += 1) {
    options[i].type = LOV_None;
    options[i].value.unsignedValue = 0;
  }
}

const char* LayoutPersistentData::getWidgetName(int idx)
{
  return zones[idx].widgetName.c_str();
}

void LayoutPersistentData::setWidgetName(int idx, const char* s)
{
  zones[idx].widgetName = s;
}

WidgetPersistentData* LayoutPersistentData::getWidgetData(int idx)
{
  return &zones[idx].widgetData;
}

bool LayoutPersistentData::hasWidget(int idx)
{
  return !zones[idx].widgetName.empty();
}

//-----------------------------------------------------------------------------

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
    Window* parent, int screenNum)
{
  const LayoutFactory* factory = getLayoutFactory(g_model.getScreenLayoutId(screenNum));
  if (factory) {
    return factory->load(parent, screenNum);
  }
  return nullptr;
}

//
// Detaches and deletes all custom screens
//
void LayoutFactory::deleteCustomScreens(bool clearTopBar)
{
  for (auto& screen : customScreens) {
    if (screen) {
      screen->deleteLater();
      screen = nullptr;
    }
  }

  if (clearTopBar)
    ViewMain::instance()->getTopbar()->removeAllWidgets();
}

void LayoutFactory::loadDefaultLayout()
{
  auto& screen = customScreens[0];

  if (screen == nullptr && defaultLayout != nullptr) {
    g_model.setScreenLayoutId(0, defaultLayout->getId());

    auto viewMain = ViewMain::instance();
    screen = defaultLayout->create(viewMain, 0);
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
    screen = loadLayout(viewMain, i);

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

  if (screen != nullptr)
    screen->deleteLater();

  auto viewMain = ViewMain::instance();
  screen = create(viewMain, customScreenIndex);

  if (!screen) return nullptr;
  viewMain->addMainView(screen, customScreenIndex);

  g_model.setScreenLayoutId(customScreenIndex, getId());

  return screen;
}

LayoutFactory::LayoutFactory(const char* id, const char* name, const LayoutOption * options, uint8_t zoneCount, uint8_t* zoneMap) :
    id(id), name(name), options(options), zoneCount(zoneCount), zoneMap(zoneMap)
{
  getRegisteredLayouts().push_back(this);

  bitmap = (MaskBitmap*)malloc(align32(BM_W * BM_H + sizeof(MaskBitmap)));
  bitmap->width = BM_W;
  bitmap->height = BM_H;

  uint8_t* bm = (uint8_t*)bitmap->data;
  memset(bm, 0, BM_W * BM_H);

  memset(bm, 0xFF, BM_W);
  memset(bm+((BM_H-1)*BM_W), 0xFF, BM_W);

  for (int y = 1; y < BM_H - 1; y += 1) {
    bm[y*BM_W] = 0xFF;
    bm[y*BM_W+BM_W-1] = 0xFF;
  }

  for (int i = 0; i < zoneCount * 4; i += 4) {
    uint8_t xo = BM_W * zoneMap[i] / LAYOUT_MAP_DIV;
    uint8_t yo = BM_H * zoneMap[i+1] / LAYOUT_MAP_DIV;
    uint8_t w = BM_W * zoneMap[i+2] / LAYOUT_MAP_DIV;
    uint8_t h = (BM_H * zoneMap[i+3] + LAYOUT_MAP_DIV/2) / LAYOUT_MAP_DIV;

    if (yo > 0)
      memset(bm+(yo*BM_W+xo), 0xFF, w);
    if (xo > 0) {
      for (int y = 0; y < h; y += 1) {
        bm[(yo+y)*BM_W+xo] = 0xFF;
      }
    }
  }
}

LayoutFactory::~LayoutFactory()
{
  if (bitmap) {
    free((void*)bitmap);
    bitmap = nullptr;
  }
}

inline LayoutOptionValueEnum layoutValueEnumFromType(LayoutOption::Type type)
{
  switch(type) {
  case LayoutOption::Bool:
    return LOV_Bool;

  case LayoutOption::Color:
    return LOV_Color;

  default:
    return LOV_None;
  }
}

void LayoutFactory::initPersistentData(int screenNum, bool setDefault) const
{
  auto layoutData = g_model.getScreenLayoutData(screenNum);

  if (setDefault) {
    layoutData->clear();
  }
  if (options) {
    int i = 0;
    for (const LayoutOption* option = options; option->name; option++, i++) {
      // TODO compiler bug? The CPU freezes ... persistentData->options[i++]
      // = option->deflt;
      auto optVal = &layoutData->options[i];
      if (setDefault)
        optVal->value.unsignedValue = option->deflt.unsignedValue;
      optVal->type = layoutValueEnumFromType(option->type);
    }
  }
}

WidgetsContainer* LayoutFactory::create(Window* parent, int screenNum) const
{
  initPersistentData(screenNum, true);
  return createNew(parent, screenNum);
}

WidgetsContainer* LayoutFactory::load(Window* parent, int screenNum) const
{
  initPersistentData(screenNum, false);
  Layout* layout = createNew(parent, screenNum);
  if (layout)
    layout->load();
  return layout;
}

//-----------------------------------------------------------------------------

Layout::Layout(Window* parent, const LayoutFactory* factory,
               int screenNum, uint8_t zoneCount,
               uint8_t* zoneMap) :
    WidgetsContainer(parent, {0, 0, LCD_W, LCD_H}, zoneCount),
    factory(factory),
    zoneMap(zoneMap), screenNum(screenNum)
{
  decoration = new ViewMainDecoration(this);
  setWindowFlag(NO_FOCUS);
  decorationUpdateMsg.subscribe(Messaging::DECORATION_UPDATE, [=](uint32_t param) { updateDecorations(); });
  updateDecorations();
  zoneUpdateRequired = false;
}

void Layout::updateDecorations()
{
  // Set visible decoration
  decoration->setSlidersVisible(hasSliders());
  decoration->setTrimsVisible(hasTrims());
  decoration->setFlightModeVisible(hasFlightMode());
  zoneUpdateRequired = true;
}

void Layout::show(bool visible)
{
  decoration->show(visible);
  if (visible && zoneUpdateRequired) {
    zoneUpdateRequired = false;
    // and update relevant windows
    updateZones();
  }
}

rect_t Layout::getMainZone() const
{
  rect_t zone = decoration->getMainZone();
  if (hasSliders() || hasTrims() || hasFlightMode()) {
    // some decoration activated
    zone.x += PAD_LARGE;
    zone.y += PAD_LARGE;
    zone.w -= 2 * PAD_LARGE;
    zone.h -= 2 * PAD_LARGE;
  }
  if (hasTopbar()) {
    zone.y += EdgeTxStyles::MENU_HEADER_HEIGHT;
    zone.h -= EdgeTxStyles::MENU_HEADER_HEIGHT;
  }
  return zone;
}

rect_t Layout::getZone(unsigned int index) const
{
  rect_t z = getMainZone();

  unsigned int i = index * 4;

  coord_t xo = z.w * zoneMap[i] / LAYOUT_MAP_DIV;
  coord_t yo = z.h * zoneMap[i + 1] / LAYOUT_MAP_DIV;
  coord_t w = z.w * zoneMap[i + 2] / LAYOUT_MAP_DIV;
  coord_t h = z.h * zoneMap[i + 3] / LAYOUT_MAP_DIV;

  if (isMirrored()) xo = z.w - xo - w;

  return {z.x + xo, z.y + yo, w, h};
}

void Layout::checkEvents()
{
  Window::checkEvents();
  rect_t z = getMainZone();
  if (z.x != lastMainZone.x || z.y != lastMainZone.y || z.w != lastMainZone.w || z.h != lastMainZone.h) {
    lastMainZone = z;
    for (int i = 0; i < zoneCount; i++)
      if (widgets[i] && widgets[i]->isFullscreen())
        return;
    updateZones();
  }
}

void Layout::removeWidget(unsigned int index)
{
  if (index >= zoneCount) return;

  g_model.getScreenLayoutData(screenNum)->clearZone(index);

  WidgetsContainer::removeWidget(index);
}

Widget* Layout::createWidget(unsigned int index,
                      const WidgetFactory* factory)
{
  if (index >= zoneCount) return nullptr;

  // remove old one if existing
  removeWidget(index);

  Widget* widget = nullptr;
  if (factory) {
    g_model.getScreenLayoutData(screenNum)->setWidgetName(index, factory->getName());
    widget = factory->create(this, getZone(index), screenNum, index);
  }
  widgets[index] = widget;

  return widget;
}

void Layout::load()
{
  unsigned int count = getZonesCount();
  for (unsigned int i = 0; i < count; i++) {
    // remove old widget
    if (widgets[i]) {
      widgets[i]->deleteLater();
      widgets[i] = nullptr;
    }
  }

  for (unsigned int i = 0; i < count; i++) {
    // and load new one if required
    if (g_model.getScreenLayoutData(screenNum)->hasWidget(i)) {
      widgets[i] = WidgetFactory::newWidget(g_model.getScreenLayoutData(screenNum)->getWidgetName(i), this, getZone(i), screenNum, i);
    }
  }
}

LayoutOptionValue* Layout::getOptionValue(unsigned int index) const
{
  return &g_model.getScreenLayoutData(screenNum)->options[index].value;
}

bool Layout::hasTopbar() const {
  return getOptionValue(LAYOUT_OPTION_TOPBAR)->boolValue;
}

bool Layout::hasFlightMode() const {
  return getOptionValue(LAYOUT_OPTION_FM)->boolValue;
}

bool Layout::hasSliders() const {
  return getOptionValue(LAYOUT_OPTION_SLIDERS)->boolValue;
}

bool Layout::hasTrims() const {
  return getOptionValue(LAYOUT_OPTION_TRIMS)->boolValue;
}

bool Layout::isMirrored() const {
  return getOptionValue(LAYOUT_OPTION_MIRRORED)->boolValue;
}

//-----------------------------------------------------------------------------
