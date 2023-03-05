set(CPU_FAMILY STM32)
set(MCU cortex-m4)
set(FPU_FLAGS "-mfloat-abi=hard -mfpu=fpv4-sp-d16")

if(NOT NATIVE_BUILD)
  # STM32 HAL/LL library
  set(STM32CUBE_DIR ${THIRDPARTY_DIR}/STM32F4xx_HAL_Driver)
  include_directories(${STM32CUBE_DIR}/Inc)

  add_library(stm32cube_ll OBJECT EXCLUDE_FROM_ALL
    ${STM32CUBE_DIR}/Src/stm32f4xx_ll_gpio.c
    ${STM32CUBE_DIR}/Src/stm32f4xx_ll_tim.c
    ${STM32CUBE_DIR}/Src/stm32f4xx_ll_dma.c
    ${STM32CUBE_DIR}/Src/stm32f4xx_ll_usart.c
    ${STM32CUBE_DIR}/Src/stm32f4xx_ll_rcc.c
    ${STM32CUBE_DIR}/Src/stm32f4xx_ll_exti.c
    ${STM32CUBE_DIR}/Src/stm32f4xx_ll_sdmmc.c
    # HAL drivers
    ${STM32CUBE_DIR}/Src/stm32f4xx_hal.c
    ${STM32CUBE_DIR}/Src/stm32f4xx_hal_pwr.c
    ${STM32CUBE_DIR}/Src/stm32f4xx_hal_pwr_ex.c
    ${STM32CUBE_DIR}/Src/stm32f4xx_hal_rcc.c
    ${STM32CUBE_DIR}/Src/stm32f4xx_hal_rtc.c
    ${STM32CUBE_DIR}/Src/stm32f4xx_hal_i2c.c
    ${STM32CUBE_DIR}/Src/stm32f4xx_hal_i2c_ex.c
    ${STM32CUBE_DIR}/Src/stm32f4xx_hal_sd.c
    ${STM32CUBE_DIR}/Src/stm32f4xx_hal_dma.c
    )

  target_compile_options(stm32cube_ll PRIVATE -DUSE_FULL_LL_DRIVER)
  target_include_directories(stm32cube_ll PRIVATE
    ${THIRDPARTY_DIR}/CMSIS/Device/ST/STM32F4xx/Include
    ${THIRDPARTY_DIR}/CMSIS/Include)

  set(FIRMWARE_SRC ${FIRMWARE_SRC} $<TARGET_OBJECTS:stm32cube_ll>)
  set(BOOTLOADER_SRC ${BOOTLOADER_SRC} $<TARGET_OBJECTS:stm32cube_ll>)
endif()

# StdPeriph library
set(STM32LIB_DIR ${THIRDPARTY_DIR}/STM32F4xx_DSP_StdPeriph_Lib_V1.8.0/Libraries)
include_directories(${STM32LIB_DIR}/STM32F4xx_StdPeriph_Driver/inc)
include_directories(${STM32LIB_DIR}/CMSIS/Device/ST/STM32F4xx/Include)
include_directories(${STM32LIB_DIR}/CMSIS/Include)

add_definitions(-DSTM32F4)

set(STM32_STDPERIPH_DIR ${STM32LIB_DIR}/STM32F4xx_StdPeriph_Driver)

set(STM32_STDPERIPH_SRC
  targets/common/arm/stm32/f4/system_stm32f4xx.c
  ${STM32_STDPERIPH_DIR}/src/misc.c
  ${STM32_STDPERIPH_DIR}/src/stm32f4xx_gpio.c
  ${STM32_STDPERIPH_DIR}/src/stm32f4xx_dbgmcu.c
  ${STM32_STDPERIPH_DIR}/src/stm32f4xx_rcc.c
  ${STM32_STDPERIPH_DIR}/src/stm32f4xx_syscfg.c
  ${STM32_STDPERIPH_DIR}/src/stm32f4xx_spi.c
  ${STM32_STDPERIPH_DIR}/src/stm32f4xx_dma.c
  ${STM32_STDPERIPH_DIR}/src/stm32f4xx_pwr.c
  ${STM32_STDPERIPH_DIR}/src/stm32f4xx_adc.c
  ${STM32_STDPERIPH_DIR}/src/stm32f4xx_flash.c
)

if(CPU_TYPE_FULL STREQUAL STM32F429xI)
  set(STM32_STDPERIPH_SRC ${STM32_STDPERIPH_SRC}
    targets/${TARGET_DIR}/startup_stm32f42_43xxx.s
    ${STM32_STDPERIPH_DIR}/src/stm32f4xx_fmc.c
    ${STM32_STDPERIPH_DIR}/src/stm32f4xx_ltdc.c
    ${STM32_STDPERIPH_DIR}/src/stm32f4xx_dma.c
    ${STM32_STDPERIPH_DIR}/src/stm32f4xx_dma2d.c
    )
else()
  set(STM32_STDPERIPH_SRC ${STM32_STDPERIPH_SRC}
    targets/${TARGET_DIR}/startup_stm32f40_41xxx.s
    )  
endif()

add_library(stm32_stdperiph OBJECT EXCLUDE_FROM_ALL
  ${STM32_STDPERIPH_SRC}
  )

set(FIRMWARE_SRC ${FIRMWARE_SRC} $<TARGET_OBJECTS:stm32_stdperiph>)
set(BOOTLOADER_SRC ${BOOTLOADER_SRC} $<TARGET_OBJECTS:stm32_stdperiph>)

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
    ${RTOS_DIR}/portable/GCC/ARM_CM4F/port.c
    )

  target_include_directories(freertos
    PUBLIC  ${RTOS_DIR}/portable/GCC/ARM_CM4F
    PRIVATE ${RTOS_DIR}/include
    )

  include_directories(${RTOS_DIR}/portable/GCC/ARM_CM4F)
  
  # EdgeTx Compat layer
  set(FIRMWARE_SRC ${FIRMWARE_SRC} rtos.c)

  #
  # System and FreeRTOS port files
  #
  set(FIRMWARE_SRC ${FIRMWARE_SRC} $<TARGET_OBJECTS:freertos>)
endif()
