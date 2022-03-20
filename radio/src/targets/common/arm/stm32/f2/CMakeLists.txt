set(CPU_FAMILY STM32)
set(MCU cortex-m3)
set(FPU_FLAGS "")

# STM32 HAL/LL library
set(STM32CUBE_DIR ${THIRDPARTY_DIR}/STM32F2xx_HAL_Driver)
include_directories(${STM32CUBE_DIR}/Inc)

add_library(stm32cube_ll OBJECT EXCLUDE_FROM_ALL
  # LL drivers
  ${STM32CUBE_DIR}/Src/stm32f2xx_ll_gpio.c
  ${STM32CUBE_DIR}/Src/stm32f2xx_ll_tim.c
  ${STM32CUBE_DIR}/Src/stm32f2xx_ll_dma.c
  ${STM32CUBE_DIR}/Src/stm32f2xx_ll_usart.c
  ${STM32CUBE_DIR}/Src/stm32f2xx_ll_rcc.c
  # HAL drivers
  ${STM32CUBE_DIR}/Src/stm32f2xx_hal_rcc.c
  ${STM32CUBE_DIR}/Src/stm32f2xx_hal_i2c.c
  )

target_compile_options(stm32cube_ll PRIVATE -DUSE_FULL_LL_DRIVER)
target_include_directories(stm32cube_ll PRIVATE
  ${THIRDPARTY_DIR}/CMSIS/Device/ST/STM32F2xx/Include
  ${THIRDPARTY_DIR}/CMSIS/Include)

set(FIRMWARE_SRC ${FIRMWARE_SRC} $<TARGET_OBJECTS:stm32cube_ll>)

# StdPeriph library
set(STM32LIB_DIR ${THIRDPARTY_DIR}/STM32F2xx_StdPeriph_Lib_V1.1.0/Libraries)
include_directories(${STM32LIB_DIR}/STM32F2xx_StdPeriph_Driver/inc)
include_directories(${STM32LIB_DIR}/CMSIS/Device/ST/STM32F2xx/Include)
include_directories(${STM32LIB_DIR}/CMSIS/Include)

add_definitions(-DSTM32F2)

set(STM32LIB_SRC
  ${STM32LIB_SRC}
  CMSIS/Device/ST/STM32F2xx/Source/Templates/gcc_ride7/startup_stm32f2xx.s
  STM32F2xx_StdPeriph_Driver/src/misc.c
  STM32F2xx_StdPeriph_Driver/src/stm32f2xx_gpio.c
  STM32F2xx_StdPeriph_Driver/src/stm32f2xx_exti.c
  STM32F2xx_StdPeriph_Driver/src/stm32f2xx_dbgmcu.c
  STM32F2xx_StdPeriph_Driver/src/stm32f2xx_rcc.c
  STM32F2xx_StdPeriph_Driver/src/stm32f2xx_syscfg.c
  STM32F2xx_StdPeriph_Driver/src/stm32f2xx_spi.c
  STM32F2xx_StdPeriph_Driver/src/stm32f2xx_i2c.c
  STM32F2xx_StdPeriph_Driver/src/stm32f2xx_dma.c
  STM32F2xx_StdPeriph_Driver/src/stm32f2xx_rtc.c
  STM32F2xx_StdPeriph_Driver/src/stm32f2xx_pwr.c
  STM32F2xx_StdPeriph_Driver/src/stm32f2xx_usart.c
  STM32F2xx_StdPeriph_Driver/src/stm32f2xx_adc.c
  )

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
    ${RTOS_DIR}/portable/GCC/ARM_CM3/port.c
    )

  target_include_directories(freertos
    PUBLIC  ${RTOS_DIR}/portable/GCC/ARM_CM3
    PRIVATE ${RTOS_DIR}/include
    )

  include_directories(${RTOS_DIR}/portable/GCC/ARM_CM3)

  # EdgeTx Compat layer
  set(FIRMWARE_SRC ${FIRMWARE_SRC} rtos.c)

  #
  # System and FreeRTOS port files
  #
  set(FIRMWARE_SRC ${FIRMWARE_SRC}
    $<TARGET_OBJECTS:freertos>
    targets/common/arm/stm32/f2/system_stm32f2xx.c
    )
endif()

