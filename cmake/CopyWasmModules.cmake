# Copy all WASM simulator modules from SRC_DIR to DST_DIR at build time.
# Invoked via cmake -P so the pattern is evaluated on every build.
file(COPY "${SRC_DIR}/" DESTINATION "${DST_DIR}"
  FILES_MATCHING PATTERN "edgetx-*-simulator.wasm")
