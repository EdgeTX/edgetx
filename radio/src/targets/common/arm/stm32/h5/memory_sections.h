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

#pragma once

#define __INIT_HOOK    __attribute__((section(".init_hook")))

#define __CCMRAM       __attribute__((section(".ccm"), aligned(4)))
#define __DMA          __attribute__((section(".ram"), aligned(4)))
#define __DMA_NO_CACHE __DMA
#define __IRAM

#if defined(SDRAM)
  #define __SDRAM      __attribute__((section(".sdram"), aligned(4)))
  #define __SDRAMFONTS __attribute__((section(".sdram_fonts"), aligned(4)))
#else
  #define __SDRAM
  #define __SDRAMFONTS
#endif

