set(BITMAP_SIZE_ARGS --size-format 2)
set(BITMAP_LZ4_ARGS ${BITMAP_SIZE_ARGS} --lz4)
set(MASK_LZ4_ARGS ${BITMAP_SIZE_ARGS} --lz4)

add_bitmaps_target(bm320_splash_logo "${RADIO_SRC_DIR}/bitmaps/320x240/splash_logo.png" "4/4/4/4" "${BITMAP_LZ4_ARGS}")
add_bitmaps_target(bm320_masks "${RADIO_SRC_DIR}/bitmaps/320x240/mask_*.png" 8bits "${MASK_LZ4_ARGS}")
add_bitmaps_target(bm320_slider_masks "${RADIO_SRC_DIR}/bitmaps/320x240/slider/*.png" 8bits "${MASK_LZ4_ARGS}")
add_bitmaps_target(bm320_volume_masks ${RADIO_SRC_DIR}/bitmaps/320x240/volume/*.png 8bits "${MASK_LZ4_ARGS}")
add_bitmaps_target(bm320_themes_masks "${RADIO_SRC_DIR}/bitmaps/320x240/default_theme/mask_*.png" 8bits "${MASK_LZ4_ARGS}")
add_bitmaps_target(bm320_themes_alpha "${RADIO_SRC_DIR}/bitmaps/320x240/default_theme/alpha_*.png" "4/4/4/4" "${BITMAP_LZ4_ARGS}")

add_bitmaps_target(bm320_bootloader_bitmaps "${RADIO_SRC_DIR}/bitmaps/320x240/bootloader/bmp_*.png" "4/4/4/4" "${BITMAP_LZ4_ARGS}")

add_custom_target(bm320_bitmaps)

add_dependencies(bm320_bitmaps
  bm320_splash_logo
  bm320_masks
  bm320_slider_masks
  bm320_themes_masks
  bm320_themes_alpha
  bm320_volume_masks
  bm320_bootloader_bitmaps
  )
