/*
 * Copyright (C) EdgeTx
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

#include "stm32_spi.h"

void stm32_spi_enable_clock(SPI_TypeDef *SPIx)
{
  if (SPIx == SPI1) {
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);
  }
#if defined(SPI2)
  else if (SPIx == SPI2) {
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI2);
  }
#endif
#if defined(SPI3)
  else if (SPIx == SPI3) {
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI3);
  }
#endif
#if defined(SPI4)
  else if (SPIx == SPI4) {
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI4);
  }
#endif
#if defined(SPI5)
  else if (SPIx == SPI5) {
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI5);
  }
#endif
#if defined(SPI6)
  else if (SPIx == SPI6) {
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI6);
  }
#endif
}

