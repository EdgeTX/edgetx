# Fetch maxLibQt source code from Github

include(FetchContent)

FetchContent_Declare(
  maxLibQt
  GIT_REPOSITORY https://github.com/edgetx/maxLibQt
  GIT_TAG        b5418f76cc4891e09f4e21276175d39dbb130f66
)

FetchContent_MakeAvailable(maxLibQt)

message("Fetched maxLibQt source code from Github: ${maxLibQt_SOURCE_DIR}")
include_directories(
  ${maxLibQt_SOURCE_DIR}
  ${maxLibQt_BINARY_DIR}
)
