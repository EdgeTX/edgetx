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

#include "opentx.h"

/*
 *  Header bitmaps
 */

const uint8_t LBM_DOT[] = {
#include "mask_dot.lbm"
};

/*
 * Main view bitmaps
 */

const uint8_t LBM_TOPMENU_GPS[] = {
#include "mask_topmenu_gps_18.lbm"
};

const uint8_t LBM_TOPMENU_USB[] = {
#include "mask_topmenu_usb.lbm"
};

const uint8_t LBM_TOPMENU_VOLUME_0[] = {
#include "mask_volume_0.lbm"
};

const uint8_t LBM_TOPMENU_VOLUME_1[] = {
#include "mask_volume_1.lbm"
};

const uint8_t LBM_TOPMENU_VOLUME_2[] = {
#include "mask_volume_2.lbm"
};

const uint8_t LBM_TOPMENU_VOLUME_3[] = {
#include "mask_volume_3.lbm"
};

const uint8_t LBM_TOPMENU_VOLUME_4[] = {
#include "mask_volume_4.lbm"
};

const uint8_t LBM_TOPMENU_VOLUME_SCALE[] = {
#include "mask_volume_scale.lbm"
};

const uint8_t LBM_TOPMENU_TXBATT[] = {
#include "mask_txbat.lbm"
};

const uint8_t LBM_TOPMENU_TXBATT_CHARGING[] = {
#include "mask_txbat_charging.lbm"
};

const uint8_t LBM_TOPMENU_ANTENNA[] = {
#include "mask_antenna.lbm"
};

const uint8_t LBM_HTRIM_FRAME[] = {
#include "mask_htrim_frame.lbm"
};

const uint8_t LBM_VTRIM_FRAME[] = {
#include "mask_vtrim_frame.lbm"
};

const uint8_t LBM_TRIM_SHADOW[] = {
#include "mask_trim_shadow.lbm"
};

const uint8_t LBM_TIMER_BACKGROUND[] = {
#include "mask_timer_bg.lbm"
};

const uint8_t LBM_TIMER[] = {
#include "mask_timer.lbm"
};

const uint8_t LBM_RSCALE[] = {
#include "mask_rscale.lbm"
};

/*
 *  Model selection screen bitmaps
 */

const uint8_t LBM_LIBRARY_SLOT[] = {
#include "mask_library_slot.lbm"
};

const uint8_t LBM_ACTIVE_MODEL[] = {
#include "mask_library_active_model.lbm"
};

const uint8_t LBM_SCORE0[] = {
#include "mask_library_score_0.lbm"
};

const uint8_t LBM_SCORE1[] = {
#include "mask_library_score_1.lbm"
};

const uint8_t LBM_STAR0[] = {
#include "mask_library_star_0.lbm"
};

const uint8_t LBM_STAR1[] = {
#include "mask_library_star_1.lbm"
};

/*
 * Other
 */

const uint8_t LBM_POINT[] = {
#include "mask_point.lbm"
};

const uint8_t LBM_CURVE_POINT[] = {
#include "mask_cvpoint.lbm"
};

const uint8_t LBM_CURVE_POINT_CENTER[] = {
#include "mask_cvpoint_center.lbm"
};

const uint8_t LBM_CURVE_COORD_SHADOW[] = {
#include "mask_coord_shadow.lbm"
};

const uint8_t LBM_SHUTDOWN_CIRCLE[] = {
#include "mask_shutdown_circle.lbm"
};

const uint8_t LBM_SLIDER_BAR_LEFT[] __ALIGNED(4) = {
#include "bar_left.lbm"
};

const uint8_t LBM_SLIDER_BAR_RIGHT[] __ALIGNED(4) = {
#include "bar_right.lbm"
};

const uint8_t LBM_SLIDER_POINT_OUT[] __ALIGNED(4) = {
#include "point_out.lbm"
};

const uint8_t LBM_SLIDER_POINT_MID[] __ALIGNED(4) = {
#include "point_mid.lbm"
};

const uint8_t LBM_SLIDER_POINT_IN[] __ALIGNED(4) = {
#include "point_in.lbm"
};

const uint8_t LBM_CARROUSSEL_LEFT[] = {
#include "mask_carroussel_left.lbm"
};

const uint8_t LBM_CARROUSSEL_RIGHT[] = {
#include "mask_carroussel_right.lbm"
};

const uint8_t LBM_SWIPE_CIRCLE[] = {
  #include "mask_swipe_circle.lbm"
};

const uint8_t LBM_SWIPE_LEFT[] = {
  #include "mask_swipe_left.lbm"
};

const uint8_t LBM_SWIPE_RIGHT[] = {
  #include "mask_swipe_right.lbm"
};

BitmapBuffer * calibStick = nullptr;
BitmapBuffer * calibStickBackground = nullptr;
BitmapBuffer * calibTrackpBackground = nullptr;
BitmapBuffer * modelselIconBitmap = nullptr;
BitmapBuffer * modelselSdFreeBitmap = nullptr;
BitmapBuffer * modelselModelQtyBitmap = nullptr;
BitmapBuffer * modelselModelNameBitmap = nullptr;
BitmapBuffer * modelselModelMoveBackground = nullptr;
BitmapBuffer * modelselModelMoveIcon = nullptr;
BitmapBuffer * modelselWizardBackground = nullptr;
BitmapBuffer * chanMonLockedBitmap = nullptr;
BitmapBuffer * chanMonInvertedBitmap = nullptr;
BitmapBuffer * mixerSetupMixerBitmap = nullptr;
BitmapBuffer * mixerSetupToBitmap = nullptr;
BitmapBuffer * mixerSetupOutputBitmap = nullptr;
BitmapBuffer * mixerSetupAddBitmap = nullptr;
BitmapBuffer * mixerSetupMultiBitmap = nullptr;
BitmapBuffer * mixerSetupReplaceBitmap = nullptr;
BitmapBuffer * mixerSetupCurveIcon = nullptr;
BitmapBuffer * mixerSetupSwitchIcon = nullptr;
BitmapBuffer * mixerSetupLabelIcon = nullptr;
BitmapBuffer * mixerSetupDelayIcon = nullptr;
BitmapBuffer * mixerSetupSlowIcon = nullptr;
BitmapBuffer * mixerSetupDelaySlowIcon = nullptr;
BitmapBuffer * mixerSetupFlightmodeIcon = nullptr;
