# Fetch gtest source code from Github

include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest
  GIT_TAG        f8d7d77c06936315286eb55f8de22cd23c188571 # v1.14.0
)

FetchContent_MakeAvailable(googletest)

include_directories(
  ${googletest_SOURCE_DIR}/googletest/include
)
