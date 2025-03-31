set(BITMAP_SIZE_ARGS --size-format 2)
set(BITMAP_LZ4_ARGS ${BITMAP_SIZE_ARGS} --lz4)
set(MASK_LZ4_ARGS ${BITMAP_SIZE_ARGS} --lz4)

add_bitmaps_target(bm800_splash_logo "${RADIO_SRC_DIR}/bitmaps/800x480/splash_logo.png" "4/4/4/4" "${BITMAP_LZ4_ARGS}")
add_bitmaps_target(bm800_masks "${RADIO_SRC_DIR}/bitmaps/800x480/mask_*.png" 8bits "${MASK_LZ4_ARGS}")
add_bitmaps_target(bm800_slider_masks "${RADIO_SRC_DIR}/bitmaps/800x480/slider/*.png" 8bits "${MASK_LZ4_ARGS}")
add_bitmaps_target(bm800_volume_masks ${RADIO_SRC_DIR}/bitmaps/800x480/volume/*.png 8bits "${MASK_LZ4_ARGS}")
add_bitmaps_target(bm800_themes_masks "${RADIO_SRC_DIR}/bitmaps/800x480/default_theme/mask_*.png" 8bits "${MASK_LZ4_ARGS}")
add_bitmaps_target(bm800_themes_alpha "${RADIO_SRC_DIR}/bitmaps/800x480/default_theme/alpha_*.png" "4/4/4/4" "${BITMAP_LZ4_ARGS}")

add_bitmaps_target(bm800_bootloader_bitmaps "${RADIO_SRC_DIR}/bitmaps/800x480/bootloader/bmp_*.png" "4/4/4/4" "${BITMAP_LZ4_ARGS}")

add_custom_target(bm800_bitmaps)

add_dependencies(bm800_bitmaps
  bm800_splash_logo
  bm800_masks
  bm800_slider_masks
  bm800_themes_masks
  bm800_themes_alpha
  bm800_volume_masks
  bm800_bootloader_bitmaps
  )
