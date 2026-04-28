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

#include "itemmodeleventhandler.h"

ItemModelEventHandler::ItemModelEventHandler(FilteredItemModel * itemModel,
                                             bool & lock,
                                             std::function<void()> updateCallback) :
  QObject(nullptr),
  m_lock(lock),
  m_updateCallback(std::move(updateCallback)),
  m_cnt(0)
{
  connect(itemModel, &FilteredItemModel::aboutToBeUpdated, [&] ()
    {
      m_lock = true;
      m_cnt++;
    }
  );

  connect(itemModel, &FilteredItemModel::updateComplete, [&] ()
    {
      m_cnt--;

      if (m_cnt < 1) {
        if (m_updateCallback)
          m_updateCallback();
        m_lock = false;
      }
    }
  );
}
