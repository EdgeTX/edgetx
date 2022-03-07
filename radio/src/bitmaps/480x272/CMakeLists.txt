set(BITMAP_SIZE_ARGS --size-format 2)
set(BITMAP_ARGS ${BITMAP_SIZE_ARGS})
set(MASK_ARGS ${BITMAP_SIZE_ARGS})

if(PCB STREQUAL NV14)
  set(BITMAP_TARGET_PREFIX nv14)
elseif(PCB STREQUAL PL18)
  set(BITMAP_TARGET_PREFIX pl18)
elseif(PCB STREQUAL X12S)
  set(BITMAP_TARGET_PREFIX x12s)
else()
  set(BITMAP_TARGET_PREFIX x10)
  if(PCBREV STREQUAL T18)
    # straight screen
  else()
#    set(BITMAP_ARGS ${BITMAP_ARGS} --reverse)
#    set(MASK_ARGS ${MASK_ARGS} --reverse)
  endif()
endif()

set(BITMAP_RLE_ARGS ${BITMAP_ARGS} --rle)
set(MASK_RLE_ARGS ${MASK_ARGS} --rle)

add_bitmaps_target(${BITMAP_TARGET_PREFIX}_bitmaps "${RADIO_SRC_DIR}/bitmaps/480x272/bmp_*.png" "5/6/5" "${BITMAP_ARGS}")
if(PCB STREQUAL PL18)
  add_png_target(${BITMAP_TARGET_PREFIX}_splash "${RADIO_SRC_DIR}/bitmaps/480x272/splash_480x320.png")
  add_png_target(${BITMAP_TARGET_PREFIX}_splash_chr "${RADIO_SRC_DIR}/bitmaps/480x272/splash_chr_480x320.png")
else()
  add_png_target(${BITMAP_TARGET_PREFIX}_splash "${RADIO_SRC_DIR}/bitmaps/480x272/splash.png")
  add_png_target(${BITMAP_TARGET_PREFIX}_splash_chr "${RADIO_SRC_DIR}/bitmaps/480x272/splash_chr.png")
endif()

add_bitmaps_target(${BITMAP_TARGET_PREFIX}_calibration_bitmaps "${RADIO_SRC_DIR}/bitmaps/480x272/calibration/bmp_*.png" "5/6/5" "${BITMAP_ARGS}")
add_bitmaps_target(${BITMAP_TARGET_PREFIX}_button_bitmaps "${RADIO_SRC_DIR}/bitmaps/480x272/button/alpha_*.png" "4/4/4/4" "${BITMAP_ARGS}")
add_bitmaps_target(${BITMAP_TARGET_PREFIX}_alpha_bitmaps "${RADIO_SRC_DIR}/bitmaps/480x272/alpha_*.png" "4/4/4/4" "${BITMAP_ARGS}")
add_bitmaps_target(${BITMAP_TARGET_PREFIX}_alpha_calibration_bitmaps "${RADIO_SRC_DIR}/bitmaps/480x272/calibration/alpha_*.png" "4/4/4/4" "${BITMAP_ARGS}")
add_bitmaps_target(${BITMAP_TARGET_PREFIX}_masks "${RADIO_SRC_DIR}/bitmaps/480x272/mask_*.png" 8bits "${MASK_ARGS}")
add_bitmaps_target(${BITMAP_TARGET_PREFIX}_slider_masks "${RADIO_SRC_DIR}/bitmaps/480x272/slider/*.png" 8bits "${MASK_ARGS}")
add_bitmaps_target(${BITMAP_TARGET_PREFIX}_layouts_masks "${RADIO_SRC_DIR}/gui/colorlcd/layouts/*.png" 8bits "${MASK_ARGS}")
add_bitmaps_target(${BITMAP_TARGET_PREFIX}_fonts ${RADIO_SRC_DIR}/fonts/truetype/*.png 8bits "${MASK_RLE_ARGS}")
add_bitmaps_target(${BITMAP_TARGET_PREFIX}_volume_masks ${RADIO_SRC_DIR}/bitmaps/480x272/volume/*.png 8bits "${MASK_ARGS}")

add_bitmaps_target(${BITMAP_TARGET_PREFIX}_themes_masks "${RADIO_SRC_DIR}/bitmaps/480x272/default_theme/mask_*.png" 8bits "${BITMAPS_ARGS}")
add_png_target(${BITMAP_TARGET_PREFIX}_themes_alpha "${RADIO_SRC_DIR}/bitmaps/480x272/default_theme/alpha_*.png")

add_bitmaps_target(${BITMAP_TARGET_PREFIX}_bootloader_bitmaps "${RADIO_SRC_DIR}/bitmaps/480x272/bootloader/bmp_*.png" "4/4/4/4" "${BITMAP_RLE_ARGS}")
add_bitmaps_target(${BITMAP_TARGET_PREFIX}_bootloader_icons "${RADIO_SRC_DIR}/bitmaps/480x272/bootloader/icon_*.png" 8bits "${MASK_ARGS}")

add_dependencies(${BITMAP_TARGET_PREFIX}_bitmaps ${BITMAP_TARGET_PREFIX}_calibration_bitmaps ${BITMAP_TARGET_PREFIX}_button_bitmaps ${BITMAP_TARGET_PREFIX}_alpha_bitmaps
                 ${BITMAP_TARGET_PREFIX}_alpha_calibration_bitmaps ${BITMAP_TARGET_PREFIX}_masks ${BITMAP_TARGET_PREFIX}_slider_masks ${BITMAP_TARGET_PREFIX}_layouts_masks
                 ${BITMAP_TARGET_PREFIX}_fonts ${BITMAP_TARGET_PREFIX}_themes_masks ${BITMAP_TARGET_PREFIX}_themes_alpha ${BITMAP_TARGET_PREFIX}_splash
				 ${BITMAP_TARGET_PREFIX}_splash_chr ${BITMAP_TARGET_PREFIX}_volume_masks 
                 ${BITMAP_TARGET_PREFIX}_bootloader_bitmaps ${BITMAP_TARGET_PREFIX}_bootloader_icons )
