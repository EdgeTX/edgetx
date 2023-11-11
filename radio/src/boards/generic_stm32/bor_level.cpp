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

#if defined(STM32F4)
#include "stm32_hal.h"

void board_set_bor_level()
{
  FLASH_OBProgramInitTypeDef OB;
  HAL_FLASHEx_OBGetConfig(&OB);

  if (OB.BORLevel != OB_BOR_LEVEL3)
  {

        HAL_FLASH_Unlock();
        HAL_FLASH_OB_Unlock();

        OB.OptionType = OPTIONBYTE_BOR;
        OB.BORLevel = OB_BOR_LEVEL3;

        if ( HAL_FLASHEx_OBProgram(&OB) != HAL_OK )
        {
            HAL_FLASH_OB_Lock();
            HAL_FLASH_Lock();
        }

        HAL_FLASH_OB_Launch();

        HAL_FLASH_OB_Lock();
        HAL_FLASH_Lock();
  }

}
#endif
