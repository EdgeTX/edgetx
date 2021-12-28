# Cross compile toolchain file for EdgeTx
# See more at : https://cmake.org/cmake/help/latest/manual/cmake-toolchains.7.html"

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES TOOLCHAIN_LOCATION)

set(tools ${TOOLCHAIN_LOCATION})

set(CMAKE_C_COMPILER ${tools}/bin/arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER ${tools}/bin/arm-none-eabi-g++)
set(CMAKE_OBJCOPY ${tools}/bin/arm-none-eabi-objcopy)
set(CMAKE_ASM_COMPILER ${tools}/bin/arm-none-eabi-as)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
