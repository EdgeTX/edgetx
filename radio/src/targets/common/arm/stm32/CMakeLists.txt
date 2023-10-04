set(LUA "NO" CACHE STRING "Lua scripts (YES/NO/NO_MODEL_SCRIPTS)")
set_property(CACHE LUA PROPERTY STRINGS YES NO NO_MODEL_SCRIPTS)
set(LUA_SCRIPT_LOAD_MODE "" CACHE STRING "Script loading mode and compilation flags [btTxcd] (see loadScript() API docs). Blank for default ('bt' on radio, 'T' on SIMU/DEBUG builds)")
option(LUA_COMPILER "Pre-compile and save Lua scripts" ON)
option(LUA_ALLOCATOR_TRACER "Trace Lua memory (de)allocations to debug port (also needs DEBUG=YES NANO=NO)" OFF)

option(USB_SERIAL "Enable USB serial (CDC)" OFF)

set(ARCH ARM)
#set(STM32USB_DIR ${THIRDPARTY_DIR}/STM32_USB-Host-Device_Lib_V2.2.0/Libraries)
set(STM32USB_DIR ${THIRDPARTY_DIR}/STM32_USB_Device_Library)
add_definitions(-DSTM32 -DLUA_INPUTS -DVARIO)

include_directories(${RADIO_SRC_DIR}/targets/common/arm/stm32)
include_directories(${STM32USB_DIR}/Core/Inc)
include_directories(${STM32USB_DIR}/Class/CDC/Inc)
include_directories(${STM32USB_DIR}/Class/HID/Inc)
include_directories(${STM32USB_DIR}/Class/MSC/Inc)

set(STM32USB_SRC
  Core/Src/usbd_core.c
  Core/Src/usbd_ctlreq.c
  Core/Src/usbd_ioreq.c
  Class/CDC/Src/usbd_cdc.c
  )

set(FIRMWARE_TARGET_SRC ${FIRMWARE_TARGET_SRC}
  ../common/arm/stm32/rtc_driver.cpp
  ../common/arm/stm32/timers_driver.cpp
  ../common/arm/stm32/delays_driver.cpp
  ../common/arm/stm32/trainer_driver.cpp
#  ../common/arm/stm32/usb_bsp.c
  ../common/arm/stm32/usbd_conf.c
  ../common/arm/stm32/usbd_desc.c
  ../common/arm/stm32/usb_device.c
  ../common/arm/stm32/usb_driver.cpp
  ../common/arm/stm32/flash_driver.cpp
  ../common/arm/stm32/usbd_storage_msd.cpp
   ../common/arm/stm32/usbd_hid_joystick.c
  ../common/arm/stm32/stm32_hal.cpp
  ../common/arm/stm32/stm32_i2c_driver.cpp
  ../common/arm/stm32/stm32_serial_driver.cpp
  ../common/arm/stm32/stm32_softserial_driver.cpp
  ../common/arm/stm32/stm32_pulse_driver.cpp
  ../common/arm/stm32/stm32_usart_driver.cpp
  ../common/arm/stm32/module_timer_driver.cpp
  ../common/arm/stm32/stm32_switch_driver.cpp
  ../common/arm/stm32/stm32_adc.cpp
  ../common/arm/stm32/stm32_spi.cpp
  ../common/arm/stm32/stm32_timer.cpp
  ../common/arm/stm32/stm32_dma.cpp
  ../common/arm/stm32/stm32_gpio_driver.cpp
  ../common/arm/stm32/stm32_exti_driver.cpp
  ../common/arm/stm32/mixer_scheduler_driver.cpp
  ../common/arm/stm32/pwr_driver.cpp
  )

if(IMU)
  add_definitions(-DIMU)
  set(FIRMWARE_SRC ${FIRMWARE_SRC}
    targets/common/arm/stm32/lsm6ds_driver.cpp
    )
  set(SRC ${SRC} gyro.cpp)
endif()

if(AUX_SERIAL OR AUX2_SERIAL)
  # set(FIRMWARE_TARGET_SRC ${FIRMWARE_TARGET_SRC}
  #   ../common/arm/stm32/aux_serial_driver.cpp
  #   )
  if(AUX_SERIAL)
    add_definitions(-DAUX_SERIAL)
  endif()
  if(AUX2_SERIAL)
    add_definitions(-DAUX2_SERIAL)
  endif()
endif()

if(FLYSKY_GIMBAL)
  set(FIRMWARE_SRC ${FIRMWARE_SRC}
    targets/common/arm/stm32/flysky_gimbal_driver.cpp
  )
  add_definitions(-DFLYSKY_GIMBAL)
endif()

if(DEBUG OR CLI OR USB_SERIAL)
  set(STM32USB_SRC ${STM32USB_SRC}
#    Class/CDC/Src/usbd_cdc.c
  )
  set(FIRMWARE_TARGET_SRC ${FIRMWARE_TARGET_SRC}
#    ../common/arm/stm32/usbd_cdc.cpp
  )
  add_definitions(-DUSB_SERIAL)
  message("-- Adding support for USB serial")
endif()

set(STM32USB_SRC ${STM32USB_SRC}
  Class/MSC/Src/usbd_msc_data.c
  Class/MSC/Src/usbd_msc_scsi.c
  Class/MSC/Src/usbd_msc_bot.c
  Class/MSC/Src/usbd_msc.c
)

if(GVARS)
  set(GUI_SRC ${GUI_SRC} ${GVAR_SCREEN})
endif()

set(FIRMWARE_SRC ${FIRMWARE_SRC} syscalls.c)


foreach(FILE ${STM32USB_SRC})
  set(FIRMWARE_SRC ${FIRMWARE_SRC} ${STM32USB_DIR}/${FILE})
endforeach()

set(TARGET_SRC ${TARGET_SRC} ../common/arm/stm32/cpu_id.cpp)
set(SRC ${SRC} io/bootloader_flash.cpp)
