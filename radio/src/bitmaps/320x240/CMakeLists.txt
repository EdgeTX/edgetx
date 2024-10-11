set(BITMAP_SIZE_ARGS --size-format 2)
set(BITMAP_ARGS ${BITMAP_SIZE_ARGS})
set(MASK_ARGS ${BITMAP_SIZE_ARGS})

set(BITMAP_TARGET_PREFIX x10)

set(BITMAP_RLE_ARGS ${BITMAP_ARGS} --rle)
set(MASK_RLE_ARGS ${MASK_ARGS} --rle)

set(BITMAP_LZ4_ARGS ${BITMAP_ARGS} --lz4)
set(MASK_LZ4_ARGS ${MASK_ARGS} --lz4)

add_bitmaps_target(${BITMAP_TARGET_PREFIX}_splash_logo "${RADIO_SRC_DIR}/bitmaps/320x240/splash_logo.png" "4/4/4/4" "${BITMAP_LZ4_ARGS}")
add_bitmaps_target(${BITMAP_TARGET_PREFIX}_masks "${RADIO_SRC_DIR}/bitmaps/320x240/mask_*.png" 8bits "${MASK_LZ4_ARGS}")
add_bitmaps_target(${BITMAP_TARGET_PREFIX}_slider_masks "${RADIO_SRC_DIR}/bitmaps/320x240/slider/*.png" 8bits "${MASK_LZ4_ARGS}")
add_bitmaps_target(${BITMAP_TARGET_PREFIX}_volume_masks ${RADIO_SRC_DIR}/bitmaps/320x240/volume/*.png 8bits "${MASK_LZ4_ARGS}")
add_bitmaps_target(${BITMAP_TARGET_PREFIX}_themes_masks "${RADIO_SRC_DIR}/bitmaps/320x240/default_theme/mask_*.png" 8bits "${MASK_LZ4_ARGS}")
add_bitmaps_target(${BITMAP_TARGET_PREFIX}_themes_alpha "${RADIO_SRC_DIR}/bitmaps/320x240/default_theme/alpha_*.png" "4/4/4/4" "${BITMAP_LZ4_ARGS}")

add_bitmaps_target(${BITMAP_TARGET_PREFIX}_bootloader_bitmaps "${RADIO_SRC_DIR}/bitmaps/320x240/bootloader/bmp_*.png" "4/4/4/4" "${BITMAP_LZ4_ARGS}")

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
