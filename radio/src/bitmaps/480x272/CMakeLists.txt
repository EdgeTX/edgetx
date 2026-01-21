set(BITMAP_SIZE_ARGS --size-format 2)
set(BITMAP_LZ4_ARGS ${BITMAP_SIZE_ARGS} --lz4)
set(MASK_LZ4_ARGS ${BITMAP_SIZE_ARGS} --lz4)

add_bitmaps_target(bm480_bmps "${RADIO_SRC_DIR}/bitmaps/480x272/bmp_*.png" "4/4/4/4" "${BITMAP_LZ4_ARGS}")
add_bitmaps_target(bm480_masks "${RADIO_SRC_DIR}/bitmaps/480x272/mask_*.png" 8bits "${MASK_LZ4_ARGS}")

add_custom_target(bm480_bitmaps)

add_dependencies(bm480_bitmaps
  bm480_bmps
  bm480_masks
  )
