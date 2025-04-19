# Fetch yaml-cpp

include(FetchContent)

FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp
  GIT_TAG        28f93bdec6387d42332220afa9558060c8016795
)

FetchContent_MakeAvailable(yaml-cpp)
message("Fetched yaml-cpp source code from Github: ${yaml-cpp_SOURCE_DIR}")
