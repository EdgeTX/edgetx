# Fetch miniz source code from Github

include(FetchContent)

FetchContent_Declare(
  miniz
  GIT_REPOSITORY https://github.com/richgel999/miniz
  GIT_TAG        89d7a5f6c3ce8893ea042b0a9d2a2d9975589ac9
)

FetchContent_MakeAvailable(miniz)

message("Fetched miniz source code from Github: ${miniz_SOURCE_DIR}")
include_directories(
  ${miniz_SOURCE_DIR}
  ${miniz_BINARY_DIR}
)

# suppress compiler warnings for this add-in only
target_compile_options(miniz PRIVATE -Wno-unused-function)
