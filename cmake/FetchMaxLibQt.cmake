# Fetch maxLibQt source code from Github

include(FetchContent)

FetchContent_Declare(
  maxLibQt
  GIT_REPOSITORY https://github.com/edgetx/maxLibQt
  GIT_TAG        98f5482fb8b836f48a6a5ee199911882fa420f2b
)

FetchContent_MakeAvailable(maxLibQt)

message("Fetched maxLibQt source code from Github: ${maxLibQt_SOURCE_DIR}")
include_directories(
  ${maxLibQt_SOURCE_DIR}
  ${maxLibQt_BINARY_DIR}
)
