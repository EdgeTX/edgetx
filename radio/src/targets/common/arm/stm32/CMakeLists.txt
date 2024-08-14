set(LUA "NO" CACHE STRING "Lua scripts (YES/NO/NO_MODEL_SCRIPTS)")
set_property(CACHE LUA PROPERTY STRINGS YES NO NO_MODEL_SCRIPTS)
set(LUA_SCRIPT_LOAD_MODE "" CACHE STRING "Script loading mode and compilation flags [btTxcd] (see loadScript() API docs). Blank for default ('bt' on radio, 'T' on SIMU/DEBUG builds)")
option(LUA_COMPILER "Pre-compile and save Lua scripts" ON)
option(LUA_ALLOCATOR_TRACER "Trace Lua memory (de)allocations to debug port (also needs DEBUG=YES NANO=NO)" OFF)

option(USB_SERIAL "Enable USB serial (CDC)" OFF)

set(ARCH ARM)
add_definitions(-DSTM32 -DLUA_INPUTS -DVARIO)

set(LINKER_DIR ${RADIO_SRC_DIR}/boards/generic_stm32/linker)
add_link_options(-L${LINKER_DIR} -L${LINKER_DIR}/${TARGET_LINKER_DIR})

set(STM32_DRIVER_DIR targets/common/arm/stm32)
include_directories(${STM32_DRIVER_DIR})

