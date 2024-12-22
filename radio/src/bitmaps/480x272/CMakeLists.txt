set(BITMAP_SIZE_ARGS --size-format 2)
set(BITMAP_ARGS ${BITMAP_SIZE_ARGS})
set(MASK_ARGS ${BITMAP_SIZE_ARGS})

if(PCB STREQUAL NV14)
  set(BITMAP_TARGET_PREFIX nv14)
elseif(PCB STREQUAL PL18)
  set(BITMAP_TARGET_PREFIX pl18)
elseif(PCB STREQUAL X12S)
  set(BITMAP_TARGET_PREFIX x12s)
elseif(PCB STREQUAL STM32H7S78_DK)
  set(BITMAP_TARGET_PREFIX h7rs)
else()
  set(BITMAP_TARGET_PREFIX x10)
endif()

set(BITMAP_RLE_ARGS ${BITMAP_ARGS} --rle)
set(MASK_RLE_ARGS ${MASK_ARGS} --rle)

set(BITMAP_LZ4_ARGS ${BITMAP_ARGS} --lz4)
set(MASK_LZ4_ARGS ${MASK_ARGS} --lz4)

add_bitmaps_target(${BITMAP_TARGET_PREFIX}_splash_logo "${RADIO_SRC_DIR}/bitmaps/480x272/splash_logo.png" "4/4/4/4" "${BITMAP_LZ4_ARGS}")
add_bitmaps_target(${BITMAP_TARGET_PREFIX}_masks "${RADIO_SRC_DIR}/bitmaps/480x272/mask_*.png" 8bits "${MASK_LZ4_ARGS}")
add_bitmaps_target(${BITMAP_TARGET_PREFIX}_slider_masks "${RADIO_SRC_DIR}/bitmaps/480x272/slider/*.png" 8bits "${MASK_LZ4_ARGS}")
add_bitmaps_target(${BITMAP_TARGET_PREFIX}_volume_masks ${RADIO_SRC_DIR}/bitmaps/480x272/volume/*.png 8bits "${MASK_LZ4_ARGS}")
add_bitmaps_target(${BITMAP_TARGET_PREFIX}_themes_masks "${RADIO_SRC_DIR}/bitmaps/480x272/default_theme/mask_*.png" 8bits "${MASK_LZ4_ARGS}")
add_bitmaps_target(${BITMAP_TARGET_PREFIX}_themes_alpha "${RADIO_SRC_DIR}/bitmaps/480x272/default_theme/alpha_*.png" "4/4/4/4" "${BITMAP_LZ4_ARGS}")

add_bitmaps_target(${BITMAP_TARGET_PREFIX}_bootloader_bitmaps "${RADIO_SRC_DIR}/bitmaps/480x272/bootloader/bmp_*.png" "4/4/4/4" "${BITMAP_LZ4_ARGS}")

add_custom_target(${BITMAP_TARGET_PREFIX}_bitmaps)

add_dependencies(${BITMAP_TARGET_PREFIX}_bitmaps
  ${BITMAP_TARGET_PREFIX}_splash_logo
  ${BITMAP_TARGET_PREFIX}_masks
  ${BITMAP_TARGET_PREFIX}_slider_masks
  ${BITMAP_TARGET_PREFIX}_themes_masks
  ${BITMAP_TARGET_PREFIX}_themes_alpha
  ${BITMAP_TARGET_PREFIX}_volume_masks
  ${BITMAP_TARGET_PREFIX}_bootloader_bitmaps
  )
