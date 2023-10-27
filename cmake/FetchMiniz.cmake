# Fetch miniz source code from Github

include(FetchContent)

FetchContent_Declare(
  miniz
  GIT_REPOSITORY https://github.com/richgel999/miniz
  GIT_TAG        293d4db1b7d0ffee9756d035b9ac6f7431ef8492 # v3.0.2
)

FetchContent_MakeAvailable(miniz)

message("Fetched miniz source code from Github: ${miniz_SOURCE_DIR}")
include_directories(
  ${miniz_SOURCE_DIR}
  ${miniz_BINARY_DIR}
)
