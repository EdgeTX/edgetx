# Native toolchain

if(APPLE)
  set(CMAKE_C_FLAGS "-Wno-asm-operand-widths -Wno-deprecated-declarations")
  set(CMAKE_CXX_FLAGS "-Wno-asm-operand-widths -Wno-deprecated-declarations")
  set(CMAKE_C_FLAGS_DEBUG "-Wno-asm-operand-widths -Wno-deprecated-declarations")
  set(CMAKE_CXX_FLAGS_DEBUG "-Wno-asm-operand-widths -Wno-deprecated-declarations")
endif()

if(MINGW OR WIN32)
  set(CMAKE_OBJECT_PATH_MAX 200)
endif()
