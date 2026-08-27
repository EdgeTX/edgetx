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

#include "messaging.h"

#include <list>
#include <algorithm>

static std::list<Messaging*> subscriptions;

Messaging::~Messaging()
{
  unsubscribe();
}

void Messaging::subscribe(uint32_t _id, std::function<void(uint32_t)> cb)
{
  unsubscribe();
  id = _id;
  callback = cb;
  subscriptions.emplace_back(this);
}

void Messaging::unsubscribe()
{
  if (id) {
    auto s = std::find_if(subscriptions.begin(), subscriptions.end(),
                              [=](Messaging* lh) -> bool { return lh == this; });
    if (s != subscriptions.end()) subscriptions.erase(s);
  }

  callback = nullptr;
  id = 0;
}

void Messaging::send(uint32_t id)
{
  send(id, 0);
}

void Messaging::send(uint32_t msgId, uint32_t msgData)
{
  for (auto it = subscriptions.rbegin(); it != subscriptions.rend(); ++it) {
    Messaging* m = *it;
    if (m->id == msgId && m->callback)
      m->callback(msgData);
  }
}
