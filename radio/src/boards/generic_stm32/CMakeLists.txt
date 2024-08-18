
# Generate hardware struct defs
AddHWGenTarget(${HW_DESC_JSON} stm32_keys stm32_keys.inc)
AddHWGenTarget(${HW_DESC_JSON} stm32_switches stm32_switches.inc)
AddHWGenTarget(${HW_DESC_JSON} stm32_adc_inputs stm32_adc_inputs.inc)
AddHWGenTarget(${HW_DESC_JSON} stm32_pwm_inputs stm32_pwm_inputs.inc)
AddHWGenTarget(${HW_DESC_JSON} hal_adc_inputs hal_adc_inputs.inc)

# Dependencies common to bootloader and firmware
set(MINIMAL_BOARD_LIB_SRC
  ${CMAKE_CURRENT_BINARY_DIR}/${HW_DESC_JSON}
  ${CMAKE_CURRENT_BINARY_DIR}/stm32_keys.inc

  boards/generic_stm32/inputs.cpp
  boards/generic_stm32/i2c_bus.cpp
  boards/generic_stm32/bl_keys.cpp
  boards/generic_stm32/aux_ports.cpp
)

# Dependencies only used in firmware
set(BOARD_LIB_SRC
  ${CMAKE_CURRENT_BINARY_DIR}/stm32_switches.inc
  ${CMAKE_CURRENT_BINARY_DIR}/stm32_adc_inputs.inc
  ${CMAKE_CURRENT_BINARY_DIR}/stm32_pwm_inputs.inc
  ${CMAKE_CURRENT_BINARY_DIR}/hal_adc_inputs.inc

  boards/generic_stm32/module_ports.cpp
  boards/generic_stm32/trainer_ports.cpp
  boards/generic_stm32/sport_update.cpp
  boards/generic_stm32/intmodule_heartbeat.cpp
  boards/generic_stm32/analog_inputs.cpp
  boards/generic_stm32/switches.cpp
  boards/generic_stm32/bor_level.cpp
  boards/generic_stm32/rgb_leds.cpp
)

add_library(minimal_board_lib OBJECT EXCLUDE_FROM_ALL ${MINIMAL_BOARD_LIB_SRC})

add_library(board_lib OBJECT EXCLUDE_FROM_ALL ${BOARD_LIB_SRC})
add_dependencies(board_lib minimal_board_lib)

set(FIRMWARE_SRC ${FIRMWARE_SRC}
  $<TARGET_OBJECTS:minimal_board_lib>
  $<TARGET_OBJECTS:board_lib>
)

set(BOOTLOADER_SRC ${BOOTLOADER_SRC}
  $<TARGET_OBJECTS:minimal_board_lib>
)
