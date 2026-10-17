cmake_minimum_required(VERSION 3.20)
project(mambo_board_tests LANGUAGES CXX)
enable_testing()

add_executable(mambo_encoder encoder_tests.cpp
  ../../radio/src/targets/mambo/encoder.cpp
  ../../radio/src/targets/common/arm/stm32/rotary_encoder_driver.cpp)
target_compile_features(mambo_encoder PRIVATE cxx_std_17)
target_include_directories(mambo_encoder PRIVATE stubs ../../radio/src
  ../../radio/src/targets/common/arm/stm32 ../../radio/src/targets/simu)
target_compile_definitions(mambo_encoder PRIVATE BOOT SIMU)
target_compile_options(mambo_encoder PRIVATE -Wall -Wextra -Werror
  -include ${CMAKE_CURRENT_SOURCE_DIR}/stubs/stm32_hal_ll.h)
add_test(NAME mambo_encoder COMMAND mambo_encoder)
