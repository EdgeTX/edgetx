# Fetch yaml-cpp

include(FetchContent)

FetchContent_Declare(
  yaml-cpp
  URL      https://github.com/jbeder/yaml-cpp/archive/28f93bdec6387d42332220afa9558060c8016795.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP true
)

FetchContent_MakeAvailable(yaml-cpp)
message("Fetched yaml-cpp source code from Github: ${yaml-cpp_SOURCE_DIR}")
