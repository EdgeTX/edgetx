# Native toolchain
set(CMAKE_CXX_STANDARD 17)

if(APPLE)
  set(CMAKE_C_FLAGS "-Wno-deprecated-declarations")
  set(CMAKE_CXX_FLAGS "-Wno-deprecated-declarations")
  set(CMAKE_C_FLAGS_DEBUG "-Wno-deprecated-declarations")
  set(CMAKE_CXX_FLAGS_DEBUG "-Wno-deprecated-declarations")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-address-of-packed-member -Wno-deprecated-declarations")

if(MINGW OR WIN32)
  set(CMAKE_OBJECT_PATH_MAX 200)
endif()
