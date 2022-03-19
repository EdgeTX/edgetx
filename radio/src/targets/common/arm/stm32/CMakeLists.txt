set(LUA "NO" CACHE STRING "Lua scripts (YES/NO/NO_MODEL_SCRIPTS)")
set_property(CACHE LUA PROPERTY STRINGS YES NO NO_MODEL_SCRIPTS)
set(LUA_SCRIPT_LOAD_MODE "" CACHE STRING "Script loading mode and compilation flags [btTxcd] (see loadScript() API docs). Blank for default ('bt' on radio, 'T' on SIMU/DEBUG builds)")
option(LUA_COMPILER "Pre-compile and save Lua scripts" ON)
option(LUA_ALLOCATOR_TRACER "Trace Lua memory (de)allocations to debug port (also needs DEBUG=YES NANO=NO)" OFF)

option(USB_SERIAL "Enable USB serial (CDC)" OFF)

set(ARCH ARM)
set(STM32USB_DIR ${THIRDPARTY_DIR}/STM32_USB-Host-Device_Lib_V2.2.0/Libraries)
add_definitions(-DSTM32 -DLUA_INPUTS -DVARIO)

include_directories(${RADIO_SRC_DIR}/targets/common/arm/stm32)
include_directories(${STM32USB_DIR}/STM32_USB_OTG_Driver/inc)
include_directories(${STM32USB_DIR}/STM32_USB_Device_Library/Core/inc)
include_directories(${STM32USB_DIR}/STM32_USB_Device_Library/Class/msc/inc)
include_directories(${STM32USB_DIR}/STM32_USB_Device_Library/Class/hid/inc)
include_directories(${STM32USB_DIR}/STM32_USB_Device_Library/Class/cdc/inc)

set(STM32USB_SRC
  STM32_USB_OTG_Driver/src/usb_core.c
  STM32_USB_OTG_Driver/src/usb_dcd.c
  STM32_USB_OTG_Driver/src/usb_dcd_int.c
  STM32_USB_Device_Library/Core/src/usbd_core.c
  STM32_USB_Device_Library/Core/src/usbd_ioreq.c
  STM32_USB_Device_Library/Core/src/usbd_req.c
  )

set(FIRMWARE_TARGET_SRC ${FIRMWARE_TARGET_SRC}
  ../common/arm/stm32/rtc_driver.cpp
  ../common/arm/stm32/delays_driver.cpp
  ../common/arm/stm32/usb_bsp.c
  ../common/arm/stm32/usbd_desc.c
  ../common/arm/stm32/usbd_usr.cpp
  ../common/arm/stm32/usb_driver.cpp
  ../common/arm/stm32/flash_driver.cpp
  ../common/arm/stm32/usbd_storage_msd.cpp
  ../common/arm/stm32/usbd_hid_joystick.c
  ../common/arm/stm32/stm32_hal.cpp
  ../common/arm/stm32/stm32_i2c_driver.cpp
  )

if(IMU)
  add_definitions(-DIMU)
  set(FIRMWARE_SRC ${FIRMWARE_SRC}
    targets/common/arm/stm32/lsm6ds_driver.cpp
    )
  set(SRC ${SRC} gyro.cpp)
endif()

if(AUX_SERIAL OR AUX2_SERIAL)
  set(FIRMWARE_TARGET_SRC ${FIRMWARE_TARGET_SRC}
    ../common/arm/stm32/aux_serial_driver.cpp
    )
  if(AUX_SERIAL)
    add_definitions(-DAUX_SERIAL)
  endif()
  if(AUX2_SERIAL)
    add_definitions(-DAUX2_SERIAL)
  endif()
endif()

if(DEBUG OR CLI OR USB_SERIAL)
  set(STM32USB_SRC ${STM32USB_SRC}
    STM32_USB_Device_Library/Class/cdc/src/usbd_cdc_core.c
  )
  set(FIRMWARE_TARGET_SRC ${FIRMWARE_TARGET_SRC}
    ../common/arm/stm32/usbd_cdc.cpp
  )
  add_definitions(-DUSB_SERIAL)
  message("-- Adding support for USB serial")
endif()

set(STM32USB_SRC ${STM32USB_SRC}
  STM32_USB_Device_Library/Class/msc/src/usbd_msc_data.c
  STM32_USB_Device_Library/Class/msc/src/usbd_msc_scsi.c
  STM32_USB_Device_Library/Class/msc/src/usbd_msc_bot.c
  STM32_USB_Device_Library/Class/msc/src/usbd_msc_core.c
  )

if(GVARS)
  set(GUI_SRC ${GUI_SRC} ${GVAR_SCREEN})
endif()

set(FIRMWARE_SRC ${FIRMWARE_SRC} syscalls.c)

foreach(FILE ${STM32LIB_SRC})
  set(FIRMWARE_SRC ${FIRMWARE_SRC} ${STM32LIB_DIR}/${FILE})
endforeach()

foreach(FILE ${STM32USB_SRC})
  set(FIRMWARE_SRC ${FIRMWARE_SRC} ${STM32USB_DIR}/${FILE})
endforeach()

set(TARGET_SRC ${TARGET_SRC} ../common/arm/stm32/cpu_id.cpp)
set(SRC ${SRC} io/bootloader_flash.cpp)
