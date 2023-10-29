# Fetch yaml-cpp

include(FetchContent)

FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp
  GIT_TAG        f7320141120f720aecc4c32be25586e7da9eb978 # v0.8.0
)

FetchContent_MakeAvailable(yaml-cpp)
message("Fetched yaml-cpp source code from Github: ${yaml-cpp_SOURCE_DIR}")
