# Fetch gtest source code from Github

include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest
  GIT_TAG        v1.15.2
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

option(BUILD_GMOCK "Build GMock" OFF)
option(INSTALL_GTEST "Install GTest" OFF)

FetchContent_MakeAvailable(googletest)

include_directories(
  ${googletest_SOURCE_DIR}/googletest/include
)
