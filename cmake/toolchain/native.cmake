# Native toolchain
set(CMAKE_CXX_STANDARD 17)

if(APPLE)
  set(CMAKE_C_FLAGS "-Wno-deprecated-declarations")
  set(CMAKE_CXX_FLAGS "-Wno-deprecated-declarations")
  set(CMAKE_C_FLAGS_DEBUG "-Wno-deprecated-declarations")
  set(CMAKE_CXX_FLAGS_DEBUG "-Wno-deprecated-declarations")
endif()

if(MINGW OR WIN32)
  set(CMAKE_OBJECT_PATH_MAX 200)
endif()
