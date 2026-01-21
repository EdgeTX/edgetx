set(BITMAP_SIZE_ARGS --size-format 2)
set(BITMAP_LZ4_ARGS ${BITMAP_SIZE_ARGS} --lz4)
set(MASK_LZ4_ARGS ${BITMAP_SIZE_ARGS} --lz4)

add_bitmaps_target(bm800_bmps "${RADIO_SRC_DIR}/bitmaps/800x480/bmp_*.png" "4/4/4/4" "${BITMAP_LZ4_ARGS}")
add_bitmaps_target(bm800_masks "${RADIO_SRC_DIR}/bitmaps/800x480/os/mask_*.png" 8bits "${MASK_LZ4_ARGS}")

add_custom_target(bm800_bitmaps)

add_dependencies(bm800_bitmaps
  bm800_bmps
  bm800_masks
  )
