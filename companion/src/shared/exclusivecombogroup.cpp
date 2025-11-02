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

#include "exclusivecombogroup.h"

#include <QListView>

ExclusiveComboGroup::ExclusiveComboGroup(QObject *parent,
                                         std::function<bool(const QVariant&)> filter) :
  QObject(parent),
  filter(std::move(filter))
{
}

void ExclusiveComboGroup::addCombo(QComboBox *comboBox)
{
  connect(comboBox, QOverload<int>::of(&QComboBox::activated), [=](int index) {
    this->handleActivated(comboBox, index);
  });

  combos.append(comboBox);
}

void ExclusiveComboGroup::handleActivated(QComboBox* target, int index)
{
  auto data = target->itemData(index);
  auto targetidx = combos.indexOf(target);

  for (auto combo : combos) {
    if (target == combo)
      continue;

    auto view = dynamic_cast<QListView*>(combo->view());
    Q_ASSERT(view);

    auto previous = combo->findData(targetidx, _role);

    if (previous >= 0) {
      view->setRowHidden(previous, false);
      combo->setItemData(previous, QVariant(), _role);
    }

    if (!filter(data)) {
      auto idx = combo->findData(data);

      if (idx >= 0) {
        view->setRowHidden(idx, true);
        combo->setItemData(idx, targetidx, _role);
      }
    }
  }
}
