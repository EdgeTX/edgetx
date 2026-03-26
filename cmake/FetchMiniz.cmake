# Fetch miniz source code from Github

include(FetchContent)

FetchContent_Declare(
  miniz
  URL      https://github.com/richgel999/miniz/archive/89d7a5f6c3ce8893ea042b0a9d2a2d9975589ac9.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP true
)

FetchContent_MakeAvailable(miniz)

message("Fetched miniz source code from Github: ${miniz_SOURCE_DIR}")
include_directories(
  ${miniz_SOURCE_DIR}
  ${miniz_BINARY_DIR}
)

# suppress compiler warnings for this add-in only
target_compile_options(miniz PRIVATE -Wno-unused-function -Wno-newline-eof)
