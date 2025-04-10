# Fetch maxLibQt source code from Github

include(FetchContent)

FetchContent_Declare(
  maxLibQt
  GIT_REPOSITORY https://github.com/edgetx/maxLibQt
  GIT_TAG        ac1988ffd005cd15a8449b92150ce6c08574a4f1
)

FetchContent_MakeAvailable(maxLibQt)

message("Fetched maxLibQt source code from Github: ${maxLibQt_SOURCE_DIR}")
include_directories(
  ${maxLibQt_SOURCE_DIR}
  ${maxLibQt_BINARY_DIR}
)
