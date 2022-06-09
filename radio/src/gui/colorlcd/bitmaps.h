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

#include "definitions.h"
#include "lz4_bitmaps.h"

DEFINE_LZ4_BITMAP(LBM_POINT);

// Model selection bitmaps
extern BitmapBuffer * modelselSdFreeBitmap;
extern BitmapBuffer * modelselModelQtyBitmap;
extern BitmapBuffer * modelselModelNameBitmap;
extern BitmapBuffer * modelselModelMoveBackground;
extern BitmapBuffer * modelselModelMoveIcon;
extern BitmapBuffer * modelselWizardBackground;

// calibration bitmaps
extern BitmapBuffer * calibStick;
extern BitmapBuffer * calibStickBackground;
extern BitmapBuffer * calibTrackpBackground;

// Channels monitor bitmaps
extern BitmapBuffer * chanMonLockedBitmap;
extern BitmapBuffer * chanMonInvertedBitmap;

// Mixer setup bitmaps
extern BitmapBuffer * mixerSetupMixerBitmap;
extern BitmapBuffer * mixerSetupToBitmap;
extern BitmapBuffer * mixerSetupOutputBitmap;
extern BitmapBuffer * mixerSetupAddBitmap;
extern BitmapBuffer * mixerSetupMultiBitmap;
extern BitmapBuffer * mixerSetupReplaceBitmap;
extern BitmapBuffer * mixerSetupLabelIcon;
extern BitmapBuffer * mixerSetupFlightmodeIcon;
extern BitmapBuffer * mixerSetupCurveIcon;
extern BitmapBuffer * mixerSetupSwitchIcon;
extern BitmapBuffer * mixerSetupDelayIcon;
extern BitmapBuffer * mixerSetupSlowIcon;
extern BitmapBuffer * mixerSetupDelaySlowIcon;

void loadBuiltinBitmaps();
const uint8_t* getBuiltinIcon(MenuIcons id);

PACK(struct _bitmap_mask {
  uint16_t w;
  uint16_t h;
  uint8_t mask[0];
});

#define MASK_WIDTH(m) (((_bitmap_mask*)m)->w)
#define MASK_HEIGHT(m) (((_bitmap_mask*)m)->h)
#define MASK_DATA(m) (((_bitmap_mask*)m)->mask)
