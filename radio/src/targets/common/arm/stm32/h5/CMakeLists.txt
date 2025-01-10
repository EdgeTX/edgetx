set(CPU_FAMILY STM32)
set(MCU cortex-m33)
set(FPU_FLAGS "-mcpu=cortex-m33 -mfpu=fpv4-sp-d16 -mfloat-abi=hard")

# CMSIS library
set(CMSIS_DIR ${THIRDPARTY_DIR}/CMSIS)
include_directories(${CMSIS_DIR}/Include)

include_directories(${CMSIS_DIR}/Device/ST/STM32H5xx/Include)
set(CMSIS_SRC
  targets/common/arm/stm32/h5/system_stm32h5xx.c
)


if(NOT NATIVE_BUILD)
  set(STM32CUBE_PREFIX stm32h5xx)
  set(STM32CUBE_DIR ${THIRDPARTY_DIR}/STM32H5xx_HAL_Driver)
  set(STM32CUBE_SRC_PREFIX ${STM32CUBE_DIR}/Src/${STM32CUBE_PREFIX})
  set(STM32CUBE_SRC
    # LL drivers
    ${STM32CUBE_SRC_PREFIX}_ll_gpio.c
    ${STM32CUBE_SRC_PREFIX}_ll_fmc.c
    ${STM32CUBE_SRC_PREFIX}_ll_tim.c
    ${STM32CUBE_SRC_PREFIX}_ll_dma.c
    ${STM32CUBE_SRC_PREFIX}_ll_usart.c
    ${STM32CUBE_SRC_PREFIX}_ll_rcc.c
    ${STM32CUBE_SRC_PREFIX}_ll_exti.c
    ${STM32CUBE_SRC_PREFIX}_ll_sdmmc.c
    ${STM32CUBE_SRC_PREFIX}_ll_adc.c
    ${STM32CUBE_SRC_PREFIX}_ll_spi.c
    ${STM32CUBE_SRC_PREFIX}_ll_usb.c
    # HAL drivers
    ${STM32CUBE_SRC_PREFIX}_hal.c
    ${STM32CUBE_SRC_PREFIX}_hal_pwr.c
    ${STM32CUBE_SRC_PREFIX}_hal_pwr_ex.c
    ${STM32CUBE_SRC_PREFIX}_hal_rcc.c
    ${STM32CUBE_SRC_PREFIX}_hal_rcc_ex.c
    ${STM32CUBE_SRC_PREFIX}_hal_rtc.c
    ${STM32CUBE_SRC_PREFIX}_hal_sd.c
    ${STM32CUBE_SRC_PREFIX}_hal_dma.c
    ${STM32CUBE_SRC_PREFIX}_hal_pcd.c
    ${STM32CUBE_SRC_PREFIX}_hal_pcd_ex.c
    ${STM32CUBE_SRC_PREFIX}_hal_flash.c
    ${STM32CUBE_SRC_PREFIX}_hal_flash_ex.c
    ${STM32CUBE_SRC_PREFIX}_hal_cortex.c
    ${STM32CUBE_SRC_PREFIX}_hal_gpio.c
  )

  # STM32 HAL/LL library
  include_directories(${STM32CUBE_DIR}/Inc)

  add_library(stm32cube_ll OBJECT EXCLUDE_FROM_ALL ${STM32CUBE_SRC})
  target_compile_definitions(stm32cube_ll PRIVATE USE_FULL_LL_DRIVER)

  set(FIRMWARE_SRC ${FIRMWARE_SRC} $<TARGET_OBJECTS:stm32cube_ll>)
  set(BOOTLOADER_SRC ${BOOTLOADER_SRC} $<TARGET_OBJECTS:stm32cube_ll>)
endif()

add_definitions(-DSTM32H5)

if(CPU_TYPE_FULL STREQUAL STM32H562xx)
  set(CMSIS_SRC ${CMSIS_SRC}
    boards/generic_stm32/startup/startup_stm32h562xx.s
  )
else()
  message("### ERROR ####: ${CPU_TYPE_FULL}")
endif()

add_library(cmsis OBJECT EXCLUDE_FROM_ALL
  ${CMSIS_SRC}
)

set(FIRMWARE_SRC ${FIRMWARE_SRC} $<TARGET_OBJECTS:cmsis>)
set(BOOTLOADER_SRC ${BOOTLOADER_SRC} $<TARGET_OBJECTS:cmsis>)

if(NOT NATIVE_BUILD)
  #
  # FreeRTOS port specific include
  #
  add_library(freertos OBJECT EXCLUDE_FROM_ALL
    ${RTOS_DIR}/list.c
    ${RTOS_DIR}/queue.c
    ${RTOS_DIR}/tasks.c
    ${RTOS_DIR}/event_groups.c
    ${RTOS_DIR}/timers.c
    ${RTOS_DIR}/stream_buffer.c
    ${RTOS_DIR}/portable/GCC/ARM_CM33_NTZ/non_secure/port.c
    )

  target_include_directories(freertos
    PUBLIC  ${RTOS_DIR}/portable/GCC/ARM_CM33_NTZ/non_secure
    PRIVATE ${RTOS_DIR}/include
    )

  include_directories(${RTOS_DIR}/portable/GCC/ARM_CM33_NTZ/non_secure)
  
  # EdgeTx Compat layer
  set(FIRMWARE_SRC ${FIRMWARE_SRC} rtos.c)

  #
  # System and FreeRTOS port files
  #
  set(FIRMWARE_SRC ${FIRMWARE_SRC} $<TARGET_OBJECTS:freertos>)
endif()
