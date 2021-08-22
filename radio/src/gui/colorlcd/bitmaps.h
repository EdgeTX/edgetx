/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#ifndef _BITMAPS_H_
#define _BITMAPS_H_

// Header bitmaps
extern const uint8_t LBM_DOT[];

// Main view icons
extern const uint8_t LBM_TOPMENU_GPS[];
extern const uint8_t LBM_TOPMENU_USB[];
extern const uint8_t LBM_TOPMENU_ANTENNA[];
extern const uint8_t LBM_TOPMENU_VOLUME_0[];
extern const uint8_t LBM_TOPMENU_VOLUME_1[];
extern const uint8_t LBM_TOPMENU_VOLUME_2[];
extern const uint8_t LBM_TOPMENU_VOLUME_3[];
extern const uint8_t LBM_TOPMENU_VOLUME_4[];
extern const uint8_t LBM_TOPMENU_VOLUME_SCALE[];
extern const uint8_t LBM_TOPMENU_TXBATT[];
extern const uint8_t LBM_TOPMENU_TXBATT_CHARGING[];
extern const uint8_t LBM_HTRIM_FRAME[];
extern const uint8_t LBM_VTRIM_FRAME[];
extern const uint8_t LBM_TRIM_SHADOW[];
extern const uint8_t LBM_TIMER_BACKGROUND[];
extern const uint8_t LBM_TIMER[];
extern const uint8_t LBM_RSCALE[];

// Model selection icons
extern const uint8_t LBM_LIBRARY_SLOT[];
extern const uint8_t LBM_ACTIVE_MODEL[];
extern const uint8_t LBM_SCORE0[];
extern const uint8_t LBM_SCORE1[];
extern const uint8_t LBM_STAR0[];
extern const uint8_t LBM_STAR1[];

// Other icons
extern const uint8_t LBM_SPLASH[];
extern const uint8_t LBM_POINT[];
extern const uint8_t LBM_CURVE_POINT[];
extern const uint8_t LBM_CURVE_POINT_CENTER[];
extern const uint8_t LBM_CURVE_COORD_SHADOW[];
extern const uint8_t LBM_SHUTDOWN_CIRCLE[];

// Slider bitmaps
extern const uint8_t LBM_SLIDER_BAR_LEFT[];
extern const uint8_t LBM_SLIDER_BAR_RIGHT[];
extern const uint8_t LBM_SLIDER_POINT_OUT[];
extern const uint8_t LBM_SLIDER_POINT_MID[];
extern const uint8_t LBM_SLIDER_POINT_IN[];

// Carroussel bitmaps
extern const uint8_t LBM_CARROUSSEL_LEFT[];
extern const uint8_t LBM_CARROUSSEL_RIGHT[];

extern const uint8_t LBM_SWIPE_CIRCLE[];
extern const uint8_t LBM_SWIPE_LEFT[];
extern const uint8_t LBM_SWIPE_RIGHT[];

//extern BitmapBuffer * modelselIconBitmap;
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

#endif // _BITMAPS_H_
