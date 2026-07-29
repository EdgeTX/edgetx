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
#include <utility>

struct MessagingSubscription {
  uint32_t id;
  std::function<void(uint32_t)> callback;
  bool active;
};

namespace
{

std::list<MessagingSubscription> subscriptions;
uint32_t dispatchDepth = 0;

void removeInactiveSubscriptions()
{
  subscriptions.remove_if(
      [](const auto& subscription) { return !subscription.active; });
}

}  // namespace

Messaging::~Messaging()
{
  unsubscribe();
}

void Messaging::subscribe(uint32_t _id, std::function<void(uint32_t)> cb)
{
  unsubscribe();
  subscriptions.push_back({_id, std::move(cb), true});
  subscription = &subscriptions.back();
}

void Messaging::unsubscribe()
{
  if (!subscription) return;

  subscription->active = false;
  subscription = nullptr;

  if (dispatchDepth == 0) removeInactiveSubscriptions();
}

void Messaging::send(uint32_t id)
{
  send(id, 0);
}

void Messaging::send(uint32_t msgId, uint32_t msgData)
{
  dispatchDepth++;

  for (auto it = subscriptions.rbegin(); it != subscriptions.rend();) {
    // Advance first so subscriptions added by the callback wait for the next
    // send.
    auto& subscription = *it++;
    if (subscription.active && subscription.id == msgId &&
        subscription.callback) {
      subscription.callback(msgData);
    }
  }

  dispatchDepth--;
  if (dispatchDepth == 0) removeInactiveSubscriptions();
}
