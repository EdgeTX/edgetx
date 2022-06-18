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

/*
    Copyright 2016 fishpepper <AT> gmail.com
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    author: fishpepper <AT> gmail.com
*/

#ifndef __TOUCH_DRIVER_H
#define __TOUCH_DRIVER_H

#ifndef __cplusplus
#error C++ only
#endif

#include <stdint.h>
#include <functional>

#include <touch.h>


struct TouchControllerDesc
{
  std::function<void()> init;
  std::function<void()> reset;
  std::function<TouchEvent()> read;
  std::function<void(std::function<void()>)> setIrqCb;
};

void registerTouchController(TouchControllerDesc tc);

TouchState touchPanelRead();
void TouchInit();
void TouchDriver();


#endif /* __TOUCH_DRIVER_H */
