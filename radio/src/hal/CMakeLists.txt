
AddHWGenTarget(${HW_DESC_JSON} hal_keys hal_keys.inc)

set(SRC ${SRC}
  ${CMAKE_CURRENT_BINARY_DIR}/hal_keys.inc

  hal/module_port.cpp
  hal/adc_driver.cpp
  hal/key_driver.cpp
)
