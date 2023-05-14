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

#include "carousel.h"

void CarouselWindow::update()
{
  auto first = min<int>(selection - 2, items.size() - count);
  first = max(0, first);
  coord_t lastPosition = 0;
  coord_t spacing = 10;
  if (!items.empty() && count > 0)
    spacing = (width() - items[0]->back->width() * (count - 1) - items[0]->front->width()) / (count - 1);

  int index = 0;
  for (auto & item: items) {
    Window * window;
    if (index == selection) {
      item->back->detach();
      window = item->front;
    }
    else {
      item->front->detach();
      window = item->back;
    }
    window->attach(this);
    window->setLeft(lastPosition);
    window->setTop((height() - window->height()) / 2);
    lastPosition += window->width() + spacing;
    index += 1;
  }

}

#if defined(HARDWARE_KEYS)
void Carousel::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString().c_str(), event);

  if (event == EVT_ROTARY_RIGHT) {
    if (body->selection < (int)body->items.size() - 1)
      select(body->selection + 1);
  }
  else if (event == EVT_ROTARY_LEFT) {
    if (body->selection > 0)
      select(body->selection - 1);
  }
  else {
    Window::onEvent(event);
  }
}
#endif
