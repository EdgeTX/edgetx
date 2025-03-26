set(BITMAP_SIZE_ARGS --size-format 2)
set(BITMAP_LZ4_ARGS ${BITMAP_SIZE_ARGS} --lz4)
set(MASK_LZ4_ARGS ${BITMAP_SIZE_ARGS} --lz4)

add_bitmaps_target(bm480_splash_logo "${RADIO_SRC_DIR}/bitmaps/480x272/splash_logo.png" "4/4/4/4" "${BITMAP_LZ4_ARGS}")
add_bitmaps_target(bm480_masks "${RADIO_SRC_DIR}/bitmaps/480x272/mask_*.png" 8bits "${MASK_LZ4_ARGS}")
add_bitmaps_target(bm480_slider_masks "${RADIO_SRC_DIR}/bitmaps/480x272/slider/*.png" 8bits "${MASK_LZ4_ARGS}")
add_bitmaps_target(bm480_volume_masks ${RADIO_SRC_DIR}/bitmaps/480x272/volume/*.png 8bits "${MASK_LZ4_ARGS}")
add_bitmaps_target(bm480_themes_masks "${RADIO_SRC_DIR}/bitmaps/480x272/default_theme/mask_*.png" 8bits "${MASK_LZ4_ARGS}")
add_bitmaps_target(bm480_themes_alpha "${RADIO_SRC_DIR}/bitmaps/480x272/default_theme/alpha_*.png" "4/4/4/4" "${BITMAP_LZ4_ARGS}")

add_bitmaps_target(bm480_bootloader_bitmaps "${RADIO_SRC_DIR}/bitmaps/480x272/bootloader/bmp_*.png" "4/4/4/4" "${BITMAP_LZ4_ARGS}")

add_custom_target(bm480_bitmaps)

add_dependencies(bm480_bitmaps
  bm480_splash_logo
  bm480_masks
  bm480_slider_masks
  bm480_themes_masks
  bm480_themes_alpha
  bm480_volume_masks
  bm480_bootloader_bitmaps
  )