if(NOT NATIVE_BUILD)
  
  # STM32 USB Device library
  set(STM32USB_DIR ${THIRDPARTY_DIR}/STM32_USB_Device_Library)

  add_library(stm32usb OBJECT EXCLUDE_FROM_ALL
    ${STM32USB_DIR}/Core/Src/usbd_core.c
    ${STM32USB_DIR}/Core/Src/usbd_ctlreq.c
    ${STM32USB_DIR}/Core/Src/usbd_ioreq.c
    ${STM32USB_DIR}/Class/CDC/Src/usbd_cdc.c
    ${STM32USB_DIR}/Class/DFU/Src/usbd_dfu.c
    ${STM32USB_DIR}/Class/MSC/Src/usbd_msc_data.c
    ${STM32USB_DIR}/Class/MSC/Src/usbd_msc_scsi.c
    ${STM32USB_DIR}/Class/MSC/Src/usbd_msc_bot.c
    ${STM32USB_DIR}/Class/MSC/Src/usbd_msc.c
  )

  target_include_directories(stm32usb PRIVATE
    ${STM32USB_DIR}/Core/Inc
    ${STM32USB_DIR}/Class/CDC/Inc
    ${STM32USB_DIR}/Class/DFU/Inc
    ${STM32USB_DIR}/Class/HID/Inc
    ${STM32USB_DIR}/Class/MSC/Inc)

  target_include_directories(board_bl PRIVATE
    ${STM32USB_DIR}/Core/Inc
    ${STM32USB_DIR}/Class/DFU/Inc)

  set(FIRMWARE_SRC ${FIRMWARE_SRC} $<TARGET_OBJECTS:stm32usb>)
  set(BOOTLOADER_SRC ${BOOTLOADER_SRC} $<TARGET_OBJECTS:stm32usb>)

  # Bootloader USB devices
  add_library(stm32usb_device_bl OBJECT EXCLUDE_FROM_ALL    
    ${STM32_DRIVER_DIR}/usbd_conf.c
    ${STM32_DRIVER_DIR}/usbd_desc.c
    ${STM32_DRIVER_DIR}/usbd_dfu.cpp
    ${STM32_DRIVER_DIR}/usbd_storage_msd.cpp
    ${STM32_DRIVER_DIR}/usb_driver.cpp
  )

  target_compile_options(stm32usb_device_bl PRIVATE -DBOOT)
  target_include_directories(stm32usb_device_bl PRIVATE
    ${STM32USB_DIR}/Core/Inc
    ${STM32USB_DIR}/Class/CDC/Inc
    ${STM32USB_DIR}/Class/DFU/Inc
    ${STM32USB_DIR}/Class/HID/Inc
    ${STM32USB_DIR}/Class/MSC/Inc)

  set(BOOTLOADER_SRC ${BOOTLOADER_SRC} $<TARGET_OBJECTS:stm32usb_device_bl>)

  # Firmware USB devices
  add_library(stm32usb_device_fw OBJECT EXCLUDE_FROM_ALL
    ${STM32_DRIVER_DIR}/usbd_conf.c
    ${STM32_DRIVER_DIR}/usbd_desc.c
    ${STM32_DRIVER_DIR}/usb_driver.cpp
    ${STM32_DRIVER_DIR}/usbd_storage_msd.cpp
    ${STM32_DRIVER_DIR}/usbd_cdc.cpp
    ${STM32_DRIVER_DIR}/usbd_hid_joystick.c
  )

  if(DEBUG OR CLI OR USB_SERIAL)
    target_compile_options(stm32usb_device_fw PRIVATE -DUSB_SERIAL)
  endif()

  target_include_directories(stm32usb_device_fw PRIVATE
    ${STM32USB_DIR}/Core/Inc
    ${STM32USB_DIR}/Class/CDC/Inc
    ${STM32USB_DIR}/Class/DFU/Inc
    ${STM32USB_DIR}/Class/HID/Inc
    ${STM32USB_DIR}/Class/MSC/Inc)

  set(FIRMWARE_SRC ${FIRMWARE_SRC} $<TARGET_OBJECTS:stm32usb_device_fw>)

  # HAL/LL based driver library
  add_library(stm32_drivers OBJECT EXCLUDE_FROM_ALL
    ${STM32_DRIVER_DIR}/stm32_hal.cpp
    ${STM32_DRIVER_DIR}/timers_driver.cpp
    ${STM32_DRIVER_DIR}/diskio_spi.cpp
    ${STM32_DRIVER_DIR}/stm32_exti_driver.cpp
    ${STM32_DRIVER_DIR}/stm32_serial_driver.cpp
    ${STM32_DRIVER_DIR}/stm32_usart_driver.cpp
    ${STM32_DRIVER_DIR}/stm32_gpio_driver.cpp
    ${STM32_DRIVER_DIR}/stm32_i2c_driver.cpp
    ${STM32_DRIVER_DIR}/stm32_timer.cpp
    ${STM32_DRIVER_DIR}/stm32_dma.cpp
    ${STM32_DRIVER_DIR}/stm32_gpio.cpp
    ${STM32_DRIVER_DIR}/stm32_spi.cpp
    ${STM32_DRIVER_DIR}/stm32_adc.cpp
  )

  # HAL/LL drivers using TRACE
  add_library(stm32_drivers_w_dbg_fw OBJECT EXCLUDE_FROM_ALL
      ${STM32_DRIVER_DIR}/sdcard_spi.cpp
      ${STM32_DRIVER_DIR}/diskio_sdio.cpp
  )
  add_library(stm32_drivers_w_dbg_bl OBJECT EXCLUDE_FROM_ALL
      ${STM32_DRIVER_DIR}/sdcard_spi.cpp
      ${STM32_DRIVER_DIR}/diskio_sdio.cpp
  )

  set(FIRMWARE_SRC ${FIRMWARE_SRC} $<TARGET_OBJECTS:stm32_drivers>)
  set(FIRMWARE_SRC ${FIRMWARE_SRC} $<TARGET_OBJECTS:stm32_drivers_w_dbg_fw>)

  set(BOOTLOADER_SRC ${BOOTLOADER_SRC} $<TARGET_OBJECTS:stm32_drivers>)
  set(BOOTLOADER_SRC ${BOOTLOADER_SRC} $<TARGET_OBJECTS:stm32_drivers_w_dbg_bl>)
endif()

if(IMU)
  add_definitions(-DIMU)
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
  add_definitions(-DUSB_SERIAL)
  message("-- Adding support for USB serial")
endif()

set(FIRMWARE_SRC ${FIRMWARE_SRC} syscalls.c)

set(TARGET_SRC ${TARGET_SRC} ../common/arm/stm32/cpu_id.cpp)
set(SRC ${SRC} io/bootloader_flash.cpp)
