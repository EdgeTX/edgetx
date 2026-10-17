cmake_minimum_required(VERSION 3.20)
project(mambo_board_tests LANGUAGES CXX)
enable_testing()

function(mambo_test target)
  add_executable(${target} ${ARGN})
  target_compile_features(${target} PRIVATE cxx_std_17)
  target_include_directories(${target} PRIVATE ../../radio/src)
  target_compile_options(${target} PRIVATE -Wall -Wextra -Werror)
  add_test(NAME ${target} COMMAND ${target})
endfunction()

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

mambo_test(mambo_hall hall_tests.cpp ../../radio/src/targets/mambo/gimbals.cpp)
mambo_test(mambo_trims trim_tests.cpp)
mambo_test(mambo_battery battery_tests.cpp
  ../../radio/src/boards/generic_stm32/battery_voltage.cpp)
target_include_directories(mambo_battery BEFORE PRIVATE battery_stubs)
target_compile_options(mambo_battery PRIVATE -U__weak)

mambo_test(mambo_external external_power_tests.cpp
  ../../radio/src/targets/mambo/external_rf.cpp)
target_include_directories(mambo_external BEFORE PRIVATE external_stubs)
target_compile_definitions(mambo_external PRIVATE RADIO_MAMBO)

foreach(polarity IN ITEMS high low)
  mambo_test(mambo_power_${polarity} power_tests.cpp
    ../../radio/src/targets/common/arm/stm32/pwr_driver.cpp)
  target_include_directories(mambo_power_${polarity} BEFORE PRIVATE power_stubs
    ../../radio/src/targets/common/arm/stm32)
  if(polarity STREQUAL "high")
    target_compile_definitions(mambo_power_${polarity} PRIVATE
      PWR_SWITCH_PIN_ACTIVE_HIGH PWR_BUTTON_PRESS)
  endif()
endforeach()

add_executable(mambo_calibration_probe calibration_probe.cpp
  ../../radio/src/targets/mambo/calibration.cpp)
target_compile_features(mambo_calibration_probe PRIVATE cxx_std_17)
target_compile_options(mambo_calibration_probe PRIVATE -Wall -Wextra -Werror)
target_include_directories(mambo_calibration_probe PRIVATE ../../radio/src)
find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_test(NAME mambo_calibration COMMAND ${Python3_EXECUTABLE}
  ${CMAKE_CURRENT_SOURCE_DIR}/test_calibration.py
  $<TARGET_FILE:mambo_calibration_probe>)
