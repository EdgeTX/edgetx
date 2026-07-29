/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - http://www.open-tx.org
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

#include "gtests.h"

#if defined(COLORLCD)

#include <memory>
#include <vector>

#include "messaging.h"

TEST(Messaging, DeliversMatchingTopicNewestFirst)
{
  std::vector<int> deliveries;
  Messaging first;
  Messaging second;
  Messaging otherTopic;

  first.subscribe(Messaging::REFRESH,
                  [&](uint32_t data) { deliveries.push_back(data + 1); });
  second.subscribe(Messaging::REFRESH,
                   [&](uint32_t data) { deliveries.push_back(data + 2); });
  otherTopic.subscribe(Messaging::COLOR_CHANGED,
                       [&](uint32_t data) { deliveries.push_back(data + 3); });

  Messaging::send(Messaging::REFRESH, 10);

  EXPECT_EQ(deliveries, (std::vector<int>{12, 11}));
}

TEST(Messaging, ExplicitUnsubscribeAndDestructionStopDelivery)
{
  int calls = 0;
  Messaging persistent;

  persistent.subscribe(Messaging::REFRESH, [&](uint32_t) { calls += 1; });
  Messaging::send(Messaging::REFRESH);
  persistent.unsubscribe();
  Messaging::send(Messaging::REFRESH);

  {
    Messaging scoped;
    scoped.subscribe(Messaging::REFRESH, [&](uint32_t) { calls += 10; });
    Messaging::send(Messaging::REFRESH);
  }
  Messaging::send(Messaging::REFRESH);

  EXPECT_EQ(calls, 11);
}

TEST(Messaging, CallbackCanUnsubscribeItself)
{
  int calls = 0;
  Messaging subscriber;

  subscriber.subscribe(Messaging::REFRESH, [&](uint32_t) {
    calls++;
    subscriber.unsubscribe();
  });

  Messaging::send(Messaging::REFRESH);
  Messaging::send(Messaging::REFRESH);

  EXPECT_EQ(calls, 1);
}

TEST(Messaging, CallbackCanDestroyItself)
{
  int calls = 0;
  auto subscriber = std::make_unique<Messaging>();

  subscriber->subscribe(Messaging::REFRESH, [&](uint32_t) {
    calls++;
    subscriber.reset();
  });

  Messaging::send(Messaging::REFRESH);
  Messaging::send(Messaging::REFRESH);

  EXPECT_EQ(calls, 1);
}

TEST(Messaging, CallbackCanDestroySubscriberWaitingForDelivery)
{
  int removedCalls = 0;
  auto removed = std::make_unique<Messaging>();
  Messaging remover;

  removed->subscribe(Messaging::REFRESH, [&](uint32_t) { removedCalls++; });
  remover.subscribe(Messaging::REFRESH, [&](uint32_t) { removed.reset(); });

  Messaging::send(Messaging::REFRESH);

  EXPECT_EQ(removedCalls, 0);
}

TEST(Messaging, CallbackCanDestroySubscriberAlreadyDelivered)
{
  int deliveredCalls = 0;
  auto delivered = std::make_unique<Messaging>();
  Messaging remover;

  remover.subscribe(Messaging::REFRESH, [&](uint32_t) { delivered.reset(); });
  delivered->subscribe(Messaging::REFRESH, [&](uint32_t) { deliveredCalls++; });

  Messaging::send(Messaging::REFRESH);
  Messaging::send(Messaging::REFRESH);

  EXPECT_EQ(deliveredCalls, 1);
}

TEST(Messaging, SubscriptionAddedDuringDeliveryWaitsForNextSend)
{
  int existingCalls = 0;
  int addedCalls = 0;
  bool added = false;
  Messaging existing;
  Messaging addedSubscriber;

  existing.subscribe(Messaging::REFRESH, [&](uint32_t) {
    existingCalls++;
    if (!added) {
      added = true;
      addedSubscriber.subscribe(Messaging::REFRESH,
                                [&](uint32_t) { addedCalls++; });
    }
  });

  Messaging::send(Messaging::REFRESH);
  EXPECT_EQ(existingCalls, 1);
  EXPECT_EQ(addedCalls, 0);

  Messaging::send(Messaging::REFRESH);
  EXPECT_EQ(existingCalls, 2);
  EXPECT_EQ(addedCalls, 1);
}

TEST(Messaging, ResubscriptionDuringDeliveryTakesEffectOnNextSend)
{
  int oldTopicCalls = 0;
  int newTopicCalls = 0;
  Messaging subscriber;

  subscriber.subscribe(Messaging::REFRESH, [&](uint32_t) {
    oldTopicCalls++;
    subscriber.subscribe(Messaging::COLOR_CHANGED,
                         [&](uint32_t) { newTopicCalls++; });
  });

  Messaging::send(Messaging::REFRESH);
  Messaging::send(Messaging::REFRESH);
  Messaging::send(Messaging::COLOR_CHANGED);

  EXPECT_EQ(oldTopicCalls, 1);
  EXPECT_EQ(newTopicCalls, 1);
}

TEST(Messaging, NestedSendIsDeterministic)
{
  std::vector<int> deliveries;
  Messaging outer;
  Messaging nested;

  nested.subscribe(Messaging::COLOR_CHANGED,
                   [&](uint32_t data) { deliveries.push_back(data); });
  outer.subscribe(Messaging::REFRESH, [&](uint32_t data) {
    deliveries.push_back(data);
    Messaging::send(Messaging::COLOR_CHANGED, data + 1);
  });

  Messaging::send(Messaging::REFRESH, 20);

  EXPECT_EQ(deliveries, (std::vector<int>{20, 21}));
}

TEST(Messaging, RepeatedSubscriptionLifetimeLeavesNoStaleDelivery)
{
  int calls = 0;

  for (int i = 0; i < 100; i++) {
    Messaging subscriber;
    subscriber.subscribe(Messaging::REFRESH, [&](uint32_t) { calls++; });
    Messaging::send(Messaging::REFRESH);
  }

  Messaging::send(Messaging::REFRESH);

  EXPECT_EQ(calls, 100);
}

#endif
