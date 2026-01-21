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

#pragma once

#include "hal/i2c_driver.h"

/* WARNING
   Ti PCA95 chip do NOT number pin sequentially, it goes PIN0 to PIN7 then PIN10 to PIN17.
   So when schematic indicate P10, it is PIN_8 in the following table
 */

#define PCA95XX_PIN_0 (1 << 0)
#define PCA95XX_PIN_1 (1 << 1)
#define PCA95XX_PIN_2 (1 << 2)
#define PCA95XX_PIN_3 (1 << 3)
#define PCA95XX_PIN_4 (1 << 4)
#define PCA95XX_PIN_5 (1 << 5)
#define PCA95XX_PIN_6 (1 << 6)
#define PCA95XX_PIN_7 (1 << 7)
#define PCA95XX_PIN_8 (1 << 8)
#define PCA95XX_PIN_9 (1 << 9)
#define PCA95XX_PIN_10 (1 << 10)
#define PCA95XX_PIN_11 (1 << 11)
#define PCA95XX_PIN_12 (1 << 12)
#define PCA95XX_PIN_13 (1 << 13)
#define PCA95XX_PIN_14 (1 << 14)
#define PCA95XX_PIN_15 (1 << 15)
#define PCA95XX_PIN_16 (1 << 16)
#define PCA95XX_PIN_17 (1 << 17)

#define PCA95XX_P0  PCA95XX_PIN_0
#define PCA95XX_P1  PCA95XX_PIN_1
#define PCA95XX_P2  PCA95XX_PIN_2
#define PCA95XX_P3  PCA95XX_PIN_3
#define PCA95XX_P4  PCA95XX_PIN_4
#define PCA95XX_P5  PCA95XX_PIN_5
#define PCA95XX_P6  PCA95XX_PIN_6
#define PCA95XX_P7  PCA95XX_PIN_7
#define PCA95XX_P10  PCA95XX_PIN_8
#define PCA95XX_P11  PCA95XX_PIN_9
#define PCA95XX_P12  PCA95XX_PIN_10
#define PCA95XX_P13  PCA95XX_PIN_11
#define PCA95XX_P14  PCA95XX_PIN_12
#define PCA95XX_P15  PCA95XX_PIN_13
#define PCA95XX_P16  PCA95XX_PIN_14
#define PCA95XX_P17  PCA95XX_PIN_15

typedef struct {
  etx_i2c_bus_t bus;
  uint16_t      addr;
  uint16_t      polarity;
  uint16_t      direction;
  uint16_t      output;
} pca95xx_t;

int pca95xx_init(pca95xx_t* dev, etx_i2c_bus_t bus, uint16_t addr);
int pca95xx_set_direction(pca95xx_t* dev, uint16_t mask, uint16_t dir);
int pca95xx_set_polarity(pca95xx_t* dev, uint16_t mask, uint16_t dir);
int pca95xx_write(pca95xx_t* dev, uint16_t mask, uint16_t value);
int pca95xx_read(pca95xx_t* dev, uint16_t mask, uint16_t* value);
