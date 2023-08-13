# Native toolchain

if(APPLE)
  set(CMAKE_C_FLAGS "-Wno-asm-operand-widths -Wno-deprecated-declarations")
  set(CMAKE_CXX_FLAGS "-Wno-asm-operand-widths -Wno-deprecated-declarations")
endif()
