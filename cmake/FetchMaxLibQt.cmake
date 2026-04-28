# Fetch maxLibQt source code from Github

include(FetchContent)

FetchContent_Declare(
  maxLibQt
  URL      https://github.com/edgetx/maxLibQt/archive/7e433da60d3f2e975d46afc91804a88029cd1b78.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP true
)

FetchContent_MakeAvailable(maxLibQt)

message("Fetched maxLibQt source code from Github: ${maxLibQt_SOURCE_DIR}")
include_directories(
  ${maxLibQt_SOURCE_DIR}
  ${maxLibQt_BINARY_DIR}
)
